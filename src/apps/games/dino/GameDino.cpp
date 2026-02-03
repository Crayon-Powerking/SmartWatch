#include "GameDino.h"
#include "assets/AppIcons.h" 
#include "assets/Lang.h"

// ==========================================
// 1. 生命周期
// ==========================================

void GameDino::onRun(AppController* sys) {
    this->sys = sys;
    this->isExiting = false;
    this->highScore = AppData.gameRecords.dinoHighScore; 
    resetGame();

    sys->btnUp.attachClick(std::bind(&GameDino::onKeyUp, this));
    sys->btnDown.attachClick(std::bind(&GameDino::onKeyDown, this));
    sys->btnSelect.attachClick(std::bind(&GameDino::onKeySelect, this));
    
    sys->btnSelect.attachLongPress(nullptr);
    sys->btnUp.attachDuringLongPress(nullptr);
    sys->btnDown.attachDuringLongPress(nullptr);
}

void GameDino::onExit() {
}

void GameDino::resetGame() {
    state = STATE_PLAYING;
    dinoY = DINO_GROUND_Y - DINO_H; 
    dinoVy = 0;
    isJumping = false;
    score = 0;
    
    // 速度微调
    speed = 0.6; 
    maxSpeed = 2.0;
    
    frameCount = 0;
    pauseMenuIndex = 0; 

    for(int i=0; i<OBSTACLE_MAX; i++) {
        obstacles[i].active = false;
        obstacles[i].x = 140 + i * 100; 
        obstacles[i].type = 0; 
    }
}

int GameDino::onLoop() {
    sys->display.clear(); 

    if (state == STATE_PLAYING) {
        updatePhysics();
        updateObstacles();
        drawGame();
        
        score++;
        if (score > highScore) highScore = score;

        // 渐进加速
        if (score % 400 == 0 && score < 1200 && speed < maxSpeed) {
            speed += 0.08f;
        }
    } 
    else if (state == STATE_PAUSED) {
        drawGame();      
        drawPauseMenu(); 
    }
    else if (state == STATE_GAMEOVER) {
        drawGame();
        drawGameOver();  
    }

    sys->display.update(); 
    return 0; 
}

void GameDino::checkHighScore() {
    if (score > AppData.gameRecords.dinoHighScore) {
        AppData.gameRecords.dinoHighScore = score;
        sys->storage.save(); 
    }
}

// ==========================================
// 2. 按键逻辑 (UP=跳, DOWN=暂停, SELECT=确认)
// ==========================================

void GameDino::onKeyUp() {
    if (state == STATE_PLAYING) {
        if (!isJumping) {
            // 起跳力度 -6.2
            dinoVy = -8.0f; 
            isJumping = true;
        }
    } 
    else if (state == STATE_PAUSED) {
        pauseMenuIndex = 0; 
    }
}

void GameDino::onKeyDown() {
    if (state == STATE_PLAYING) {
        state = STATE_PAUSED;
        pauseMenuIndex = 0; 
    } 
    else if (state == STATE_PAUSED) {
        pauseMenuIndex = 1; 
    }
}

void GameDino::onKeySelect() {
    if (state == STATE_PLAYING) return; 

    if (state == STATE_PAUSED) {
        if (pauseMenuIndex == 0) state = STATE_PLAYING; 
        else sys->quitApp(); 
    } 
    else if (state == STATE_GAMEOVER) {
        resetGame(); 
    }
}

// ==========================================
// 3. 物理与逻辑
// ==========================================

void GameDino::updatePhysics() {
    dinoY += (int)dinoVy;
    dinoVy += 0.16f; 

    if (dinoY >= DINO_GROUND_Y - DINO_H) {
        dinoY = DINO_GROUND_Y - DINO_H;
        dinoVy = 0;
        isJumping = false;
    }
}

void GameDino::updateObstacles() {
    frameCount++;
    for(int i=0; i<OBSTACLE_MAX; i++) {
        if (obstacles[i].active) {
            obstacles[i].x -= speed;

            // 碰撞检测
            int dBoxX = DINO_X + 6; 
            int dBoxY = dinoY + 6; 
            int dBoxW = DINO_W - 14; 
            int dBoxH = DINO_H - 10;  

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

            if (dBoxX < obsBoxX + obsBoxW &&
                dBoxX + dBoxW > obsBoxX &&
                dBoxY < obsBoxY + obsBoxH &&
                dBoxY + dBoxH > obsBoxY) {
                
                state = STATE_GAMEOVER;
                checkHighScore(); 
            }

            if (obstacles[i].x < -20) obstacles[i].active = false;
        } else {
            if (rand() % 100 < 2) { 
                bool canSpawn = true;
                for(int j=0; j<OBSTACLE_MAX; j++) {
                    if (obstacles[j].active && obstacles[j].x > 70) canSpawn = false;
                }
                
                if (canSpawn) {
                    obstacles[i].active = true;
                    obstacles[i].x = 130 + rand() % 80;
                    obstacles[i].type = (rand() % 100 < 30) ? 1 : 0; 
                }
            }
        }
    }
}

// ==========================================
// 4. 渲染绘制
// ==========================================

void GameDino::drawGame() {
    sys->display.setDrawColor(1);
    sys->display.drawLine(0, DINO_GROUND_Y, 128, DINO_GROUND_Y);

    const unsigned char* dinoBmp = icon_dino_jump;
    if (!isJumping) {
        dinoBmp = ((frameCount / 8) % 2 == 0) ? icon_dino_run1 : icon_dino_run2;
    }
    sys->display.drawIcon(DINO_X, dinoY, DINO_W, DINO_H, dinoBmp);

    for(int i=0; i<OBSTACLE_MAX; i++) {
        if (obstacles[i].active) {
            if (obstacles[i].type == 0) {
                sys->display.drawIcon((int)obstacles[i].x, DINO_GROUND_Y - CACTUS_SMALL_H, CACTUS_SMALL_W, CACTUS_SMALL_H, icon_cactus_small);
            } else {
                sys->display.drawIcon((int)obstacles[i].x, DINO_GROUND_Y - CACTUS_BIG_H, CACTUS_BIG_W, CACTUS_BIG_H, icon_cactus_big);
            }
        }
    }

    // --- HUD (抬头显示) ---
    // 使用小字体显示数字，不切换语言
    sys->display.setFont(u8g2_font_tom_thumb_4x6_t_all);
    
    char buf[32];
    
    // 显示最高分 "HI 12345" (左上角)
    sprintf(buf, "HI %ld", highScore); 
    sys->display.drawText(2, 6, buf); 

    // 显示当前分 "00012" (右上角)
    sprintf(buf, "%05ld", score);
    sys->display.drawText(100, 6, buf); 
}

void GameDino::drawPauseMenu() {
    int lang = AppData.systemConfig.languageIndex;
    
    // 弹窗菜单使用中文字体
    sys->display.setFont(u8g2_font_wqy12_t_gb2312);

    int boxW = 86; 
    int boxH = 46; 
    int boxX = (128 - boxW) / 2;
    int boxY = (64 - boxH) / 2;
    
    sys->display.setDrawColor(0); sys->display.drawRBox(boxX, boxY, boxW, boxH, 4);
    sys->display.setDrawColor(1); sys->display.drawRBox(boxX, boxY, boxW, boxH, 4); 
    sys->display.setDrawColor(0); sys->display.drawRBox(boxX+1, boxY+1, boxW-2, boxH-2, 4); 
    sys->display.setDrawColor(1); 

    // 【使用 Lang.h 中的字符串】
    if (pauseMenuIndex == 0) sys->display.drawText(boxX + 10, boxY + 18, ">");
    sys->display.drawText(boxX + 20, boxY + 18, STR_DINO_CONTINUE[lang]);

    if (pauseMenuIndex == 1) sys->display.drawText(boxX + 10, boxY + 36, ">");
    sys->display.drawText(boxX + 20, boxY + 36, STR_DINO_QUIT[lang]);
}

void GameDino::drawGameOver() {
    int lang = AppData.systemConfig.languageIndex;
    sys->display.setFont(u8g2_font_wqy12_t_gb2312);

    // 【使用 Lang.h 中的字符串】
    const char* title = STR_DINO_GAMEOVER[lang];

    int textW = sys->display.getStrWidth(title);
    int boxW = textW + 24; 
    int boxH = 26;         
    int boxX = (128 - boxW) / 2;
    int boxY = (64 - boxH) / 2 - 4; 
    
    sys->display.setDrawColor(0); sys->display.drawRBox(boxX, boxY, boxW, boxH, 4); 
    sys->display.setDrawColor(1); sys->display.drawRBox(boxX, boxY, boxW, boxH, 4); 
    sys->display.setDrawColor(0); sys->display.drawRBox(boxX+1, boxY+1, boxW-2, boxH-2, 4); 
    
    sys->display.setDrawColor(1);
    sys->display.drawText(boxX + 12, boxY + 18, title);

    // 下方分数直接用英文 "Score:" 即可，简洁通用
    char buf[32];
    snprintf(buf, 32, "Score: %ld", score);
    
    int scoreW = sys->display.getStrWidth(buf);
    sys->display.drawText((128 - scoreW) / 2, boxY + boxH + 14, buf);
}