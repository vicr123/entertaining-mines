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
#include "onlinemenuscreen.h"
#include "ui_onlinemenuscreen.h"

#include <QJsonObject>
#include "onlinecontroller.h"
#include <online/friendsdialog.h>
#include <online/accountdialog.h>

OnlineMenuScreen::OnlineMenuScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineMenuScreen)
{
    ui->setupUi(this);

    this->setFocusProxy(ui->createLobby);

    ui->exitButton->setProperty("type", "destructive");

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Main Menu"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->exitButton->click();
    });

    ui->focusBarrierTop->setBounceWidget(ui->createLobby);
    ui->focusBarrierBottom->setBounceWidget(ui->exitButton);

    QPalette pal = ui->stackedWidget->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->stackedWidget->setPalette(pal);
}

OnlineMenuScreen::~OnlineMenuScreen()
{
    delete ui;
}

void OnlineMenuScreen::on_exitButton_clicked()
{
    emit quitOnline();
}

void OnlineMenuScreen::on_friendsButton_clicked()
{
    FriendsDialog* d = new FriendsDialog(this);
    connect(d, &FriendsDialog::done, d, &FriendsDialog::deleteLater);
}

void OnlineMenuScreen::on_createLobby_clicked()
{
    //Request to join a new room
    OnlineController::instance()->sendJsonO({
        {"type", "createRoom"}
    });
}

void OnlineMenuScreen::on_joinLobby_clicked()
{
    //Request the list of available rooms
    OnlineController::instance()->sendJsonO({
        {"type", "availableRooms"}
    });
}

void OnlineMenuScreen::on_accountButton_clicked()
{
    AccountDialog* d = new AccountDialog(this);
    connect(d, &AccountDialog::done, d, &AccountDialog::deleteLater);
}
