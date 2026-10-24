# OpenblH743VIT6 — Betaflight 配置

STM32H743VIT6（2MB Flash）定制飞控的 Betaflight unified target 配置。引脚以 ArduPilot 的 `hwdef.dat` 为硬件真相。

## 硬件配置

| 外设 | 接口 | 引脚 |
|------|------|------|
| 陀螺仪/加速度计 IMU0 | SPI2 | SCK=PB13, MISO=PB14, MOSI=PB15, CS=PB12 |
| 陀螺仪/加速度计 IMU1 | SPI3 | SCK=PB3, MISO=PB4, MOSI=PD6, CS=PD7 |
| OSD (MAX7456) | SPI1 | SCK=PA5, MISO=PA6, MOSI=PA7, CS=PC4 |
| 气压计 SPL06 | I2C1 | SCL=PB6, SDA=PB7，0x77 —— 已通过本地驱动补丁支持，见下 |
| 磁力计 QMC5883P | I2C1 | SCL=PB6, SDA=PB7，0x2C —— 已通过本地驱动补丁支持，见下 |
| SD 卡 | SDMMC1 4-bit | ⚠️ 暂不支持（初始化卡住，已禁用） |
| 12 路 PWM | — | 见下表 |

IMU 方向：`CW90_DEG_FLIP`（上机实测校准）。

> **Flash 布局注意**：Betaflight 的 H743 linker 把中断向量放在 `0x08000000`、BF 配置区放在
> `0x08020000`、固件放在 `0x08040000`。这会**覆盖 PX4/ArduPilot 使用的 APM bootloader**。
> 刷 BF 时建议全片擦除；要切回 PX4/ArduPilot，需要先重新烧回 APM bootloader。

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

> **SD 卡暂不支持；SPL06 气压计和 QMC5883P 磁力计已通过 `../patches/` 的本地补丁支持。**

1. **SD 卡（SDMMC1）暂不支持**：`USE_SDCARD_SDIO` 开启后，SD 卡初始化会卡住 `init()` 流程，挡住后面的 USB 初始化，导致「烧录后 USB 完全不识别」。当前已在 config.h 里**注释掉 `USE_SDCARD` / `USE_SDCARD_SDIO`** 绕过。后续版本修复 SDMMC 初始化死等（类似 INAV 的 "SD card Startup failed"，但 BF 这里没加超时）后再启用。

## SPL06 气压计（本地补丁）

Betaflight 4.5.3 没有 SPL06 驱动。`../patches/` 里是从 INAV 移植的补丁，自动探测 **0x76 / 0x77**；板载芯片在 0x77。

编译前先应用：

```bash
cp -r patches/drivers/barometer/* ../betaflight/src/main/drivers/barometer/
cp -r patches/sensors/*           ../betaflight/src/main/sensors/
cp    patches/cli/settings.c      ../betaflight/src/main/cli/
```

`config.h` 里已启用 `USE_BARO` 和 `USE_BARO_SPL06`。烧录后 `status` 应显示 Barometer；
如果 AUTO 没识别，执行 `set baro_hardware = SPL06` + `save`。

## QMC5883P 磁力计（本地补丁）

Betaflight 4.5.3 只有 QMC5883L 驱动（地址 0x0D），板载 QMC5883P 是 0x2C。
`../patches/drivers/compass/` 里是**从 INAV 移植**的 QMC5883P 驱动。

`config.h` 已启用 `USE_MAG` 和 `USE_MAG_QMC5883P`，并设置
`MAG_QMC5883P_ALIGN = CW270_DEG_FLIP`（对应 PX4 的 `-R 10`，ROLL_180_YAW_90）。
烧录后 `status` 应显示 Compass；如果 AUTO 没识别，执行
`set mag_hardware = QMC5883P` + `save`，然后做罗盘校准。
**需要上机确认航向方向**——如果方向不对，调整 `align_mag` 后重新校准。

补丁只适用于 Betaflight 4.5.3，升级 BF 后需要重新应用。

## 编译方法

已用 Betaflight 4.5.3 + GCC 10.2.1 实测（官方 4.5.3 工具链是 10.3.1）。

```bash
git clone https://github.com/betaflight/betaflight.git   # 或 gitcode 镜像
cd betaflight
git checkout 4.5.3

# 先应用 SPL06 补丁（本仓库 Betafight 目录下）
cp -r <repo>/firmware固件/src编译源代码资料/Betafight/patches/drivers/barometer/* src/main/drivers/barometer/
cp -r <repo>/firmware固件/src编译源代码资料/Betafight/patches/drivers/compass/*   src/main/drivers/compass/
cp -r <repo>/firmware固件/src编译源代码资料/Betafight/patches/sensors/*           src/main/sensors/
cp    <repo>/firmware固件/src编译源代码资料/Betafight/patches/cli/settings.c      src/main/cli/

# 把本文件夹（OpenblH743VIT6）放到 src/config/configs/ 目录下
# 4.5.3 不要执行 `make configs`（会去 clone GitHub 的 betaflight/config 仓库）

make OpenblH743VIT6 ARM_SDK_DIR=$HOME/gcc-arm-none-eabi-10-2020-q4-major \
  GCC_REQUIRED_VERSION=10.2.1 -j32
# 固件：obj/betaflight_4.5.3_STM32H743_OpenblH743VIT6.hex
```

烧录后 CLI 执行一次 `defaults` + `save`（否则 IMU 方向可能仍读旧配置值）。

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6
