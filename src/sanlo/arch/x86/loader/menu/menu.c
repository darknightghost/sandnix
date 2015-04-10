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
#include "../k_loader/k_loader.h"

static	bool		check_version(pfile fp);
static	bool		analyse_cfg_file(pfile fp, pmenu p_boot_menu);
static	pfile		open_cfg_file();
static	void		print_menu(u16 line, pmenu p_boot_menu);
static	char*		get_word(pfile fp, char* buf, size_t buf_size);
static	char*		get_line(pfile fp, char* buf, size_t buf_size);
static	void		jmp_space(pfile fp);

void show_menu()
{
	pfile fp;
	menu boot_menu;
	u32 pressed_key;
	u32 i;
	pmenu_item p_menu_item;
	print_string(
		GET_REAL_ADDR("Searching for /boot/sanlo.cfg...\n"),
		FG_BRIGHT_WHITE | BG_BLACK,
		BG_BLACK);
	//Open config file
	fp = open_cfg_file();

	if(fp == NULL) {
		panic(EXCEPTION_NO_CONFIG_FILE);
	}

	//Check file version
	if(!check_version(fp)) {
		panic(EXCEPTION_UNEXPECT_CONFIG_FILE);
	}

	//Analyse config file
	if(!analyse_cfg_file(fp, &boot_menu)) {
		panic(EXCEPTION_UNEXPECT_CONFIG_FILE);
	}

	close(fp);
	//Show menu
	cls(BG_BLACK | FG_BRIGHT_WHITE);
	print_string(
		GET_REAL_ADDR("Enter a choice or press <ESC> to restart your computer:\n"),
		FG_BRIGHT_WHITE | BG_BLACK,
		BG_BLACK);
	print_menu(1, &boot_menu);

	while(1) {
		pressed_key = get_keyboard_input();

		if(pressed_key == KEY_ENTER_PRESSED) {
			p_menu_item = boot_menu.menu_list;

			for(i = 0; i < boot_menu.selected_item; i++) {
				p_menu_item = p_menu_item->p_next;
			}

			if(!load_os_kernel(p_menu_item->kernel_path, p_menu_item->paramter)) {
				panic(EXCEPTION_NO_KERNEL);
			}
		} else if(pressed_key == KEY_UP_PRESSED) {
			if(boot_menu.selected_item > 0) {
				(boot_menu.selected_item)--;
				print_menu(1, &boot_menu);
			}
		} else if(pressed_key == KEY_DOWN_PRESSED) {
			if(boot_menu.selected_item < boot_menu.item_num - 1) {
				(boot_menu.selected_item)++;
				print_menu(1, &boot_menu);
			}
		} else if(pressed_key == KEY_ESC_PRESSED) {
			break;
		}
	}

	return;
}

pfile open_cfg_file()
{
	u32 disk;
	u32 partition;
	u32 t;
	char* path;
	char* p;
	pfile fp;
	path = malloc(128);

	//Search for sanlo.cfg
	//Enumerate partitions
	for(disk = 0; disk < 4; disk++) {
		//Primary partitions
		for(partition = 0; partition < 4; partition++) {
			if(get_partition_info(disk, partition, &t, &t, (u8*)&t) != PARTITION_NOT_FOUND) {
				strcpy(path, GET_REAL_ADDR("(hd"));
				dectostr(disk, path + 3);
				strcat(path, GET_REAL_ADDR(","));
				dectostr(partition, path + strlen(path));
				strcat(path, GET_REAL_ADDR(")"));
				p = path + strlen(path);
				strcpy(p, GET_REAL_ADDR("/boot/sanlo.cfg"));
				fp = open(path);

				if(fp != NULL) {
					free(path);
					return fp;
				}

				strcpy(p, GET_REAL_ADDR("/sanlo.cfg"));
				fp = open(path);

				if(fp != NULL) {
					free(path);
					return fp;
				}
			} else {
				break;
			}
		}

		//Logic partitions
		for(partition = 4;
			get_partition_info(disk, partition, &t, &t, (u8*)&t) != PARTITION_NOT_FOUND;
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
				free(path);
				return fp;
			}

			strcpy(p, GET_REAL_ADDR("/sanlo.cfg"));
			fp = open(path);

			if(fp != NULL) {
				free(path);
				return fp;
			}
		}
	}

	free(path);
	return NULL;
}

char* get_word(pfile fp, char* buf, size_t buf_size)
{
	bool quote_flag;
	char byte;
	char* p;
	quote_flag = false;
	p = buf;

	while(p - buf < buf_size - 1) {
		if(read(fp, &byte, 1) == 0) {
			break;
		}

		if(byte == '\"') {
			quote_flag = !quote_flag;
		} else if((byte == ' ' || byte == '\t')
				  && !quote_flag) {
			break;
		} else if(byte == '\n') {
			break;
		} else {
			*p = byte;
			p++;
		}
	}

	*p = '\0';
	return buf;
}

char* get_line(pfile fp, char* buf, size_t buf_size)
{
	char byte;
	char* p;
	p = buf;

	while(p - buf < buf_size - 1) {
		if(read(fp, &byte, 1) == 0) {
			break;
		}

		if(byte == '\n') {
			break;
		} else {
			*p = byte;
			p++;
		}
	}

	*p = '\0';
	return buf;
}

void jmp_space(pfile fp)
{
	char byte;

	while(read(fp, &byte, 1) != 0) {
		if(byte != '\t' && byte != ' ' && byte != '\n') {
			seek(fp, -1, FILE_POS_CURRENT);
			break;
		}
	}

	return;
}

bool check_version(pfile fp)
{
	char buf[64];
	u32 ver;
	u32 num;
	char* p;
	jmp_space(fp);

	if(strcmp(get_word(fp, buf, 64), GET_REAL_ADDR("sanlo_version")) != 0) {
		return false;
	}

	jmp_space(fp);
	num = 0;
	ver = 0;
	get_word(fp, buf, 64);

	for(p = buf; *p != '\0'; p++) {
		if(*p >= '0' && *p <= '9') {
			num = num * 10 + (*p - '0');
		} else if(*p == '.') {
			ver = ver * 0x100 + num;
		} else {
			return false;
		}
	}

	if(ver > SANLO_VERSION) {
		return false;
	}

	return true;
}

bool analyse_cfg_file(pfile fp, pmenu p_boot_menu)
{
	char* buf;
	pmenu_item p_item;
	buf = malloc(1024);
	memset(p_boot_menu, 0, sizeof(menu));

	while(1) {
		jmp_space(fp);
		get_word(fp, buf, 1024);

		if(strcmp(buf, GET_REAL_ADDR("")) == 0) {
			break;
		}

		if(strcmp(buf, GET_REAL_ADDR("menu_entry")) != 0) {
			break;
		}

		jmp_space(fp);
		p_item = malloc(sizeof(menu_item));
		//Get name
		get_word(fp, buf, 1024);
		p_item->name = malloc(strlen(buf) + 1);
		strcpy(p_item->name, buf);
		//Get kernel path
		jmp_space(fp);
		get_word(fp, buf, 1024);

		if(strcmp(buf, GET_REAL_ADDR("kernel")) != 0) {
			return false;
		}

		jmp_space(fp);
		get_word(fp, buf, 1024);
		p_item->kernel_path = malloc(strlen(buf) + 1);
		strcpy(p_item->kernel_path, buf);
		//Get paramters
		jmp_space(fp);
		get_line(fp, buf, 1024);
		p_item->paramter = malloc(strlen(buf) + 1);
		strcpy(p_item->paramter, buf);

		if(p_boot_menu->menu_list == NULL) {
			p_item->p_prev = p_item;
			p_item->p_next = p_item;
			p_boot_menu->menu_list = p_item;
		} else {
			p_item->p_prev = p_boot_menu->menu_list->p_prev;
			p_item->p_next = p_boot_menu->menu_list;
			p_boot_menu->menu_list->p_prev->p_next = p_item;
			p_boot_menu->menu_list->p_prev = p_item;
		}

		(p_boot_menu->item_num)++;
	}

	free(buf);

	if(p_boot_menu->item_num == 0) {
		return false;
	}

	return true;
}

void print_menu(u16 line, pmenu p_boot_menu)
{
	u32 i;
	pmenu_item p_item;
	set_cursor_pos(line, 0);
	p_item = p_boot_menu->menu_list;

	for(i = 0; i < p_boot_menu->item_num; i++) {
		print_string(
			GET_REAL_ADDR("\t"),
			FG_BRIGHT_WHITE | BG_BLACK,
			BG_BLACK);

		if(i == p_boot_menu->selected_item) {
			print_string(
				p_item->name,
				FG_BLACK | BG_WHITE,
				BG_BLACK);
		} else {
			print_string(
				p_item->name,
				FG_BRIGHT_WHITE | BG_BLACK,
				BG_BLACK);
		}

		print_string(
			GET_REAL_ADDR("\n"),
			FG_BRIGHT_WHITE | BG_BLACK,
			BG_BLACK);
		p_item = p_item->p_next;
	}

	return;
}
