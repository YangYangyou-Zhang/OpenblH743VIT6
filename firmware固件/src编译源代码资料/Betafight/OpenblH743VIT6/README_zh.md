# OpenblH743VIT6 — Betaflight 配置

STM32H743VIT6（2MB Flash）定制飞控的 Betaflight unified target 配置。引脚以 ArduPilot 的 `hwdef.dat` 为硬件真相。

## 硬件配置

| 外设 | 接口 | 引脚 |
|------|------|------|
| 陀螺仪/加速度计 IMU0 | SPI2 | SCK=PB13, MISO=PB14, MOSI=PB15, CS=PB12 |
| 陀螺仪/加速度计 IMU1 | SPI3 | SCK=PB3, MISO=PB4, MOSI=PD6, CS=PD7 |
| OSD (MAX7456) | SPI1 | SCK=PA5, MISO=PA6, MOSI=PA7, CS=PC4 |
| 气压计 SPL06 | I2C1 | SCL=PB6, SDA=PB7（⚠️ 暂不支持，见下） |
| 磁力计 QMC5883P | I2C1 | SCL=PB6, SDA=PB7（⚠️ 暂不支持，见下） |
| SD 卡 | SDMMC1 4-bit | ⚠️ 暂不支持（初始化卡住，已禁用） |
| 12 路 PWM | — | 见下表 |

IMU 方向：`CW90_DEG_FLIP`（上机实测校准）。

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

## DShot 设置

- **单向 DShot**：直接选协议即可，走普通 DMA，无需额外设置。
- **双向 DShot（回传 telemetry）**：在 CLI 执行 `set dshot_bidir = ON` 然后 `save`。
  - 注意：开启双向会触发 Betaflight 的 bitbang 模式（`dshot_bitbang` 默认 `AUTO`），
    此时所有电机全局切到 bitbang，CPU 开销大，12 路全开不推荐。
  - 强制单向也走 bitbang：`set dshot_bitbang = ON`（同样全局切换，一般不需要）。

## 已知问题 / 未完成项

> **气压计（SPL06）、磁力计（QMC5883P）和 SD 卡暂不支持，等待后续 Betaflight 版本更新再支持。**

1. **SD 卡（SDMMC1）暂不支持**：`USE_SDCARD_SDIO` 开启后，SD 卡初始化会卡住 `init()` 流程，挡住后面的 USB 初始化，导致「烧录后 USB 完全不识别」。当前已在 config.h 里**注释掉 `USE_SDCARD` / `USE_SDCARD_SDIO`** 绕过。后续版本修复 SDMMC 初始化死等（类似 INAV 的 "SD card Startup failed"，但 BF 这里没加超时）后再启用。

2. **气压计 SPL06 暂不支持**：Betaflight 4.5.3 没有 SPL06 驱动（只有 BMP085/BMP280/BMP388），板载 SPL06 无法识别。等待后续版本加入 SPL06 驱动（或从 INAV 移植 `barometer_spl06`）后再启用。

3. **磁力计 QMC5883P 暂不支持**：BF 只有 `compass_qmc5883l.c`（QMC5883L 地址 0x0D），板载 QMC5883P 地址 0x2C。已尝试复用（给 `compass_qmc5883l.c` 的 `QMC5883L_MAG_I2C_ADDRESS` 加 `#ifndef` 保护 + config.h 里定义 0x2C），暂不启用，等待后续版本。启用时取消 config.h 里 `USE_MAG` / `USE_MAG_QMC5883` 的注释即可。

## 编译方法

```bash
# 更新软件包列表并升级系统
sudo apt update && sudo apt upgrade

# 安装基础编译工具（git、make、clang、python 等）
sudo apt -y install build-essential git curl clang-18 python3 python-is-python3

# 克隆 Betaflight 源代码并切换到稳定版本（例如 4.5.3）
git clone https://github.com/betaflight/betaflight.git
cd betaflight
git checkout 4.5.3

# 自动下载并安装所需工具链
make arm_sdk_install

# 首次编译或切换分支后，生成所有目标板的默认配置
make configs

# 把本文件夹（OpenblH743VIT6）放到 ~/betaflight/src/config/configs/ 目录下，然后编译
make config OpenblH743VIT6

# 固件在 ~/betaflight/obj/ 下：betaflight_4.5.3_STM32H743_OpenblH743VIT6.hex
```

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6
