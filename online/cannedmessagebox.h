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
#ifndef CANNEDMESSAGEBOX_H
#define CANNEDMESSAGEBOX_H

#include <QWidget>

namespace Ui {
    class CannedMessageBox;
}

struct CannedMessageBoxPrivate;
class CannedMessageBox : public QWidget
{
        Q_OBJECT

    public:
        explicit CannedMessageBox(QString cannedMessage, QWidget* carouselItem, QWidget *parent);
        ~CannedMessageBox();

        void show();
        void hide();

    private:
        Ui::CannedMessageBox *ui;
        CannedMessageBoxPrivate* d;

        void paintEvent(QPaintEvent* event);
        bool eventFilter(QObject* watched, QEvent* event);

        QRect endGeometry();
        void updatePosition();
};

#endif // CANNEDMESSAGEBOX_H
