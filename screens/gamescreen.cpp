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
#include "gamescreen.h"
#include "ui_gamescreen.h"

#include <the-libs_global.h>
#include <QIcon>
#include <QRandomGenerator>
#include "game/gametile.h"
#include "game/gameover.h"
#include "game/congratulation.h"
#include <pauseoverlay.h>
#include "pausescreen.h"
#include <musicengine.h>
#include <QUrl>
#include <QShortcut>
#include <tpromise.h>
#include <focusbarrier.h>
#include <discordintegration.h>

struct GameScreenPrivate {
    QVector<GameTile*> tiles;

    int width = 0;
    int mines = 0;

    int remainingTileCount = 0;
    int minesRemaining = 0;

    bool gameStarted = false;
    bool gameIsOver = false;

    bool showDateTime = false;
    QString dateTimeNotShownReason;

    QDateTime startDateTime;
    QTimer* dateTimeTimer;

    QPushButton* focusPreventer;
};

GameScreen::GameScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameScreen)
{
    ui->setupUi(this);
    d = new GameScreenPrivate();

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Reveal"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonX, tr("Flag"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonStart, [=] {
        ui->menuButton->click();
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, [=] {
        GameTile* currentTile = this->currentTile();
        if (currentTile != nullptr) currentTile->revealOrSweep();
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonX, [=] {
        GameTile* currentTile = this->currentTile();
        if (currentTile != nullptr) currentTile->toggleFlagStatus();
    });

    QShortcut* pauseShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(pauseShortcut, &QShortcut::activated, this, [=] {
        ui->menuButton->click();
    });

    d->dateTimeTimer = new QTimer();
    d->dateTimeTimer->setInterval(1000);
    connect(d->dateTimeTimer, &QTimer::timeout, this, &GameScreen::updateTimer);
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
//    bar1->setVisible(true);
//    d->focusPreventer->setVisible(true);
//    bar2->setVisible(true);
    layout->setParent(this);
}

GameScreen::~GameScreen()
{
    delete ui;
    delete d;
}

bool GameScreen::hasGameStarted()
{
    return d->gameStarted;
}

bool GameScreen::isGameOver()
{
    return d->gameIsOver;
}

QSize GameScreen::gameArea()
{
    return ui->gameArea->size();
}

GameTile*GameScreen::tileAt(QPoint location)
{
    return d->tiles.at(pointToIndex(location));
}

GameTile*GameScreen::currentTile()
{
    for (GameTile* tile : d->tiles) {
        if (tile->hasFocus()) return tile;
    }
    return nullptr;
}

QSize GameScreen::boardDimensions()
{
    return QSize(d->width, d->tiles.count() / d->width);
}

void GameScreen::revealedTile()
{
    d->remainingTileCount--;
    if (d->remainingTileCount == 0) {
        performGameOver();
    }
}

void GameScreen::flagChanged(bool didFlag)
{
    if (didFlag) {
        d->minesRemaining--;
    } else {
        d->minesRemaining++;
    }

    ui->minesRemainingLabel->setText(QString::number(d->minesRemaining));
}

QPoint GameScreen::indexToPoint(int index)
{
    return QPoint(index % d->width, index / d->width);
}

int GameScreen::pointToIndex(QPoint point)
{
    return point.y() * d->width + point.x();
}

void GameScreen::resizeEvent(QResizeEvent*event)
{
    int targetHeight = qMax(SC_DPI(50), static_cast<int>(this->height() * 0.05));
    int fontHeight = targetHeight - 18;

    QFont fnt = ui->hudWidget->font();
    fnt.setPixelSize(fontHeight);
    ui->hudWidget->setFont(fnt);

    QSize iconSize(fontHeight, fontHeight);
    ui->menuButton->setIconSize(iconSize);
    ui->mineIcon->setPixmap(QIcon(":/tiles/mine.svg").pixmap(iconSize));
    ui->timeIcon->setPixmap(QIcon(":/tiles/clock.svg").pixmap(iconSize));

    emit boardResized();
}

void GameScreen::setup()
{
    //Clear out the tiles
    for (GameTile* tile : d->tiles) {
        ui->gameGrid->removeWidget(tile);
        tile->deleteLater();
    }
    d->tiles.clear();

    MusicEngine::setBackgroundMusic("crypto");
    MusicEngine::playBackgroundMusic();
}

void GameScreen::finishSetup()
{
    ui->minesRemainingLabel->setText(QString::number(d->minesRemaining));

    d->tiles.first()->setFocus();
    this->setFocusProxy(d->tiles.first());

    DiscordIntegration::instance()->setPresence({
        {"state", tr("In Game")},
        {"details", tr("%1×%2 board with %3 mines").arg(d->width).arg(boardDimensions().height()).arg(d->mines)},
        {"startTimestamp", QDateTime::currentDateTimeUtc()}
    });

    updateTimer();
}

void GameScreen::startGame(int width, int height, int mines)
{
    d->width = width;
    d->mines = mines;

    //Ensure that the number of mines is valid for this game
    if (mines > width * height - 1) mines = width * height - 1;

    setup();

    //Create new tiles
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            GameTile* tile = new GameTile(this, x, y);
            ui->gameGrid->addWidget(tile, y, x);
            d->tiles.append(tile);
            connect(tile, &GameTile::currentTileChanged, this, &GameScreen::currentTileChanged);
        }
    }

    d->gameStarted = false;
    d->gameIsOver = false;
    d->showDateTime = true;
    d->startDateTime = QDateTime::currentDateTimeUtc();

    d->remainingTileCount = width * height - mines;
    d->minesRemaining = mines + 1;
    flagChanged(true);

    MusicEngine::playSoundEffect(MusicEngine::Selection);

    finishSetup();
}

void GameScreen::loadGame(QDataStream*stream)
{
    setup();

    //Start loading in data
    *stream >> d->width;
    *stream >> d->remainingTileCount;
    *stream >> d->mines;
    *stream >> d->gameStarted;
    *stream >> d->gameIsOver;

    //Create new tiles
    int tileCount;
    *stream >> tileCount;

    d->minesRemaining = d->mines;

    int x = 0;
    int y = 0;
    for (int i = 0; i < tileCount; i++) {
        GameTile* tile = new GameTile(this, x, y);
        ui->gameGrid->addWidget(tile, y, x);
        d->tiles.append(tile);
        connect(tile, &GameTile::currentTileChanged, this, &GameScreen::currentTileChanged);

        char* bytes;
        uint len;
        stream->readBytes(bytes, len);

        QByteArray ba(bytes, static_cast<int>(len));
        tile->fromByteArray(ba);

        delete[] bytes;

        if (tile->isFlagged()) d->minesRemaining--;

        x++;
        if (x == d->width) {
            x = 0;
            y++;
        }
    }

    //Disable the time counting as it is now inaccurate
    d->showDateTime = false;
    d->dateTimeNotShownReason = tr("Loading a save invalidates the timer.");

    //Tell all the tiles to update themselves
    for (GameTile* tile : d->tiles) {
        tile->afterLoadComplete();
    }
    finishSetup();
}

void GameScreen::saveGame(QDataStream*stream)
{
    *stream << d->width;
    *stream << d->remainingTileCount;
    *stream << d->mines;
    *stream << d->gameStarted;
    *stream << d->gameIsOver;

    *stream << d->tiles.count();
    for (GameTile* tile : d->tiles) {
        QByteArray ba = tile->toByteArray();
        stream->writeBytes(ba.constData(), static_cast<uint>(ba.length()));
    }
}

void GameScreen::distributeMines(QPoint clickLocation)
{
    Q_ASSERT(!d->gameStarted);

    //Distribute the mines across the board
    for (int i = 0; i < d->mines; i++) {
        int tileNumber = QRandomGenerator::global()->bounded(d->tiles.count());

        QPoint location = indexToPoint(tileNumber);
        if (location == clickLocation) {
            //Never spawn a mine on the user's click point
            i--;
            continue;
        }

        GameTile* tile = d->tiles.at(tileNumber);
        if (tile->isMine()) {
            //This tile is already a mine
            i--;
            continue;
        }

        //Set this tile as a mine
        tile->setIsMine(true);
    }
    d->gameStarted = true;
}

void GameScreen::performGameOver()
{
    d->gameIsOver = true;
    d->focusPreventer->setFocus();

    if (d->remainingTileCount == 0) {
        Congratulation* go = new Congratulation(this);
        connect(go, &Congratulation::playAgain, this, [=] {
            go->deleteLater();
            startGame(d->width, boardDimensions().height(), d->mines);
        });
        connect(go, &Congratulation::mainMenu, this, [=] {
            go->deleteLater();
            emit returnToMainMenu();
        });
        connect(go, &Congratulation::review, this, [=] {
            go->deleteLater();
        });
    } else {
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
            QThread::sleep(1);
        }))->then([=] {
            GameOver* go = new GameOver(this);
            connect(go, &GameOver::playAgain, this, [=] {
                go->deleteLater();
                startGame(d->width, boardDimensions().height(), d->mines);
            });
            connect(go, &GameOver::mainMenu, this, [=] {
                go->deleteLater();
                emit returnToMainMenu();
            });
            connect(go, &GameOver::review, this, [=] {
                go->deleteLater();
            });
        });
    }

    for (GameTile* tile : d->tiles) {
        tile->update();
    }
}

void GameScreen::currentTileChanged()
{
    GameTile* tile = this->currentTile();
    if (tile != nullptr) {
        //Update button text accordingly
        QString buttonA = "";
        QString buttonX = "";

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

        if (buttonA == "") {
            ui->gamepadHud->removeText(QGamepadManager::ButtonA);
        } else {
            ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, buttonA);
        }

        if (buttonX == "") {
            ui->gamepadHud->removeText(QGamepadManager::ButtonX);
        } else {
            ui->gamepadHud->setButtonText(QGamepadManager::ButtonX, buttonX);
        }
    }
}

void GameScreen::updateTimer()
{
    if (d->showDateTime) {
        QString seconds = QString::number(d->startDateTime.secsTo(QDateTime::currentDateTimeUtc()));
        QString display = seconds.rightJustified(3, QLatin1Char('0'), true);
        ui->timeLabel->setText(display);
    } else {
        ui->timeLabel->setText("XXX");
    }
}

void GameScreen::on_menuButton_clicked()
{
    //Disable the time counting as it is now inaccurate
    d->showDateTime = false;
    d->dateTimeNotShownReason = tr("Pausing invalidates the timer.");
    updateTimer();

    MusicEngine::pauseBackgroundMusic();
    MusicEngine::playSoundEffect(MusicEngine::Pause);

    PauseScreen* screen = new PauseScreen(this);
    connect(screen, &PauseScreen::resume, this, [=] {
        screen->deleteLater();
        d->tiles.first()->setFocus();
    });
    connect(screen, &PauseScreen::mainMenu, this, [=] {
        screen->deleteLater();
        emit returnToMainMenu();
    });
    connect(screen, &PauseScreen::provideMetadata, this, [=](QVariantMap* metadata) {
        //TODO
        QStringList description;
        description.append(tr("%1 × %2 board").arg(d->width).arg(boardDimensions().height()));
        description.append(tr("%1 mines").arg(d->mines));
        description.append(tr("%1 flagged").arg(d->mines - d->minesRemaining));
        description.append(tr("%1 mines to go").arg(d->minesRemaining));
        description.append(tr("%1% cleared").arg(static_cast<int>(static_cast<float>(d->remainingTileCount) / d->tiles.count() * 100)));

        metadata->insert("description", description.join(" ∙ "));
    });
    connect(screen, &PauseScreen::provideSaveData, this, &GameScreen::saveGame);
    screen->show();
}
