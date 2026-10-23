# OpenblH743VIT6 — INAV 配置

STM32H743VIT6（2MB Flash）定制飞控的 INAV target。引脚以 ArduPilot 的 `hwdef.dat` 为硬件真相。

## 硬件配置

| 外设 | 接口 | 引脚 |
|------|------|------|
| 陀螺仪/加速度计 IMU0 | SPI2 | SCK=PB13, MISO=PB14, MOSI=PB15, CS=PB12 |
| 陀螺仪/加速度计 IMU1 | SPI3 | SCK=PB3, MISO=PB4, MOSI=PD6, CS=PD7 |
| OSD (MAX7456) | SPI1 | SCK=PA5, MISO=PA6, MOSI=PA7, CS=PC4 |
| 气压计 BMP280 / SPL06 | I2C1 | SCL=PB6, SDA=PB7 |
| 磁力计 QMC5883P | I2C1 | SCL=PB6, SDA=PB7 |
| SD 卡 | SDMMC1 4-bit | 黑盒日志 |
| 12 路 PWM | — | 见下表 |

IMU 方向：`CW90_DEG_FLIP`（上机实测校准，对应 hwdef 的 `ROTATION_ROLL_180` + 90° yaw）。

## PWM 输出与 DShot

12 路 PWM 中 10 路支持 DShot，2 路因 STM32H743 硬件没有对应的 DMA 请求，只能做普通 PWM：

| 输出 | 引脚 | 定时器 | DShot |
|------|------|--------|-------|
| PWM1–4 | PE9 / PE11 / PE13 / PE14 | TIM1_CH1–4 | ✅ |
| PWM5–7 | PD12 / PD13 / PD14 | TIM4_CH1–3 | ✅ |
| PWM8 | PD15 | TIM4_CH4 | ❌ 无 DMA |
| PWM9 | PE5 | TIM15_CH1 | ✅ |
| PWM10 | PE6 | TIM15_CH2 | ❌ 无 DMA |
| PWM11–12 | PB1 / PB0 | TIM3_CH4 / TIM3_CH3 | ✅ |

> PWM8 / PWM10 只能做 PWM 输出（舵机或 PWM 模式电调），不能 DShot。
> INAV 没有 bitbang，DShot 只能走 DMA，所以这两路在 INAV 里无法 DShot。

## 双 IMU（gyro_to_use）

板上有两个 MPU6500（IMU0=SPI2，IMU1=SPI3）。INAV 的 `MAX_GYRO_COUNT=1`，**同一时刻只用一个 IMU**，不支持双 gyro 冗余。用 `gyro_to_use` 参数选择：

| 值 | 用哪个 IMU |
|----|-----------|
| `0`（默认） | 第一个（SPI2，CS=PB12） |
| `1` | 第二个（SPI3，CS=PD7） |

CLI 切换（例如切到第二个 IMU）：
```
set gyro_to_use = 1
save
```

切回第一个：`set gyro_to_use = 0` 然后 `save`。验证是否切成功：`resource` 里看 `SPI3 SCK/MISO/MOSI` 或 `SPI2 SCK/MISO/MOSI` 哪组被占用。

## SD 卡

SDMMC1 4-bit（PC8–PC12, PD2）。默认高速模式（约 30MHz）在卡槽布线不好时会初始化失败（status 显示 `SD card: Startup failed`），已在 target.h 里开启 `SDCARD_SDIO_NORMAL_SPEED` 降速到约 15MHz。若仍失败，基本是卡槽接触/上拉电阻等硬件问题。

## 编译方法

```bash
# 更新系统并安装必要工具
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install git make ruby cmake gcc-arm-none-eabi

# 克隆 iNav 源代码
git clone https://github.com/iNavFlight/inav.git
cd inav

# 把本文件夹（OpenblH743VIT6）放到 ~/inav/src/main/target/ 目录下

# 1. 创建构建目录
mkdir -p build

# 2. 运行 CMake 配置
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake

# 3. 编译目标（-j4 表示 4 个 CPU 线程编译）
cmake --build build --target OpenblH743VIT6 -j4

# 固件一般在 ~/inav/build/ 目录下
```

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6
