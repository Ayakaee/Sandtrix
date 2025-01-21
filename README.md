## 项目简述
- 这是一个沙子版的俄罗斯方块，玩家可以控制形状为不同形状的沙块，其中的沙粒会根据简单的物理规则下落，像沙堆一样堆叠起来。若某些相同颜色的沙粒从左到右联通，系统会自动消除这些沙粒，此时玩家得分。
- 游戏有多个模式，如经典俄罗斯方块的“无尽模式、限时模式、40行挑战”，此外，游戏额外增加了娱乐模式：异色方块和障碍赛模式
- 这个项目灵感来源于2023年独立游戏开发者 mslivo 制作的沙子版俄罗斯方块《Sandtrix》（原名Setris），网站：https://www.sandtrix.net/
## 文件说明
- src： 源代码
- resouce.qrc： Qt资源文件
- styles： Qt样式表
- images： 项目所需图片
- fonts： 字体文件
- app_icon.rc： 用于自定义编译程序的图标
- CMakeLists.txt:  cmake文件
- .vscode： vscode配置文件
- CMakePreset.json： CMake配置文件
## 项目环境
- Qt版本：6.8.1
- 编译器/编译工具链： mingw64位
