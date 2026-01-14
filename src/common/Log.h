/**
 * @file Log.h
 * @brief 轻量级日志系统
 * @details 提供分级日志打印功能，可通过宏开关完全禁用以节省资源。
 *          格式: [级别][时间戳][函数名] 消息
 */
#ifndef LOG_H
#define LOG_H

#include <Arduino.h>

// ==========================================
// 配置区域
// ==========================================

// 全局日志总开关 (1: 开启, 0: 关闭)
// 关闭后，所有日志宏将展开为空，不占用任何 Flash 和 CPU 时间
#define LOG_ENABLE  1

// 日志级别定义
#define LOG_LEVEL_NONE  0
#define LOG_LEVEL_ERROR 1 // 仅错误
#define LOG_LEVEL_INFO  2 // 关键流程信息
#define LOG_LEVEL_DEBUG 3 // 详细调试信息

// 当前使用的日志级别
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

// ==========================================
// 实现区域
// ==========================================

#if LOG_ENABLE

    // 串口初始化宏 (仅在启用日志时有效)
    #define LOG_INIT(baud) Serial.begin(baud)
    
    // 等待串口发送完成 (用于休眠前)
    #define LOG_FLUSH()    Serial.flush()

    // 基础打印格式: [I][1234][setup] System Started
    // ##__VA_ARGS__ 是 GCC 扩展，允许参数为空
    #define LOG_FORMAT(tag, format, ...) \
        Serial.printf("[%s][%lu][%s] " format "\n", tag, millis(), __FUNCTION__, ##__VA_ARGS__)

    // 裸打印 (不带前缀)
    #define LOG_RAW(...) Serial.printf(__VA_ARGS__)

    // ERROR 级别
    #if LOG_LEVEL >= LOG_LEVEL_ERROR
        #define LOG_E(format, ...) LOG_FORMAT("E", format, ##__VA_ARGS__)
    #else
        #define LOG_E(...)
    #endif

    // INFO 级别
    #if LOG_LEVEL >= LOG_LEVEL_INFO
        #define LOG_I(format, ...) LOG_FORMAT("I", format, ##__VA_ARGS__)
    #else
        #define LOG_I(...)
    #endif

    // DEBUG 级别
    #if LOG_LEVEL >= LOG_LEVEL_DEBUG
        #define LOG_D(format, ...) LOG_FORMAT("D", format, ##__VA_ARGS__)
    #else
        #define LOG_D(...)
    #endif

#else

    // 禁用日志时，所有宏定义为空
    #define LOG_INIT(baud)
    #define LOG_FLUSH()
    #define LOG_RAW(...)
    #define LOG_E(...)
    #define LOG_I(...)
    #define LOG_D(...)

#endif

#endif // LOG_H
