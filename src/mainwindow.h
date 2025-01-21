#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QDialog>
#include <QTextBrowser>
#include <QKeyEvent>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QIcon>
#include <QLabel>
#include "gamewidget.h"

class MenuButton : public QPushButton {
    Q_OBJECT
public:
    explicit MenuButton(const QString &text, QWidget *parent = nullptr) : QPushButton(text, parent) {}
};
class SettingButton : public QPushButton {
    Q_OBJECT
public:
    explicit SettingButton(const QString &text, QWidget *parent = nullptr) : QPushButton(text, parent) {}
};

const QString settingsPath = "settings.json";
class SettingsDialog;

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, int DropSpeed = 4, int ColorCount = 4);
    SettingsDialog *settingsDialog;
    void paintEvent(QPaintEvent *event);

private slots:
    void quitGame();
    void applySettings(int ColorCount, int DropSpeed);

private:
    int dropSpeed;
    int colorCount;
    
    QVBoxLayout *layout;
    QLabel *title;
    MenuButton *startButton;
    MenuButton *quitButton;
    MenuButton *settingsButton;
    MenuButton *helpButton;
    MenuButton *loadButton;

    void keyPressEvent(QKeyEvent *event);

signals:

    void startGameClicked();
public: signals:
    void startLoadedGame();
    void refreshStyle();
};

class HelpDialog : public QDialog {
    Q_OBJECT

public:
    HelpDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("帮助");
        QVBoxLayout *layout = new QVBoxLayout(this);

        // 创建 QTextBrowser 用于显示 HTML 格式的帮助信息
        QTextBrowser *textBrowser = new QTextBrowser(this);
        textBrowser->setReadOnly(true);
        textBrowser->setHtml(R"(
            <html>
            <head>
                <style>
                    body {
                        font-family: Arial, sans-serif;
                        margin: 10px;
                    }
                    h1 {
                        color: #333;
                        font-size: 20px;
                    }
                    ul {
                        padding-left: 20px;
                    }
                    li {
                        margin-bottom: 5px;
                        font-size: 16px;
                    }
                    .key {
                        font-weight: bold;
                        color: #0056b3;
                    }
                </style>
            </head>
            <body>
                <h1>按键说明</h1>
                <ul>
                    <li><span class="key">A</span>：方块左移</li>
                    <li><span class="key">D</span>：方块右移</li>
                    <li><span class="key">S</span>：加速下落</li>
                    <li><span class="key">C</span>：立即下落</li>
                    <li><span class="key">R</span>：重新开始</li>
                    <li><span class="key">Q</span>：返回主菜单/退出游戏</li>
                    <li><span class="key">Esc</span>：游戏暂停/继续</li>
                </ul>
                <h1>游戏规则</h1>
                <ul>
                    <li>1. 尽可能消除更多的沙粒。</li>
                    <li>2. 使沙粒从左到右相连，可以消除沙粒，获得分数。</li>
                    <li>3. 当方块接触地面或其他沙粒后，按照重力规律下落。</li>
                    <li>4. 当沙粒堆叠到顶部时游戏结束。</li>
                </ul>
                <h1>模式说明</h1>
                <ul>
                    <li>经典模式：无尽模式，沙粒堆叠到顶结束</li>
                    <li>限时模式：一定时间内获得最高分数</li>
                    <li>障碍赛：  方块左右移动时可以吞噬障碍物，尽情消灭障碍物吧！</li>
                    <li>异色方块：方块可以由两种颜色组成</li>
                    <li>40行挑战：最快消除40次沙粒</li>
                </ul>
            </body>
            </html>
        )");

        layout->addWidget(textBrowser);
        QPushButton *closeButton = new QPushButton("我知道啦", this);
        closeButton->setObjectName("helpButton");
        layout->addWidget(closeButton);
        textBrowser->setStyleSheet("background color: #f3f3f3;");
        connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
        setLayout(layout);
        setMinimumSize(600, 600);
    }
};

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = nullptr, int dropSpeed = 2, int colorCount = 4) : 
    QDialog(parent) {
        setWindowTitle("设置");
        saved = false;

        QVBoxLayout *layout = new QVBoxLayout(this);

        // 颜色数量选项
        GameLabel *colorsLabel = new GameLabel("颜色数量：", this);
        layout->addWidget(colorsLabel);

        QSpinBox *colorSpinBox = new QSpinBox(this);
        colorSpinBox->setRange(3, 6);
        colorSpinBox->setValue(colorCount);
        layout->addWidget(colorSpinBox);

        // 下落速度选项
        GameLabel *speedLabel = new GameLabel("下落速度：", this);
        layout->addWidget(speedLabel);

        QSpinBox *speedSpinBox = new QSpinBox(this);
        speedSpinBox->setRange(1, 4); // 1为最慢，10为最快
        speedSpinBox->setValue(dropSpeed);    // 默认值
        layout->addWidget(speedSpinBox);

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        SettingButton *confirmButton = new SettingButton("确认", this);
        SettingButton *cancelButton = new SettingButton("取消", this);
        buttonLayout->addWidget(confirmButton);
        buttonLayout->addWidget(cancelButton);
        layout->addLayout(buttonLayout);

        setLayout(layout);

        // 信号槽连接
        connect(confirmButton, &QPushButton::clicked, [=]() {
            emit settingSet(colorSpinBox->value(), speedSpinBox->value());
            accept();
        });

        connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
        //connect(dialog, &SettingsDialog::settingsChanged, gameWidget, &GameWidget::setDifficulty);
    }

private:
    bool saved;

    // bool saveToFile(const QString &filePath) {
    //     QJsonObject jsonObj;
    //     jsonObj["colorCount"] = colorCount;
    //     jsonObj["dropSpeed"] = dropSpeed;
    //     QJsonDocument jsonDoc(jsonObj);
    //     QFile file(filePath);
    //     if (!file.open(QIODevice::WriteOnly)) {
    //         qWarning() << "无法打开文件用于写入:" << filePath;
    //         return false;
    //     }
    //     file.write(jsonDoc.toJson());
    //     file.close();
    //     return true;
    // }
    // bool loadFromFile(const QString &filePath) {
    //     QFile file(filePath);
    //     if (!file.open(QIODevice::ReadOnly)) {
    //         qWarning() << "无法打开文件用于读取:" << filePath;
    //         return false;
    //     }
    //     QByteArray data = file.readAll();
    //     file.close();
    //     QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    //     if (!jsonDoc.isObject()) {
    //         qWarning() << "JSON 格式错误";
    //         return false;
    //     }
    //     QJsonObject jsonObj = jsonDoc.object();
    //     if (jsonObj.contains("colorCount")) {
    //         colorCount = jsonObj["colorCount"].toInt();
    //     }
    //     if (jsonObj.contains("dropSpeed")) {
    //         dropSpeed = jsonObj["dropSpeed"].toInt();
    //     }
    //     return true;
    // }

public: signals:
    void settingSet(int ColorCount, int DropSpeed);
};

#endif // MAINWINDOW_H