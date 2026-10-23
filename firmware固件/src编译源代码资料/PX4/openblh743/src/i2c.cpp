/****************************************************************************
 *
 *   Copyright (c) 2026 洋洋柚 (@zhangyangyangyou)
 *   Board port for the OpenblH743VIT6, based on the PX4 reference board
 *   configuration. Licensed under BSD-3-Clause (see the PX4 project).
 *
 ****************************************************************************/

#include <px4_arch/i2c_hw_description.h>

#include <lib/drivers/device/Device.hpp>
#include <px4_platform_common/i2c.h>

constexpr px4_i2c_bus_t px4_i2c_buses[I2C_BUS_MAX_BUS_ITEMS] = {
	initI2CBusExternal(1),
	initI2CBusExternal(2),
};

bool px4_i2c_device_external(const uint32_t device_id)
{
	// Both I2C buses are declared external so that -X scans everything.
	// The onboard QMC5883P (I2C1, 0x2C) is physically on the board, so
	// report it as internal: PX4 then uses it as the reference compass
	// when auto-detecting the orientation of external magnetometers.
	device::Device::DeviceId onboard_mag{};
	onboard_mag.devid_s.bus_type = device::Device::DeviceBusType_I2C;
	onboard_mag.devid_s.bus = 1;
	onboard_mag.devid_s.address = 0x2C;
	onboard_mag.devid_s.devtype = DRV_MAG_DEVTYPE_QMC5883P;

	if (onboard_mag.devid == device_id) {
		return false;
	}

	device::Device::DeviceId dev_id{};
	dev_id.devid = device_id;
	return px4_i2c_bus_external(dev_id.devid_s.bus);
}
