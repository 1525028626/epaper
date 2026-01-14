#include <Arduino.h>
#include "bsp/bsp_epd.h"
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== White Test ===");
    bsp_epd_init();
    bsp_epd_clear(0xFF); // White

}
void loop() {
}
    
//  g_touch_x = EPD_HEIGHT - 1 - tp.y;
//            g_touch_y = tp.x;