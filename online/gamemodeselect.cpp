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
#include "gamemodeselect.h"
#include "ui_gamemodeselect.h"

#include <the-libs_global.h>

GamemodeSelect::GamemodeSelect(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GamemodeSelect)
{
    ui->setupUi(this);

    ui->mainContainer->setFixedWidth(SC_DPI(700));

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->backButton->click();
    });

    ui->focusBarrier->setBounceWidget(ui->playCooperative);
    ui->focusBarrier_2->setBounceWidget(ui->playCompetitive);

    this->setFocusProxy(ui->playCooperative);
}

GamemodeSelect::~GamemodeSelect()
{
    delete ui;
}

void GamemodeSelect::on_playCooperative_clicked()
{
    emit accepted("cooperative");
}

void GamemodeSelect::on_playCompetitive_clicked()
{
    emit accepted("competitive");
}

void GamemodeSelect::on_backButton_clicked()
{
    emit rejected();
}

void GamemodeSelect::on_turnBasedCooperative_clicked()
{
    emit accepted("tb-cooperative");
}
