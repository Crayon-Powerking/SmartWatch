#pragma once
// 这是一个抽象基类 (Interface)
class AppController; // 前向声明

class AppBase {
public:
    virtual ~AppBase() {}                         // 虚析构函数，确保派生类正确析构
    virtual void onRun(AppController* sys) = 0;   // 入口，传入系统控制器指针
    virtual int onLoop() = 0;                     // 主循环，返回值表示是否继续运行
    virtual void onExit() {}                      // 退出前的清理工作
    virtual bool isKeepAlive() { return false; }  // 保持屏幕常亮 (默认否)
};