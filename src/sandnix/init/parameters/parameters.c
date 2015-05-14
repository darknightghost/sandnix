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

#include "parameters.h"
#include "../../debug/debug.h"
#include "../../mm/mm.h"
#include "../../rtl/rtl.h"
#include "../../exceptions/exceptions.h"

static	bool	get_parameter_name(char* buf, char** p);
static	bool	get_parameter_value(char* buf, char** p);
static	void	check_parameters();

param_info		parameters;


void get_parameters()
{
	char* p_param;
	char* buf;

	rtl_memset(&parameters, 0, sizeof(param_info));
	p_param = *(char**)(KERNEL_PARAMETER);
	buf = mm_heap_alloc(512, NULL);

	dbg_print("Analysing parameters...\n");

	while(get_parameter_name(buf, &p_param)) {
		if(rtl_strcmp(buf, "root") == 0) {
			//Root partition
			if(get_parameter_value(buf, &p_param)) {
				dbg_print("%s,%d\n", buf, rtl_strlen(buf));
				parameters.root_partition = mm_heap_alloc(rtl_strlen(buf) + 1, NULL);
				rtl_strcpy_s(parameters.root_partition, rtl_strlen(buf) + 1, buf);
			}

		} else if(rtl_strcmp(buf, "driver_init") == 0) {
			//Driver_init
			if(get_parameter_value(buf, &p_param)) {
				dbg_print("%s,%d\n", buf, rtl_strlen(buf));
				parameters.driver_init = mm_heap_alloc(rtl_strlen(buf) + 1, NULL);
				rtl_strcpy_s(parameters.driver_init, rtl_strlen(buf) + 1, buf);
			}

		} else if(rtl_strcmp(buf, "init") == 0) {
			//Init
			if(get_parameter_value(buf, &p_param)) {
				dbg_print("%s,%d\n", buf, rtl_strlen(buf));
				parameters.init = mm_heap_alloc(rtl_strlen(buf) + 1, NULL);
				rtl_strcpy_s(parameters.init, rtl_strlen(buf) + 1, buf);
			}

		} else {
			//Ignore
			get_parameter_value(buf, &p_param);
		}
	}

	mm_heap_free(buf, NULL);
	check_parameters();
	return;
}

bool get_parameter_name(char* buf, char** p_p_param)
{
	char* p;
	char* p_buf;

	p = *p_p_param;

	if(*p == '\0') {
		//No more parameters
		*buf = '\0';
		return false;
	}

	//Jump spaces
	while(*p == ' '
	      || *p == '\t') {
		p++;
	}

	//Get name
	for(p_buf = buf;
	    (*p >= '0' && *p <= '9')
	    || (*p >= 'a' && *p <= 'z')
	    || (*p >= 'A' && *p <= 'Z')
	    || *p == '_';
	    p++, p_buf++) {
		*p_buf = *p;
	}

	*p_buf = '\0';

	if(p_buf == buf) {
		return false;
	}

	*p_p_param = p;
	return true;
}

bool get_parameter_value(char* buf, char** p_p_param)
{
	char* p;
	char* p_buf;

	bool quote_flag;
	bool backslash_flag;

	p = *p_p_param;

	//Jump spaces
	while(*p == ' '
	      || *p == '\t') {
		p++;
	}

	if(*p != '=') {
		//No more parameters
		return false;
	}

	p++;

	//Jump spaces
	while(*p == ' '
	      || *p == '\t') {
		p++;
	}

	quote_flag = false;
	backslash_flag = false;

	//Get name
	for(p_buf = buf;
	    *p != '\0' && (quote_flag || *p != ' ');
	    p++) {
		if(*p == '\\') {
			if(backslash_flag) {
				*p_buf = *p;
				p_buf++;
				backslash_flag = false;

			} else {
				backslash_flag = true;
			}

		} else if(*p == '\"') {
			if(backslash_flag) {
				*p_buf = *p;
				p_buf++;
				backslash_flag = false;

			} else {
				quote_flag = !quote_flag;
			}

		} else {
			*p_buf = *p;
			p_buf++;
			backslash_flag = false;
		}
	}

	*p_buf = '\0';
	*p_p_param = p;
	return true;
}

void check_parameters()
{
	/*if(parameters.root_partition == NULL) {
		excpt_panic(EXCEPTION_UNSPECIFIED_ROOT_PARTITION,
		            "%s\n", "You should use root=(hdm,n) to specify a root partition.");

	} else if(parameters.driver_init == NULL) {
		parameters.driver_init = "/driver/init";

	} else if(parameters.init == NULL) {
		parameters.init = "/sbin/init";
	}*/

	dbg_print("root partition=%s\n", parameters.root_partition);
	dbg_print("driver init=%s\n", parameters.driver_init);
	dbg_print("init=%s\n", parameters.init);

	return;
}
