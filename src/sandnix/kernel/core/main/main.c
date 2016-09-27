/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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

#include "main.h"
#include "../../hal/early_print/early_print.h"
#include "../kconsole/kconsole.h"

void core_main_main()
{
    hal_early_print_printf("\nEntering kernel main...\n");

    core_kconsole_print_panic("%s\n", "Panic info.");
    core_kconsole_print_err("%s\n", "Error info.");
    core_kconsole_print_warning("%s\n", "Warning info.");
    core_kconsole_print_info("%s\n", "Normal info.");
    core_kconsole_print_debug("%s\n", "Debugging info.");

    while(1);
}
