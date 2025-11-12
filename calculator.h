#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class calculator : public QDialog
{
    Q_OBJECT

public:
    calculator(QWidget *parent = nullptr);
    ~calculator();

private slots:
    void digitClicked();
    void binaryOperatorClicked();
    void equalsClicked();
    void clearClicked();

private:
    QPushButton *createButton(const QString &text, const char *member);
    void abortOperation();
    bool calculate(double rightOperand, const QString &pendingOperator);

    double sumInMemory;
    QString pendingOperator;
    QLineEdit *display;
    bool waitingForOperand;
};

#endif // CALCULATOR_H
