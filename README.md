# STM32 天气时钟 (FreeRTOS + LVGL)

基于 STM32F407 的天气时钟，通过 ESP32-C3 WiFi 模块获取心知天气数据，LVGL 驱动 2.4 寸 LCD 显示。

## 硬件平台

- MCU: STM32F407VET6
- 屏幕: 2.4" TFT (SPI, ILI9341)
- WiFi: ESP32-C3 (AT 指令)
- 温湿度: DHT11
- RTC: 内部 RTC

## 功能

- FreeRTOS 多任务: UI 渲染 / 网络请求 / 传感器采集
- LVGL 图形界面: 主页面 / 欢迎页 / WiFi 配置页 / 错误页
- 心知天气 API: 实时温度 / 天气状况 / 城市
- DHT11 室内温湿度采集
- RTC 时间显示
- UI 消息队列解耦绘制与业务逻辑

## 工程结构

```
├── app/           # 应用层: main, main_loop, ui
├── page/          # UI 页面: main_page, welcome_page, wifi_page, error_page
├── driver/        # 驱动: lcd24, esp32c3, weather, dht11, rtc, flex_key
├── mdk/           # Keil 工程
└── Middlewares/   # FreeRTOS + LVGL
```

## 编译

- Keil 5 + ARMCC 5.06
- 工程文件: `mdk/stm32f4_template.uvprojx`
