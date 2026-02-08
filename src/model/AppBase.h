#pragma once

// -- 宏定义 ----------------------------------------------------------------------
#define DEFAULT_FRAME_RATE 30

// -- 前向声明 --------------------------------------------------------------------
class AppController; 

// -- 类定义 ----------------------------------------------------------------------
class AppBase {
public:
    virtual ~AppBase() {}                         // 虚析构函数

    // 核心生命周期
    virtual void onRun(AppController* sys) = 0;   // 应用入口
    virtual int onLoop() = 0;                     // 主循环
    virtual void onExit() {}                      // 清理工作
    
    // 属性配置
    virtual bool isKeepAlive() { return false; }  // 默认不保持屏幕常亮
    int getFrameRate() { return _frameRate; }     // 获取期望帧率

protected:
    void setFrameRate(int fps) {
        if (fps < 1) fps = 1;
        if (fps > 60) fps = 60; 
        _frameRate = fps;
    }

private:
    int _frameRate = DEFAULT_FRAME_RATE;
};