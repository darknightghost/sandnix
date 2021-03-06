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
    core_kconsole_print_info("\nEntering kernel main...\n");

    core_kconsole_print_info("\nInitializing exception handling framework...\n");
    core_exception_init();

    core_kconsole_print_info("\nInitializing mm module...\n");
    core_mm_init();

    core_kconsole_print_info("\nInitializing kernel clock...\n");
    core_kclock_init();

    core_kconsole_print_info("\nInitializing process management...\n");
    core_pm_init();
    core_exception_regist_thrd_ref();

    void test();
    test();

    while(1);
}


void* thread_func2(u32 thread_id, void* p_args);
void* thread_func1(u32 thread_id, void* p_args)
{
    core_kconsole_print_info("thread1 id = %p, arg = %p\n",
                             thread_id, p_args);
    u32 thread2_id = core_pm_thread_create(thread_func2, 0, PRIORITY_KRNL_NORMAL,
                                           (void*)0x02);

    u64 ns = 100000000;
    core_pm_sleep(&ns);

    u32 retval;

    u32 joined_id = core_pm_join(true, thread2_id, (void**)&retval);

    if(joined_id == thread2_id) {
        core_kconsole_print_info("thread 2 joined, return value = %u\n", retval);
    }

    while(true);

    return NULL;
}

void* thread_func2(u32 thread_id, void* p_args)
{
    core_kconsole_print_info("thread2 id = %p, arg = %p\n",
                             thread_id, p_args);

    return (void*)1;
}

void test()
{
    core_kconsole_print_info("\nCondition variable test\n");
    core_pm_set_currnt_thrd_priority(PRIORITY_HIGHEST);
    core_pm_thread_create(thread_func1, 0, PRIORITY_KRNL_NORMAL, (void*)0x01);
    core_pm_set_currnt_thrd_priority(PRIORITY_IDLE);
}

