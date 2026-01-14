#include "App_Home.h"
#include "../ui/ui.h" // SquareLine 生成的 UI 代码
#include "system/SysController.h"
#include "common/Log.h" // 引入日志系统

/**
 * @file app_home.cpp
 * @brief 主页应用实现文件
 */

/**
 * @brief App 启动回调
 * @details
 * 当 PageManager 切换到此 App 时调用。
 * 负责：
 * 1. 检查并初始化 UI 资源 (ui_HomePage)。
 * 2. 执行切页动画。
 * 3. 发送初始业务指令 (如请求天气)。
 */
void App_Home::onStart() {
    LOG_I("[App] Home: Start");

    // 1. 初始化 UI (按需加载)
    // 检查 ui_HomePage 是否为空，防止重复创建导致内存泄漏
    if (!ui_HomePage) {
        ui_HomePage_screen_init();
    }
    
    // 2. 切换画面 (直接切换，无动画)
    // 墨水屏不适合淡入动画，会导致大量中间帧刷新
    lv_scr_load_anim(ui_HomePage, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);

    // 3. 业务逻辑: 请求刷新天气
    // 向 Worker 线程发送 CMD_FETCH_WEATHER 指令
    // 这样不会阻塞当前 GUI 线程
    SysController::sendToWorker(CMD_FETCH_WEATHER);
}

/**
 * @brief App 停止回调
 * @details
 * 当 PageManager 切换到其他 App 前调用。
 * 负责：
 * 1. 销毁 UI 对象，回收 RAM。
 * 2. 必须将全局 UI 指针置空 (ui_HomePage = NULL)，以便下次判断。
 */
void App_Home::onStop() {
    LOG_I("[App] Home: Stop");
    
    // 1. 销毁 UI (释放 RAM)
    if (ui_HomePage) {
        lv_obj_del(ui_HomePage);
        ui_HomePage = NULL; // 关键步骤：指针置空
    }
}

/**
 * @brief 事件处理回调
 * @param event 来自 Worker 线程的事件
 * @details
 * 处理数据更新逻辑。
 * 例如：收到 EVT_DATA_WEATHER 后，更新 UI 上的温度标签。
 */
void App_Home::onEvent(sys_event_t* event) {
    switch (event->type) {
        case EVT_DATA_WEATHER: {
            int temp = event->arg;
            LOG_I("[App] Home: Weather Update -> %d C", temp);
            
            // TODO: 更新实际的 UI 组件
            // 示例: if (ui_LabelTemp) lv_label_set_text_fmt(ui_LabelTemp, "%d°C", temp);
            break;
        }
        default: break;
    }
}
