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
#include <pauseoverlay.h>
#include "pausescreen.h"
#include "dialogueoverlay.h"
#include <musicengine.h>
#include <QUrl>
#include <QShortcut>

struct GameScreenPrivate {
    QVector<GameTile*> tiles;

    int width = 0;
    int mines = 0;

    int remainingTileCount = 0;
    int minesRemaining = 0;

    bool gameStarted = false;
    bool gameIsOver = false;

    DialogueOverlay* dialogue;
};

GameScreen::GameScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameScreen)
{
    ui->setupUi(this);
    d = new GameScreenPrivate();

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Reveal"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonX, tr("Flag"));

    d->dialogue = new DialogueOverlay(this);
    connect(d->dialogue, QOverload<QString>::of(&DialogueOverlay::progressDialogue), this, [=](QString selectedOption) {
        if (selectedOption == "mainmenu") {
            MusicEngine::pauseBackgroundMusic();
            emit returnToMainMenu();
        } else if (selectedOption == "newgame") {
            startGame(d->width, boardDimensions().height(), d->mines);
        }
        d->dialogue->dismiss();
    });

    QShortcut* pauseShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(pauseShortcut, &QShortcut::activated, this, [=] {
        ui->menuButton->click();
    });
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

    emit boardResized();
}

void GameScreen::startGame(int width, int height, int mines)
{
    d->width = width;
    d->mines = mines;

    //Ensure that the number of mines is valid for this game
    if (mines > width * height - 1) mines = width * height - 1;

    //Clear out the tiles
    for (GameTile* tile : d->tiles) {
        ui->gameGrid->removeWidget(tile);
        tile->deleteLater();
    }
    d->tiles.clear();

    //Create new tiles
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            GameTile* tile = new GameTile(this, x, y);
            ui->gameGrid->addWidget(tile, y, x);
            d->tiles.append(tile);
        }
    }

    d->gameStarted = false;
    d->gameIsOver = false;

    d->remainingTileCount = width * height - mines;
    d->minesRemaining = mines + 1;
    flagChanged(true);

    MusicEngine::setBackgroundMusic(QUrl("qrc:/audio/crypto.ogg"));
    MusicEngine::playBackgroundMusic();

    MusicEngine::playSoundEffect(MusicEngine::Selection);

    this->setFocusProxy(d->tiles.first());
    d->tiles.first()->setFocus();
}

void GameScreen::distributeMines(QPoint clickLocation)
{
    Q_ASSERT(!d->gameStarted);

    //Distribute the mines across the board
    for (int i = 0; i < d->mines; i++) {
        int tileNumber = QRandomGenerator::global()->bounded(d->tiles.count());

        QPoint location = indexToPoint(tileNumber);
        if (location == clickLocation) continue; //Never spawn a mine on the user's click point

        GameTile* tile = d->tiles.at(tileNumber);
        if (tile->isMine()) continue; //This tile is already a mine

        //Set this tile as a mine
        tile->setIsMine(true);
    }
    d->gameStarted = true;
}

void GameScreen::performGameOver()
{
    d->gameIsOver = true;

    if (d->remainingTileCount == 0) {
        d->dialogue->setMultiDialogue({
            tr("Congratulations! You won!"),
            tr("What do you want to do now?")
        }, {
            {"review", tr("Review the game")},
            {"newgame", tr("Start a new game")},
            {"mainmenu", tr("Return to the Main Menu")},
        });
    } else {
        d->dialogue->setMultiDialogue({
            tr("You stepped on a mine"),
            tr("What do you want to do now?")
        }, {
            {"review", tr("Review the game")},
            {"newgame", tr("Start a new game")},
            {"mainmenu", tr("Return to the Main Menu")},
        });
    }

    for (GameTile* tile : d->tiles) {
        tile->update();
    }
}

void GameScreen::on_menuButton_clicked()
{
    MusicEngine::pauseBackgroundMusic();
    MusicEngine::playSoundEffect(MusicEngine::Pause);

    PauseScreen* screen = new PauseScreen();
    PauseOverlay* overlay = new PauseOverlay(screen);
    overlay->showOverlay(this);
    connect(screen, &PauseScreen::resume, this, [=] {
        MusicEngine::playSoundEffect(MusicEngine::Backstep);
        MusicEngine::playBackgroundMusic();
        overlay->hideOverlay();
        overlay->deleteLater();
        screen->deleteLater();
    });
    connect(screen, &PauseScreen::mainMenu, this, [=] {
        emit returnToMainMenu();
        MusicEngine::playSoundEffect(MusicEngine::Selection);
        overlay->hideOverlay();
        overlay->deleteLater();
        screen->deleteLater();
    });

    screen->setFocus();
}
