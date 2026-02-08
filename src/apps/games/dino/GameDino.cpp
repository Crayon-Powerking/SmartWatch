#include "apps/games/dino/GameDino.h"
#include "assets/AppIcons.h"
#include "assets/Lang.h"
#include <cstdio>
#include <cmath>

// -- 辅助逻辑 --------------------------------------------------------------------

void GameDino::resetGame() {
    state = STATE_PLAYING;
    // 恐龙初始高度 (地面 - 恐龙判定高度近似值)
    dinoY = DINO_GROUND_Y - 32; // Assuming DINO_H is 32 based on icons
    dinoVy = 0;
    isJumping = false;
    score = 0;

    // 速度参数初始化
    speed = 0.6f;
    maxSpeed = 2.0f;

    frameCount = 0;
    pauseMenuIndex = 0;

    // 重置障碍物池
    for (int i = 0; i < OBSTACLE_MAX; i++) {
        obstacles[i].active = false;
        obstacles[i].x = 140 + i * 100;
        obstacles[i].type = 0;
    }
}

void GameDino::updatePhysics() {
    dinoY += (int)dinoVy;
    dinoVy += 0.16f; // 重力加速度

    // 地面碰撞检测
    // 注意：这里的 32 是恐龙的大致高度
    if (dinoY >= DINO_GROUND_Y - 32) {
        dinoY = DINO_GROUND_Y - 32;
        dinoVy = 0;
        isJumping = false;
    }
}

void GameDino::updateObstacles() {
    frameCount++;
    for (int i = 0; i < OBSTACLE_MAX; i++) {
        if (obstacles[i].active) {
            obstacles[i].x -= speed;

            // --- 碰撞检测逻辑 ---
            // 使用原始代码中的判定参数
            int dBoxX = DINO_X + 6; 
            int dBoxY = dinoY + 6; 
            int dBoxW = 32 - 14; // DINO_W assumed 32
            int dBoxH = 32 - 10; // DINO_H assumed 32

            int obsBoxX = (int)obstacles[i].x + 2;
            int obsBoxW, obsBoxH, obsBoxY;

            if (obstacles[i].type == 0) {
                obsBoxW = 4; 
                obsBoxH = CACTUS_SMALL_H - 2;
                obsBoxY = DINO_GROUND_Y - CACTUS_SMALL_H + 2;
            } else {
                obsBoxW = 8;
                obsBoxH = CACTUS_BIG_H - 4;
                obsBoxY = DINO_GROUND_Y - CACTUS_BIG_H + 4;
            }

            // AABB 碰撞检测
            if (dBoxX < obsBoxX + obsBoxW &&
                dBoxX + dBoxW > obsBoxX &&
                dBoxY < obsBoxY + obsBoxH &&
                dBoxY + dBoxH > obsBoxY) {
                
                state = STATE_GAMEOVER;
                checkHighScore(); 
            }

            // 移除屏幕外的障碍物
            if (obstacles[i].x < -20) obstacles[i].active = false;

        } else {
            // --- 障碍物生成逻辑 ---
            if (rand() % 100 < 2) {
                bool canSpawn = true;
                // 确保与前一个障碍物保持足够距离
                for (int j = 0; j < OBSTACLE_MAX; j++) {
                    if (obstacles[j].active && obstacles[j].x > 70) canSpawn = false;
                }

                if (canSpawn) {
                    obstacles[i].active = true;
                    obstacles[i].x = 130 + rand() % 80;
                    obstacles[i].type = (rand() % 100 < 30) ? 1 : 0; // 30%概率大仙人掌
                }
            }
        }
    }
}

void GameDino::checkHighScore() {
    if (score > AppData.gameRecords.dinoHighScore) {
        AppData.gameRecords.dinoHighScore = score;
        sys->storage.save();
    }
}

// -- 按键处理 --------------------------------------------------------------------

void GameDino::onKeyUp() {
    if (state == STATE_PLAYING) {
        if (!isJumping) {
            dinoVy = -8.0f; // 起跳力度
            isJumping = true;
        }
    } else if (state == STATE_PAUSED) {
        pauseMenuIndex = 0;
    }
}

void GameDino::onKeyDown() {
    if (state == STATE_PLAYING) {
        state = STATE_PAUSED;
        pauseMenuIndex = 0;
    } else if (state == STATE_PAUSED) {
        pauseMenuIndex = 1;
    }
}

void GameDino::onKeySelect() {
    if (state == STATE_PLAYING) return;

    if (state == STATE_PAUSED) {
        if (pauseMenuIndex == 0) {
            state = STATE_PLAYING;
        } else {
            sys->quitApp();
        }
    } else if (state == STATE_GAMEOVER) {
        resetGame();
    }
}

// -- 生命周期 --------------------------------------------------------------------

void GameDino::onRun(AppController* sys) {
    this->sys = sys;
    this->isExiting = false;
    this->highScore = AppData.gameRecords.dinoHighScore;

    resetGame();

    // 使用 Lambda 表达式绑定按键，并调用 processInput 防止屏幕休眠
    sys->btnUp.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onKeyUp();
    });

    sys->btnDown.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onKeyDown();
    });

    sys->btnSelect.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onKeySelect();
    });

    // 清除冲突的长按事件
    sys->btnSelect.attachLongPress(nullptr);
    sys->btnUp.attachDuringLongPress(nullptr);
    sys->btnDown.attachDuringLongPress(nullptr);
}

int GameDino::onLoop() {
    if (this->isExiting) return 1;

    // 游戏循环逻辑
    if (state == STATE_PLAYING) {
        updatePhysics();
        updateObstacles();

        score++;
        if (score > highScore) highScore = score;

        // 难度曲线：渐进加速
        if (score % 400 == 0 && score < 1200 && speed < maxSpeed) {
            speed += 0.08f;
        }
    }

    render();
    return 0;
}

void GameDino::onExit() {
    // 退出时无需特殊处理
}

// -- 绘图渲染 --------------------------------------------------------------------

void GameDino::render() {
    sys->display.clear();

    if (state == STATE_PLAYING) {
        drawGame();
    } else if (state == STATE_PAUSED) {
        drawGame();
        drawPauseMenu();
    } else if (state == STATE_GAMEOVER) {
        drawGame();
        drawGameOver();
    }
}

void GameDino::drawGame() {
    sys->display.setDrawColor(1);
    sys->display.drawLine(0, DINO_GROUND_Y, 128, DINO_GROUND_Y);

    // 恐龙动画
    const unsigned char* dinoBmp = icon_dino_jump;
    if (!isJumping) {
        dinoBmp = ((frameCount / 8) % 2 == 0) ? icon_dino_run1 : icon_dino_run2;
    }
    
    // 开启透明模式
    sys->display.setFontMode(1); 
    
    sys->display.drawIcon(DINO_X, dinoY, 32, 32, dinoBmp); 

    // 绘制障碍物
    for (int i = 0; i < OBSTACLE_MAX; i++) {
        if (obstacles[i].active) {
            if (obstacles[i].type == 0) {
                sys->display.drawIcon((int)obstacles[i].x, DINO_GROUND_Y - CACTUS_SMALL_H, CACTUS_SMALL_W, CACTUS_SMALL_H, icon_cactus_small);
            } else {
                sys->display.drawIcon((int)obstacles[i].x, DINO_GROUND_Y - CACTUS_BIG_H, CACTUS_BIG_W, CACTUS_BIG_H, icon_cactus_big);
            }
        }
    }
    
    sys->display.setFontMode(0); // 恢复默认模式

    // HUD 信息
    sys->display.setFont(u8g2_font_tom_thumb_4x6_t_all);

    char buf[32];
    sprintf(buf, "HI %ld", highScore);
    sys->display.drawText(2, 6, buf);

    sprintf(buf, "%05ld", score);
    sys->display.drawText(100, 6, buf);
}

void GameDino::drawPauseMenu() {
    int lang = AppData.systemConfig.languageIndex;
    sys->display.setFont(u8g2_font_wqy12_t_gb2312);

    int boxW = 86;
    int boxH = 46;
    int boxX = (128 - boxW) / 2;
    int boxY = (64 - boxH) / 2;

    // 绘制圆角弹窗 (白底黑框)
    sys->display.setDrawColor(0);
    sys->display.drawRBox(boxX, boxY, boxW, boxH, 4);
    sys->display.setDrawColor(1);
    sys->display.drawRBox(boxX, boxY, boxW, boxH, 4);
    sys->display.setDrawColor(0);
    sys->display.drawRBox(boxX + 1, boxY + 1, boxW - 2, boxH - 2, 4);
    sys->display.setDrawColor(1);

    if (pauseMenuIndex == 0) sys->display.drawText(boxX + 10, boxY + 18, ">");
    sys->display.drawText(boxX + 20, boxY + 18, STR_DINO_CONTINUE[lang]);

    if (pauseMenuIndex == 1) sys->display.drawText(boxX + 10, boxY + 36, ">");
    sys->display.drawText(boxX + 20, boxY + 36, STR_DINO_QUIT[lang]);
}

void GameDino::drawGameOver() {
    int lang = AppData.systemConfig.languageIndex;
    sys->display.setFont(u8g2_font_wqy12_t_gb2312);

    const char* title = STR_DINO_GAMEOVER[lang];

    int textW = sys->display.getStrWidth(title);
    int boxW = textW + 24;
    int boxH = 26;
    int boxX = (128 - boxW) / 2;
    int boxY = (64 - boxH) / 2 - 4;

    sys->display.setDrawColor(0);
    sys->display.drawRBox(boxX, boxY, boxW, boxH, 4);
    sys->display.setDrawColor(1);
    sys->display.drawRBox(boxX, boxY, boxW, boxH, 4);
    sys->display.setDrawColor(0);
    sys->display.drawRBox(boxX + 1, boxY + 1, boxW - 2, boxH - 2, 4);

    sys->display.setDrawColor(1);
    sys->display.drawText(boxX + 12, boxY + 18, title);

    char buf[32];
    snprintf(buf, 32, "Score: %ld", score);

    int scoreW = sys->display.getStrWidth(buf);
    sys->display.drawText((128 - scoreW) / 2, boxY + boxH + 14, buf);
}