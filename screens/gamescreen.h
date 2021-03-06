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
#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <QWidget>

namespace Ui {
    class GameScreen;
}

class GameTile;
class AbstractGameScreen : public QWidget
{
    Q_OBJECT

    public:
        explicit AbstractGameScreen(QWidget* parent = nullptr) : QWidget(parent) {}

        virtual bool hasGameStarted() = 0;
        virtual bool isGameOver() = 0;
        virtual QSize gameArea() = 0;

        virtual GameTile* tileAt(QPoint location) = 0;

    public slots:
        virtual void distributeMines(QPoint clickLocation) = 0;
        virtual void performGameOver() = 0;

    signals:
        void boardResized();

    protected:
        friend GameTile;
        virtual QSize boardDimensions() = 0;

        virtual void revealedTile() = 0;
        virtual void flagChanged(bool didFlag) = 0;
};

struct GameScreenPrivate;
class GameScreen : public AbstractGameScreen
{
        Q_OBJECT

    public:
        explicit GameScreen(QWidget *parent = nullptr);
        ~GameScreen();

        bool hasGameStarted(); //Returns whether the user has clicked on the first tile yet
        bool isGameOver();

        QSize gameArea();

        GameTile* tileAt(QPoint location);
        GameTile* currentTile();

    public slots:
        void startGame(int width, int height, int mines);
        bool loadGame(QDataStream* stream);
        void saveGame(QDataStream* stream);
        void distributeMines(QPoint clickLocation);
        void performGameOver();

    private slots:
        void currentTileChanged();
        void updateTimer();

    signals:
        void returnToMainMenu();

    protected:
        friend GameTile;
        QSize boardDimensions();

        void revealedTile();
        void flagChanged(bool didFlag);

    private slots:
        void on_menuButton_clicked();

    private:
        Ui::GameScreen *ui;
        GameScreenPrivate* d;

        QPoint indexToPoint(int index);
        int pointToIndex(QPoint point);

        void resizeEvent(QResizeEvent* event);

        void resizeTiles();

        void setup();
        void finishSetup();
};

#endif // GAMESCREEN_H
