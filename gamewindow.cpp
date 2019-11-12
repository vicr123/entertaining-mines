/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "gamewindow.h"
#include "ui_gamewindow.h"

#include <focuspointer.h>
#include <textinputoverlay.h>
#include <discordintegration.h>
#include <pauseoverlay.h>
#include <questionoverlay.h>
#include <notificationengine.h>
#include "online/logindialog.h"
#include "online/onlinecontroller.h"

struct GameWindowPrivate {
    QMap<quint64, DiscordJoinRequestCallback*> joinCallbacks;
};

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GameWindow)
{
    ui->setupUi(this);

    ui->menubar->setVisible(false);

    d = new GameWindowPrivate();

    PauseOverlay::registerOverlayForWindow(this, ui->centralwidget);

    this->setMinimumSize(SC_DPI_T(QSize(600, 600), QSize));

    connect(ui->mainScreen, &MainScreen::startGame, ui->gameScreen, &GameScreen::startGame);
    connect(ui->mainScreen, &MainScreen::startGame, this, [=] {
        ui->stackedWidget->setCurrentWidget(ui->gameScreen);
        ui->gameScreen->setFocus();
    });
    connect(ui->mainScreen, &MainScreen::loadGame, this, [=](QDataStream* stream) {
        if (ui->gameScreen->loadGame(stream)) {
            ui->stackedWidget->setCurrentWidget(ui->gameScreen);
            ui->gameScreen->setFocus();
        } else {
            QuestionOverlay* question = new QuestionOverlay(this);
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Corrupt File"));
            question->setText(tr("Sorry, that file is corrupt and needs to be deleted."));
            question->setButtons(QMessageBox::Ok);
            connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
            connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
        }
    });
    connect(ui->mainScreen, &MainScreen::openSettings, this, [=] {
        ui->settingsScreen->updateSettings();
        ui->stackedWidget->setCurrentWidget(ui->settingsScreen);
        ui->settingsScreen->setFocus();
    });
    connect(ui->mainScreen, &MainScreen::playOnline, this, &GameWindow::playOnline);

    connect(ui->settingsScreen, &SettingsScreen::goBack, this, [=] {
        ui->stackedWidget->setCurrentWidget(ui->mainScreen);
    });

    connect(ui->gameScreen, &GameScreen::returnToMainMenu, this, [=] {
        ui->stackedWidget->setCurrentWidget(ui->mainScreen);

        DiscordIntegration::instance()->setPresence({
            {"state", tr("Idle")},
            {"details", tr("Main Menu")}
        });
    });

    connect(ui->onlineScreen, &MainOnlineScreen::quitOnline, this, [=] {
        ui->stackedWidget->setCurrentWidget(ui->mainScreen);
    });

    FocusPointer::enableAutomaticFocusPointer();

    DiscordIntegration::instance()->setPresence({
        {"state", tr("Idle")},
        {"details", tr("Main Menu")}
    });

    connect(DiscordIntegration::instance(), &DiscordIntegration::joinRequest, this, [=](DiscordJoinRequestCallback* callback) {
        auto notifyUser = [=](QPixmap icon) {
            NotificationData notification;
            notification.title = tr("Join Game");
            notification.text = tr("%1 wants to join your room").arg(callback->userTag());
            notification.actions = {
                {"accept", "Accept"},
                {"decline", "Decline"}
            };
            notification.dismissable = false;
            notification.dismissTimer = 30000;
            notification.icon = QIcon(icon);
            quint64 notificationId = NotificationEngine::push(notification);

            d->joinCallbacks.insert(notificationId, callback);
        };

        callback->profilePicture()->then([=](QPixmap pixmap) {
            notifyUser(pixmap);
        })->error([=](QString error) {
            notifyUser(QPixmap());
        });
    });
    connect(DiscordIntegration::instance(), &DiscordIntegration::joinGame, this, [=](QString joinSecret) {
        OnlineController::instance()->setDiscordJoinSecret(joinSecret);

        playOnline();
    });

    connect(NotificationEngine::instance(), &NotificationEngine::actionClicked, this, [=](quint64 notificationId, QString key) {
        if (d->joinCallbacks.contains(notificationId)) {
            DiscordJoinRequestCallback* callback = d->joinCallbacks.value(notificationId);
            if (key == "accept") {
                callback->accept();
            } else {
                callback->reject();
            }
        }
    });
}

GameWindow::~GameWindow()
{
    delete d;
    delete ui;
}

void GameWindow::playOnline()
{
    LoginDialog* login = new LoginDialog(this);
    if (login->exec()) {
        ui->stackedWidget->setCurrentWidget(ui->onlineScreen);
        ui->onlineScreen->connectToOnline();
    } else {
        //Clear the join secret if there is one
        OnlineController::instance()->discordJoinSecret();
    }
}

