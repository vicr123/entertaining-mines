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
#ifndef MAINONLINESCREEN_H
#define MAINONLINESCREEN_H

#include <QWidget>

namespace Ui {
    class MainOnlineScreen;
}

struct MainOnlineScreenPrivate;
class MainOnlineScreen : public QWidget
{
        Q_OBJECT

    public:
        explicit MainOnlineScreen(QWidget *parent = nullptr);
        ~MainOnlineScreen();

        void connectToOnline();

    signals:
        void quitOnline();

    private:
        Ui::MainOnlineScreen *ui;
        MainOnlineScreenPrivate* d;

        void paintEvent(QPaintEvent* event);
};

#endif // MAINONLINESCREEN_H
