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
#include "gametile.h"

#include "screens/gamescreen.h"
#include <QPainter>
#include <the-libs_global.h>
#include <QSvgRenderer>
#include <QMouseEvent>

#include <focuspointer.h>

struct GameTilePrivate {
    GameScreen* parent;
    GameTile::State state = GameTile::Idle;

    int x;
    int y;

    int numMinesAdjacent = -1;

    bool mouseIsPressed = false;
    bool drawMouseIsPressed = false;

    bool isMine = false;
};

GameTile::GameTile(GameScreen* parent, int x, int y) : QWidget(parent)
{
    d = new GameTilePrivate();
    d->parent = parent;
    d->x = x;
    d->y = y;

    this->setMouseTracking(true);
    this->setFocusPolicy(Qt::StrongFocus);

    connect(d->parent, &GameScreen::boardResized, this, [=] {
        this->setFixedSize(this->sizeHint());
    });
}

GameTile::~GameTile()
{
    delete d;
}

QSize GameTile::sizeHint() const
{
    int size;

    //Calculate two sizes based on the height and width, and use whichever is smaller
    QSize boardDimensions = d->parent->boardDimensions();
    int width = (d->parent->gameArea().width()) / boardDimensions.width();
    int height = (d->parent->gameArea().height()) / boardDimensions.height();

    size = qMin(width, height);
    return QSize(size, size);
}

bool GameTile::isMine()
{
    return d->isMine;
}

void GameTile::setIsMine(bool isMine)
{
    d->isMine = isMine;
}

int GameTile::minesAdjacent()
{
    if (d->numMinesAdjacent == -1) {
        d->numMinesAdjacent = 0;
        if (!this->isMine()) {
            //Calculate number of mines adjacent
            for (GameTile* tile : adjacentTiles()) {
                if (tile->isMine()) d->numMinesAdjacent++;
            }
        }
    }

    return d->numMinesAdjacent;
}

bool GameTile::isFlagged()
{
    return d->state == Flagged;
}

void GameTile::reveal()
{
    if (d->state == Idle) {
        if (!d->parent->hasGameStarted()) d->parent->distributeMines(QPoint(d->x, d->y));
        d->state = Revealed;

        if (this->isMine()) {
            d->parent->performGameOver();
        } else {
            //Tell the game that we've revealed a good tile
            d->parent->revealedTile();

            //Calculate the number of adjacent tiles with a mine if needed
            if (this->minesAdjacent() == 0) {
                //Attempt to reveal all adjacent tiles
                for (GameTile* tile : adjacentTiles()) {
                    tile->reveal();
                }
            }
        }

        this->update();
    }
}

void GameTile::toggleFlagStatus()
{
    switch (d->state) {
        case Idle:
            d->state = Flagged;
            d->parent->flagChanged(true);
            break;
        case Flagged:
            d->state = Idle; //TODO: Add marked state
            d->parent->flagChanged(true);
            break;
        case Marked:
            d->state = Idle;
            break;
        default:
            break;
    }
    this->update();
}

void GameTile::sweep()
{
    int numFlags = 0;
    QList<GameTile*> tilesToSweep;
    for (GameTile* tile : adjacentTiles()) {
        if (tile->isFlagged()) {
            numFlags++;
        } else {
            tilesToSweep.append(tile);
        }
    }

    if (minesAdjacent() == numFlags) {
        //Sweep the tile
        for (GameTile* tile : tilesToSweep) {
            tile->reveal();
        }
    }
}

QList<GameTile*> GameTile::adjacentTiles()
{
    QList<GameTile*> tiles;
    QRect boardDimensions = QRect(QPoint(0, 0), d->parent->boardDimensions());
    auto checkAndAddPoint = [=, &tiles](QPoint point) {
        if (boardDimensions.contains(point)) tiles.append(d->parent->tileAt(point));
    };

    QPoint thisPoint(d->x, d->y);
    checkAndAddPoint(thisPoint + QPoint(-1, -1));
    checkAndAddPoint(thisPoint + QPoint(-1, 0));
    checkAndAddPoint(thisPoint + QPoint(-1, 1));
    checkAndAddPoint(thisPoint + QPoint(0, 1));
    checkAndAddPoint(thisPoint + QPoint(1, 1));
    checkAndAddPoint(thisPoint + QPoint(1, 0));
    checkAndAddPoint(thisPoint + QPoint(1, -1));
    checkAndAddPoint(thisPoint + QPoint(0, -1));

    return tiles;
}

void GameTile::paintEvent(QPaintEvent*event)
{
    QPainter painter(this);

    QFont font = this->font();
    font.setPixelSize(this->height() - SC_DPI(18));
    painter.setFont(font);

    //Paint the background
    switch (d->state) {
        case Idle:
        case Flagged:
        case Marked:
            if ((!FocusPointer::isEnabled() && this->underMouse()) || (FocusPointer::isEnabled() && this->hasFocus())) {
                if (d->drawMouseIsPressed) {
                    paintSvg(&painter, ":/tiles/backgroundRev.svg");
                } else {
                    paintSvg(&painter, ":/tiles/backgroundHov.svg");
                }
            } else {
                paintSvg(&painter, ":/tiles/background.svg");
            }
            break;
        case Revealed:
            paintSvg(&painter, ":/tiles/backgroundRev.svg");
    }

    //Paint the tile
    if (d->parent->isGameOver() && this->isMine()) {
        //Draw the mine tile
        paintSvg(&painter, ":/tiles/mine.svg");
    } else {
        switch (d->state) {
            case Idle:
                //Do nothing
                break;
            case Flagged:
                if (d->parent->isGameOver()) {
                    //Draw the incorrect flag tile
                    paintSvg(&painter, ":/tiles/flagNot.svg");
                } else {
                    //Draw a flag
                    paintSvg(&painter, ":/tiles/flag.svg");
                }
                break;
            case Marked:
                //Draw a mark
                break;
            case Revealed:
                //Draw the number of tiles
                if (d->numMinesAdjacent != 0) {
                    painter.setPen(Qt::white);
                    painter.drawText(0, 0, this->height(), this->width(), Qt::AlignCenter, QString::number(d->numMinesAdjacent));
                }
        }
    }
}

void GameTile::paintSvg(QPainter*painter, QString filePath)
{
    QSvgRenderer renderer(filePath);
    renderer.render(painter, QRectF(0, 0, this->width(), this->height()));
}

void GameTile::enterEvent(QEvent*event)
{
    this->update();
}

void GameTile::leaveEvent(QEvent*event)
{
    this->update();
}

void GameTile::keyPressEvent(QKeyEvent*event)
{
    QRect boardDimensions = QRect(QPoint(0, 0), d->parent->boardDimensions());
    auto handOffFocus = [=](QPoint point) {
        if (boardDimensions.contains(point)) d->parent->tileAt(point)->setFocus();
    };

    QPoint thisPoint(d->x, d->y);
    switch (event->key()) {
        case Qt::Key_Left:
            handOffFocus(thisPoint + QPoint(0, -1));
            break;
        case Qt::Key_Right:
            handOffFocus(thisPoint + QPoint(0, 1));
            break;
        case Qt::Key_Up:
            handOffFocus(thisPoint + QPoint(-1, 0));
            break;
        case Qt::Key_Down:
            handOffFocus(thisPoint + QPoint(1, 0));
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (d->state == Revealed) {
                //Sweep this tile
                this->sweep();
            } else {
                //Reveal this tile
                this->reveal();
            }
            break;
        case Qt::Key_Space:
        case Qt::Key_Z:
            //Flag this tile
            this->toggleFlagStatus();
            break;
    }
}

void GameTile::mousePressEvent(QMouseEvent*event)
{
    d->mouseIsPressed = true;
    if (event->button() == Qt::LeftButton) {
        d->drawMouseIsPressed = true;
    } else if (event->button() == Qt::MiddleButton) {

    }
    this->update();
}

void GameTile::mouseMoveEvent(QMouseEvent*event)
{
    this->update();
}

void GameTile::mouseReleaseEvent(QMouseEvent*event)
{
    d->mouseIsPressed = false;
    if (event->button() == Qt::LeftButton) {
        d->drawMouseIsPressed = false;
    }
    this->update();

    if (this->underMouse()) {
        //Do cool stuff
        switch (event->button()) {
            case Qt::LeftButton:
                //Reveal this tile
                this->reveal();
                break;
            case Qt::RightButton:
                //Flag this tile
                this->toggleFlagStatus();
                break;
            case Qt::MiddleButton:
                //Sweep this tile
                this->sweep();
                break;
            default:
                //Do nothing
                break;
        }
    }
}
