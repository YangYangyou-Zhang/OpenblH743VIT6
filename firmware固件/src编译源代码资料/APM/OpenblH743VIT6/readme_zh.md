# OpenblH743VIT6 飞控

基于 STM32H743VIT6 的 ArduPilot 飞控硬件定义。
固件版本：**ArduPilot Copter-4.7.1 / Plane-4.7.1**

## 硬件配置

| 特性 | 详情 |
|---------|--------|
| MCU | STM32H743VIT6 |
| IMU0 | MPU6500 @ SPI2, ROTATION_ROLL_180 |
| IMU1 | MPU6500 @ SPI3, ROTATION_ROLL_180 |
| 气压计 | BMP280 / SPL06 @ I2C1 (0x76, 0x77) |
| OSD | AT7456E @ SPI1 |
| 存储 | microSD @ SDMMC1 |
| 串口 0 | USB OTG1 |
| 串口 1 | USART1 (GPS0) |
| 串口 2 | USART2 (MSP DisplayPort) |
| 串口 3 | USART3 (GPS1) |
| 串口 4 | UART4 (MAVLink2, 460 baud) |
| 串口 5 | USART6 (RCIN) |
| 串口 6 | UART7 (ESC 遥测) |
| 串口 7 | UART8 (MAVLink2, 460 baud) |
| CAN | CAN1 @ PB8/PB9 |
| PWM 输出 | 11（TIM1 CH1-4, TIM4 CH1-4, TIM15 CH1-2, TIM3 CH4） |
| RGB LED | PE2(R) / PE3(G) / PE4(B) |
| 蜂鸣器（无源） | PB0 TIM3_CH3 ALARM (ToneAlarm) |
| 安全开关 | PB2（默认禁用，按下锁定） |
| 罗盘 | I2C2 (I2C:0)，自动探测（IST8310 等） |

## 电池 ADC

- 分压电阻：51k / 1k（scale = 52.0）
- 电压检测：PC0 (ADC1_IN10, pin 10)
- 电流检测：PC1 (ADC1_IN11, pin 11)

### ADC 偏移校准

STM32H743 ADC 在未接电池时有约 1.78V 的偏移。由于这个非线性偏移，需要从读数中减去一个固定值。

由于 ArduPilot 的 hwdef 层不提供电压偏移参数（只有乘法 scale），可以：

- 烧录后用 Mission Planner 的电池校准页面校准
- 或者若需要精确读数又不想用地面站校准，则在模拟量源码中添加偏移

预期校准：`BATT_VOLT_OFFSET = 1.78`

## 编译要求

- GCC 10.2.1（`gcc-arm-none-eabi-10-2020-q4-major`）
- Python 3.x 及所需包
- GCC 13.x 生成的 STM32H7 代码无法启动——请勿使用

## 编译命令

```bash
# 设置 GCC 10 路径
export PATH=/path/to/gcc-arm-none-eabi-10-2020-q4-major/bin:$PATH

# 编译 bootloader
./waf distclean
./waf configure --board OpenblH743VIT6 --bootloader
./waf clean
./waf bootloader

# 编译 ArduCopter
./waf configure --board OpenblH743VIT6
./waf copter

# 编译 ArduPlane
./waf plane
```

## 输出文件

| 文件 | 用途 |
|------|-----|
| `AP_Bootloader.hex` | ST-LINK 用的 bootloader |
| `arducopter_with_bl.hex` | ArduCopter + bootloader（ST-LINK） |
| `arducopter.apj` | ArduCopter（Mission Planner 更新） |
| `arduplane_with_bl.hex` | ArduPlane + bootloader（ST-LINK） |
| `arduplane.apj` | ArduPlane（Mission Planner 更新） |

## 烧录

### 首次（ST-LINK）
使用 STM32CubeProgrammer 烧录 `arducopter_with_bl.hex` 或 `arduplane_with_bl.hex`。

### 通过地面站更新
通过 Mission Planner 固件更新页面上传 `arducopter.apj` 或 `arduplane.apj`。

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6
