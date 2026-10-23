/****************************************************************************
 *
 *   Copyright (c) 2026 洋洋柚 (@zhangyangyangyou)
 *   Board port for the OpenblH743VIT6, based on the PX4 reference board
 *   configuration. Licensed under BSD-3-Clause (see the PX4 project).
 *
 ****************************************************************************/

#include <px4_arch/spi_hw_description.h>
#include <drivers/drv_sensor.h>
#include <nuttx/spi/spi.h>
constexpr px4_spi_bus_t px4_spi_buses[SPI_BUS_MAX_BUS_ITEMS] = {
 initSPIBus(SPI::Bus::SPI1, {
  initSPIDevice(DRV_OSD_DEVTYPE_ATXXXX, SPI::CS{GPIO::PortC, GPIO::Pin4}),
 }),
 initSPIBus(SPI::Bus::SPI2, {
  initSPIDevice(DRV_IMU_DEVTYPE_MPU6500, SPI::CS{GPIO::PortB, GPIO::Pin12}),
  initSPIDevice(DRV_IMU_DEVTYPE_ICM42605, SPI::CS{GPIO::PortB, GPIO::Pin12}),
 }),
 initSPIBus(SPI::Bus::SPI3, {
  initSPIDevice(DRV_IMU_DEVTYPE_MPU6500, SPI::CS{GPIO::PortD, GPIO::Pin7}),
  initSPIDevice(DRV_IMU_DEVTYPE_ICM42605, SPI::CS{GPIO::PortD, GPIO::Pin7}),
  initSPIDevice(DRV_IMU_DEVTYPE_MPU6500, SPI::CS{GPIO::PortD, GPIO::Pin5}),
  initSPIDevice(DRV_IMU_DEVTYPE_ICM42605, SPI::CS{GPIO::PortD, GPIO::Pin5}),
 }),
};
static constexpr bool unused = validateSPIConfig(px4_spi_buses);
