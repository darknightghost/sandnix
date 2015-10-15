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

#include "../../common/common.h"
#include "../debug/debug.h"
#include "parameters/parameters.h"
#include "../exceptions/exceptions.h"
#include "../io/io.h"
#include "../mm/mm.h"
#include "../pm/pm.h"
#include "../rtl/rtl.h"
#include "../msg/msg.h"
#include "../syscall/syscall.h"


void test();

void kernel_main()
{
	dbg_init();

	dbg_cls();

	dbg_print("%s", "Sandnix 0.0.1 kernel loaded.\n");

	io_set_crrnt_int_level(INT_LEVEL_DISPATCH);

	//Initialize
	get_kernel_param();
	io_init();
	excpt_init();
	mm_init();
	pm_init();
	//io_int_msg_init();

	//Create daemon threads
	io_enable_interrupt();
	dbg_print("Creating interrupt dispatcher thread...\n");
	pm_create_thrd(io_dispatch_int, true, false, INT_LEVEL_EXCEPTION, NULL);

	//Initialize
	msg_init();
	vfs_init();

	test();

	io_int_msg_init();
	syscall_init();

	io_set_crrnt_int_level(INT_LEVEL_USR_HIGHEST);

	//Create driver_init process

	//IDLE
	io_set_crrnt_int_level(INT_LEVEL_IDLE);

	while(1);

	return;
}

void recv_theard(u32 id, void* p_args)
{
	u32 msg_queue_id;
	pmsg_t p_msg;

	dbg_print("Recive thread created,thread id = %u.\n", id);
	msg_queue_id = (u32)p_args;

	while(1) {
		msg_recv(&p_msg, msg_queue_id, true);

		if(!OPERATE_SUCCESS) {
			dbg_print("Failed");

			while(1);
		}

		dbg_print("Received message\n");

		if(p_msg->message == MSG_CLOSE) {
			msg_complete(p_msg, ESUCCESS);
			break;

		} else {
			msg_complete(p_msg, ESUCCESS);
		}
	}

	dbg_print("Recive thread exited.\n", id);
	pm_exit_thrd(0);
}

void test()
{
	u32 queue_id;
	pmsg_t p_msg;
	u32 i;
	k_status complete_status;
	u32 result;

	io_set_crrnt_int_level(INT_LEVEL_USR_HIGHEST);
	dbg_print("\nMessage test started\n");

	queue_id = msg_queue_create();
	dbg_print("Message queue %u created.\n", queue_id);

	pm_create_thrd(recv_theard, true, false, INT_LEVEL_USR_HIGHEST, (void*)queue_id);

	for(i = 0; i < 30; i++) {
		msg_create(&p_msg, sizeof(msg_t));

		if(!OPERATE_SUCCESS) {
			dbg_print("Failed");

			while(1);
		}

		dbg_print("Created\n");

		if(i != 29) {
			p_msg->message = MSG_OPEN;

		} else {
			p_msg->message = MSG_CLOSE;
		}

		p_msg->flags.flags = MFLAG_DIRECTBUF;

		msg_send(p_msg,
		         queue_id,
		         &result,
		         &complete_status);

		if(!OPERATE_SUCCESS) {
			dbg_print("Failed");

			while(1);
		}

		dbg_print("MSTATUS: %d\ncomplete stat: %d\n", result, complete_status);
	}

	dbg_print("\n/test\n");
	msg_queue_destroy(queue_id);
	pm_join(0);

	while(1);
}
