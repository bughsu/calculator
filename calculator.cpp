#include "calculator.h"
#include <QDebug>
#include <QVariant>
#include <QLayout>

// 為了簡化,定義一個結構來儲存按鈕配置
struct ButtonConfig {
    const char *text;
    const char *slotName;
    int row;
    int col;
    int rowSpan;
    int colSpan;
};

// Constructor
calculator::calculator(QWidget *parent)
    : QDialog(parent), sumInMemory(0.0), waitingForOperand(true)
{
    // --- 1. 顯示螢幕 (Display) 設定 ---
    display = new QLineEdit("0");
    display->setReadOnly(true);
    display->setAlignment(Qt::AlignRight);
    display->setMaxLength(15);
    QFont font = display->font();
    font.setPointSize(font.pointSize() + 20);
    display->setFont(font);

    // --- 2. 按鈕創建與佈局 ---
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    // 顯示螢幕
    mainLayout->addWidget(display, 0, 0, 1, 4);

    // 按鈕數據定義
    const ButtonConfig buttons[] = {
                                    {"\u2190", "clearClicked()", 1, 0, 1, 1},
                                    {"/", "binaryOperatorClicked()", 1, 1, 1, 1},
                                    {"*", "binaryOperatorClicked()", 1, 2, 1, 1},
                                    {"-", "binaryOperatorClicked()", 1, 3, 1, 1},
                                    {"7", "digitClicked()", 2, 0, 1, 1},
                                    {"8", "digitClicked()", 2, 1, 1, 1},
                                    {"9", "digitClicked()", 2, 2, 1, 1},
                                    {"+", "binaryOperatorClicked()", 2, 3, 2, 1},
                                    {"4", "digitClicked()", 3, 0, 1, 1},
                                    {"5", "digitClicked()", 3, 1, 1, 1},
                                    {"6", "digitClicked()", 3, 2, 1, 1},
                                    {"1", "digitClicked()", 4, 0, 1, 1},
                                    {"2", "digitClicked()", 4, 1, 1, 1},
                                    {"3", "digitClicked()", 4, 2, 1, 1},
                                    {"=", "equalsClicked()", 4, 3, 2, 1},
                                    {"0", "digitClicked()", 5, 0, 1, 2},
                                    {".", "digitClicked()", 5, 2, 1, 1},
    };

    // 迴圈遍歷按鈕數據並將其加入佈局
    for (const auto& config : buttons) {
        QString text = config.text;
        if (text == "=") {
            text = "\u21B5";
        }

        QPushButton *button = new QPushButton(text);
        button->setProperty("buttonText", QString(config.text));

        // 設置按鈕為正方形 (固定大小)
        int buttonSize = 60; // 按鈕邊長

        // 根據按鈕跨越的格數設置寬度和高度
        int width = buttonSize * config.colSpan;
        int height = buttonSize * config.rowSpan;

        button->setMinimumSize(width, height);
        button->setMaximumSize(width, height);

        // 設置字體大小
        QFont buttonFont = button->font();
        buttonFont.setPointSize(14);
        button->setFont(buttonFont);

        mainLayout->addWidget(button, config.row, config.col, config.rowSpan, config.colSpan);

        // 直接連接信號與槽
        if (QString(config.slotName) == "digitClicked()") {
            connect(button, SIGNAL(clicked()), this, SLOT(digitClicked()));
        } else if (QString(config.slotName) == "binaryOperatorClicked()") {
            connect(button, SIGNAL(clicked()), this, SLOT(binaryOperatorClicked()));
        } else if (QString(config.slotName) == "equalsClicked()") {
            connect(button, SIGNAL(clicked()), this, SLOT(equalsClicked()));
        } else if (QString(config.slotName) == "clearClicked()") {
            connect(button, SIGNAL(clicked()), this, SLOT(clearClicked()));
        }
    }

    setLayout(mainLayout);
    setWindowTitle("Image Layout Calculator");
}

calculator::~calculator() {}



void calculator::digitClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    QString digit = clickedButton->property("buttonText").toString();

    if (display->text() == "Error") {
        display->setText("0");
        waitingForOperand = true;
    }

    if (waitingForOperand) {
        if (digit == ".") {
            display->setText("0.");
        } else {
            display->setText(digit);
        }
        waitingForOperand = false;
    } else {
        if (digit == ".") {
            if (!display->text().contains('.')) {
                display->setText(display->text() + digit);
            }
        } else {
            if (display->text() == "0") {
                display->setText(digit);
            } else if (display->text().length() < 15) {
                display->setText(display->text() + digit);
            }
        }
    }
}

void calculator::binaryOperatorClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    QString clickedOperator = clickedButton->property("buttonText").toString();
    double operandValue = display->text().toDouble();

    // 如果有待處理的運算符,先執行計算
    if (!pendingOperator.isEmpty()) {
        if (!calculate(operandValue, pendingOperator)) {
            abortOperation();
            return;
        }
        display->setText(QString::number(sumInMemory));
    } else {
        sumInMemory = operandValue;
    }

    pendingOperator = clickedOperator;
    waitingForOperand = true;
}

void calculator::equalsClicked()
{
    double operandValue = display->text().toDouble();

    if (!pendingOperator.isEmpty()) {
        if (!calculate(operandValue, pendingOperator)) {
            abortOperation();
            return;
        }
        pendingOperator.clear();
    } else {
        sumInMemory = operandValue;
    }

    display->setText(QString::number(sumInMemory));
    sumInMemory = 0.0;
    waitingForOperand = true;
}

void calculator::clearClicked()
{
    sumInMemory = 0.0;
    pendingOperator.clear();
    display->setText("0");
    waitingForOperand = true;
}

bool calculator::calculate(double rightOperand, const QString &pendingOp)
{
    if (pendingOp == "+") {
        sumInMemory += rightOperand;
    } else if (pendingOp == "-") {
        sumInMemory -= rightOperand;
    } else if (pendingOp == "*") {
        sumInMemory *= rightOperand;
    } else if (pendingOp == "/") {
        if (rightOperand == 0.0) {
            return false;
        }
        sumInMemory /= rightOperand;
    }
    return true;
}

void calculator::abortOperation()
{
    clearClicked();
    display->setText("Error");
}
