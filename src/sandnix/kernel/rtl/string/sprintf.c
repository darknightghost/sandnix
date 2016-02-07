/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "string.h"

#define	TYPE_MASK				0xF0

#define	FLAG_LEFT_ALIGN			0x01
#define	FLAG_SIGN				0x02
#define	FLAG_ZERO				0x04
#define	FLAG_SPACE				0x08
#define	FLAG_POUND				0x10

#define	TYPE_CHAR				0x00

#define TYPE_INT				0x10
#define	TYPE_INT_16				0x11
#define	TYPE_INT_32				0x12
#define	TYPE_INT_64				0x13

#define TYPE_UINT				0x20
#define	TYPE_UINT_16			0x20
#define	TYPE_UINT_32			0x21
#define	TYPE_UINT_64			0x22

#define TYPE_OCTAL				0x30
#define	TYPE_OCTAL_16			0x30
#define	TYPE_OCTAL_32			0x32
#define	TYPE_OCTAL_64			0x33

#define	TYPE_HEX				0x40
#define	TYPE_HEX_16_L			0x40
#define	TYPE_HEX_32_L			0x42
#define	TYPE_HEX_64_L			0x43
#define	TYPE_HEX_16_C			0x44
#define	TYPE_HEX_32_C			0x45
#define	TYPE_HEX_64_C			0x46

#define	TYPE_STRING				0x50

#define	TYPE_POINTER			0x60
#define	TYPE_POINTER_L			0x60
#define	TYPE_POINTER_C			0x61

//Not realized.
#define	TYPE_UNKNOW				0xFF

static	u32			get_flag(char** p_p_fmt);
static	u32			get_width(char** p_p_fmt);
static	u32			get_prec(char** p_p_fmt);
static	u32			get_type(char** p_p_fmt);
static	u32			write_buf(char* dest, size_t size, char** p_p_output, char* src);
static	void		get_num_str(u32 flag, u64 num, u32 width, u32 prec, int sign, u32 num_sys, bool capital, char* buf);

u32 rtl_snprintf(char* buf, size_t buf_size, char* fmt, ...)
{
	va_list args;
	u32 ret;
	va_start(args, fmt);
	ret = rtl_vnprintf(buf, buf_size, fmt, args);
	va_end(args);
	return ret;
}

u32 rtl_vnprintf(char* buf, size_t buf_size, char* fmt, va_list args)
{
	char* p_fmt;
	char* p_output;
	char num_buf[128];
	u32 flag;
	u32 width;
	u32 prec;
	u32 type;
	int sign;
	u32 i;
	char* data_str;
	size_t data_str_len;
	s64 data_signed;
	u64 data_unsigned;
	bool capital;

	p_fmt = fmt;
	p_output = buf;

	while(*p_fmt != '\0') {
		if(*p_fmt == '%') {
			p_fmt++;

			//"%%"
			if(*p_fmt == '%') {
				*p_output = '%';
				p_output++;
				p_fmt++;

				if((u32)(p_output - buf) > buf_size - 2) {
					*p_output = '\0';
					return p_output - buf;
				}

				continue;

			} else {
				flag = get_flag(&p_fmt);
				width = get_width(&p_fmt);
				prec = get_prec(&p_fmt);
				type = get_type(&p_fmt);

				switch(type & TYPE_MASK) {
					case TYPE_CHAR:

						//Width
						if(width <= 1) {
							num_buf[0] = va_arg(args, char);
							num_buf[1] = '\0';

						} else if(flag & FLAG_LEFT_ALIGN) {
							num_buf[0] = va_arg(args, char);

							for(i = 1; i < width; i++) {
								num_buf[i] = ' ';
							}

							num_buf[i] = '\0';

						} else {
							width = width - 1;

							for(i = 0; i < width; i++) {
								num_buf[i] = ' ';
							}

							num_buf[i] = va_arg(args, char);
							i++;
							num_buf[i] = '\0';
						}

						write_buf(buf, buf_size, &p_output, num_buf);

						if((u32)(p_output - buf) > buf_size - 2) {
							*p_output = '\0';
							return p_output - buf;
						}

						break;

					case TYPE_INT:
						switch(type) {
							case TYPE_INT_16:
								data_signed = (va_arg(args, s16));
								break;

							case TYPE_INT_32:
								data_signed = (va_arg(args, s32));
								break;

							case TYPE_INT_64:
								data_signed = (va_arg(args, s64));
								break;
						}

						//Sign
						if(data_signed < 0) {
							sign = -1;
							data_signed = -data_signed;

						} else {
							sign = 1;
						}

						get_num_str(flag, (u64)data_signed, width, prec,
						            sign, 10, false, num_buf);
						write_buf(buf, buf_size, &p_output, num_buf);

						if((u32)(p_output - buf) > buf_size - 2) {
							*p_output = '\0';
							return p_output - buf;
						}

						break;

					case TYPE_UINT:
						switch(type) {
							case TYPE_UINT_16:
								data_unsigned = va_arg(args, u16);
								break;

							case TYPE_UINT_32:
								data_unsigned = va_arg(args, u32);
								break;

							case TYPE_UINT_64:
								data_unsigned = va_arg(args, u64);
								break;
						}

						get_num_str(flag, data_unsigned, width, prec,
						            1, 10, false, num_buf);
						write_buf(buf, buf_size, &p_output, num_buf);

						if((u32)(p_output - buf) > buf_size - 2) {
							*p_output = '\0';
							return p_output - buf;
						}

						break;

					case TYPE_OCTAL:
						switch(type) {
							case TYPE_OCTAL_16:
								data_unsigned = va_arg(args, u16);
								break;

							case TYPE_OCTAL_32:
								data_unsigned = va_arg(args, u32);
								break;

							case TYPE_OCTAL_64:
								data_unsigned = va_arg(args, u64);
								break;
						}

						get_num_str(flag, data_unsigned, width, prec,
						            1, 8, false, num_buf);
						write_buf(buf, buf_size, &p_output, num_buf);

						if((u32)(p_output - buf) > buf_size - 2) {
							*p_output = '\0';
							return p_output - buf;
						}

						break;

					case TYPE_HEX:
						switch(type) {
							case TYPE_HEX_16_L:
								data_unsigned = va_arg(args, u16);
								capital = false;
								break;

							case TYPE_HEX_32_L:
								data_unsigned = va_arg(args, u32);
								capital = false;
								break;

							case TYPE_HEX_64_L:
								capital = false;
								data_unsigned = va_arg(args, u64);
								break;

							case TYPE_HEX_16_C:
								capital = true;
								data_unsigned = va_arg(args, u16);
								break;

							case TYPE_HEX_32_C:
								capital = true;
								data_unsigned = va_arg(args, u32);
								break;

							case TYPE_HEX_64_C:
								capital = true;
								data_unsigned = va_arg(args, u64);
								break;
						}

						get_num_str(flag, data_unsigned, width, prec,
						            1, 16, capital, num_buf);
						write_buf(buf, buf_size, &p_output, num_buf);

						if((u32)(p_output - buf) > buf_size - 2) {
							*p_output = '\0';
							return p_output - buf;
						}

						break;

					case TYPE_STRING:
						//Width
						data_str = va_arg(args, char*);

						if(data_str == NULL) {
							data_str = "<NULL>";
						}

						data_str_len = rtl_strlen(data_str);

						if(data_str_len < width) {
							if(flag & FLAG_LEFT_ALIGN) {
								write_buf(buf, data_str_len, &p_output, data_str);

								for(i = 0; i < width - data_str_len; i++) {
									write_buf(buf, 1, &p_output, " ");
								}

							} else {
								for(i = 0; i < width - data_str_len; i++) {
									write_buf(buf, 1, &p_output, " ");
								}

							}
						}

						write_buf(buf, buf_size, &p_output, data_str);

						break;

					case TYPE_POINTER:
						#ifdef	X86
						data_unsigned = va_arg(args, u32);
						prec = 8;
						#endif
						#ifdef AMD64
						data_unsigned = va_arg(args, u64);
						prec = 16;
						#endif

						switch(type) {
							case TYPE_POINTER_L:
								capital = false;
								break;

							case TYPE_POINTER_C:
								capital = true;
								break;
						}

						flag |= FLAG_POUND;
						get_num_str(flag, data_unsigned, width, prec,
						            1, 16, capital, num_buf);
						write_buf(buf, buf_size, &p_output, num_buf);

						if((u32)(p_output - buf) > buf_size - 2) {
							*p_output = '\0';
							return p_output - buf;
						}

						break;
				}
			}

		}

		else {
			//Copy characters
			*p_output = *p_fmt;
			p_output++;
			p_fmt++;

			if((u32)(p_output - buf) > buf_size - 2) {
				*p_output = '\0';
				return p_output - buf;
			}
		}
	}

	*p_output = '\0';
	return p_output - buf;
}

u32 get_flag(char** p_p_fmt)
{
	u32 ret = 0;

	while(1) {
		if(**p_p_fmt == '-') {
			ret |= FLAG_LEFT_ALIGN;

		} else if(**p_p_fmt == '+') {
			ret |= FLAG_SIGN;

		} else if(**p_p_fmt == '0') {
			ret |= FLAG_ZERO;

		} else if(**p_p_fmt == ' ') {
			ret |= FLAG_SPACE;

		} else if(**p_p_fmt == '#') {
			ret |= FLAG_POUND;

		} else {
			return ret;
		}

		(*p_p_fmt)++;
	}
}

u32 get_width(char** p_p_fmt)
{
	u32 ret;

	if(**p_p_fmt < '0' || **p_p_fmt > '9') {
		return 0;
	}

	ret = (s32)rtl_atoi(*p_p_fmt, 10);

	while(**p_p_fmt >= '0' &&**p_p_fmt <= '9') {
		(*p_p_fmt)++;
	}

	return ret;
}

u32 get_prec(char** p_p_fmt)
{
	u32 ret;

	if(**p_p_fmt != '.') {
		return 0;
	}

	(*p_p_fmt)++;
	ret = (s32)rtl_atoi(*p_p_fmt, 10);

	while(**p_p_fmt >= '0' &&**p_p_fmt <= '9') {
		(*p_p_fmt)++;
	}

	return ret;
}

u32 get_type(char** p_p_fmt)
{
	bool ll_flag;
	bool h_flag;
	u32 ret;
	ll_flag = false;
	h_flag = false;

	//Get length
	if(**p_p_fmt == 'l') {
		(*p_p_fmt)++;

		if(**p_p_fmt == 'l') {
			(*p_p_fmt)++;
			ll_flag = true;

		}

	} else if(**p_p_fmt == 'h') {
		(*p_p_fmt)++;
		h_flag = true;
	}

	switch(**p_p_fmt) {
		case 'i':
		case 'd':
			if(h_flag) {
				ret = TYPE_INT_16;

			} else if(ll_flag) {
				ret = TYPE_INT_64;

			} else {
				ret = TYPE_INT_32;
			}

			break;

		case 'o':
			if(h_flag) {
				ret = TYPE_OCTAL_16;

			} else if(ll_flag) {
				ret = TYPE_OCTAL_64;

			} else {
				ret = TYPE_OCTAL_32;
			}

			break;

		case 'u':
			if(h_flag) {
				ret = TYPE_UINT_16;

			} else if(ll_flag) {
				ret = TYPE_UINT_64;

			} else {
				ret = TYPE_UINT_32;
			}

			break;

		case 'x':
			if(h_flag) {
				ret = TYPE_HEX_16_L;

			} else if(ll_flag) {
				ret = TYPE_HEX_64_L;

			} else {
				ret = TYPE_HEX_32_L;
			}

			break;

		case 'X':
			if(h_flag) {
				ret = TYPE_HEX_16_C;

			} else if(ll_flag) {
				ret = TYPE_HEX_64_C;

			} else {
				ret = TYPE_HEX_32_C;
			}

			break;

		case 'c':
			ret = TYPE_CHAR;
			break;

		case 's':
			ret = TYPE_STRING;
			break;

		case 'p':
			ret = TYPE_POINTER_L;
			break;

		case 'P':
			ret = TYPE_POINTER_C;
			break;

		default:
			ret = TYPE_UNKNOW;
	}

	(*p_p_fmt)++;
	return ret;
}

u32 write_buf(char* dest, size_t size, char** p_p_output, char* src)
{
	u32 len;
	len = rtl_strlen(src);

	if(len + (*p_p_output - dest) > size - 2) {
		len = size - 2 - (*p_p_output - dest);
	}

	rtl_memcpy(*p_p_output, src, len);
	(*p_p_output) += len;
	return len;
}

void get_num_str(u32 flag, u64 num, u32 width, u32 prec,
                 int sign, u32 num_sys, bool capital, char* buf)
{
	size_t num_len;
	u32 len_to_move;
	u32 i;

	//Convert number to string
	rtl_itoa(buf, num, num_sys, capital);
	num_len = rtl_strlen(buf);

	//Prec
	if(num_len < prec) {
		rtl_memmove(
		    buf + (prec - num_len),
		    buf,
		    num_len + 1);

		for(i = 0; i < prec - num_len; i++) {
			buf[i] = '0';
		}

		num_len = prec;
	}

	//FLAG_POUND
	if(flag & FLAG_POUND) {
		switch(num_sys) {
			case 8:
				rtl_memmove(
				    buf + 1,
				    buf,
				    num_len + 1);
				num_len++;
				buf[0] = '0';
				break;

			case 16:
				rtl_memmove(
				    buf + 2,
				    buf,
				    num_len + 1);
				buf[0] = '0';
				buf[1] = 'x';
				num_len += 2;
				break;
		}
	}

	//FLAG_SIGN
	if(sign < 0) {
		rtl_memmove(
		    buf + 1,
		    buf,
		    num_len + 1);
		buf[0] = '-';
		num_len++;

	} else if((flag & FLAG_SIGN) || (flag & FLAG_SPACE)) {
		rtl_memmove(
		    buf + 1,
		    buf,
		    num_len + 1);

		if(flag & FLAG_SIGN) {
			buf[0] = '+';

		} else {
			buf[0] = ' ';
		}

		num_len++;
	}

	//Width
	if(num_len < width) {
		//FLAG_LEFT_ALIGN
		if(flag & FLAG_LEFT_ALIGN) {
			while(num_len < width) {
				buf[num_len] = ' ';
				num_len++;
			}

			buf[num_len] = '\0';

		} else {
			len_to_move = width - num_len;
			rtl_memmove(
			    buf + len_to_move,
			    buf,
			    num_len + 1);

			if(flag & FLAG_ZERO) {
				rtl_memset(buf, '0', len_to_move);

			} else {
				rtl_memset(buf, ' ', len_to_move);
			}
		}
	}

	return;
}

