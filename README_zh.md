# OpenblH743VIT6 — STM32H743 飞控

基于 **STM32H743VIT6** 的定制飞控（Board ID 18848），支持 PX4、ArduPilot、Betaflight 和 INAV。
仓库包含硬件设计（原理图、PCB、BOM、Gerber）、结构模型、参数配置和预编译固件。

[English](README.md)

## 特性

- STM32H743VIT6，**Board ID 18848**，与 ArduPilot 共用 **APM bootloader**
- 板载两路 MPU6500；三个 IMU 片选位置（PB12 / PD7 / PD5）都兼容 **ICM-42605 / ICM-42688-P**
- SPL06 气压计、QMC5883P 罗盘、AT7456E OSD、microSD、CAN/UAVCAN、USB
- DShot + 电调遥测，可选 INA238 电流计
- PX4 固件带固定翼/VTOL 支持、状态 LED 和完整串口映射
- 提供 PX4、ArduPilot（Copter/Plane）、Betaflight、INAV 预编译固件

## 硬件

| 项目 | 说明 |
|---|---|
| MCU | STM32H743VIT6（Cortex-M7），Board ID 18848 |
| IMU0 / IMU1 | 2× MPU6500（SPI2 CS PB12 / SPI3 CS PD7）；两个位置也兼容 ICM-42605 / ICM-42688-P |
| 可选 IMU2 | SPI3 CS PD5 —— MPU6500 或 ICM-42605 / ICM-42688-P |
| 气压计 | SPL06（I2C，0x76/0x77 自动扫描） |
| 罗盘 | QMC5883P（I2C1，0x2C），支持外接罗盘 |
| OSD | AT7456E（SPI1，CS PC4） |
| 存储 | microSD（SDMMC1） |
| CAN | UAVCAN / DroneCAN，PB8(RX) / PB9(TX) |
| 遥控输入 | USART6（PX4 `ttyS5`） |
| GPS | USART1（GPS1）、USART3（GPS2） |
| 遥测 | USART2（TEL1）、UART4（TEL2）、UART8（TEL3）、UART7（URT6） |
| PWM | 12 路（参数存 flash 的变体为 10 路，PB0 改为无源蜂鸣器） |
| 电池 | PC0（电压）/ PC1（电流）ADC；可选 INA238 |
| 安全开关 | PB2 |

硬件设计文件同时包含 **VIT6** 和 **MINI** 两块板；本仓库的固件板级移植与预编译目标以 VIT6 命名。

## 固件

| 系列 | 版本 | 目标 | 说明 |
|---|---|---|---|
| PX4 | v1.17.0-alpha1 | `openbl_h743_default`、`openbl_h743_dds`、`openbl_h743fp_default`、`openbl_h743fp_dds` | `default` = CAN/UAVCAN，`dds` = uXRCE-DDS；`h743` = 参数存 SD 卡，`h743fp` = 参数存内部 flash |
| ArduPilot | Copter 4.7.1 / Plane 4.7.1 | `arducopter`、`arduplane`、bootloader | 板级 hwdef 在 `firmware固件/src编译源代码资料/APM/` |
| Betaflight | 4.5.3 | 单一目标 | 配置在 `firmware固件/src编译源代码资料/Betafight/` |
| INAV | 9.0.1 | 单一目标 | 配置在 `firmware固件/src编译源代码资料/INAV/` |

预编译固件在 [`firmware固件/built已编译固件/`](firmware固件/built已编译固件/)。
PX4 的合并 hex 已包含 bootloader，用 ST-Link 烧录；`.px4` 可用 QGC 烧录。

## 仓库结构

| 路径 | 说明 |
|---|---|
| [`Circuit_diagram电路图/`](Circuit_diagram电路图/) | 原理图（PDF）、PCB Gerber、BOM、坐标文件（VIT6 与 MINI） |
| [`STL图纸/`](STL图纸/) | 结构件 3D 模型与钢网框 |
| [`firmware固件/built已编译固件/`](firmware固件/built已编译固件/) | PX4 / ArduPilot / Betaflight / INAV 预编译固件 |
| [`firmware固件/src编译源代码资料/`](firmware固件/src编译源代码资料/) | PX4 板级源码、ArduPilot hwdef、Betaflight/INAV 配置、编译说明 |
| [`param参数/`](param参数/) | INA238（DPM238）电流计的 SD 卡参数包 |
| [`PPT/`](PPT/) | 接口映射文档与四张标注图 |
| `LICENSE`、`LICENSE-HARDWARE`、`NOTICE.md` | 许可与第三方声明 |

## 文档

| 文档 | English | 中文 |
|---|---|---|
| PX4 板级（参数存 SD） | [`readme.md`](firmware固件/src编译源代码资料/PX4/openblh743/readme.md) | [`readme_zh.md`](firmware固件/src编译源代码资料/PX4/openblh743/readme_zh.md) |
| PX4 参数存 flash 变体 | [`readme.md`](firmware固件/src编译源代码资料/PX4/openblh743fp/readme.md) | [`readme_zh.md`](firmware固件/src编译源代码资料/PX4/openblh743fp/readme_zh.md) |
| GPIO / 引脚参考 | [`gpio.md`](firmware固件/src编译源代码资料/PX4/openblh743/gpio.md) | [`gpio_zh.md`](firmware固件/src编译源代码资料/PX4/openblh743/gpio_zh.md) |
| ArduPilot 移植 | [`readme.md`](firmware固件/src编译源代码资料/APM/readme.md) | [`readme_zh.md`](firmware固件/src编译源代码资料/APM/readme_zh.md) |
| Betaflight 编译说明 | [`how_to_build_en.txt`](firmware固件/src编译源代码资料/Betafight/how_to_build_en.txt) | [`编译方法.txt`](firmware固件/src编译源代码资料/Betafight/编译方法.txt) |
| INAV 编译说明 | [`how_to_build_en.txt`](firmware固件/src编译源代码资料/INAV/how_to_build_en.txt) | [`编译方法.txt`](firmware固件/src编译源代码资料/INAV/编译方法.txt) |
| INA238 / DPM238 配置 | [`INSTRUCTIONS_EN.txt`](param参数/OpenblH743_DPM238_SD_px4/INSTRUCTIONS_EN.txt) | [`使用说明.txt`](param参数/OpenblH743_DPM238_SD_px4/使用说明.txt) |
| 接口映射（VIT6 / MINI） | [`interface_map_en.md`](PPT/interface_map_en.md) | [`接口说明.md`](PPT/接口说明.md) |

## 从源码编译

**PX4** —— 使用 PX4 Autopilot 源码树，把本仓库的 `boards/openbl/h743` 和
`boards/openbl/h743fp` 复制到 `boards/openbl/`，用 GCC 10.2.1
（`arm-none-eabi-10-2020-q4-major`）编译；GCC 13.x 生成的 H7 代码无法启动。

```bash
make openbl_h743_default
make openbl_h743_dds
make openbl_h743fp_default
make openbl_h743fp_dds
```

PX4 板级文档里记录了裁剪项、flash 预算、对 PX4 公共代码的本地补丁（`shared_src/`）
以及合并 hex 的步骤。

**ArduPilot**

```bash
./waf configure --board OpenblH743VIT6
./waf copter     # 或：./waf plane
```

bootloader 编译和产物说明见 ArduPilot 文档。

**Betaflight / INAV** —— 见对应源码目录下的编译说明。

## 注意事项

- **参数存储**：`openbl_h743_*` 参数存 SD 卡；`openbl_h743fp_*` 参数存最后一个 128 KB flash
  扇区，无 SD 卡也能保存参数（飞行日志仍需要 SD 卡）。
- **空速**：参考机型没有空速计；VTOL 前转换按姿态/时间完成，TECS 在无空速模式下运行。
  固定翼飞行前建议加空速管。
- **精确着陆**：保留了着陆目标估计器，供后续机载电脑（MAVLink `LANDING_TARGET`）或 IRLock
  使用；仓库内未包含目标传感器。
- **SPI2** 在本板修订上为轮询模式（DMA 流冲突）；SPI3 使用 DMA。
- **ICM-42605 驱动兼容性**：本地补丁同时接受 ICM-42688-P（WHO_AM_I `0x47`），以
  ICM-42605 兼容模式运行（16-bit FIFO、复位默认滤波）。飞行前请先台架验证传感器数据。

## 许可

本仓库同时包含原创内容和第三方内容，完整声明见 [`NOTICE.md`](NOTICE.md)。

- **原创内容** —— 硬件设计文件、结构模型、参数配置以及自行编写的固件代码：
  **CC BY-NC-SA 4.0**（署名-非商业性使用-相同方式共享），见 [`LICENSE`](LICENSE) 和
  [`LICENSE-HARDWARE`](LICENSE-HARDWARE)。商业使用需事先取得作者书面授权。
- **PX4 衍生**固件与源码：**BSD-3-Clause**（© PX4 Development Team）。
- **ArduPilot / Betaflight / INAV 衍生**内容：**GPL-3.0**，遵循各自原始许可。
- 第三方文件保留其原始版权头和许可。

## 作者

**洋洋柚**

- Gitee：[@zhangyangyangyou](https://gitee.com/zhangyangyangyou)
- GitHub：[@YangYangyou-Zhang](https://github.com/YangYangyou-Zhang)
- B站 / 抖音：洋洋柚Y

联系方式与权威的许可/署名声明以 [`NOTICE.md`](NOTICE.md) 为准。
