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
#ifndef ONLINEMENUSCREEN_H
#define ONLINEMENUSCREEN_H

#include <QWidget>

namespace Ui {
    class OnlineMenuScreen;
}

class OnlineMenuScreen : public QWidget
{
        Q_OBJECT

    public:
        explicit OnlineMenuScreen(QWidget *parent = nullptr);
        ~OnlineMenuScreen();

    signals:
        void quitOnline();

    private slots:
        void on_exitButton_clicked();

        void on_friendsButton_clicked();

        void on_createLobby_clicked();

        void on_joinLobby_clicked();

        void on_accountButton_clicked();

    private:
        Ui::OnlineMenuScreen *ui;
};

#endif // ONLINEMENUSCREEN_H
