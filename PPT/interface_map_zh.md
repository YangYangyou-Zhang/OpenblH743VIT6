# OpenblH743VIT6 / MINI 接口映射说明

对应固件：PX4 v1.17.0-alpha1、ArduPilot Copter/Plane 4.7.1、Betaflight 4.5.3、INAV 9.0.1。

标注图（`PPT/annotated/`）：

- [H743VIT6 正面](annotated/H743VIT6_top_annotated.png)
- [H743VIT6 背面](annotated/H743VIT6_back_annotated.png)
- [H743MINI 正面](annotated/H743MINI_top_annotated.png)
- [H743MINI 背面](annotated/H743MINI_back_annotated.png)

---

## 1. 串口总表

| 硬件串口 | TX / RX 引脚 | PX4 | ArduPilot | Betaflight / INAV | 备注 |
|---|---|---|---|---|---|
| USART1 | PA9 / PA10 | **GPS1**（`/dev/ttyS0`） | SERIAL1 | UART1 | MINI 为 4pin，VIT6 在背面 6pin |
| USART2 | PA2 / PA3 | **TELEM1**（`/dev/ttyS1`） | SERIAL2 | UART2 | 与 VTX / 接收机组合口共用 |
| USART3 | PD8 / PD9 | **GPS2**（`/dev/ttyS2`） | SERIAL3 | UART3 | VIT6 丝印 "GPS1" 的 10pin 口 |
| UART4 | PA0 / PA1 | **TELEM2**（`/dev/ttyS3`） | SERIAL4 | UART4 | |
| USART6 | PC6 / PC7 | **RC**（`/dev/ttyS5`） | SERIAL5 | UART6 | 接收机 / SBUS / CRSF |
| UART7 | PE8 / PE7 | **URT6**（`/dev/ttyS6`） | SERIAL6 | UART7 | ESC 遥测 / 通用串口 |
| UART8 | PE1 / PE0 | **TELEM3**（`/dev/ttyS7`） | SERIAL7 | UART8 | |

> **APM 说明**：上表 "SERIALn" 对应 ArduPilot 的 `SERIALn_*` 参数；`SERIAL0` 是 USB。
>
> **BF/INAV 说明**：UART 号等于 STM32 的 USART 编号；INAV 目标默认 `SERIALRX_UART = USART6`（CRSF）。

### ⚠️ GPS1 / GPS2 命名差异（VIT6）

VIT6 正面 10pin 连接器的丝印是 **GPS1**，但它接的是 **USART3**：

| 物理接口 | USART | PX4 | ArduPilot | BF/INAV |
|---|---|---|---|---|
| VIT6 正面丝印 **GPS1**（10pin，CN3） | USART3 | **GPS2** | SERIAL3 | UART3 |
| VIT6 背面 6pin（U10） | USART1 | **GPS1** | SERIAL1 | UART1 |

所以：**PX4 的 GPS1 在背面 6pin U10；PX4 的 GPS2 才是正面丝印 GPS1 的 10pin 口。**
MINI 没有这个歧义：丝印直接标了 TX1/RX1 与 TX3/RX3。

---

## 2. VIT6 正反面接口

### 2.1 正面

| 标注 | 丝印 / 位置 | 原理图 Ref | 引脚 | USART | PX4 | APM | BF/INAV | 备注 |
|---|---|---|---|---|---|---|---|---|
| 1 | GPS1（10pin） | CN3 `ZX-GH1.25-10P` | 1 4V3 · 2 TX3 · 3 RX3 · 4 SCL2 · 5 SDA2 · 6 SAFETY_SW · 7 NC · 8 3V3 · 9 BEEP · 10 GND | USART3 | **GPS2** | SERIAL3 | UART3 | 含 I2C2 罗盘、安全开关、蜂鸣器 |
| 2 | TELEM1 / VTX | U13 `BX-SH1.0-5PWT` | TX2 · RX2 · VID_OUT · GND · VCC | USART2 | **TELEM1** | SERIAL2 | UART2 | DJI 图传视频口共用 |
| 3 | TELEM2 | CN6 `ZX-GH1.25-6PLT` | +5V · TX4 · RX4 · NC · NC · GND | UART4 | **TELEM2** | SERIAL4 | UART4 | 5V 供电 |
| 4 | TELEM3 | CN7 `ZX-GH1.25-6PLT` | +5V · TX8 · RX8 · NC · NC · GND | UART8 | **TELEM3** | SERIAL7 | UART8 | 5V 供电 |
| 5 | TELEM/RC | CN8 `ZX-GH1.25-6PLT` | 4V3 · TX6 · RX6 · NC · NC · GND | USART6 | **RC** | SERIAL5 | UART6 | 4V3 供电 |
| 6 | SBUS | CN5 `HC-GH1.25-5PLT` | 4V3 · RX6 · NC · NC · GND | USART6 | **RC** | SERIAL5 | UART6 | 仅 RX |
| 7 | RX7/TX7/GND | 3pin `BX-SH1.0-3PWT` | RX7 · TX7 · GND | UART7 | **URT6** | SERIAL6 | UART7 | ESC 遥测 / 通用串口 |
| 8 | CAN | CN2 `ZX-GH1.25-4PWT` | +5V · CANH · CANL · GND | — | CAN1 | CAN1 | — | MINI 无 CAN |
| 9 | I2C1 | CN4 `ZX-GH1.25-4PLT` | 4V3 · GND · SCL1 · SDA1 | — | I2C1 | — | — | 外接罗盘 |
| 10 | 电源模块 | — | GND · VOE(电压) · CUR(电流) · 4V3 · 5V | — | ADC | — | — | |
| 11 | USB | — | DP / DM / 5V / GND | — | OTG1 | SERIAL0 | — | |
| 12 | PWM 输出 | — | PWM1-11（FP 变体 1-10） · 5V · GND | — | — | — | — | FP 变体 PB0 为蜂鸣器 |
| 13 | SD 卡槽（顶部） | — | microSD · SDMMC1 | — | — | — | — | 日志 / 参数存储 |
| 14 | CAM 接口（顶部） | 3pin | VID_IN · 5V · GND | — | — | — | — | 模拟相机 |

### 2.2 背面

| 标注 | 丝印 / 位置 | 原理图 Ref | 引脚 | USART | PX4 | APM | BF/INAV | 备注 |
|---|---|---|---|---|---|---|---|---|
| 1 | 无丝印，6pin | U10 `BX-SH1.0-6PWT` | 4V3 · GND · RX1 · TX1 · SDA2 · SCL2 | USART1 | **GPS1** | SERIAL1 | UART1 | 含 I2C2 罗盘 |
| 2 | 大疆 HD 图传/数传（6pin，DJI HD VTX） | U15 `BX-SH1.0-6PWT` | RX6 · GND · RX2 · TX2 · GND · VCC | USART6 RX + USART2 | **RC + TELEM1** | SERIAL5 + SERIAL2 | UART6 + UART2 | 适配 DJI O3 Air Unit 等；VCC 供电 |
| 3 | RC/数传（4pin） | U12 `BX-SH1.0-4PWT` | 4V3 · GND · RX6 · TX6 | USART6 | **RC** | SERIAL5 | UART6 | |
| 4 | USB | — | DP / DM / 5V / GND | — | OTG1 | SERIAL0 | — | |
| 5 | BOOT 按钮（背面 SW1） | SW1 | BOOT0 | — | — | — | — | 按住上电进入 bootloader，用于 DFU/烧录 |

> USART2 同时出现在正面的 TELEM1/VTX（U13）和背面的**大疆 HD 图传/数传口**（U15），两者是并联的，**不能同时接两个设备**。
> USART6 同样被 TELEM/RC、SBUS、U12、U15 的 RX6 共用，实际使用时只接一个接收机/设备。

---

## 3. MINI 正反面接口

### 3.1 正面

| 标注 | 丝印 / 位置 | 原理图 Ref | 引脚 | USART | PX4 | APM | BF/INAV | 备注 |
|---|---|---|---|---|---|---|---|---|
| 1 | GPS1（4pin） | U9 `BX-SH1.0-4PWT` | 4V3 · GND · RX1 · TX1 | USART1 | **GPS1** | SERIAL1 | UART1 | |
| 2 | GPS2（6pin） | U10 `BX-SH1.0-6PWT` | 4V3 · GND · RX3 · TX3 · SDA2 · SCL2 | USART3 | **GPS2** | SERIAL3 | UART3 | 含 I2C2 罗盘 |
| 3 | TELEM1 / VTX（5pin） | `BX-SH1.0-5PWT` | TX2 · RX2 · VID_OUT · GND · VCC | USART2 | **TELEM1** | SERIAL2 | UART2 | DJI 图传共用 |
| 4 | TELEM2（4pin） | U11 `BX-SH1.0-4PWT` | 4V3 · GND · RX4 · TX4 | UART4 | **TELEM2** | SERIAL4 | UART4 | |
| 5 | RC（4pin） | U14 `BX-SH1.0-4PWT` | 4V3 · GND · RX6 · TX6 | USART6 | **RC** | SERIAL5 | UART6 | |
| 6 | 大疆 HD 图传/数传（6pin，DJI HD VTX） | U17 `BX-SH1.0-6PWT` | RX6 · GND · RX2 · TX2 · GND · VCC | USART6 RX + USART2 | **RC + TELEM1** | SERIAL5 + SERIAL2 | UART6 + UART2 | 适配 DJI O3 Air Unit 等；VCC 供电 |
| 7 | I2C1（4pin） | U8 `BX-SH1.0-4PWT` | 4V3 · GND · SDA1 · SCL1 | — | I2C1 | — | — | 外接罗盘 |
| 8 | 相机（3pin） | `BX-SH1.0-3PWT` | CAM / VID_IN · GND · 4V3 | — | — | — | — | 模拟相机 |
| 9 | BOOT 按钮 | SW1 | BOOT0 | — | — | — | — | 按住上电进入 bootloader |
| 10 | SD 卡槽 | — | microSD · SDMMC1 | — | — | — | — | 日志 / 参数存储 |

### 3.2 背面

| 标注 | 丝印 / 位置 | 原理图 Ref | 引脚 | USART | PX4 | APM | BF/INAV | 备注 |
|---|---|---|---|---|---|---|---|---|
| 1 | 四合一电调 S1-S4（左侧 8pin） | U16 `BX-SH1.0-8PWT` | VCC · GND · CUR · RX7 · PWM1-4 | UART7 RX | **URT6** | SERIAL6 | UART7 | 含电流检测；RX7 为 ESC 遥测 |
| 2 | TELEM3（4pin） | U12 `BX-SH1.0-4PWT` | 4V3 · GND · RX8 · TX8 | UART8 | **TELEM3** | SERIAL7 | UART8 | |
| 3 | USB | — | DP / DM / 5V / GND | — | OTG1 | SERIAL0 | — | |
| 4 | 四合一电调 S5-S8（左侧 8pin） | U19 `BX-SH1.0-8PWT` | VCC · GND · NC · RX7 · PWM5-8 | UART7 RX | **URT6** | SERIAL6 | UART7 | RX7 为 ESC 遥测 |

> **MINI 没有 CAN 接口**（原理图标注 "CAN通讯接口（无）"）。
> MINI 的 UART7 只引出了 RX7（ESC 遥测输入），没有独立的 TX7 引脚。

---

## 4. 快速接线建议

| 用途 | VIT6 | MINI |
|---|---|---|
| PX4 GPS1 | 背面 U10（6pin，USART1） | 正面 U9（4pin，USART1） |
| PX4 GPS2 | 正面丝印 GPS1 的 10pin（USART3） | 正面 U10（6pin，USART3） |
| PX4 TELEM1 | 正面 TELEM1/VTX（USART2） | 正面 TELEM1/VTX（USART2） |
| PX4 TELEM2 | 正面 TELEM2（UART4） | 正面 U11（UART4） |
| PX4 TELEM3 | 正面 TELEM3（UART8） | 背面 U12（UART8） |
| PX4 RC / 接收机 | 正面 TELEM/RC 或 SBUS（USART6） | 正面 U14 或 U17（USART6） |
| 大疆 HD 图传/数传 | 背面 U15（USART6 RX + USART2 + VCC） | 正面 U17（USART6 RX + USART2 + VCC） |
| PX4 URT6 / ESC 遥测 | 正面 RX7/TX7（UART7） | 背面 ESC 口 RX7（S1-S4 / S5-S8） |
| 外接罗盘 | 正面 I2C1（CN4） | 正面 U8（I2C1） |
| CAN | 正面 CN2 | 无 |

---

## 5. 固件侧确认命令

PX4 NSH：

```sh
listener sensor_gps -n 1      # 看 GPS 是否在对应 ttyS 上出现
listener vehicle_gps_position -n 1
listener manual_control -n 1  # RC 输入
```

参数里可直接设置串口功能，例如：

```sh
param set GPS_1_CONFIG 101    # 以实际固件枚举值为准
```

ArduPilot：`SERIALn_PROTOCOL` / `SERIALn_BAUD`；BF/INAV：在 Ports 页把对应 UART 设为 GPS / Serial RX / MSP 等。

---

*标注图与本文档基于 2026-10-23 的固件配置和原理图（Rev 1.0）整理。*

