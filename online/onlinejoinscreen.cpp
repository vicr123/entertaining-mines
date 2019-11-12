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
#include "onlinejoinscreen.h"
#include "ui_onlinejoinscreen.h"

#include <QJsonObject>
#include <QJsonArray>
#include <musicengine.h>
#include "onlinecontroller.h"
#include <pauseoverlay.h>

struct OnlineJoinScreenPrivate {

};

OnlineJoinScreen::OnlineJoinScreen(QJsonObject reply, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineJoinScreen)
{
    ui->setupUi(this);
    d = new OnlineJoinScreenPrivate();

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Join"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, [=] {
        on_joinableGames_activated(ui->joinableGames->currentIndex());
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        MusicEngine::playSoundEffect(MusicEngine::Backstep);
        ui->backButton->click();
    });

    QJsonArray rooms = reply.value("rooms").toArray();
    for (QJsonValue val : rooms) {
        QJsonObject room = val.toObject();

        QListWidgetItem* item = new QListWidgetItem(ui->joinableGames);
        item->setText(room.value("friend").toString());
        item->setData(Qt::UserRole, room.value("roomId").toInt());
        item->setData(Qt::UserRole + 1, room.value("pinRequired").toBool());
    }

    PauseOverlay::overlayForWindow(parent)->pushOverlayWidget(this);
}

OnlineJoinScreen::~OnlineJoinScreen()
{
    delete ui;
    delete d;
}

void OnlineJoinScreen::on_backButton_clicked()
{
    PauseOverlay::overlayForWindow(this)->popOverlayWidget();
}

void OnlineJoinScreen::on_joinableGames_activated(const QModelIndex &index)
{
    //Join the game
    OnlineController::instance()->sendJsonO({
        {"type", "joinRoom"},
        {"roomId", index.data(Qt::UserRole).toInt()}
    });
    PauseOverlay::overlayForWindow(this)->popOverlayWidget();
}
