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
#include "gamewindow.h"
#include "ui_gamewindow.h"

#include <focuspointer.h>
#include <textinputoverlay.h>

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GameWindow)
{
    ui->setupUi(this);

    ui->menubar->setVisible(false);

    connect(ui->mainScreen, &MainScreen::startGame, ui->gameScreen, &GameScreen::startGame);
    connect(ui->mainScreen, &MainScreen::startGame, this, [=] {
        ui->stackedWidget->setCurrentWidget(ui->gameScreen);
        ui->gameScreen->setFocus();
    });
    connect(ui->mainScreen, &MainScreen::loadGame, this, [=](QDataStream* stream) {
        ui->stackedWidget->setCurrentWidget(ui->gameScreen);
        ui->gameScreen->loadGame(stream);
        ui->gameScreen->setFocus();
    });

    connect(ui->gameScreen, &GameScreen::returnToMainMenu, this, [=] {
        ui->stackedWidget->setCurrentWidget(ui->mainScreen);
    });

    FocusPointer::enableAutomaticFocusPointer();
}

GameWindow::~GameWindow()
{
    delete ui;
}

