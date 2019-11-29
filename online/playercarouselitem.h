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
#ifndef PLAYERCAROUSELITEM_H
#define PLAYERCAROUSELITEM_H

#include <QWidget>

namespace Ui {
    class PlayerCarouselItem;
}

struct PlayerCarouselItemPrivate;
class PlayerCarouselItem : public QWidget
{
        Q_OBJECT

    public:
        explicit PlayerCarouselItem(QWidget *parent = nullptr);
        ~PlayerCarouselItem();

        void setPlayerName(QString playerName);
        void setPlayerColor(QColor col);
        void setProfilePicture(QImage picture);
        void setCurrentTurn(qint64 timeout);
        void setSessionId(int sessionId);
        void clearCurrentTurn();

        int sessionId();

    private:
        Ui::PlayerCarouselItem *ui;
        PlayerCarouselItemPrivate* d;

        void resizeEvent(QResizeEvent* event);
        void paintEvent(QPaintEvent* event);
};

#endif // PLAYERCAROUSELITEM_H
