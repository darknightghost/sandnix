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
#include "../io/io.h"

u32 get_hdd_info(u8 dev)
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
	u16 alter_status_reg;
	u32 ret;
	ide_device_reg dev_reg_value;
	u16 status;
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
		alter_status_reg = data_reg + 0x0206;
		dev_reg_value.value = 0;

		if(checked_dev % 2 == 0) {
			dev_reg_value.members.drv_slave_flag = 0;

		} else {
			dev_reg_value.members.drv_slave_flag = 1;
		}

		out_byte(dev_reg_value.value, device_reg);
		out_byte(0x45, sector_count_reg);
		out_byte(0x55, lba_low_reg);

		if(in_byte(sector_count_reg) == 0x45) {
			//Check if it is a harddisk
			out_byte(dev_reg_value.value, device_reg);
			out_byte(0xec, status_reg);

			while(!((status = in_byte(status_reg)) & 0x08)) {		//0x08=0000 1000
				if(status & 0x81) {
					dev_found--;
					break;
				}
			}

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

bool hdd_read(u32 hdd_info, u32 start_sector, u8 sector_num, u8* buf)
{
	u16 data_reg;
	u16 error_reg;
	u16 sector_count_reg;
	u16 lba_low_reg;
	u16 lba_mid_reg;
	u16 lba_high_reg;
	u16 device_reg;
	u16 status_reg;
	u16 alter_status_reg;
	ide_device_reg dev_reg_value;
	u8 status;

	if(hdd_info & DEVICE_NOT_EXISTS) {
		return false;
	}

	//Compute I/O port
	if(hdd_info & DEVICE_PORT_PRIMARY_FLAG) {
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
	alter_status_reg = data_reg + 0x0206;
	//Setup IDE regs
	out_byte(0x02, alter_status_reg);
	out_byte(0, error_reg);
	out_byte(sector_num, sector_count_reg);
	out_byte((u8)(start_sector & 0xFF), lba_low_reg);
	out_byte((u8)((start_sector >> 8) & 0xFF), lba_mid_reg);
	out_byte((u8)((start_sector >> 16) & 0xFF), lba_high_reg);
	dev_reg_value.value = 0;
	dev_reg_value.members.lba_high = (u8)((start_sector >> 24) & 0x0F);
	dev_reg_value.members.lba_mode = 1;
	dev_reg_value.members.always_1_1 = 1;
	dev_reg_value.members.always_1_2 = 1;

	if(hdd_info & DEVICE_MASTER_FLAG) {
		dev_reg_value.members.drv_slave_flag = 0;

	} else {
		dev_reg_value.members.drv_slave_flag = 1;
	}

	out_byte(dev_reg_value.value, device_reg);
	out_byte(0x20, status_reg);

	//Read disk
	while(!(in_byte(status_reg) & 0x08));		//0x08=0000 1000

	status = in_byte(error_reg);

	if(status & 0x01) {
		return false;
	}

	in_words(data_reg, HDD_SECTOR_SIZE * sector_num / 2, buf);

	return true;
}


