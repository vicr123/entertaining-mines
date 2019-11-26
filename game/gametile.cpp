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
#include <tvariantanimation.h>
#include <QJsonObject>

#include <musicengine.h>
#include <focuspointer.h>

struct GameTilePrivate {
    AbstractGameScreen* parent;
    GameTile::State state = GameTile::Idle;

    int x;
    int y;

    int curtain;
    QPointF touchStart;

    int numMinesAdjacent = -1;

    bool mouseIsPressed = false;
    bool drawMouseIsPressed = false;

    bool isMine = false;
    bool paintIsGameOver = false;

    bool paintHighlighted = false;

    bool middleClicked = false;

    bool remote = false;

    tVariantAnimation* flashAnim;
    QSettings settings;
};

GameTile::GameTile(AbstractGameScreen* parent, int x, int y) : QWidget(parent)
{
    d = new GameTilePrivate();
    d->parent = parent;
    d->x = x;
    d->y = y;

    d->flashAnim = new tVariantAnimation();
    d->flashAnim->setStartValue(127);
    d->flashAnim->setEndValue(0);
    d->flashAnim->setDuration(500);
    d->flashAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(d->flashAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        qDebug() << value;
        this->update();
    });

    this->setMouseTracking(true);
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    this->setFocusPolicy(Qt::StrongFocus);

    connect(d->parent, &GameScreen::boardResized, this, [=] {
        this->setFixedSize(this->sizeHint());
    });
}

GameTile::~GameTile()
{
    delete d;
}

void GameTile::setIsRemote(bool remote)
{
    d->remote = remote;
}

void GameTile::setRemoteParameters(QJsonObject parameters)
{
    d->state = static_cast<GameTile::State>(parameters.value("state").toInt());
    if (parameters.contains("number")) {
        d->numMinesAdjacent = parameters.value("number").toInt();
        d->isMine = parameters.value("isMine").toBool();
    }

    d->curtain = 0;
    this->update();
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

void GameTile::performGameOver()
{
    d->paintIsGameOver = true;
    if (this->isMine() && this->state() != Revealed) {
        MusicEngine::playSoundEffect("explode");
        flash();
    }
    this->update();
}

bool GameTile::isFlagged()
{
    return d->state == Flagged;
}

GameTile::State GameTile::state()
{
    return d->state;
}

QByteArray GameTile::toByteArray()
{
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::ReadWrite);

    stream << static_cast<int>(d->state);
    stream << d->isMine;

    return ba;
}

bool GameTile::fromByteArray(QByteArray ba)
{
    QDataStream stream(ba);

    int state;
    stream >> state;
    stream >> d->isMine;

    d->state = static_cast<State>(state);

    if (stream.status() != QDataStream::Ok) return false;

    return true;
}

void GameTile::afterLoadComplete()
{
    if (this->state() == Revealed) {
        this->minesAdjacent();
    }

    this->update();
}

void GameTile::setHighlighted(bool highlighted)
{
    d->paintHighlighted = highlighted;
    this->update();
}

void GameTile::reveal()
{
    if (d->remote) {
        emit revealTile();
        return;
    }

    if (d->parent->isGameOver()) return;

    if (d->state == Idle) {
        if (!d->parent->hasGameStarted()) d->parent->distributeMines(QPoint(d->x, d->y));
        d->state = Revealed;

        if (this->isMine()) {
            MusicEngine::playSoundEffect("explode");
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
        currentTileChanged();
    }
}

void GameTile::toggleFlagStatus()
{
    if (d->remote) {
        emit flagTile();
        return;
    }

    d->curtain = 0;
    if (d->parent->isGameOver()) return;

    switch (d->state) {
        case Idle:
            d->state = Flagged;
            d->parent->flagChanged(true);
            MusicEngine::playSoundEffect("dig");
            break;
        case Flagged:
            if (d->settings.value("behaviour/marks", true).toBool()) {
                d->state = Marked;
                d->parent->flagChanged(false);
                MusicEngine::playSoundEffect("question");
                break;
            }

            //Fall through
        case Marked:
            d->state = Idle;
            break;
        default:
            break;
    }
    this->update();
    currentTileChanged();
}

void GameTile::sweep()
{
    if (d->remote) {
        emit sweepTile();
        return;
    }

    if (d->parent->isGameOver()) return;

    if (this->state() == Revealed) {
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
        currentTileChanged();
    }
}

void GameTile::revealOrSweep()
{
    if (d->state == Revealed) {
        //Sweep this tile
        this->sweep();
    } else {
        //Reveal this tile
        this->reveal();
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

    //Draw the tile
    paintState(&painter, d->state, QPoint(0, 0));

    //Draw the curtain
    State curtainState = d->state;
    if (d->state == Idle) curtainState = Flagged;
    if (d->state == Flagged) {
        if (d->settings.value("behaviour/marks", true).toBool()) {
            curtainState = Marked;
        } else {
            curtainState = Idle;
        }
    }
    if (d->state == Marked) curtainState = Idle;

    if (curtainState != d->state) paintState(&painter, curtainState, QPoint(0, d->curtain - this->height()));

    if (d->flashAnim->state() == tVariantAnimation::Running) {
        painter.setBrush(QColor(255, 255, 255, d->flashAnim->currentValue().toInt()));
        painter.setPen(Qt::transparent);
        painter.drawRect(0, 0, this->width(), this->height());
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
        if (boardDimensions.contains(point)) {
            d->parent->tileAt(point)->setFocus();
        } else {
            MusicEngine::playSoundEffect(MusicEngine::FocusChangedFailed);
        }
    };

    QPoint thisPoint(d->x, d->y);
    switch (event->key()) {
        case Qt::Key_Left:
            handOffFocus(thisPoint + QPoint(-1, 0));
            break;
        case Qt::Key_Right:
            handOffFocus(thisPoint + QPoint(1, 0));
            break;
        case Qt::Key_Up:
            handOffFocus(thisPoint + QPoint(0, -1));
            break;
        case Qt::Key_Down:
            handOffFocus(thisPoint + QPoint(0, 1));
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            //Reveal or sweep this tile
            this->revealOrSweep();
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
    if (event->buttons() == (Qt::LeftButton | Qt::RightButton) || event->buttons() == Qt::MiddleButton) {
        d->middleClicked = true;
        d->drawMouseIsPressed = false;
        for (GameTile* tile : this->adjacentTiles()) {
            tile->setHighlighted(true);
        }
    } else if (event->button() == Qt::LeftButton) {
        d->drawMouseIsPressed = true;
    }
    this->update();
}

void GameTile::mouseMoveEvent(QMouseEvent*event)
{
    this->update();
}

void GameTile::mouseReleaseEvent(QMouseEvent*event)
{
    if (event->button() == Qt::LeftButton) {
        d->drawMouseIsPressed = false;
    }
    this->update();

    for (GameTile* tile : this->adjacentTiles()) {
        tile->setHighlighted(false);
    }


    QRect globalGeometry;
    globalGeometry.setSize(this->size());
    globalGeometry.moveTopLeft(this->mapToGlobal(QPoint(0, 0)));
    if (globalGeometry.contains(event->globalPos()) && event->buttons() == Qt::NoButton) {
        //Perform an action
        if (d->middleClicked) {
            //Handle middle click seperately for left + right click
            //Sweep this tile
            this->sweep();
        } else {
            switch (event->button()) {
                case Qt::LeftButton:
                    //Reveal this tile
                    this->reveal();
                    break;
                case Qt::RightButton:
                    //Flag this tile
                    this->toggleFlagStatus();
                    break;
                default:
                    //Do nothing
                    break;
            }
        }
    }

    if (event->buttons() == Qt::NoButton) {
        d->mouseIsPressed = false;
        d->middleClicked = false;
    }
}

bool GameTile::event(QEvent*event)
{
    if (event->type() == QEvent::TouchBegin) {
        QTouchEvent* e = static_cast<QTouchEvent*>(event);
        d->touchStart = e->touchPoints().first().pos();
        e->accept();
        return true;
    } else if (event->type() == QEvent::TouchUpdate) {
        QTouchEvent* e = static_cast<QTouchEvent*>(event);
        d->curtain = static_cast<int>(e->touchPoints().first().pos().y() - d->touchStart.y());
        if (d->curtain > this->height()) d->curtain = this->height();

        this->update();
        e->accept();
        return true;
    } else if (event->type() == QEvent::TouchEnd) {
        QTouchEvent* e = static_cast<QTouchEvent*>(event);

        QRect tapRect;
        tapRect.setSize(SC_DPI_T(QSize(10, 10), QSize));
        tapRect.moveCenter(d->touchStart.toPoint());
        if (d->curtain > this->height() * 0.8) {
            this->toggleFlagStatus();
        } else if (tapRect.contains(e->touchPoints().first().pos().toPoint())) {
            this->revealOrSweep();
        } else {
            tVariantAnimation* anim = new tVariantAnimation();
            anim->setStartValue(d->curtain);
            anim->setEndValue(0);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->setDuration(500);
            connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
                d->curtain = value.toInt();
                this->update();
            });
            connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
            anim->start();
        }
        e->accept();
        return true;
    } else if (event->type() == QEvent::TouchCancel) {
        d->curtain = 0;
        this->update();
        event->accept();
        return true;
    }
    return QWidget::event(event);
}

void GameTile::paintState(QPainter*painter, GameTile::State state, QPoint topLeft)
{
    painter->save();
    painter->translate(topLeft);

    QFont font = this->font();
    font.setPixelSize(this->height() - SC_DPI(18));
    painter->setFont(font);

    //Paint the background
    switch (state) {
        case Idle:
            paintSvg(painter, ":/tiles/background.svg");
            break;
        case Flagged:
            paintSvg(painter, ":/tiles/backgroundFlag.svg");
            break;
        case Marked:
            paintSvg(painter, ":/tiles/backgroundMark.svg");
            break;
        case Revealed:
            if (this->isMine()) {
                paintSvg(painter, ":/tiles/backgroundMine.svg");
            } else {
                paintSvg(painter, ":/tiles/backgroundRev.svg");
            }
    }

    if (state != Revealed && ((!FocusPointer::isEnabled() && this->underMouse()) || (FocusPointer::isEnabled() && this->hasFocus()))) {
        if (d->drawMouseIsPressed) {
            painter->setBrush(QColor(0, 0, 0, 50));
        } else {
            painter->setBrush(QColor(255, 255, 255, 50));
        }
        painter->setPen(Qt::transparent);
        painter->drawRect(0, 0, this->width(), this->height());
    } else if (state == Idle && d->paintHighlighted) {
        painter->setBrush(QColor(0, 100, 0, 50));
        painter->setPen(Qt::transparent);
        painter->drawRect(0, 0, this->width(), this->height());
    }

    //Paint the tile
    switch (state) {
        case Idle:
            if (d->paintIsGameOver && this->isMine()) {
                //Draw the mine tile
                paintSvg(painter, ":/tiles/mine.svg");
            }
            //Do nothing
            break;
        case Flagged:
            if (d->paintIsGameOver && !this->isMine()) {
                //Draw the incorrect flag tile
                paintSvg(painter, ":/tiles/flagNot.svg");
            } else {
                //Draw a flag
                paintSvg(painter, ":/tiles/flag.svg");
            }
            break;
        case Marked:
            if (d->paintIsGameOver && this->isMine()) {
                //Draw the mine tile
                paintSvg(painter, ":/tiles/mine.svg");
            } else {
                //Draw a mark
                paintSvg(painter, ":/tiles/mark.svg");
            }
            break;
        case Revealed:
            //Draw the number of tiles
            if (this->isMine()) {
                //Draw the mine tile
                paintSvg(painter, ":/tiles/mine.svg");
            } else if (d->numMinesAdjacent != 0) {
                painter->setPen(Qt::white);
                painter->drawText(0, 0, this->height(), this->width(), Qt::AlignCenter, QString::number(d->numMinesAdjacent));
            }
    }

    painter->restore();
}

void GameTile::focusInEvent(QFocusEvent*event)
{
    emit currentTileChanged();
}

void GameTile::flash()
{
    d->flashAnim->start();
}
