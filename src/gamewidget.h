#ifndef SANDTRIX_H
#define SANDTRIX_H

#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QPushButton>
#include <vector>
#include <utility>
#include <QLabel>
#include <fstream>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

const int GRID_WIDTH = 120;
const int GRID_HEIGHT = 200;
const int CELL_SIZE = 3;
const int vGap = 5;
const int hGap = 5;
extern std::ofstream out;

enum Color {
    none = 0,
    rock = 1,
    red = 7,
    green,
    blue,
    cyan,
    magenta,
    yellow,
};

Color& operator+=(Color &a, Color &b);
Color& operator-=(Color &a, Color &b);
typedef std::pair<int, int> point;
typedef std::pair<point, Color> sand;

struct Block {
    point begin;
    int shape;
    int w;
    Color color;
    Block(point b, int s, Color c) : begin(b), shape(s), color(c) {}
    Block() {}
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["beginx"] = begin.first;
        obj["beginy"] = begin.second;
        obj["shape"] = shape;
        obj["w"] = w;
        obj["color"] = static_cast<int>(color);
        return obj;
    }
};
class Grid {
    public:
    Color grid[GRID_HEIGHT + 2 * vGap][GRID_WIDTH + 2 * hGap];
    Grid() {
        for (int i=0;i<GRID_HEIGHT + 2 * vGap;i++) for (int j=0;j<GRID_WIDTH + 2 * hGap;j++) {
            grid[i][j] = rock;
        }
        for (int i=vGap;i<GRID_HEIGHT + vGap;i++) for (int j=hGap;j<GRID_WIDTH + hGap;j++) {
            grid[i][j] = none;
        }
    }
    Color& operator[](point p) {
        return grid[p.first + vGap][p.second + hGap];
    }
    void clear() {
        for (int i=vGap;i<GRID_HEIGHT + vGap;i++) for (int j=hGap;j<GRID_WIDTH + hGap;j++) {
            grid[i][j] = none;
        }
    }
};
extern Grid grid;

class GameButton : public QPushButton {
    Q_OBJECT
public:
    explicit GameButton(const QString &text, QWidget *parent = nullptr) : QPushButton(text, parent) {}
};

class GameLabel : public QLabel {
    Q_OBJECT
public:
    explicit GameLabel(const QString &text, QWidget *parent = nullptr) : QLabel(text, parent) {}
    explicit GameLabel(QWidget *parent = nullptr) : QLabel(parent) {}
};


class GameWidget : public QWidget {
    Q_OBJECT

public:
    
    explicit GameWidget(QWidget *parent = nullptr, int DropSpeed = 4, int ColorCount = 4);
    ~GameWidget();
    void setMode(int mode);void setDifficulty(int ColorCount, int DropSpeed);
    void loadGame(const QString& filePath);
    void loadGameStart(const QString& filePath);

signals:
    void exitToMode();

protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void gameUpdate();
    void togglePause();
    void resetGame(bool timerConnect = false);
    void exitGame();

private:
    std::vector<point> fall;
    std::vector<sand> fallSand;
    int paintGrid[GRID_HEIGHT][GRID_WIDTH];
    int isFall[GRID_HEIGHT][GRID_WIDTH];
    Block fallBlock; 
    bool isFalling;
    bool isPaused;
    bool continuous;
    bool downEnabled;
    bool connected;
    bool toLeft;
    bool toRight;
    bool over;
    int score;
    int multi;
    int overLine;
    int dropSpeed;
    int colorCount;
    int nextShape;
    int mode;
    int clearLines;
    float leftTime;
    float totalTime;
    Color nextColor;
    Color nextColor2;
    QTimer *gameTimer;
    QWidget *nextDraw;
    GameButton *pauseButton;
    GameButton *resetButton;
    GameButton *exitButton;
    GameButton *saveButton;
    QLabel *mlabel;
    QLabel *scoreMessageLabel;
    QLabel *scoreLabel;
    QLabel *nextLabel;
    QLabel *timeLabel;
    QLabel *lineLabel;

    void setupUI();
    void updateSand(int &change);
    void paintUi(int ui);
    void checkOver();
    void gameOver(QString str = "Game Over!");
    point addBlock(int shape, int w, Color color1, Color color2);
    void addOneBlock(int x, int y, int w, Color color, bool addFall = false);
    bool clear();
    
    void moveLeft();
    void moveRight();
    void moveDown();
    void saveGame(const QString& filePath);
    void rotateClock();
    void rotateAntiClock();
    bool checkAndClear(Color color);
    void dfs(int y, int x, int color, std::vector<std::vector<bool>> &visited, std::vector<point> &toClear, bool &find);
};

Block fromJson(const QJsonObject& obj);
#endif // SANDTRIX_H
