/**
 * @file types.h
 * @brief 通用类型定义文件
 * @details 定义了项目全局使用的枚举、结构体、宏和错误码。
 */
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>  // 提供 uint8_t, int32_t 等
#include <stdbool.h> // 提供 bool, true, false
#include <stddef.h>  // 提供 size_t, NULL

/* ==================================================================
 * 1. 常用宏定义 (Common Macros)
 * ================================================================== */
#ifndef BIT
#define BIT(x) (1UL << (x))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef UNUSED
#define UNUSED(x) (void)(x) 
#endif

/* ==================================================================
 * 2. 系统状态码 (System Status Codes)
 * ================================================================== */
typedef enum {
    SYS_OK              = 0,    ///< 操作成功
    SYS_FAIL            = -1,   ///< 通用错误
    SYS_ERR_TIMEOUT     = -2,   ///< 超时
    SYS_ERR_BUSY        = -3,   ///< 资源忙
    SYS_ERR_INVALID_ARG = -4,   ///< 参数无效 (空指针或越界)
    SYS_ERR_NO_MEM      = -5,   ///< 内存不足
    SYS_ERR_NOT_FOUND   = -6,   ///< 资源未找到
    SYS_ERR_NOT_SUPPORT = -7    ///< 功能不支持
} sys_status_t;

/* ==================================================================
 * 3. 系统事件枚举 (System Events)
 * 用于 FreeRTOS 队列或事件组，解耦网络和 UI。
 * ================================================================== */
typedef enum {
    EVENT_NONE = 0,
    
    // 网络相关
    EVENT_WIFI_CONNECTED,
    EVENT_WIFI_DISCONNECTED,
    EVENT_NTP_SYNCED,      ///< 时间同步完成
    EVENT_WEATHER_UPDATED, ///< 天气数据更新
    
    // 电源/系统相关
    EVENT_LOW_BATTERY,
    EVENT_ENTERING_SLEEP,
    
    // 硬件相关
    EVENT_TOUCH_DETECTED
} sys_event_type_t;

/* ==================================================================
 * 4. 通用数据结构 (Common Structs)
 * ================================================================== */

/**
 * @brief 简单的 2D 坐标
 */
typedef struct {
    int16_t x;
    int16_t y;
} point_t;

/**
 * @brief 系统事件消息体
 * @details 用于 xQueueSend 发送的消息结构
 */
typedef struct {
    sys_event_type_t type; ///< 事件类型
    void *payload;         ///< 可选的数据指针 (注意: 接收方需负责释放内存，或约定使用静态内存)
    uint32_t timestamp;    ///< 事件发生的时间戳 (millis)
} sys_event_msg_t;

/* ==================================================================
 * 兼容性
 * 如果你的旧代码大量使用了 u8, u32，可以在这里兼容，
 * 但建议新代码使用 uint8_t。
 * ================================================================== */
// typedef uint8_t  u8;
// typedef uint16_t u16;
// typedef uint32_t u32;

#ifdef __cplusplus
}
#endif

#endif // COMMON_TYPES_H