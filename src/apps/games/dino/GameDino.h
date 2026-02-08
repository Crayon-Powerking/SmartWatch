#pragma once

#include "model/AppBase.h"
#include "controller/AppController.h"

// -- 宏定义与常量 ----------------------------------------------------------------
#define DINO_GROUND_Y   62      // 地面 Y 坐标
#define DINO_X          5       // 恐龙逻辑 X 坐标 (注意：绘制时有偏移)
#define OBSTACLE_MAX    3       // 最大障碍物数量

// -- 数据结构 --------------------------------------------------------------------
struct Obstacle {
    float x;        // X坐标 (使用float实现平滑移动)
    int type;       // 类型: 0=小仙人掌, 1=大仙人掌
    bool active;    // 是否激活
};

enum GameState {
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER
};

// -- 类定义 ----------------------------------------------------------------------
class GameDino : public AppBase {
public:
    GameDino() {}
    virtual ~GameDino() {}

    void onRun(AppController* sys) override;
    int onLoop() override;
    void onExit() override;
    
    // 游戏需要保持屏幕常亮
    virtual bool isKeepAlive() override { return true; }

private:
    AppController* sys = nullptr;
    
    // 状态变量
    GameState state = STATE_PLAYING;
    bool isExiting = false;
    int pauseMenuIndex = 0;

    // 物理与分数
    int dinoY = 0;
    float dinoVy = 0.0f;
    bool isJumping = false;
    
    unsigned long score = 0;
    unsigned long highScore = 0;
    float speed = 0.0f;
    float maxSpeed = 0.0f;
    unsigned long frameCount = 0;
    
    Obstacle obstacles[OBSTACLE_MAX];

    // 内部逻辑
    void resetGame();
    void updatePhysics();
    void updateObstacles();
    void checkHighScore();

    // 按键处理
    void onKeyUp();
    void onKeyDown();
    void onKeySelect();

    // 渲染方法
    void render();
    void drawGame();
    void drawPauseMenu();
    void drawGameOver();
};