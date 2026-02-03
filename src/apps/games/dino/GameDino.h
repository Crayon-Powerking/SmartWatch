#pragma once
#include "model/AppBase.h"
#include "controller/AppController.h"

// 游戏配置
#define DINO_GROUND_Y 62   
#define DINO_X 5           
#define OBSTACLE_MAX 3     

struct Obstacle {
    float x;     // 用 float 实现平滑移动
    int type;    // 0=小仙人掌, 1=大仙人掌
    bool active;
};

class GameDino : public AppBase {
public:
    GameDino() {}
    virtual ~GameDino() {}

    void onRun(AppController* sys) override;
    int  onLoop() override;
    void onExit() override;
    virtual bool isKeepAlive() override { return true; }

private:
    AppController* sys = nullptr;

    enum GameState { STATE_PLAYING, STATE_PAUSED, STATE_GAMEOVER };
    GameState state;

    bool isExiting = false;  // 退出标志位
    int pauseMenuIndex;      // 暂停菜单选项索引

    // --- 物理与数据 ---
    int dinoY;               // 恐龙垂直位置
    float dinoVy;            // 垂直速度
    bool isJumping;          // 是否在跳跃中
    
    unsigned long score;     // 当前分数
    unsigned long highScore; // 最高分
    float speed;             // 当前速度
    float maxSpeed;          // 最高速度
    unsigned long frameCount;// 帧计数器，用于计分
    
    
    Obstacle obstacles[OBSTACLE_MAX]; // 障碍物数组

    // --- 内部函数 ---
    void resetGame();      // 重置游戏状态
    void updatePhysics();  // 更新物理状态
    void updateObstacles();// 更新障碍物位置
    void checkHighScore(); // 保存最高分
    
    void drawGame();       // 绘制游戏画面
    void drawPauseMenu();  // 绘制暂停菜单
    void drawGameOver();   // 绘制游戏结束画面

    // 按键回调
    void onKeyUp();
    void onKeyDown();
    void onKeySelect();
};