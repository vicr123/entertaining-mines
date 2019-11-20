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
#include "onlinegamescreen.h"
#include "ui_onlinegamescreen.h"

#include <the-libs_global.h>
#include <QIcon>
#include <QRandomGenerator>
#include "game/gametile.h"
#include "game/gameover.h"
#include "game/congratulation.h"
#include <pauseoverlay.h>
//#include "pausescreen.h"
#include <musicengine.h>
#include <QUrl>
#include <QShortcut>
#include <tvariantanimation.h>
#include <tpromise.h>
#include <focusbarrier.h>
#include <discordintegration.h>
#include "onlinecontroller.h"
#include <online/onlineapi.h>

struct OnlineGameScreenPrivate {
    QVector<GameTile*> tiles;

    int currentRoomId = -1;
    int userCount = 0;
    int userMax = 0;

    int width = 0;
    int mines = 0;

    QDateTime startDateTime;
    QTimer* dateTimeTimer;

    QString gamemode;

    QPushButton* focusPreventer;
};

OnlineGameScreen::OnlineGameScreen(QWidget *parent) :
    AbstractGameScreen(parent),
    ui(new Ui::OnlineGameScreen)
{
    ui->setupUi(this);
    d = new OnlineGameScreenPrivate();

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Reveal"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonX, tr("Flag"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, [=] {
        GameTile* currentTile = this->currentTile();
        if (currentTile != nullptr) currentTile->revealOrSweep();
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonX, [=] {
        GameTile* currentTile = this->currentTile();
        if (currentTile != nullptr) currentTile->toggleFlagStatus();
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonY, [=] {
        if (d->gamemode == "tb-cooperative") {
            //Skip turn
            OnlineController::instance()->sendJsonO({
                {"type", "boardAction"},
                {"action", "skip"}
            });
        }
    });

    d->dateTimeTimer = new QTimer();
    d->dateTimeTimer->setInterval(1000);
    connect(d->dateTimeTimer, &QTimer::timeout, this, &OnlineGameScreen::updateTimer);
    d->dateTimeTimer->start();

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    FocusBarrier* bar1 = new FocusBarrier(this);
    FocusBarrier* bar2 = new FocusBarrier(this);
    d->focusPreventer = new QPushButton();
    bar1->setBounceWidget(d->focusPreventer);
    bar2->setBounceWidget(d->focusPreventer);
    layout->addWidget(bar1);
    layout->addWidget(d->focusPreventer);
    layout->addWidget(bar2);
    layout->setGeometry(QRect(-50, -50, 5, 5));
    layout->setParent(this);

    connect(OnlineController::instance(), &OnlineController::jsonMessage, this, [=](QJsonDocument doc) {
        QJsonObject obj = doc.object();
        QString type = obj.value("type").toString();
        if (type == "boardSetup") {
            startGame(obj.value("width").toInt(), obj.value("height").toInt(), obj.value("mines").toInt());
        } else if (type == "tileUpdate") {
            if (d->tiles.count() > obj.value("tile").toInt()) {
                GameTile* t = d->tiles.at(obj.value("tile").toInt());
                t->setRemoteParameters(obj);
            }
        } else if (type == "endGame") {
            if (obj.value("victory").toBool()) {
                //Congratulations
                ui->gameOverBlameUsername->setText(tr("Congratulations!"));
                ui->gameOverBlameDescription->setText(tr("You swept this board completely!"));
                ui->gameOverBlamePicture->setPixmap(QIcon(":/icons/entertaining-mines.svg").pixmap(QSize(ui->gameOverBlameContents->sizeHint().height(), ui->gameOverBlameContents->sizeHint().height())));

                MusicEngine::pauseBackgroundMusic();
            } else {
                //Game Over
                ui->gameOverBlameUsername->setText(obj.value("user").toString());
                ui->gameOverBlameDescription->setText(tr("stepped on a mine and blew everything up!"));
                OnlineApi::instance()->profilePicture(obj.value("picture").toString(), ui->gameOverBlameContents->sizeHint().height())->then([=](QImage image) {
                    ui->gameOverBlamePicture->setPixmap(QPixmap::fromImage(image));
                });

                this->performGameOver();
            }

            ui->playerCarousel->collapse();
            tVariantAnimation* anim = new tVariantAnimation();
            anim->setStartValue(0);
            anim->setEndValue(ui->gameOverBlameWidget->sizeHint().height());
            anim->setDuration(500);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
                ui->gameOverBlameWidget->setFixedHeight(value.toInt());
                this->resizeTiles();
            });
            connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
            anim->start();
        } else if (type == "lobbyChange") {
            d->currentRoomId = obj.value("lobbyId").toInt();
        } else if (type == "roomUpdate") {
            d->userCount = obj.value("users").toArray().count();
            d->userMax = obj.value("maxUsers").toInt();
        } else if (type == "gamemodeChange") {
            d->gamemode = obj.value("gamemode").toString();

            updateHudText();
        } else if (type == "minesRemainingChanged") {
            ui->minesRemainingLabel->setText(QString::number(obj.value("minesRemaining").toInt()));
        }
    });
}

OnlineGameScreen::~OnlineGameScreen()
{
    delete ui;
    delete d;
}

bool OnlineGameScreen::hasGameStarted()
{
    return true;
}

bool OnlineGameScreen::isGameOver()
{
    return false;
}

QSize OnlineGameScreen::gameArea()
{
    return ui->gameArea->size();
}

GameTile*OnlineGameScreen::tileAt(QPoint location)
{
    return d->tiles.at(pointToIndex(location));
}

GameTile*OnlineGameScreen::currentTile()
{
    for (GameTile* tile : d->tiles) {
        if (tile->hasFocus()) return tile;
    }
    return nullptr;
}

QSize OnlineGameScreen::boardDimensions()
{
    return QSize(d->width, d->tiles.count() / d->width);
}

void OnlineGameScreen::revealedTile()
{
    //noop
}

void OnlineGameScreen::flagChanged(bool didFlag)
{
    //noop
}

QPoint OnlineGameScreen::indexToPoint(int index)
{
    return QPoint(index % d->width, index / d->width);
}

int OnlineGameScreen::pointToIndex(QPoint point)
{
    return point.y() * d->width + point.x();
}

void OnlineGameScreen::resizeEvent(QResizeEvent*event)
{
    resizeTiles();
}

void OnlineGameScreen::resizeTiles()
{
    int targetHeight = qMax(SC_DPI(50), static_cast<int>(this->height() * 0.05));
    int fontHeight = targetHeight - 18;

    QFont fnt = ui->hudWidget->font();
    fnt.setPixelSize(fontHeight);
    ui->hudWidget->setFont(fnt);

    QSize iconSize(fontHeight, fontHeight);
    ui->mineIcon->setPixmap(QIcon(":/tiles/mine.svg").pixmap(iconSize));
    ui->timeIcon->setPixmap(QIcon(":/tiles/clock.svg").pixmap(iconSize));

    emit boardResized();
}

void OnlineGameScreen::updateHudText()
{
    GameTile* tile = this->currentTile();
    if (tile != nullptr) {
        //Update button text accordingly
        ui->gamepadHud->removeText(QGamepadManager::ButtonA);
        ui->gamepadHud->removeText(QGamepadManager::ButtonX);
        ui->gamepadHud->removeText(QGamepadManager::ButtonY);

        QString buttonA = "";
        QString buttonX = "";
        QString buttonY = "";

        switch (tile->state()) {
            case GameTile::Idle:
                buttonA = tr("Reveal");
                buttonX = tr("Flag");
                break;
            case GameTile::Flagged:
                buttonX = tr("Mark");
                break;
            case GameTile::Marked:
                buttonX = tr("Unflag");
                break;
            case GameTile::Revealed:
                buttonA = tr("Sweep");
        }

        if (d->gamemode == "tb-cooperative") {
            buttonY = tr("Skip Turn");
        }

        if (buttonA != "") ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, buttonA);
        if (buttonX != "") ui->gamepadHud->setButtonText(QGamepadManager::ButtonX, buttonX);
        if (buttonY != "") ui->gamepadHud->setButtonText(QGamepadManager::ButtonY, buttonY);
    }
}

void OnlineGameScreen::setup()
{
    //Clear out the tiles
    for (GameTile* tile : d->tiles) {
        ui->gameGrid->removeWidget(tile);
        tile->deleteLater();
    }
    d->tiles.clear();

}

void OnlineGameScreen::finishSetup()
{
    d->tiles.first()->setFocus();
    this->setFocusProxy(d->tiles.first());

    QString friendlyGamemode;
    if (d->gamemode == "cooperative") friendlyGamemode = tr("Cooperative");
    if (d->gamemode == "tb-cooperative") friendlyGamemode = tr("Turn-Based Cooperative");
    if (d->gamemode == "competitive") friendlyGamemode = tr("Competitive");

    DiscordIntegration::instance()->setPresence({
        {"state", tr("Online Game")},
        {"details", tr("%1: %2×%3 board with %n mines", nullptr, d->mines).arg(friendlyGamemode).arg(d->width).arg(boardDimensions().height())},
        {"startTimestamp", QDateTime::currentDateTimeUtc()},
        {"partyId", QString::number(d->currentRoomId)},
        {"partySize", d->userCount},
        {"partyMax", d->userMax}
    });

    ui->gameOverBlameWidget->setFixedHeight(0);
    ui->playerCarousel->expand();

    updateTimer();
    resizeTiles();

    MusicEngine::setBackgroundMusic("crypto");
    MusicEngine::playBackgroundMusic();
}

void OnlineGameScreen::startGame(int width, int height, int mines)
{
    ui->gameOverBlameWidget->setFixedHeight(0);
    d->width = width;
    d->mines = mines;

    //Ensure that the number of mines is valid for this game
    if (mines > width * height - 1) mines = width * height - 1;

    setup();

    //Create new tiles
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            GameTile* tile = new GameTile(this, x, y);
            tile->setIsRemote(true);
            ui->gameGrid->addWidget(tile, y, x);
            d->tiles.append(tile);
            connect(tile, &GameTile::currentTileChanged, this, &OnlineGameScreen::currentTileChanged);

            connect(tile, &GameTile::revealTile, this, [=] {
                OnlineController::instance()->sendJsonO({
                    {"type", "boardAction"},
                    {"action", "reveal"},
                    {"tile", width * y + x}
                });
            });
            connect(tile, &GameTile::flagTile, this, [=] {
                OnlineController::instance()->sendJsonO({
                    {"type", "boardAction"},
                    {"action", "flag"},
                    {"tile", width * y + x}
                });
            });
            connect(tile, &GameTile::sweepTile, this, [=] {
                OnlineController::instance()->sendJsonO({
                    {"type", "boardAction"},
                    {"action", "sweep"},
                    {"tile", width * y + x}
                });
            });
        }
    }

    d->startDateTime = QDateTime::currentDateTimeUtc();

    MusicEngine::playSoundEffect(MusicEngine::Selection);

    ui->minesRemainingLabel->setText(QString::number(mines));

    finishSetup();
}

void OnlineGameScreen::currentTileChanged()
{
    updateHudText();
}

void OnlineGameScreen::updateTimer()
{
//    if (d->showDateTime) {
        QString seconds = QString::number(d->startDateTime.secsTo(QDateTime::currentDateTimeUtc()));
        QString display = seconds.rightJustified(3, QLatin1Char('0'), true);
        ui->timeLabel->setText(display);
//    } else {
//        ui->timeLabel->setText("XXX");
//    }
}

void OnlineGameScreen::distributeMines(QPoint clickLocation)
{
    //noop
}

void OnlineGameScreen::performGameOver()
{
    MusicEngine::pauseBackgroundMusic();
    (new tPromise<void>([=](QString error) {
        for (int i = 0; i < d->tiles.count(); i++) {
            d->tiles.at(i)->metaObject()->invokeMethod(d->tiles.at(i), "performGameOver", Qt::QueuedConnection);

            if (i < 100) {
                QThread::msleep(20);
            } else if (i < 200) {
                QThread::msleep(10);
            } else {
                QThread::msleep(5);
            }

        }
    }))->then([=] {

    });
}
