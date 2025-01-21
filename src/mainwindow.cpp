#include "mainwindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QPainter>
#include <QWindow>

MainWindow::MainWindow(QWidget *parent, int DropSpeed, int ColorCount) : 
    QWidget(parent), dropSpeed(DropSpeed), colorCount(ColorCount) {
    setMinimumSize(600, 400);
    layout = new QVBoxLayout(this);
    layout->setObjectName("layout");

    QLabel *title = new QLabel(this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 24px; font-weight: bold;");
    layout->addWidget(title);
    settingsDialog = new SettingsDialog(this, dropSpeed / 2, colorCount); 
    connect(settingsDialog, &SettingsDialog::settingSet, this, &MainWindow::applySettings);

    startButton = new MenuButton("开始游戏", this);
    quitButton = new MenuButton("退出游戏", this);
    helpButton = new MenuButton("游戏帮助", this);
    settingsButton = new MenuButton("难度设置", this);
    loadButton = new MenuButton("读取存档", this);
    MenuButton* btns[] = {startButton, quitButton, helpButton, settingsButton, loadButton};
    for (int i = 0; i < 5; ++i) {
        QHBoxLayout *hLayout = new QHBoxLayout;
        hLayout->addStretch();
        hLayout->addWidget(btns[i]); 
        hLayout->addStretch(); 
        layout->addLayout(hLayout); // 把水平布局添加到主垂直布局中
    }
    
    connect(startButton, &QPushButton::clicked, [this]() { emit startGameClicked(); });
    connect(quitButton, &QPushButton::clicked, this, &MainWindow::quitGame);
    connect(helpButton, &QPushButton::clicked, this, [this]() {
        HelpDialog HelpDialog(this);
        HelpDialog.exec();
    });
    connect(settingsButton, &QPushButton::clicked, this, [=]() {
        settingsDialog->exec();
    });
    connect(loadButton, &QPushButton::clicked, this, [=]() { emit startLoadedGame(); });
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Escape:
    case Qt::Key_Q:
        quitGame();
        break;
    case Qt::Key_F:
        emit refreshStyle();
        break;
    }
}

void MainWindow::quitGame() {
    QApplication::quit();
}

void MainWindow::applySettings(int ColorCount, int DropSpeed) {
    dropSpeed = 2 * DropSpeed;
    colorCount = ColorCount;
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QPixmap pixmap(":/images/back.png");
    if (!pixmap.isNull()) {
        QSize targetSize(width(), height()); // 调整为窗口大小
        QPixmap scaledPixmap = pixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        painter.drawPixmap(0, 0, scaledPixmap);
    }
    // pixmap = QPixmap(":/images/button.png");
    // if (!pixmap.isNull()) {
    //     QSize targetSize(startButton->width(), startButton->height()); // 调整为窗口大小
    //     QPixmap scaledPixmap = pixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    //     MenuButton* btns[] = {startButton, quitButton, helpButton, settingsButton, loadButton};
    //     for (int i=0;i<5;i++) {
    //         painter.drawPixmap(btns[i], 0, scaledPixmap);
    //     }
        
    // }
    QWidget::paintEvent(event); // 调用基类的 paintEvent 保持其他绘制行为
}
