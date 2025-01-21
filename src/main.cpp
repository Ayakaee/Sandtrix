#include <QApplication>
#include <QStackedWidget>
#include <QFileDialog>
#include <QTimer>
#include <QPalette>
#include <QFontDataBase>
#include "mainwindow.h"
#include "modewidget.h"
#include "gamewidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow mainWindow;
    ModeWidget modeWidget;
    GameWidget gameWidget;
    QFileDialog fileDialog;
    QFontDatabase::addApplicationFont(":/fonts/HYWenHei.ttf");

    // 添加界面到 QStackedWidget
    QStackedWidget stackedWidget;
    stackedWidget.addWidget(&mainWindow);
    stackedWidget.addWidget(&modeWidget);
    stackedWidget.addWidget(&gameWidget);
    stackedWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QSize gameSize(GRID_WIDTH * CELL_SIZE + 100, GRID_HEIGHT * CELL_SIZE);
    QSize mainSzie(400, 300);
    
    // 连接信号槽切换界面
    QObject::connect(&mainWindow, &MainWindow::startGameClicked, [&]() {
        stackedWidget.setCurrentWidget(&modeWidget);
    });
    QObject::connect(&modeWidget, &ModeWidget::modeSelected, [&](int mode) {
        gameWidget.setFocusPolicy(Qt::StrongFocus);
        gameWidget.setMode(mode);
        stackedWidget.setCurrentWidget(&gameWidget);
        stackedWidget.resize(gameSize);
        //stackedWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    });
    QObject::connect(&gameWidget, &GameWidget::exitToMode, [&]() {
        stackedWidget.setCurrentWidget(&modeWidget);
        stackedWidget.resize(mainSzie);
    });
    QObject::connect(&modeWidget, &ModeWidget::quitToMenu, [&]() {
        stackedWidget.setCurrentWidget(&mainWindow);
    });
    QObject::connect(mainWindow.settingsDialog, &SettingsDialog::settingSet, [&](int ColorCount, int DropSpeed) {
        gameWidget.setDifficulty(ColorCount, DropSpeed);
    });
    QObject::connect(&mainWindow, &MainWindow::startLoadedGame, [&]() {
        QString path = fileDialog.getOpenFileName(nullptr, "选择输入文件", "", "Json Files (*.json);;All Files (*)");
        if (path.isEmpty() || path.isNull()) return;
        gameWidget.loadGameStart(path);
        gameWidget.setFocusPolicy(Qt::StrongFocus);
        stackedWidget.setCurrentWidget(&gameWidget);
        stackedWidget.resize(gameSize);
    });
    QObject::connect(&mainWindow, &MainWindow::refreshStyle, [&]() {
        QFile file(":/styles/style.qss");
        if (file.open(QFile::ReadOnly)) {
            QString styleSheet = QLatin1String(file.readAll());
            app.setStyleSheet(styleSheet);
        }
    });

    QFile file(":/styles/style.qss");
    if (file.open(QFile::ReadOnly)) {
        out = std::ofstream("out.txt", std::ios::app);
        out<<"toggle start\n";
        out.close();
        QString styleSheet = file.readAll();
       app.setStyleSheet(styleSheet);
    }
    stackedWidget.show();
    return app.exec();
}
