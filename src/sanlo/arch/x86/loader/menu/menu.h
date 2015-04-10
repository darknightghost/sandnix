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

#ifndef	MENU_H_INCLUDE
#define	MENU_H_INCLUDE

#include "../types.h"

typedef	struct	_menu_item {
	struct	_menu_item*	p_prev;
	struct	_menu_item*	p_next;
	char*				name;
	char*				kernel_path;
	char*				parameter;
} menu_item, *pmenu_item, *menu_item_list;

typedef	struct	_menu {
	u32				selected_item;
	u32				item_num;
	menu_item_list	menu_list;
} menu, *pmenu;

void	show_menu();

#endif	//!	MENU_H_INCLUDE
