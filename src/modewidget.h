#ifndef MODESELECTIONWIDGET_H
#define MODESELECTIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainter>
#include <QPixmap>
#include <QPropertyAnimation>
#include "mainwindow.h"

class ModeButton : public QPushButton {
    Q_OBJECT
public:
    explicit ModeButton(const QString &text, QWidget *parent = nullptr) : 
    QPushButton(text, parent) {}
};

class ModeWidget : public QWidget {
    Q_OBJECT

public:
    ModeWidget(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);

        // 添加模式按钮
        ModeButton *classicModeButton = new ModeButton("经典模式", this);
        ModeButton *timeChallengeButton = new ModeButton("限时挑战", this);
        ModeButton *fortyLinesButton = new ModeButton("40行挑战", this);
        ModeButton *entertainment1ModeButton = new ModeButton("娱乐模式：异色方块", this);
        ModeButton *entertainment2ModeButton = new ModeButton("娱乐模式：障碍赛", this);
        ModeButton *quitButton = new ModeButton("返回", this);

        ModeButton* btns[] = {classicModeButton, timeChallengeButton, fortyLinesButton, entertainment1ModeButton, entertainment2ModeButton, quitButton};
        for (int i = 0; i < 6; ++i) {
            QHBoxLayout *hLayout = new QHBoxLayout;
            hLayout->addStretch();
            hLayout->addWidget(btns[i]);
            hLayout->addStretch();
            layout->addLayout(hLayout); // 把水平布局添加到主垂直布局中
        }

        connect(classicModeButton, &QPushButton::clicked, [this]() { emit modeSelected(0); });
        connect(timeChallengeButton, &QPushButton::clicked, [this]() { emit modeSelected(1); });
        connect(fortyLinesButton, &QPushButton::clicked, [this]() { emit modeSelected(2); });
        connect(entertainment1ModeButton, &QPushButton::clicked, [this]() { emit modeSelected(3); });
        connect(entertainment2ModeButton, &QPushButton::clicked, [this]() { emit modeSelected(4); });
        connect(quitButton, &QPushButton::clicked, [this]() { emit quitToMenu(); });
    }


    void keyPressEvent(QKeyEvent *event) {
        switch (event->key()) {
        case Qt::Key_Escape:
        case Qt::Key_Q:
            emit quitToMenu();
            break;
        }
}


signals:
    void modeSelected(int mode); // 发射选中的模式编号
    void quitToMenu();
};
#endif // MODESELECTIONWIDGET_H