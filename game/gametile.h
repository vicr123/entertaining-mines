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
#ifndef GAMETILE_H
#define GAMETILE_H

#include <QWidget>

struct GameTilePrivate;
class AbstractGameScreen;
class GameTile : public QWidget
{
        Q_OBJECT
    public:
        enum State {
            Idle = 0,
            Revealed = 1,
            Flagged = 2,
            Marked = 3
        };

        explicit GameTile(AbstractGameScreen *parent, int x, int y);
        ~GameTile();

        void setIsRemote(bool remote);
        void setRemoteParameters(QJsonObject parameters);
        void addRemoteColor(QColor color);
        void resetRemoteColors();

        QSize sizeHint() const;

        bool isMine();
        void setIsMine(bool isMine);
        int minesAdjacent();

        bool isFlagged();

        State state();

        QByteArray toByteArray();
        bool fromByteArray(QByteArray ba);
        void afterLoadComplete();

        void setHighlighted(bool highlighted);

    signals:
        void currentTileChanged();

        void revealTile();
        void sweepTile();
        void flagTile();

    public slots:
        void reveal();
        void toggleFlagStatus();
        void sweep();
        void revealOrSweep();

        void performGameOver();

    private:
        GameTilePrivate* d;

        QList<GameTile*> adjacentTiles();

        void paintEvent(QPaintEvent* event);
        void paintSvg(QPainter* painter, QString filePath);

        void enterEvent(QEvent* event);
        void leaveEvent(QEvent* event);
        void keyPressEvent(QKeyEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
        bool event(QEvent* event);

        void paintState(QPainter* painter, State state, QPoint topLeft);

        void focusInEvent(QFocusEvent* event);

        void flash();
};

#endif // GAMETILE_H
