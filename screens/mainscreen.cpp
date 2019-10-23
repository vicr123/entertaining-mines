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
#include "mainscreen.h"
#include "ui_mainscreen.h"

#include <QApplication>
#include <gamepadbuttons.h>

MainScreen::MainScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainScreen)
{
    ui->setupUi(this);

    ui->focusBarrierTop->setBounceWidget(ui->startEasy);
    ui->focusBarrierBottom->setBounceWidget(ui->exitButton);

    ui->AHudButton->setText(tr("%1 Select").arg(GamepadButtons::stringForButton(QGamepadManager::ButtonA)));
    ui->BHudButton->setText(tr("%2 Exit").arg(GamepadButtons::stringForButton(QGamepadManager::ButtonB)));

    ui->exitButton->setProperty("type", "destructive");
}

MainScreen::~MainScreen()
{
    delete ui;
}

void MainScreen::resizeEvent(QResizeEvent*event)
{
    ui->leftSpacing->changeSize(this->width() * 0.4, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->layout()->invalidate();
}

void MainScreen::on_startEasy_clicked()
{
    emit startGame(9, 9, 10);
}

void MainScreen::on_startIntermediate_clicked()
{
    emit startGame(16, 16, 40);
}

void MainScreen::on_startDifficult_clicked()
{
    emit startGame(30, 16, 99);
}

void MainScreen::on_exitButton_clicked()
{
    QApplication::exit();
}
