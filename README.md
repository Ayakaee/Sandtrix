## 项目简述
- 这是一个沙子版的俄罗斯方块，玩家可以控制形状为不同形状的沙块，其中的沙粒会根据简单的物理规则下落，像沙堆一样堆叠起来。若某些相同颜色的沙粒从左到右联通，系统会自动消除这些沙粒，此时玩家得分。
- 游戏有多个模式，如经典俄罗斯方块的“无尽模式、限时模式、40行挑战”，此外，游戏额外增加了娱乐模式：异色方块和障碍赛模式
- 这个项目灵感来源于2023年独立游戏开发者 mslivo 制作的沙子版俄罗斯方块《Sandtrix》（原名Setris），网站：https://www.sandtrix.net/
- 不同于俄罗斯方块，这个游戏**尚未发现开源项目，因此代码为原创**，没有抄袭雷同之嫌 ~~（因此项目结构安排比较幼稚）~~
## 文件说明
- src： 源代码
- resouce.qrc： Qt资源文件
- styles： Qt样式表
- images： 项目所需图片
- fonts： 字体文件
- app_icon.rc： 用于自定义编译程序的图标
- CMakeLists.txt:  cmake文件
- out/default/build： 编译目标位置
- Release： **编译好的程序，可以直接运行**
- .vscode： vscode配置文件
- CMakePreset.json： CMake配置文件
## 项目环境
- Qt版本：6.8.1
- 使用的库
    1. Qt6.8.1库：QWidget, QtGUI, QtCore……
    2. STL:queue, vector, string, utility
    3. fstream
- 编译器/生成器/编译工具链： mingw64位
- 系统： windows11
- 工具： vscode
## 碎碎念
- Cmake直接编译出的程序需要Qt的dll，因此out/default/build中的程序**不能直接运行**，需要使用Qt提供的**windeployqt命令把依赖文件打包**才能在任意Windows上使用（即这里的Release文件夹）
- 若要编译，需要修改CmakeLists.txt的编译器配置 ~~并解决可能出现的Qt奇奇怪怪的环境报错~~