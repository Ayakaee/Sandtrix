#include "gamewidget.h"
#include <QPainter>
#include <QTimer>
#include <iostream>
#include <QDebug>
#include <QVBoxLayout>
#include <random>
#include <QKeyEvent>
#include <QFileDialog>

Grid grid;
std::ofstream out;
GameWidget::GameWidget(QWidget *parent, int DropSpeed, int ColorCount) : 
    QWidget(parent), isPaused(false), isFalling(false), multi(1), score(-256),
    continuous(false), overLine(20), colorCount(ColorCount), dropSpeed(DropSpeed),
    mode(0), clearLines(0), totalTime(0), connected(true) {
    setupUI();
    this->setFocus();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> shape(1, 6);
    std::uniform_int_distribution<> color(7, 6 + colorCount);
    nextShape = shape(gen);
    nextColor = Color(color(gen));
    nextColor2 = (mode == 3)? Color(color(gen)) : nextColor;
    memset(paintGrid, 0, sizeof(paintGrid));
    memset(isFall, 0, sizeof(isFall));
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &GameWidget::gameUpdate);
    connected = true;
}
GameWidget::~GameWidget() {
    out.close();
}

template <typename T>
void logw(T content) {
    out = std::ofstream("out.txt", std::ios::app);
    out<<content<<"\n";
    out.close();
}

void GameWidget::setMode(int Mode) {
    mode = Mode;
    resetGame();
    if (mode != 2) lineLabel->setText(QString());
    if (mode != 1 || mode != 2) timeLabel->setText(QString());
    if (mode == 1) leftTime = 120.0;
    else if (mode == 2 || mode == 4) {
        clearLines = 0;
        totalTime = 0;
    }
    gameTimer->start(50);   
}

void GameWidget::setupUI() {
    out<<":sfes";
    out.close();
    pauseButton = new GameButton("暂停游戏", this);
    resetButton = new GameButton("重新开始", this);
    exitButton = new GameButton("退出", this);
    saveButton = new GameButton("存档", this);
    pauseButton->setAutoDefault(false);
    resetButton->setAutoDefault(false);
    exitButton->setAutoDefault(false);
    saveButton->setAutoDefault(false);
    scoreLabel = new GameLabel(this);
    nextLabel = new GameLabel(this);
    timeLabel = new GameLabel(this);
    lineLabel = new GameLabel(this);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // To paint sand
    QWidget *drawingArea = new QWidget(this);
    drawingArea->resize(GRID_WIDTH * CELL_SIZE, GRID_HEIGHT * CELL_SIZE);
    drawingArea->setMinimumWidth(GRID_WIDTH * CELL_SIZE);

    QWidget *controlPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(controlPanel);
    nextDraw = new QWidget(this);
    nextDraw->setFixedSize(20 * CELL_SIZE, 20 * CELL_SIZE);

    void* wgts[] = {nextLabel, nextDraw, scoreLabel, timeLabel, lineLabel, pauseButton, pauseButton, resetButton, exitButton, saveButton};
    for (int i = 0; i < 10; ++i) {
        QHBoxLayout *hLayout = new QHBoxLayout;
        hLayout->addStretch();
        if (i == 1) hLayout->addWidget(static_cast<QWidget*>(wgts[i]));
        else if (i >= 5) hLayout->addWidget(static_cast<GameButton*>(wgts[i]));
        else hLayout->addWidget(static_cast<GameLabel*>(wgts[i]));
        hLayout->addStretch(); 
        rightLayout->addLayout(hLayout);
    }
    mainLayout->addWidget(drawingArea);
    mainLayout->addWidget(controlPanel);
    setLayout(mainLayout);

    nextLabel->setText("下一个颜色");
    scoreLabel->setAlignment(Qt::AlignCenter);
    nextLabel->setAlignment(Qt::AlignCenter);
    timeLabel->setAlignment(Qt::AlignCenter);
    lineLabel->setAlignment(Qt::AlignCenter);
    if (mode == 1) timeLabel->setText("时间：" + QString::number(leftTime, 'f', 2));
    else if (mode == 2 || mode == 4) timeLabel->setText("时间：" + QString::number(totalTime, 'f', 2));
    if (mode == 2) lineLabel->setText(QString("消除行数：%1").arg(clearLines));

    connect(pauseButton, &QPushButton::clicked, this, &GameWidget::togglePause);
    connect(resetButton, &QPushButton::clicked, this, &GameWidget::resetGame);
    connect(exitButton, &QPushButton::clicked, this, &GameWidget::exitGame);
    connect(saveButton, &QPushButton::clicked, [&]() {
        QString path = QFileDialog::getOpenFileName(this, "选择输入文件", "", "Json Files (*.json);;All Files (*)");
        togglePause();
        saveGame(path);
    });
}

void GameWidget::keyPressEvent(QKeyEvent *event) {
    if (isPaused) {
        switch (event->key()) {
        case Qt::Key_Escape:
            togglePause();
            break;
        case Qt::Key_R:
            resetGame();
            break;
        case Qt::Key_M:
            exitGame();
            break;
        case Qt::Key_Q:
            exitGame();
            break;
        }
        return;
    }

    switch (event->key()) {
    case Qt::Key_Escape:
        togglePause();
        break;
    case Qt::Key_R:
        resetGame();
        break;
    case Qt::Key_Q:
        exitGame();
        break;
    case Qt::Key_Direction_L:
    case Qt::Key_A:
        toLeft = true;
        break;
    case Qt::Key_Direction_R:
    case Qt::Key_D:
        toRight = true;
        break;
    case Qt::Key_S:
        dropSpeed *= 2;
        break;
    case Qt::Key_Space:
    case Qt::Key_C:
        moveDown();
        break;
    default:
        out<<"Key Pressed:"<<event->key()<<'\n';
        break;
    }
    
}

void GameWidget::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_S) dropSpeed/= 2;
}

void GameWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QPainter painter(this);
    
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            if (grid[point(y, x)] == rock) {
                painter.setBrush(Qt::gray); // Fixed blocks
                painter.setPen(Qt::gray);
            } else if (grid[point(y, x)] == none) {
                QColor color;
                color.setRgb(248, 248, 248);
                painter.setBrush(color); // Empty space
                painter.setPen(color);
            } else if (grid[point(y, x)] >= red && grid[point(y, x)] <= yellow){
                painter.setBrush(Qt::GlobalColor(grid[point(y, x)])); // Sand
                painter.setPen(Qt::GlobalColor(grid[point(y, x)]));
            }
            painter.drawRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
        }
    }
    int linew = 1;
    painter.setBrush(Qt::black);
    painter.setPen(Qt::black);
    painter.drawRect(0, overLine * CELL_SIZE, GRID_WIDTH * CELL_SIZE, linew  * CELL_SIZE);

    // paint next block on the right
    QPoint pos = nextDraw->mapToGlobal(QPoint(0, 0)) - mapToGlobal(QPoint(0, 0));
    int x = pos.x(), y = pos.y();
    int blockSize = nextDraw->width();
    painter.setBrush(Qt::GlobalColor(nextColor));
    painter.setPen(Qt::GlobalColor(nextColor));
    painter.drawRect(x, y, blockSize, blockSize / 2);
    painter.setBrush(Qt::GlobalColor(nextColor2));
    painter.setPen(Qt::GlobalColor(nextColor2));
    painter.drawRect(x, y + blockSize / 2, blockSize, blockSize / 2);
    
    if (over && mode != 4) {
        QPixmap pixmap(":/images/overbig.png");
        if (!pixmap.isNull()) {
            logw("painted");
            float w = GRID_WIDTH * CELL_SIZE;
            QSize size = pixmap.size();
            QSize targetSize(w, w / size.width() * size.height());
            QPixmap scaledPixmap = pixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            painter.drawPixmap(0, 0, scaledPixmap);
        }
    } else if (over) {
        QPixmap pixmap(":/images/win.png");
        if (!pixmap.isNull()) {
            logw("painted");
            float w = GRID_WIDTH * CELL_SIZE;
            QSize size = pixmap.size();
            QSize targetSize(w, w / size.width() * size.height());
            QPixmap scaledPixmap = pixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            painter.drawPixmap(0, 0, scaledPixmap);
        }
    }
    // QPixmap pixmap(":/images/over.jpg");
    //     if (!pixmap.isNull()) {
    //         logw("painted");
    //         int w = GRID_WIDTH * CELL_SIZE;
    //         QSize size = pixmap.size();
    //         QSize targetSize(w, w / size.width() * size.height());
    //         QPixmap scaledPixmap = pixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    //         painter.drawPixmap(0, 0, scaledPixmap);
    //     }
}

bool GameWidget::clear() {
    bool cleared = false;
    for (int color = 7; color <= 12; ++color) {
        if (checkAndClear(Color(color))) {
            cleared = true;
            clearLines++;
        }
    }
    return cleared;
}

bool GameWidget::checkAndClear(Color color) {
    std::vector<std::vector<bool>> visited(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false));
    std::vector<point> toClear;

    // 检测每行的左边是否存在该颜色的起点
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        if (grid[point(y, 0)] == color && !visited[y][0]) {
            bool find = false;
            dfs(y, 0, color, visited, toClear, find);
            if (find) {
                out<<"find clear\n";
                score += toClear.size() * multi * 4;
                multi *= (continuous && multi < 128)? 2 : 1;
                continuous = true;
                scoreLabel->setText(QString("分数：%1").arg(score));
                for (auto p : toClear) {
                    grid[p] = none; 
                    // 新方块产生逻辑为消除检测之后再产生，如果更改，需要修改此处关于fallSand, isFall
                    // 但是不能把新生成的方块清除
                }
                return true;
            } else toClear.clear();
        }
    }
    return false;
}

void GameWidget::dfs(int y, int x, int color, std::vector<std::vector<bool>> &visited, std::vector<point> &toClear, bool &find) {
    if (x == GRID_WIDTH - 1) {
        find = true;
    }
    visited[y][x] = true;
    toClear.emplace_back(y, x);

    static const int dx[] = {1, -1, 0, 0};
    static const int dy[] = {0, 0, 1, -1};
    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT && !visited[ny][nx] && grid[point(ny, nx)] == color) {
            dfs(ny, nx, color, visited, toClear, find);
        }
    }
}

void GameWidget::gameUpdate() {
    int changeNum = 0;
    leftTime -= 0.05;
    totalTime += 0.05;
    updateSand(changeNum);
    // 移动沙子数量小于10时判定消除和产生新方块
    if (!isFalling && changeNum < 10) {
        if (clear()) continuous = true;
        else continuous = false;
        if (!continuous) {
            fallBlock.begin = addBlock(nextShape, 10, nextColor, nextColor2);
            fallBlock.color = nextColor;
            fallBlock.shape = nextShape;
            fallBlock.w = 10;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> shape(1, 6);
            std::uniform_int_distribution<> color(7, 6 + colorCount);
            nextShape = shape(gen);
            nextColor = Color(color(gen));
            nextColor2 = (mode == 3)? Color(color(gen)) : nextColor;
            isFalling = true;
            continuous = false;
            score += 256;
            scoreLabel->setText(QString("分数：%1").arg(score));
            downEnabled = true;
        }
    }
    update();
}

void GameWidget::updateSand(int &change) {
    if (toLeft) moveLeft();
    if (toRight) moveRight();
    toLeft = toRight = false;
    change = 0;
    int steps = 2, times = dropSpeed / steps;
    if (mode == 1) {
        timeLabel->setText(QString("时间：" + QString::number(leftTime, 'f', 2)));
        if (abs(leftTime < 0.05)) {
            timeLabel->setText(QString("时间：0.00"));
            gameOver();
            return;
        }
    } else if (mode == 2) {
        timeLabel->setText("时间：" + QString::number(totalTime, 'f', 2));
        lineLabel->setText(QString("消除行数：%1").arg(clearLines));
        if (clearLines == 40) {
            gameOver();
            return;
        }
    } else if (mode == 4) {
        timeLabel->setText("时间：" + QString::number(totalTime, 'f', 2));
        int rocks = 0;
        float beg = 0.4, end = 0.7;
        int w = 30;
        for (int y= beg * GRID_HEIGHT; y < end * GRID_HEIGHT;y++)
        for (int x=0;x<w;x++) if (grid[point(y, x)] == rock) rocks++;
        for (int y= beg * GRID_HEIGHT; y < end * GRID_HEIGHT;y++)
        for (int x=GRID_WIDTH-w;x < GRID_WIDTH;x++) if (grid[point(y, x)] == rock) rocks++;
        if (rocks == 0) gameOver("You Win!");
    }
    
    // 判断是否落地
    for (int _=0;_<times && isFalling;_++){
        for (auto p : fall) {
            auto prev_p = p;
            p.first += steps;
            if (grid[p] != none) {
                if (fallBlock.begin.first <= overLine) {
                    gameOver();
                    return;
                }
                out<<"not falling"<<p.first-steps<<' '<<p.first<<' '<<p.second<<'\n';
                isFalling = false;
                downEnabled = false;
                for (auto p : fallSand) isFall[p.first.first][p.first.second] = false;
                fall.clear();
                fallSand.clear();
                break;
            }
        }
        if (isFalling) {
            for (auto &p : fall) p.first += steps;
            for (auto &p : fallSand) {
                grid[p.first] -= p.second; // 用加减代替赋值，防止被覆盖
                isFall[p.first.first][p.first.second]--;
                p.first.first += steps;
                grid[p.first] += p.second;
                isFall[p.first.first][p.first.second]++;
                change++;
            }
            fallBlock.begin.first += steps;
        }
    }

    // 落地后下落判定
    for (int _=0;_<2;_++){
        for (int y = GRID_HEIGHT - 2; y >= 0; --y) for (int x = 0; x < GRID_WIDTH; ++x) {
            if (grid[point(y, x)] > 1 && !isFall[y][x]) {
                if (grid[point(y + 2, x)] == 0) {
                    std::swap(grid[point(y + 2, x)], grid[point(y, x)]);
                    change++;
                } else if (x > 0 && grid[point(y + 1, x - 1)] == 0) {
                    std::swap(grid[point(y + 1, x - 1)], grid[point(y, x)]);
                    change++;
                } else if (x < GRID_WIDTH - 1 && grid[point(y + 1, x + 1)] == 0) {
                    std::swap(grid[point(y + 1, x + 1)], grid[point(y, x)]);
                    change++;
                }
            }
        }
    }
}

void GameWidget::addOneBlock(int x, int y, int w, Color color, bool addFall) {
    for (int i=y; i<y+w;i++) for (int j=x; j<x+w;j++) {
        grid[point(i, j)] = color;
        fallSand.push_back(sand(point(i, j), color));
        isFall[i][j] = true;
    }
    if (!addFall) return;
    for (int j=x; j<x+w;j++) {
        fall.push_back(point(y + w - 1, j));
    }
}

point GameWidget::addBlock(int shape, int w, Color color1, Color color2) {
    int mid = GRID_WIDTH / 2;
    switch (shape) {
    case 1: // 正方形
        addOneBlock(mid - w, 0, w, color1);
        addOneBlock(mid, 0, w, color1);
        addOneBlock(mid - w, w, w, color2, true);
        addOneBlock(mid, w, w, color2, true);
        return point(0, mid - w);
        break;
    case 2: // 上T型
        addOneBlock(mid - 0.5 * w, 0, w, color1);
        addOneBlock(mid - 1.5 * w, w, w, color2, true);
        addOneBlock(mid - 0.5 * w, w, w, color2, true);
        addOneBlock(mid + 0.5 * w, w, w, color2, true);
        return point(0, mid - 1.5 * w);
        break;
    case 3: // 下T型
        addOneBlock(mid - 1.5 * w, 0, w, color1, true);
        addOneBlock(mid - 0.5 * w, 0, w, color1);
        addOneBlock(mid + 0.5 * w, 0, w, color1, true);
        addOneBlock(mid - 0.5 * w, w, w, color2, true);
        return point(0, mid - 1.5 * w);
        break;
    case 4: // 左L型
        addOneBlock(mid - 1.5 * w, 0, w, color1);
        addOneBlock(mid - 1.5 * w, w, w, color2, true);
        addOneBlock(mid - 0.5 * w, w, w, color2, true);
        addOneBlock(mid + 0.5 * w, w, w, color2, true);
        return point(0, mid - 1.5 * w);
        break;
    case 5: // 右L型
        addOneBlock(mid + 0.5 * w, 0, w, color1);
        addOneBlock(mid - 1.5 * w, w, w, color2, true);
        addOneBlock(mid - 0.5 * w, w, w, color2, true);
        addOneBlock(mid + 0.5 * w, w, w, color2, true);
        return point(0, mid - 1.5 * w);
        break;
    case 6: // 长条型
        addOneBlock(mid - 2 * w, 0, w, color1, true);
        addOneBlock(mid -  w, 0, w, color1, true);
        addOneBlock(mid, 0, w, color2, true);
        addOneBlock(mid + 1 * w, 0, w, color2, true);
        return point(0, mid - 2 * w);
        break;
    default:
        return point(0, 0);
        break;
    }
}

void GameWidget::moveLeft() {
    if (!isFalling) return;
    int w = 20;
    int xMin = GRID_WIDTH;
    for (auto p : fallSand) xMin = std::min(p.first.second, xMin);
    int d = std::min(w, xMin); // d:移动距离
    for (auto &p : fallSand) {
        grid[p.first] = none;
        isFall[p.first.first][p.first.second]--;
        p.first.second -= d;
        grid[p.first] = p.second;
        isFall[p.first.first][p.first.second]++;
    }
    for (auto &p : fall) p.second -= d;
    fallBlock.begin.second -= d;
}

void GameWidget::moveRight() {
    if (!isFalling) return;
    int w = 20;
    int xMax = 0;
    for (auto p : fallSand) xMax = std::max(p.first.second, xMax);
    int d = std::min(w, GRID_WIDTH - xMax - 1); // d:移动距离
    for (auto p : fallSand) {
        grid[p.first] = none;
        isFall[p.first.first][p.first.second]--;
    }
    for (auto &p : fallSand) {
        p.first.second += d;
        grid[p.first] = p.second;
        isFall[p.first.first][p.first.second]++;
    }
    for (auto &p : fall) p.second += d;
    fallBlock.begin.second += d;
}

void GameWidget::moveDown() {
    // 只负责移动到落地前的位置，落地状态修改仍在updateSand中进行
    if (!downEnabled) return;
    downEnabled = true;
    int steps = 2;
    while (true) {
        for (auto p : fall) {
            auto prev_p = p;
            p.first += steps;
            if (grid[p] != none) {
                if (fallBlock.begin.first <= overLine) {
                    gameOver();
                    return;
                }
                return;
            }
        }
        if (isFalling) {
            for (auto &p : fall) p.first += steps;
            for (auto &p : fallSand) {
                grid[p.first] -= p.second; // 用加减代替赋值，防止被覆盖
                isFall[p.first.first][p.first.second]--;
                p.first.first += steps;
                grid[p.first] += p.second;
                isFall[p.first.first][p.first.second]++;
            }
            fallBlock.begin.first += steps;
        }
    }
}

void GameWidget::gameOver(QString str) {
    gameTimer->stop();
    over = true;
    update();
    connected = false;
}

void GameWidget::togglePause() {
    isPaused = !isPaused;
    pauseButton->setText(isPaused ? "游戏继续" : "游戏暂停");
    
    if (isPaused) {
        out = std::ofstream("out.txt", std::ios::app);
        out<<"toggle start\n";
        out.close();
        connected = false;
        gameTimer->stop();
        out = std::ofstream("out.txt", std::ios::app);
        out<<"toggle ok\n";
        out.close();
    } else {
        gameTimer->start(50);
        connected = true;
    }
}

void GameWidget::resetGame(bool timerConnect) {
    if (over) gameTimer->start(50);
    over = false;
    grid.clear();
    int w = 30;
    float beg = 0.4, end = 0.7;
    if (mode == 4) {
        for (int y=beg * GRID_HEIGHT; y < end * GRID_HEIGHT;y++)
        for (int x=0;x<w;x++) grid[point(y, x)] = rock;
        for (int y=beg * GRID_HEIGHT; y < end * GRID_HEIGHT;y++)
        for (int x=GRID_WIDTH-w;x < GRID_WIDTH;x++) grid[point(y, x)] = rock;
    }
    leftTime = 120.0;
    clearLines = 0;
    score = -256;
    multi = 1;
    continuous = false;
    isFalling = false;
    toLeft = toRight = false;
    fall.clear();
    fallSand.clear();
    update();
    connected = true;
}

void GameWidget::exitGame() {
    gameTimer->stop();
    emit exitToMode();
}

void GameWidget::saveGame(const QString& filePath) {
    QJsonObject saveData;
    QJsonArray fallArray;

    saveData["fallBlock"] = fallBlock.toJson();
    saveData["isFalling"] = isFalling;
    saveData["isPaused"] = isPaused;
    saveData["continuous"] = continuous;
    saveData["downEnabled"] = downEnabled;
    saveData["connected"] = connected;
    saveData["score"] = score;
    saveData["multi"] = multi;
    saveData["overLine"] = overLine;
    saveData["dropSpeed"] = dropSpeed;
    saveData["colorCount"] = colorCount;
    saveData["nextShape"] = nextShape;
    saveData["mode"] = mode;
    saveData["clearLines"] = clearLines;
    saveData["leftTime"] = leftTime;
    saveData["totalTime"] = totalTime;
    saveData["nextColor"] = static_cast<int>(nextColor);
    saveData["nextColor2"] = static_cast<int>(nextColor2);

    for (const auto& point : fall) {
        QJsonObject pointObj;
        pointObj["x"] = point.first;
        pointObj["y"] = point.second;
        fallArray.append(pointObj);
    }
    saveData["fall"] = fallArray;
    QJsonArray fallSandArray;
    for (const auto& p : fallSand) {
        
        QJsonObject pointObj;
        pointObj["px"] = p.first.first;
        pointObj["py"] = p.first.second;
        pointObj["c"] = static_cast<int>(p.second);
        fallSandArray.append(pointObj);
    }
    saveData["fallSand"] = fallSandArray;
    QJsonArray gridArray;
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        QJsonArray rowArray;
        for (int j = 0; j < GRID_WIDTH; ++j) {
            rowArray.append(grid[point(i, j)]);
        }
        gridArray.append(rowArray);
    }
    saveData["Grid"] = gridArray;
    QJsonArray isFallArray;
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        QJsonArray rowArray;
        for (int j = 0; j < GRID_WIDTH; ++j) {
            rowArray.append(isFall[i][j]);
        }
        isFallArray.append(rowArray);
    }
    saveData["isFall"] = isFallArray;

    QFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }
    QJsonDocument saveDoc(saveData);
    saveFile.write(saveDoc.toJson());
}

void GameWidget::loadGame(const QString& filePath) {
    QFile loadFile(filePath);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject jsonObj = loadDoc.object();

    fall.clear();
    QJsonArray fallArray = jsonObj["fall"].toArray();
    for (const auto& pointValue : fallArray) {
        QJsonObject pointObj = pointValue.toObject();
        fall.emplace_back(pointObj["x"].toInt(), pointObj["y"].toInt());
    }
    fallSand.clear();
    QJsonArray fallSandArray = jsonObj["fallSand"].toArray();
    for (const auto& sandValue : fallSandArray) {
        sand s;
        auto obj = sandValue.toObject();
        s.first.first = obj["px"].toInt();
        s.first.second = obj["py"].toInt();
        s.second = static_cast<Color>(obj["c"].toInt());
        fallSand.push_back(s);
    }
    QJsonArray gridArray = jsonObj["Grid"].toArray();
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        QJsonArray rowArray = gridArray[i].toArray();
        for (int j = 0; j < GRID_WIDTH; ++j) {
            grid[point(i, j)] = static_cast<Color>(rowArray[j].toInt());
        }
    }
    QJsonArray isFallArray = jsonObj["isFall"].toArray();
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        QJsonArray rowArray = isFallArray[i].toArray();
        for (int j = 0; j < GRID_WIDTH; ++j) {
            isFall[i][j] = rowArray[j].toInt();
        }
    }

    fallBlock = fromJson(jsonObj["fallBlock"].toObject());
    isFalling = jsonObj["isFalling"].toBool();
    isPaused = jsonObj["isPaused"].toBool();
    continuous = jsonObj["continuous"].toBool();
    downEnabled = jsonObj["downEnabled"].toBool();
    connected = jsonObj["connected"].toBool();
    score = jsonObj["score"].toInt();
    multi = jsonObj["multi"].toInt();
    overLine = jsonObj["overLine"].toInt();
    dropSpeed = jsonObj["dropSpeed"].toInt();
    colorCount = jsonObj["colorCount"].toInt();
    nextShape = jsonObj["nextShape"].toInt();
    mode = jsonObj["mode"].toInt();
    clearLines = jsonObj["clearLines"].toInt();
    leftTime = jsonObj["leftTime"].toDouble();
    totalTime = jsonObj["totalTime"].toDouble();
    nextColor = static_cast<Color>(jsonObj["nextColor"].toInt());
    nextColor2 = static_cast<Color>(jsonObj["nextColor2"].toInt());
}

void GameWidget::loadGameStart(const QString& filePath) {
    connected = false;
    gameTimer = new QTimer(this);
    resetGame();
    loadGame(filePath);
    isPaused = false;
    connect(gameTimer, &QTimer::timeout, this, &GameWidget::gameUpdate);
    connected = true;
    gameTimer->start(50);
    disconnect(gameTimer, &QTimer::timeout, this, &GameWidget::gameUpdate);
    connect(gameTimer, &QTimer::timeout, this, &GameWidget::gameUpdate);
}

Color& operator+=(Color &a, Color &b) {
    a = Color(int(a) + int(b));
    return a;
}

Color& operator-=(Color &a, Color &b) {
    a = Color(int(a) - int(b));
    return a;
}

void GameWidget::setDifficulty(int ColorCount, int DropSpeed) {
    colorCount = ColorCount;
    dropSpeed = 2 * DropSpeed;
}

Block fromJson(const QJsonObject& obj) {
    Block block;
    block.begin.first = obj["beginx"].toInt();
    block.begin.second = obj["beginy"].toInt();
    block.shape = obj["shape"].toInt();
    block.w = obj["w"].toInt();
    block.color = static_cast<Color>(obj["color"].toInt());
    return block;
}
