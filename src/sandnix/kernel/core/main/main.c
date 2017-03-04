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
#include "../exception/exception.h"
#include "../mm/mm.h"
#include "../kclock/kclock.h"
#include "../pm/pm.h"

void core_main_main()
{
    core_kconsole_init();
    core_kconsole_print_info("Entering kernel main...\n");

    core_kconsole_print_info("\nInitializing exception handling framework...\n");
    core_exception_init();

    core_kconsole_print_info("\nInitializing mm module...\n");
    core_mm_init();

    core_kconsole_print_info("\nInitializing kernel clock...\n");
    core_kclock_init();

    core_kconsole_print_info("\nInitializing process management...\n");
    core_pm_init();

    void test();
    test();

    while(1);
}

void test()
{
}
