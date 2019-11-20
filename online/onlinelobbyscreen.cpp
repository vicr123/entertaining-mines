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
#include "onlinelobbyscreen.h"
#include "ui_onlinelobbyscreen.h"

#include <QJsonObject>
#include <QJsonArray>
#include <discordintegration.h>
#include <online/onlineapi.h>
#include "onlinecontroller.h"
#include "gamemodeselect.h"

#include <tpopover.h>
#include <QPainter>

struct OnlineLobbyScreenPrivate {
    int currentRoomId = -1;
    int currentRoomPin = -1;
};

OnlineLobbyScreen::OnlineLobbyScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineLobbyScreen)
{
    ui->setupUi(this);
    d = new OnlineLobbyScreenPrivate();

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Leave Room"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->backButton->click();
    });

    ui->membersInRoom->setItemDelegate(new LobbyListDelegate(ui->membersInRoom));

    connect(OnlineController::instance(), &OnlineController::jsonMessage, this, [=](QJsonDocument doc) {
        QJsonObject obj = doc.object();
        QString type = obj.value("type").toString();
        if (type == "roomUpdate") {
            ui->membersInRoom->clear();
            QJsonArray users = obj.value("users").toArray();
            for (QJsonValue val : users) {
                QJsonObject user = val.toObject();

                QStringList supplementaryText;
                if (user.value("isHost").toBool()) {
                    supplementaryText.append("Host");
                }

                QPixmap px(1, 1);
                px.fill(QColor(user.value("colour").toVariant().toUInt()));

                QListWidgetItem* item = new QListWidgetItem();
                item->setText(user.value("username").toString());
                item->setIcon(QIcon(px));
                item->setData(Qt::UserRole, supplementaryText.join(" "));
                ui->membersInRoom->addItem(item);

                //Cache the profile picture
                OnlineApi::instance()->profilePicture(user.value("picture").toString(), 256);
            }

            DiscordIntegration::instance()->setPresence({
                {"state", tr("In Lobby")},
                {"details", tr("Waiting for players")},
                {"partyId", QString::number(d->currentRoomId)},
                {"partySize", users.count()},
                {"partyMax", obj.value("maxUsers").toInt()},
                {"joinSecret", QStringLiteral("%1:%2").arg(d->currentRoomId).arg(d->currentRoomPin)}
            });
        } else if (type == "lobbyChange") {
            d->currentRoomId = obj.value("lobbyId").toInt();
        } else if (type == "hostUpdate") {
            if (obj.value("isHost").toBool()) {
                ui->settingsWidget->setEnabled(true);
            } else {
                ui->settingsWidget->setEnabled(false);
            }
        } else if (type == "gamemodeChange") {
            QString gamemode = obj.value("gamemode").toString();
            if (gamemode == "cooperative") {
                ui->gamemodeButton->setText(tr("Cooperative"));
            } else if (gamemode == "tb-cooperative") {
                ui->gamemodeButton->setText(tr("Turn-Based Cooperative"));
            } else if (gamemode == "competitive") {
                ui->gamemodeButton->setText(tr("Competitive"));
            }
        } else if (type == "boardParamsChange") {
            QSignalBlocker b1(ui->widthBox);
            QSignalBlocker b2(ui->heightBox);
            QSignalBlocker b3(ui->minesBox);
            ui->widthBox->setValue(obj.value("width").toInt());
            ui->heightBox->setValue(obj.value("height").toInt());
            ui->minesBox->setValue(obj.value("mines").toInt());
        }
    });

    ui->settingsWidget->setFixedWidth(SC_DPI(300));

    ui->widthBox->setRange(5, 50);
    ui->heightBox->setRange(5, 50);
    ui->minesBox->setRange(5, 200);

    ui->widthBox->setValue(9);
    ui->heightBox->setValue(9);
    ui->minesBox->setValue(10);
}

OnlineLobbyScreen::~OnlineLobbyScreen()
{
    delete d;
    delete ui;
}

void OnlineLobbyScreen::on_backButton_clicked()
{
    //Tell the server that we want to leave the room
    OnlineController::instance()->sendJsonO({
        {"type", "leaveRoom"}
    });
}

void OnlineLobbyScreen::on_startButton_clicked()
{
    OnlineController::instance()->sendJsonO({
        {"type", "startGame"}
    });
}

void OnlineLobbyScreen::on_widthBox_valueChanged(int arg1)
{
    sendBoardParams();
}

void OnlineLobbyScreen::on_heightBox_valueChanged(int arg1)
{
    sendBoardParams();
}

void OnlineLobbyScreen::on_minesBox_valueChanged(int arg1)
{
    sendBoardParams();
}

void OnlineLobbyScreen::sendBoardParams()
{
    OnlineController::instance()->sendJsonO({
        {"type", "changeBoardParams"},
        {"width", ui->widthBox->value()},
        {"height", ui->heightBox->value()},
        {"mines", ui->minesBox->value()}
    });
}

LobbyListDelegate::LobbyListDelegate(QWidget*parent) : QStyledItemDelegate(parent)
{

}

void LobbyListDelegate::paint(QPainter*painter, const QStyleOptionViewItem&option, const QModelIndex&index) const
{
    painter->setPen(Qt::transparent);

    QPen textPen;
    QPen supplementaryPen;
    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.brush(QPalette::Highlight));
        textPen = option.palette.color(QPalette::HighlightedText);
        supplementaryPen = option.palette.color(QPalette::HighlightedText);
    } else if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        painter->setBrush(col);
        textPen = option.palette.color(QPalette::HighlightedText);
        supplementaryPen = option.palette.color(QPalette::HighlightedText);
    } else {
        painter->setBrush(option.palette.brush(QPalette::Base));
        textPen = option.palette.color(QPalette::WindowText);

        QColor suppColor = option.palette.color(QPalette::WindowText);
        suppColor.setAlpha(127);
        supplementaryPen = suppColor;
    }
    painter->drawRect(option.rect);

    QRect iconRect = option.rect, textRect = option.rect, supplementaryTextRect = option.rect;

    textRect.setHeight(option.rect.height() - 2);
    supplementaryTextRect.setHeight(option.rect.height() - 2);

    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (!icon.isNull()) {
        const QAbstractItemView* itemView = qobject_cast<const QAbstractItemView*>(option.widget);

        QSize iconSize = SC_DPI_T(QSize(16, 16), QSize);
        iconRect.setSize(iconSize);

        QImage iconImage = icon.pixmap(iconSize).toImage();
        iconRect.moveLeft(option.rect.left() + 2);
        iconRect.moveTop(option.rect.top() + (option.rect.height() / 2) - (iconRect.height() / 2));
        painter->drawImage(iconRect, iconImage);
        textRect.setLeft(iconRect.right() + 6);
    } else {
        textRect.setLeft(option.rect.left() + 6);
    }

    painter->setPen(textPen);
    painter->setFont(option.font);

    textRect.setWidth(option.fontMetrics.horizontalAdvance(index.data(Qt::DisplayRole).toString()));
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());

    if (index.data(Qt::UserRole).toString() != "") {
        supplementaryTextRect.setLeft(textRect.right() + 6);
        painter->setPen(supplementaryPen);
        painter->drawText(supplementaryTextRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::UserRole).toString());
    }
}

void OnlineLobbyScreen::on_gamemodeButton_clicked()
{

    GamemodeSelect* options = new GamemodeSelect();
    tPopover* popover = new tPopover(options);
    popover->setPopoverSide(tPopover::Bottom);
    popover->setPopoverWidth(options->sizeHint().height());
    connect(options, &GamemodeSelect::rejected, popover, &tPopover::dismiss);
    connect(options, &GamemodeSelect::accepted, this, [=](QString newGamemode) {
        OnlineController::instance()->sendJsonO({
            {"type", "changeGamemode"},
            {"gamemode", newGamemode}
        });

        popover->dismiss();
    });
    connect(popover, &tPopover::dismissed, options, &GamemodeSelect::deleteLater);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, this, [=] {
        ui->gamemodeButton->setFocus();
    });
    popover->show(this);
}
