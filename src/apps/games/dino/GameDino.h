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
    GameDino(AppController* sys);
    virtual ~GameDino();

    void onRun(AppController* sys) override;
    int  onLoop() override;
    void onExit() override;

private:
    AppController* app;

    enum GameState { STATE_PLAYING, STATE_PAUSED, STATE_GAMEOVER };
    GameState state;
    int pauseMenuIndex; 

    // --- 物理与数据 ---
    int dinoY;
    float dinoVy; 
    bool isJumping;
    
    unsigned long score;
    unsigned long highScore; // 最高分
    float speed;
    float maxSpeed;
    unsigned long frameCount;
    
    
    Obstacle obstacles[OBSTACLE_MAX];

    // --- 内部函数 ---
    void resetGame();
    void updatePhysics();
    void updateObstacles();
    void checkHighScore(); // 保存最高分
    
    void drawGame();
    void drawPauseMenu();
    void drawGameOver();

    // 按键回调
    void onKeyUp();
    void onKeyDown();
    void onKeySelect();
};