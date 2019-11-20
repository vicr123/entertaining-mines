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
#ifndef ONLINEGAMESCREEN_H
#define ONLINEGAMESCREEN_H

#include <QWidget>
#include "screens/gamescreen.h"

namespace Ui {
    class OnlineGameScreen;
}

struct OnlineGameScreenPrivate;
class GameTile;
class OnlineGameScreen : public AbstractGameScreen
{
        Q_OBJECT

    public:
        explicit OnlineGameScreen(QWidget *parent = nullptr);
        ~OnlineGameScreen();

        bool hasGameStarted();
        bool isGameOver();

        QSize gameArea();

        GameTile* tileAt(QPoint location);
        GameTile* currentTile();

    private slots:
        void currentTileChanged();
        void updateTimer();

    public slots:
        void startGame(int width, int height, int mines);
        void distributeMines(QPoint clickLocation);
        void performGameOver();

    protected:
        friend GameTile;
        QSize boardDimensions();

        void revealedTile();
        void flagChanged(bool didFlag);

    private:
        Ui::OnlineGameScreen *ui;
        OnlineGameScreenPrivate* d;

        QPoint indexToPoint(int index);
        int pointToIndex(QPoint point);

        void resizeEvent(QResizeEvent* event);

        void resizeTiles();
        void updateHudText();

        void setup();
        void finishSetup();
};

#endif // ONLINEGAMESCREEN_H
