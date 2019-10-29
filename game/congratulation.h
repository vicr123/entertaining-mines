#ifndef CONGRATULATION_H
#define CONGRATULATION_H

#include <QWidget>

namespace Ui {
    class Congratulation;
}

struct CongratulationPrivate;
class Congratulation : public QWidget
{
        Q_OBJECT

    public:
        explicit Congratulation(QWidget *parent = nullptr);
        ~Congratulation();

    signals:
        void mainMenu();
        void playAgain();
        void review();

    private slots:
        void on_mainMenuButton_clicked();

        void on_playAgainButton_clicked();

        void on_saveButton_clicked();

    private:
        Ui::Congratulation *ui;
        CongratulationPrivate* d;
};

#endif // CONGRATULATION_H
