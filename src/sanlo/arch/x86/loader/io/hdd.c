/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hdd.h"

u32 get_hdd_status(u8 dev)
{
	u8 dev_found;
	u8 checked_dev;
	u16 data_reg;
	u16 error_reg;
	u16 sector_count_reg;
	u16 lba_low_reg;
	u16 lba_mid_reg;
	u16 lba_high_reg;
	u16 device_reg;
	u16 status_reg;
	u16 alter_statuc_reg;
	u32 ret;
	ide_device_reg dev_reg_value;
	dev_found = 0;

	for(checked_dev = 0; checked_dev < 4; checked_dev++) {
		//Compute I/O port
		if(checked_dev < 2) {
			data_reg = 0x01F0;
		} else {
			data_reg = 0x0170;
		}

		error_reg = data_reg + 1;
		sector_count_reg = data_reg + 2;
		lba_low_reg = data_reg + 3;
		lba_mid_reg = data_reg + 4;
		lba_high_reg = data_reg + 5;
		device_reg = data_reg + 6;
		status_reg = data_reg + 7;
		alter_statuc_reg = data_reg + 0x0206;
		dev_reg_value.value = 0;

		if(checked_dev % 2 == 0) {
			dev_reg_value.drv_slave_flag = 0;
		} else {
			dev_reg_value.drv_slave_flag = 1;
		}

		out_byte(dev_reg_value.value, device_reg);
		out_byte(0x45, sector_count_reg);
		out_byte(0x55, lba_low_reg);

		if(in_byte(sector_count_reg) == 0x45) {
			dev_found++;

			//Return device status
			if(dev_found > dev) {
				ret = 0;

				if(checked_dev < 2) {
					ret |= DEVICE_PORT_PRIMARY_FLAG;
				}

				if(checked_dev % 2 == 0) {
					ret |= DEVICE_MASTER_FLAG;
				}

				return ret;
			}
		}
	}

	return DEVICE_NOT_EXISTS;
}

u32 hdd_read(u32 start_sector, u32 sector_num, u8* buf);


