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

#include "menu.h"
#include "../fs/fs.h"
#include "../io/stdout.h"
#include "../memory/memory.h"
#include "../io/keyboard.h"
#include "../string/string.h"
#include "../exception/exception.h"

void show_menu()
{
	u32 disk;
	u32 partition;
	pfile fp;
	u32 t;
	char* path;
	char* p;
	print_string(
		GET_REAL_ADDR("Searching for /boot/sanlo.cfg...\n"),
		FG_BRIGHT_WHITE | BG_BLACK,
		BG_BLACK);
	path = malloc(128);
	//Search for sanlo.cfg
	//Enumerate partitions

	for(disk = 0; disk < 4; disk++) {
		//Primary partitions
		for(partition = 0; partition < 4; partition++) {
			if(get_partition_info(disk, partition, &t, &t, &t) != PARTITION_NOT_FOUND) {
				strcpy(path, GET_REAL_ADDR("(hd"));
				dectostr(disk, path + 3);
				strcat(path, GET_REAL_ADDR(","));
				dectostr(partition, path + strlen(path));
				strcat(path, GET_REAL_ADDR(")"));
				p = path + strlen(path);
				strcpy(p, GET_REAL_ADDR("/boot/sanlo.cfg"));
				fp = open(path);

				if(fp != NULL) {
					goto _ANALYSE_CFG;
				}

				strcpy(p, GET_REAL_ADDR("/sanlo.cfg"));
				fp = open(path);

				if(fp != NULL) {
					goto _ANALYSE_CFG;
				}
			} else {
				break;
			}
		}

		//Logic partitions
		for(partition = 4;
			get_partition_info(disk, partition, &t, &t, &t) != PARTITION_NOT_FOUND;
			partition++) {
			strcpy(path, GET_REAL_ADDR("(hd"));
			dectostr(disk, path + 3);
			strcat(path, GET_REAL_ADDR(","));
			dectostr(partition, path + strlen(path));
			strcat(path, GET_REAL_ADDR(")"));
			p = path + strlen(path);
			strcpy(p, GET_REAL_ADDR("/boot/sanlo.cfg"));
			fp = open(path);

			if(fp != NULL) {
				goto _ANALYSE_CFG;
			}

			strcpy(p, GET_REAL_ADDR("/sanlo.cfg"));
			fp = open(path);

			if(fp != NULL) {
				goto _ANALYSE_CFG;
			}
		}
	}
	panic(EXCEPTION_NO_CONFIG_FILE);
	//Analyse config file
_ANALYSE_CFG:
	print_string(
		GET_REAL_ADDR("Found\n"),
		FG_BRIGHT_WHITE | BG_BLACK,
		BG_BLACK);
	free(path);
	close(fp);

	while(1);

	return;
}
