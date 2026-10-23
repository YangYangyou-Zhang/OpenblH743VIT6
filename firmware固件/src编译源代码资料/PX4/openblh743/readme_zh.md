# OpenblH743VIT6 — 项目背景与编译/烧录说明

## 项目背景

- 定制飞控，主控 **STM32H743VIT6**，**Board ID = 18848**。
- 硬件从 MicoAir743v2 改来，跑 **PX4 固件**。
- 与 ArduPilot 共用 **APM bootloader**（PX4 自带的 NuttX bootloader 会损坏 flash，已弃用）。

传感器：

| 外设 | 型号 | 总线 | 说明 |
|------|------|------|------|
| IMU0 | MPU6500 或 ICM-42605 | SPI2，CS PB12 | 轮询模式；两种芯片均可贴装 |
| IMU1 | MPU6500 或 ICM-42605 | SPI3，CS PD7 | 两种芯片均可贴装 |
| IMU2（可选） | MPU6500 或 ICM-42605 | SPI3，CS PD5 | ICM 驱动已编译，**未启动**（见下文） |
| 气压计 | SPL06 | I2C（0x76/0x77 自动扫描） | |
| 罗盘 | QMC5883P | I2C1（0x2C） | 驱动默认旋转 `-R 10`；`CAL_MAG0_ROT` 保持 **-1**（见下文）|
| 电池 | ADC1 ch10/11 | PC0(电压)/PC1(电流) | 电压通道有下拉电阻，未接电池时读 ~0 属正常 |
| CAN | UAVCAN/DroneCAN | PB8(RX)/PB9(TX) | 1 Mbps，主板启用 |

参数存 **SD 卡**（`CONFIG_BOARD_PARAM_FILE=/fs/microsd/params`）。

### 可选的第二 IMU：ICM-42605（已编译，未启动）

`icm42605` 驱动已编入四个固件变体。MPU6500 和 ICM-42605（以及通过兼容补丁支持的 ICM-42688-P）
都注册在**全部三个 IMU 片选位置**：

| 总线 | CS | 已注册器件类型 |
|---|---|---|
| SPI2 | PB12 | MPU6500、ICM42605 |
| SPI3 | PD7 | MPU6500、ICM42605 |
| SPI3 | PD5 | MPU6500、ICM42605 |

`rc.board_sensors` 里原来的 MPU6500 启动保持不变。`icm42605` **没有**被启动（安装方向还需要实测）。
单独测试某个位置：

```
icm42605 start -s -b 3 -c 5  -R <旋转>   # PD5
icm42605 start -s -b 3 -c 7  -R <旋转>   # PD7
icm42605 start -s -b 2 -c 12 -R <旋转>   # PB12
```

不带 `-b`/`-c` 会遍历所有已注册的 ICM42605 设备；`-c` 按 CS 引脚号选择。

- 驱动接受 WHO_AM_I `0x42`（ICM-42605）和 `0x47`（ICM-42688-P，需下文所述的本地补丁）。插 42688P 时它运行在 **ICM-42605 兼容模式**（16-bit FIFO、复位默认滤波、不解析温度/FIFO 时间戳）；原生的 42688P 驱动未编译（约 +7.6 KB）。
- 不需要 DRDY 线：没有 DRDY 时驱动会退回定时读 FIFO。
- SPI3 是唯一启用 DMA 的 IMU 总线（`CONFIG_STM32H7_SPI3_DMA`）；SPI2（IMU0）为轮询模式。

## 本地对 PX4 公共代码的改动

除板级目录 `boards/openbl/` 外，这棵源码树还包含三处对 **PX4 公共代码**的本地修改。
它们不在上游，`git pull` / `git checkout` 会覆盖，升级前请先备份。
这三处修改后的文件已收录在 `../shared_src/`（目录结构与 PX4 源码树一致），可直接覆盖到对应位置。

### 1. VL53L1X 激光测距驱动

文件：`src/drivers/distance_sensor/vl53l1x/vl53l1x.cpp`

| 项 | 原值 | 本版 |
|---|---|---|
| `VL53L1X_DELAY`（轮询间隔） | 50000 µs | **20000 µs** |
| `VL53L1X_SAMPLE_RATE`（timing budget） | 200 ms | **100 ms** |
| `VL53L1X_INTER_MEAS_MS`（测量周期） | 250 ms | **110 ms** |
| 信号质量字段 | 把 ST range status 直接写入 `quality` | **`rangeStatus == 0 → 100`，否则 `0`** |

- **采样率**约 4 Hz → **约 9 Hz**；**量程** 4 m → **约 2.5~3 m**
- 本版修正了上游驱动的一处字段映射：I2C 通信状态（成功恒为 0）被写进了 `quality`，而 PX4 规定
  `quality == 0` 表示"信号无效"，会丢弃全部有效测距。修正后 EKF 才会真正使用测距数据。
- 启用方式：

  ```
  param set SENS_EN_VL53L1X 1     # 重启生效
  param set EKF2_RNG_CTRL 2       # 始终参与高度融合
  ```

### 2. uXRCE-DDS 的 microcdr 仓库路径

文件：`src/modules/uxrce_dds_client/Micro-XRCE-DDS-Client/cmake/SuperBuild.cmake`

`GIT_REPOSITORY` 由硬编码的 build 目录改为 `$ENV{HOME}/gitcache/microcdr`（Micro-CDR v2.0.1
的本地克隆），否则清理 build 后编译 dds 目标会因 `Failed to clone repository` 失败。
缓存可以这样创建：

```bash
git clone --branch v2.0.1 https://github.com/eProsima/Micro-CDR.git ~/gitcache/microcdr
```

或者把路径改成你自己的副本。**这是子模块内的改动，`git submodule update` 会冲掉。**

### 3. ICM-42605 驱动兼容 ICM-42688-P

文件：`src/drivers/imu/invensense/icm42605/ICM42605.cpp`、
`src/drivers/imu/invensense/icm42605/InvenSense_ICM42605_registers.hpp`

在 `probe()` 和复位后的检查中都额外接受 `WHO_AM_I = 0x47`（ICM-42688-P）。驱动其余部分不变，
因此 42688P 完全按 42605 的方式驱动：

- 16-bit FIFO（packet 3）、8 kHz ODR、±2000 dps / ±16 g；
- 不使用 20-bit hires，不配置 AAF/陷波滤波，不解析温度和 FIFO 时间戳；
- `INTF_CONFIG0/1` 和滤波寄存器依赖芯片复位默认值。

**这是兼容性 hack，不是原生驱动。** 飞行前必须台架验证加速度计/陀螺数据（重力轴 1 g、静止零偏、
旋转方向）。补丁文件已放入 `../shared_src/`；`git pull` / `git checkout` 会覆盖它们。

## 编译

1. 工具链必须用 **GCC 10.2.1**（`arm-none-eabi-10-2020-q4-major`）。GCC 13.x 生成的 H7 代码无法启动。
2. 在 PX4 源码树中执行。

**两个版本**（flash 只有 1920KB，CAN 和 DDS 二选一，放不下两个同时启用）：

| 版本 | 命令 | 配置 |
|------|------|------|
| 主板（带 CAN） | `make openbl_h743_default` | `default.px4board`：启用 UAVCAN，关闭 UXRCE_DDS |
| DDS 变体 | `make openbl_h743_dds` | `dds.px4board`：启用 UXRCE_DDS，关闭 UAVCAN |

> 注意：切换版本编译时，如果 CMake 缓存有残留（比如 DDS 版还带着 UAVCAN），先 `rm -rf build/openbl_h743_dds`（或对应版本目录）再编译，确保配置干净。

3. 产物：
   - `build/openbl_h743_<label>/openbl_h743_<label>.px4`
   - `build/openbl_h743_<label>/openbl_h743_<label>.bin`

> 本地归档命名：`<target>_1.17.0-alpha1.hex`（已合并 bootloader）/ `<target>_1.17.0-alpha1.px4`。

## 合并 bootloader + 固件（merged hex）

用 arm-none-eabi-objcopy 把 bootloader（`0x08000000`）和固件（`0x08020000`）合并成一个 hex，供 ST-Link 一次性烧录：

```bash
O=arm-none-eabi-objcopy
$O -I binary -O ihex --change-addresses 0x08000000 openbl_h743_bootloader.bin /tmp/bl.hex
$O -I binary -O ihex --change-addresses 0x08020000 openbl_h743_default.bin /tmp/fw.hex
grep -v ':00000001FF' /tmp/bl.hex > openbl_h743_merged.hex
cat /tmp/fw.hex >> openbl_h743_merged.hex
```

## 烧录

- 用 **ST-Link** 刷 `openbl_h743_merged.hex`（bootloader + 固件一次性烧入）。
- Flash 布局：

| 区域 | 地址 | 大小 |
|------|------|------|
| Bootloader | 0x08000000 | 128 KB |
| 固件 | 0x08020000 | 1920 KB |

## 外设/接口状态（2026-08-13）

- **USB 设备名**：`OpenblH743VIT6`（已改名；见 `nuttx-config/nsh/defconfig` 的 `CDCACM_PRODUCTSTR/VENDORSTR` 和 `firmware.prototype`）。
- **罗盘归类（2026-10-19）**：PX4 此版本**没有 `CAL_MAGx_EXTERNAL` 参数**，罗盘的"内置/外置"由所在 I2C 总线决定。`i2c.cpp` 把 **I2C1 和 I2C2 都标记为外置**（这样 `-X` 能一次扫描全部总线），同时通过板级覆盖 `px4_i2c_device_external()` 把**板载 QMC5883P（I2C1, 0x2C）单独报告为 internal**。这样 PX4 才会把它当作参照物，自动推算**外接**磁罗盘的安装朝向（`mag_calibration.cpp`：外置罗盘的朝向是通过与内置罗盘逐点比对得出的）。
- **罗盘方向（2026-10-19）**：板载罗盘的安装补偿已写进**驱动默认旋转** —— `qmc5883p -X -b 1 -R 10 start`（`ROTATION_ROLL_180_YAW_90`）。因此 **`CAL_MAG0_ROT` 必须保持默认值 `-1`**，不需要也不能再手设 YAW 180。
  - ⚠️ `CAL_MAG0_ROT` 一旦被写成有效值（例如跑完磁罗盘校准），就会**覆盖**驱动 `-R`；校准后需手动改回 `-1`。
  - 飞控整体换安装方向时改 **`SENS_BOARD_ROT`**（IMU 的 `CAL_ACC/GYRO_ROT` 也是 -1，会一起跟随），不要改 `CAL_MAG0_ROT`。
- **串口分配（2026-08-29）**：`RC` 用 `CONFIG_BOARD_SERIAL_RC`（**`RC_INPUT` 这个名字 PX4 不识别**，RC 实际靠 `board_config.h` 的 `RC_SERIAL_PORT` 绑定 `/dev/ttyS5`=USART6=RCIN PC6/PC7）；新增 `URT6=/dev/ttyS6`（=UART7 通用串口，PE8/PE7）、`TEL3=/dev/ttyS7`（=UART8，PE1/PE0）；交换 `GPS2`/`TEL2`（硬件上 **USART3 是 GPS2、UART4 是 TEL2** → GPS2=/dev/ttyS2、TEL2=/dev/ttyS3）。串口功能在 `default.px4board`/`dds.px4board` 的 `CONFIG_BOARD_SERIAL_*` 里定义（可选名仅 `URT6`/`GPS1~5`/`TEL1~5`/`RC`/`WIFI`/`EXT2`），QGC 只显示固件定义的口。
- **安全开关 PB2**：已启用 `safety_button` 驱动（`GPIO_BTN_SAFETY`，按下接 3.3V 高有效、配 `GPIO_PULLDOWN`，`safety_button start`）。
- **蜂鸣器**：已移除（禁用 `CONFIG_DRIVERS_TONE_ALARM`）。原 PB0 无源蜂鸣器与 PWM11 共用 TIM3，tone_alarm 需独占定时器改频率、与 PWM 冲突，最终放弃蜂鸣器，PB0 改为 PWM12。
- **AT7456E OSD（2026-09-05）**：已加入固件（`CONFIG_DRIVERS_OSD_ATXXXX`）。硬件：SPI1 = PA5(SCK)/PA6(MISO)/PA7(MOSI)，CS = PC4（与 APM hwdef 一致）。板级改动：`nuttx-config/include/board.h` 增加 `GPIO_SPI1_*` 定义、`nuttx-config/nsh/defconfig` 开 `CONFIG_STM32H7_SPI1`、`src/spi.cpp` 增加 SPI1 总线（`DRV_OSD_DEVTYPE_ATXXXX`，CS PC4）、`init/rc.board_extras` 在 `OSD_ATXXXX_CFG` 非 0 时执行 `atxxxx start -s`（`-s`=内置 SPI 总线）。用法：QGC 参数设 **`OSD_ATXXXX_CFG` = 1(NTSC) 或 2(PAL)**，重启生效。

## VTOL/固定翼支持（2026-10-22）

固件已启用 PX4 固定翼控制链：

- `fw_mode_manager`：FW 模式管理（Position/Altitude/Manual/Stabilized/Acro）、起飞/降落、发射检测、跑道起飞
- `fw_lat_lon_control`：TECS、横纵向控制、NPFG 路径跟随

`rc.vtol_apps` 会自动启动这两个模块；VTOL 转换开关为 `RC_MAP_TRANS_SW`。
注意：本机**没有空速计**，尾座式前转换按姿态/时间完成（不等 `VT_ARSP_TRANS=10 m/s`），TECS 走无空速模式；首次固定翼飞行前建议加空速计，并设置 `VT_FW_MIN_ALT`、`VT_QC_ALT_LOSS` 等保护参数。

## LED 指示（2026-10-22）

硬件：PE2 = R、PE3 = G、PE4 = B（低电平点亮）。PX4 的颜色码是 `BLUE=0, RED=1, SAFETY=2, GREEN=3`，
`led.c` 的 `g_ledmap` 必须按这个顺序排列（旧版写成了 G/B/R，导致蓝色逻辑点亮了绿灯）。

当前行为（为兼容只有单颗红灯的硬件版本，已把解锁状态和过载的颜色对调）：

| 状态 | LED |
|---|---|
| 未通过预检（禁止解锁） | **红灯 10 Hz 快闪** |
| 可解锁 | **红灯 1 Hz 慢闪** |
| 已解锁、正常 | **红灯常亮**（`BOARD_ARMED_LED = LED_RED`） |
| 已解锁 + failsafe | 红灯 2 Hz |
| 过载/错误 | **蓝灯 10 Hz** |

## 精确着陆（2026-10-23）

固件保留了 `landing_target_estimator`（LTE，约 3.4 KB）和 navigator 的 `AUTO_PRECLAND` 状态机，
供后续加机载电脑使用。目前板上没有任何目标传感器，也没有接入目标源，所以该功能暂时是“已编译、无输入”。

| 目标输入 | 路径 | 是否经过 LTE |
|---|---|---|
| MAVLink `LANDING_TARGET`，`position_valid=1`、`frame=MAV_FRAME_LOCAL_NED` | 直接发布为 `landing_target_pose` | 不需要 |
| 角度式 `LANDING_TARGET`（`position_valid=0`）或 IRLock | `irlock_report` → LTE → `landing_target_pose` | 需要 |

- 触发方式：任务里的 Land 项设置 `land_precision=1`（机会式）/ `2`（强制式）；或 `RTL_PLD_MD=1/2`；
  或 `commander mode auto:precland`。
- 默认参数：`PLD_SRCH_ALT=10 m`、`PLD_SRCH_TOUT=10 s`、`PLD_BTOUT=5 s`、`PLD_HACC_RAD=0.2 m`、`PLD_FAPPR_ALT=0.1 m`。
- 没有目标时会先在搜索高度盘旋寻找，超时后回退普通降落。

## 已知注意事项

- **SPI2（IMU0）的 DMA 存在冲突**，目前临时关闭 `CONFIG_STM32H7_SPI2_DMA` 走轮询模式。功能正常，但会略占 CPU；后续可排查 DMA 流分配后正确启用。
- SPI 引脚（以 APM `hwdef.dat` 为硬件真相）：
  - SPI1：PA5(SCK) / PA6(MISO) / PA7(MOSI) / PC4(CS) —— AT7456E OSD
  - SPI2：PB13(SCK) / PB14(MISO) / PB15(MOSI) / PB12(CS)
  - SPI3：PB3(SCK) / PB4(MISO) / PD6(MOSI)；PD7 = IMU CS，PD5 = IMU CS（均可贴 MPU6500 或 ICM-42605）
- **PWM 映射**（`timer_config.cpp`）：
  - PWM 1-4 = TIM1 CH1-4：PE9/PE11/PE13/PE14
  - PWM 5-8 = TIM4 CH1-4：PD12/PD13/PD14/PD15
  - PWM 9-10 = TIM15：PE5/PE6
  - PWM 11 = TIM3 CH4：PB1
  - PWM 12 = TIM3 CH3：PB0
- **flash 上限 1920KB**：CAN（UAVCAN）约 142KB、DDS（UXRCE_DDS）约 65KB，两者不能同时启用。主板带 CAN、DDS 变体带 DDS。当前占用（含固定翼/VTOL 模块、可选 ICM-42605 驱动与全部 14 个磁罗盘驱动）：default **93.22%**（1,832,884 B，余量约 130 KB）、dds **89.20%**（1,753,708 B，余量约 207 KB）。为容纳 `fw_mode_manager` + `fw_lat_lon_control`（约 45 KB），已关闭 `SYSTEMCMDS_TESTS`、`GNSS_SEPTENTRIO`、`LOCAL_POSITION_ESTIMATOR`、`COMMON_TELEMETRY`、`TAP_ESC`、`DRIVERS_SMART_BATTERY_BATMON`；CAN 版另关 `UXRCE_DDS_CLIENT`。着陆目标估计器（`landing_target_estimator`）保留，供后续机载电脑做精确着陆。

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6
