#pragma once
// 这是一个抽象基类 (Interface)
class AppController; // 前向声明

class AppBase {
public:
    virtual ~AppBase() {}
    // 1. 启动时 (Setup)
    virtual void onRun(AppController* sys) = 0; 
    // 2. 运行时 (Loop) - 返回 0 继续，返回 1 退出
    virtual int onLoop() = 0;   
    // 3. 退出时 (Cleanup)
    virtual void onExit() {}    
};