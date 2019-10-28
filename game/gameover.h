#ifndef GAMEOVER_H
#define GAMEOVER_H

#include <QWidget>

namespace Ui {
    class GameOver;
}

struct GameOverPrivate;
class GameOver : public QWidget
{
        Q_OBJECT

    public:
        explicit GameOver(QWidget *parent = nullptr);
        ~GameOver();

    signals:
        void mainMenu();
        void playAgain();
        void review();

    private slots:
        void on_mainMenuButton_clicked();

        void on_playAgainButton_clicked();

        void on_saveButton_clicked();

    private:
        Ui::GameOver *ui;
        GameOverPrivate* d;
};

#endif // GAMEOVER_H
