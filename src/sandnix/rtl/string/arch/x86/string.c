/*
	Copyright 2015,袁行 <yuanxing_nepu@126.com>

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

#include "../../string.h"
#include "../../../../mm/mm.h"


#define	FLAG_LEFT_ALIGN			0x01
#define	FLAG_SIGN				0x02
#define	FLAG_ZERO				0x04
#define	FLAG_SPACE				0x08
#define	FLAG_POUND				0x10

#define	TYPE_CHAR				0x00
#define	TYPE_INT_16				0x01
#define	TYPE_INT_32				0x02
#define	TYPE_INT_64				0x03

#define	TYPE_UINT_16			0x10
#define	TYPE_UINT_32			0x11
#define	TYPE_UINT_64			0x12

#define	TYPE_OCTAL_16			0x20
#define	TYPE_OCTAL_32			0x22
#define	TYPE_OCTAL_64			0x23

#define	TYPE_HEX_16_L			0x30
#define	TYPE_HEX_32_L			0x32
#define	TYPE_HEX_64_L			0x33
#define	TYPE_HEX_16_C			0x34
#define	TYPE_HEX_32_C			0x35
#define	TYPE_HEX_64_C			0x36

//Float & double are not realized.
#define	TYPE_FLOAT_32			0x40
#define	TYPE_FLOAT_64			0x41
#define	TYPE_E_FLOAT_32_C		0x42
#define	TYPE_E_FLOAT_64_C		0x43
#define	TYPE_G_FLOAT_32_C		0x44
#define	TYPE_G_FLOAT_64_C		0x45
#define	TYPE_E_FLOAT_32_L		0x46
#define	TYPE_E_FLOAT_64_L		0x47
#define	TYPE_G_FLOAT_32_L		0x48
#define	TYPE_G_FLOAT_64_L		0x49

#define	TYPE_STRING				0x50

#define	TYPE_POINTER			0x60

//Not realized.
#define	TYPE_NUM				0x70


#define	TYPE_UNKNOW				0xFF

static	u32 		get_flag(char** p_p_fmt);
static	u32			get_width(char** p_p_fmt);
static	u32			get_prec(char** p_p_fmt);
static	u32			get_type(char** p_p_fmt);
static	u32			write_buf(char* dest, size_t size, char** p_p_output, char* src);

void* rtl_memcpy(void* dest, void* src, size_t len)
{
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%edi\n\t"
	    "movl		%1,%%esi\n\t"
	    "movl		%2,%%ecx\n\t"
	    "rep		movsb\n\t"
	    ::"m"(dest), "m"(src), "m"(len));
	return dest;
}

void* rtl_memset(void* dest, u8 val, size_t len)
{
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%edi\n\t"
	    "movl		%1,%%ecx\n\t"
	    "movb		%2,%%al\n\t"
	    "rep		stosb\n\t"
	    ::"m"(dest), "m"(len), "m"(val));
	return dest;
}

void* rtl_memmove(void* dest, void* src, size_t n)
{
	if(dest < src) {
		__asm__ __volatile__(
		    "cld\n\t"
		    "movl		%0,%%ecx\n\t"
		    "movl		%1,%%esi\n\t"
		    "movl		%2,%%edi\n\t"
		    "rep		movsb"
		    ::"m"(n), "m"(src), "m"(dest));

	} else {
		src = src + n - 1;
		dest = dest + n - 1;
		__asm__ __volatile__(
		    "std\n\t"
		    "movl		%0,%%ecx\n\t"
		    "movl		%1,%%esi\n\t"
		    "movl		%2,%%edi\n\t"
		    "rep		movsb\n\t"
		    ::"m"(n), "m"(src), "m"(dest));
	}

	return dest;
}

u32 rtl_strlen(char* str)
{
	u32 ret;
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		$0xFFFFFFFF,%%ecx\n\t"
	    "movl		%1,%%edi\n\t"
	    "xorl		%%eax,%%eax\n\t"
	    "repnz		scasb\n\t"
	    "movl		$0xFFFFFFFF,%%eax\n\t"
	    "subl		%%ecx,%%eax\n\t"
	    "decl		%%eax\n\t"
	    :"=%%eax"(ret)
	    :"m"(str));
	return ret;
}

char* rtl_strcpy_s(char* dest, size_t buf_size, char* src)
{
	u32 len;

	//How many characters to copy
	if(rtl_strlen(src) > buf_size - 1) {
		len = buf_size - 1;

	} else {
		len = rtl_strlen(src);
	}

	//Copy string
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%ecx\n\t"
	    "movl		%1,%%esi\n\t"
	    "movl		%2,%%edi\n\t"
	    "rep		movsb\n\t"
	    "movb		$0,(%%edi)"
	    ::"m"(len), "m"(src), "m"(dest));
	return dest;
}

s32 rtl_strcmp(char* str1, char* str2)
{
	s32 ret;
	char* p1;
	char* p2;

	for(p1 = str1, p2 = str2;
	    *p1 != '\0' && *p2 != '\0';
	    p1++, p2++) {
		ret = *p1 - *p2;

		if(ret != 0) {
			break;
		}
	}

	if(*p1 != '\0' || *p2 != '\0') {
		ret = *p1 - *p2;
	}

	return ret;
}

char* rtl_strcat_s(char* dest, size_t buf_size, char* src)
{
	u32 len;

	//How many characters to copy
	if(rtl_strlen(src) + rtl_strlen(dest) > buf_size - 1) {
		len = buf_size - 1 - rtl_strlen(dest);

	} else {
		len = rtl_strlen(src);
	}

	//Copy string
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%ecx\n\t"
	    "movl		%1,%%esi\n\t"
	    "movl		%2,%%edi\n\t"
	    "rep		movsb\n\t"
	    "movb		$0,(%%edi)"
	    ::"m"(len), "m"(src), "m"(dest));
	return dest;
}

bool rtl_is_sub_string(char* str, char* substr)
{
	char* p;
	char* p_sub;

	for(p_sub = substr, p = str;
	    *p_sub != '\0';
	    p++, p_sub++) {
		if(*p != *p_sub) {
			return false;
		}
	}

	return true;
}

u32 rtl_sprintf_s(char* buf, size_t buf_size, char* fmt, ...)
{
	va_list args;
	u32 ret;
	va_start(args, fmt);
	ret = rtl_vprintf_s(buf, buf_size, fmt, args);
	va_end(args);
	return ret;
}

u32 rtl_vprintf_s(char* buf, size_t buf_size, char* fmt, va_list args)
{
	char* p_fmt;
	char* p_output;
	char num_buf[128];
	u32 num_len;
	u32 flag;
	u32 width;
	u32 prec;
	u32 type;
	p_fmt = fmt;
	p_output = buf;
	u32 i;
	s32 sign;
	s16 data_s16;
	s32 data_s32;
	s64 data_s64;
	char* data_str;

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

				switch(type) {
				//%c
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

				//hd
				case TYPE_INT_16:
					data_s16 = va_arg(args, s16);

					if(data_s16 > 0) {
						sign = 1;

					} else {
						sign = -1;
					}

					rtl_itoa(num_buf, data_s32 * sign);
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						//Sign
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_SIGN) {
								rtl_memmove(
								    num_buf + 1,
								    num_buf,
								    num_len + 1);

								if(sign > 0) {
									num_buf[0] = '+';

								} else {
									num_buf[0] = '-';
								}

								num_len++;

							} else if(sign < 0) {
								rtl_memmove(
								    num_buf + 1,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '-';
								num_len++;
							}

							for(i = num_len; i < width; i++) {
								num_buf[i] = ' ';
							}

							num_buf[i] = '\0';

						} else {
							rtl_memmove(
							    num_buf + (width - num_len),
							    num_buf,
							    num_len + 1);

							if(flag & FLAG_ZERO) {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = '0';
								}

							} else {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = ' ';
								}
							}

							//Sign
							if(flag & FLAG_SIGN) {
								if(sign > 0) {
									num_buf[0] = '+';

								} else {
									num_buf[0] = '-';
								}

							} else if(sign < 0) {
								num_buf[0] = '-';
							}
						}

					} else {
						//Sign
						if(flag & FLAG_SIGN) {
							rtl_memmove(
							    num_buf + 1,
							    num_buf,
							    num_len + 1);

							if(sign > 0) {
								num_buf[0] = '+';

							} else {
								num_buf[0] = '-';
							}

						} else if(sign < 0) {
							rtl_memmove(
							    num_buf + 1,
							    num_buf,
							    num_len + 1);
							num_buf[0] = '-';
						}
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//ld
				case TYPE_INT_32:
					data_s32 = va_arg(args, s32);

					if(data_s32 > 0) {
						sign = 1;

					} else {
						sign = -1;
					}

					rtl_itoa(num_buf, data_s32 * sign);
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						//Sign
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_SIGN) {
								rtl_memmove(
								    num_buf + 1,
								    num_buf,
								    num_len + 1);

								if(sign > 0) {
									num_buf[0] = '+';

								} else {
									num_buf[0] = '-';
								}

								num_len++;

							} else if(sign < 0) {
								rtl_memmove(
								    num_buf + 1,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '-';
								num_len++;
							}

							for(i = num_len; i < width; i++) {
								num_buf[i] = ' ';
							}

							num_buf[i] = '\0';

						} else {
							rtl_memmove(
							    num_buf + (width - num_len),
							    num_buf,
							    num_len + 1);

							if(flag & FLAG_ZERO) {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = '0';
								}

							} else {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = ' ';
								}
							}

							//Sign
							if(flag & FLAG_SIGN) {
								if(sign > 0) {
									num_buf[0] = '+';

								} else {
									num_buf[0] = '-';
								}

							} else if(sign < 0) {
								num_buf[0] = '-';
							}
						}

					} else {
						//Sign
						if(flag & FLAG_SIGN) {
							rtl_memmove(
							    num_buf + 1,
							    num_buf,
							    num_len + 1);

							if(sign > 0) {
								num_buf[0] = '+';

							} else {
								num_buf[0] = '-';
							}

						} else if(sign < 0) {
							rtl_memmove(
							    num_buf + 1,
							    num_buf,
							    num_len + 1);
							num_buf[0] = '-';
						}
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//lld
				case TYPE_INT_64:
					data_s64 = *(s64*)args;
					args += 8;

					if(data_s64 > 0) {
						sign = 1;

					} else {
						sign = -1;
					}

					rtl_itoa(num_buf, data_s32 * sign);
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						//Sign
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_SIGN) {
								rtl_memmove(
								    num_buf + 1,
								    num_buf,
								    num_len + 1);

								if(sign > 0) {
									num_buf[0] = '+';

								} else {
									num_buf[0] = '-';
								}

								num_len++;

							} else if(sign < 0) {
								rtl_memmove(
								    num_buf + 1,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '-';
								num_len++;
							}

							for(i = num_len; i < width; i++) {
								num_buf[i] = ' ';
							}

							num_buf[i] = '\0';

						} else {
							rtl_memmove(
							    num_buf + (width - num_len),
							    num_buf,
							    num_len + 1);

							if(flag & FLAG_ZERO) {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = '0';
								}

							} else {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = ' ';
								}
							}

							//Sign
							if(flag & FLAG_SIGN) {
								if(sign > 0) {
									num_buf[0] = '+';

								} else {
									num_buf[0] = '-';
								}

							} else if(sign < 0) {
								num_buf[0] = '-';
							}
						}

					} else {
						//Sign
						if(flag & FLAG_SIGN) {
							rtl_memmove(
							    num_buf + 1,
							    num_buf,
							    num_len + 1);

							if(sign > 0) {
								num_buf[0] = '+';

							} else {
								num_buf[0] = '-';
							}

						} else if(sign < 0) {
							rtl_memmove(
							    num_buf + 1,
							    num_buf,
							    num_len + 1);
							num_buf[0] = '-';
						}
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%hu
				case TYPE_UINT_16:
					rtl_itoa(num_buf, va_arg(args, u16));
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							for(i = num_len; i < width; i++) {
								num_buf[i] = ' ';
							}

							num_buf[i] = '\0';

						} else {
							rtl_memmove(
							    num_buf + (width - num_len),
							    num_buf,
							    num_len + 1);

							if(flag & FLAG_ZERO) {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = '0';
								}

							} else {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = ' ';
								}
							}
						}
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%lu
				case TYPE_UINT_32:
					rtl_itoa(num_buf, va_arg(args, u32));
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							for(i = num_len; i < width; i++) {
								num_buf[i] = ' ';
							}

							num_buf[i] = '\0';

						} else {
							rtl_memmove(
							    num_buf + (width - num_len),
							    num_buf,
							    num_len + 1);

							if(flag & FLAG_ZERO) {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = '0';
								}

							} else {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = ' ';
								}
							}
						}
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%llu
				case TYPE_UINT_64:
					rtl_itoa(num_buf, *(u64*)args);
					args += 8;
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							for(i = num_len; i < width; i++) {
								num_buf[i] = ' ';
							}

							num_buf[i] = '\0';

						} else {
							rtl_memmove(
							    num_buf + (width - num_len),
							    num_buf,
							    num_len + 1);

							if(flag & FLAG_ZERO) {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = '0';
								}

							} else {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = ' ';
								}
							}
						}
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%ho
				case TYPE_OCTAL_16:
					rtl_otoa(num_buf, va_arg(args, u16));
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_POUND) {
								//#
								rtl_memmove(
								    num_buf + 1,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '0';

								for(i = num_len + 1; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';

							} else {
								for(i = num_len; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';
							}

						} else {
							rtl_memmove(
							    num_buf + (width - num_len),
							    num_buf,
							    num_len + 1);

							if(flag & FLAG_ZERO) {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = '0';
								}

							} else {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i - 1] = '0';
							}
						}

					} else if(flag & FLAG_POUND) {
						//#
						rtl_memmove(
						    num_buf + 1,
						    num_buf,
						    num_len + 1);
						num_buf[0] = '0';
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%lo
				case TYPE_OCTAL_32:
					rtl_otoa(num_buf, va_arg(args, u32));
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_POUND) {
								//#
								rtl_memmove(
								    num_buf + 1,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '0';

								for(i = num_len + 1; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';

							} else {
								for(i = num_len; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';
							}

						} else {
							rtl_memmove(
							    num_buf + (width - num_len),
							    num_buf,
							    num_len + 1);

							if(flag & FLAG_ZERO) {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = '0';
								}

							} else {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i - 1] = '0';
							}
						}

					} else if(flag & FLAG_POUND) {
						//#
						rtl_memmove(
						    num_buf + 1,
						    num_buf,
						    num_len + 1);
						num_buf[0] = '0';
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%llo
				case TYPE_OCTAL_64:
					rtl_otoa(num_buf, *(u64*)args);
					args += 8;
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_POUND) {
								//#
								rtl_memmove(
								    num_buf + 1,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '0';

								for(i = num_len + 1; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';

							} else {
								for(i = num_len; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';
							}

						} else {
							rtl_memmove(
							    num_buf + (width - num_len),
							    num_buf,
							    num_len + 1);

							if(flag & FLAG_ZERO) {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = '0';
								}

							} else {
								for(i = 0; i < width - num_len; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i - 1] = '0';
							}
						}

					} else if(flag & FLAG_POUND) {
						//#
						rtl_memmove(
						    num_buf + 1,
						    num_buf,
						    num_len + 1);
						num_buf[0] = '0';
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%hx
				case TYPE_HEX_16_L:
					rtl_htoa(num_buf, va_arg(args, u16), false);
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_POUND) {
								//#
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '0';
								num_buf[1] = 'x';

								for(i = num_len + 2; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';

							} else {
								for(i = num_len; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';
							}

						} else {
							if(width - num_len >= 2
							   || !(flag & FLAG_POUND)) {
								rtl_memmove(
								    num_buf + (width - num_len),
								    num_buf,
								    num_len + 1);

								if(flag & FLAG_ZERO) {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = '0';
									}

									if(flag & FLAG_POUND) {
										num_buf[0] = '0';
										num_buf[1] = 'x';
									}

								} else {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = ' ';
									}

									if(flag & FLAG_POUND) {
										num_buf[i - 2] = '0';
										num_buf[i - 1] = 'x';
									}
								}

							} else {
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[i - 2] = '0';
								num_buf[i - 1] = 'x';
							}
						}

					} else if(flag & FLAG_POUND) {
						//#
						rtl_memmove(
						    num_buf + 2,
						    num_buf,
						    num_len + 1);
						num_buf[0] = '0';
						num_buf[1] = 'x';
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%lx
				case TYPE_HEX_32_L:
					rtl_htoa(num_buf, va_arg(args, u32), false);
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_POUND) {
								//#
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '0';
								num_buf[1] = 'x';

								for(i = num_len + 2; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';

							} else {
								for(i = num_len; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';
							}

						} else {
							if(width - num_len >= 2
							   || !(flag & FLAG_POUND)) {
								rtl_memmove(
								    num_buf + (width - num_len),
								    num_buf,
								    num_len + 1);

								if(flag & FLAG_ZERO) {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = '0';
									}

									if(flag & FLAG_POUND) {
										num_buf[0] = '0';
										num_buf[1] = 'x';
									}

								} else {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = ' ';
									}

									if(flag & FLAG_POUND) {
										num_buf[i - 2] = '0';
										num_buf[i - 1] = 'x';
									}
								}

							} else {
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[i - 2] = '0';
								num_buf[i - 1] = 'x';
							}
						}

					} else if(flag & FLAG_POUND) {
						//#
						rtl_memmove(
						    num_buf + 2,
						    num_buf,
						    num_len + 1);
						num_buf[0] = '0';
						num_buf[1] = 'x';
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%llx
				case TYPE_HEX_64_L:
					rtl_htoa(num_buf, *(u64*)args, false);
					args += 8;
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_POUND) {
								//#
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '0';
								num_buf[1] = 'x';

								for(i = num_len + 2; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';

							} else {
								for(i = num_len; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';
							}

						} else {
							if(width - num_len >= 2
							   || !(flag & FLAG_POUND)) {
								rtl_memmove(
								    num_buf + (width - num_len),
								    num_buf,
								    num_len + 1);

								if(flag & FLAG_ZERO) {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = '0';
									}

									if(flag & FLAG_POUND) {
										num_buf[0] = '0';
										num_buf[1] = 'x';
									}

								} else {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = ' ';
									}

									if(flag & FLAG_POUND) {
										num_buf[i - 2] = '0';
										num_buf[i - 1] = 'x';
									}
								}

							} else {
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[i - 2] = '0';
								num_buf[i - 1] = 'x';
							}
						}

					} else if(flag & FLAG_POUND) {
						//#
						rtl_memmove(
						    num_buf + 2,
						    num_buf,
						    num_len + 1);
						num_buf[0] = '0';
						num_buf[1] = 'x';
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%hX
				case TYPE_HEX_16_C:
					rtl_htoa(num_buf, va_arg(args, u16), true);
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_POUND) {
								//#
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '0';
								num_buf[1] = 'x';

								for(i = num_len + 2; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';

							} else {
								for(i = num_len; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';
							}

						} else {
							if(width - num_len >= 2
							   || !(flag & FLAG_POUND)) {
								rtl_memmove(
								    num_buf + (width - num_len),
								    num_buf,
								    num_len + 1);

								if(flag & FLAG_ZERO) {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = '0';
									}

									if(flag & FLAG_POUND) {
										num_buf[0] = '0';
										num_buf[1] = 'x';
									}

								} else {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = ' ';
									}

									if(flag & FLAG_POUND) {
										num_buf[i - 2] = '0';
										num_buf[i - 1] = 'x';
									}
								}

							} else {
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[i - 2] = '0';
								num_buf[i - 1] = 'x';
							}
						}

					} else if(flag & FLAG_POUND) {
						//#
						rtl_memmove(
						    num_buf + 2,
						    num_buf,
						    num_len + 1);
						num_buf[0] = '0';
						num_buf[1] = 'x';
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%lX
				case TYPE_HEX_32_C:
					rtl_htoa(num_buf, va_arg(args, u32), true);
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_POUND) {
								//#
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '0';
								num_buf[1] = 'x';

								for(i = num_len + 2; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';

							} else {
								for(i = num_len; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';
							}

						} else {
							if(width - num_len >= 2
							   || !(flag & FLAG_POUND)) {
								rtl_memmove(
								    num_buf + (width - num_len),
								    num_buf,
								    num_len + 1);

								if(flag & FLAG_ZERO) {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = '0';
									}

									if(flag & FLAG_POUND) {
										num_buf[0] = '0';
										num_buf[1] = 'x';
									}

								} else {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = ' ';
									}

									if(flag & FLAG_POUND) {
										num_buf[i - 2] = '0';
										num_buf[i - 1] = 'x';
									}
								}

							} else {
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[i - 2] = '0';
								num_buf[i - 1] = 'x';
							}
						}

					} else if(flag & FLAG_POUND) {
						//#
						rtl_memmove(
						    num_buf + 2,
						    num_buf,
						    num_len + 1);
						num_buf[0] = '0';
						num_buf[1] = 'x';
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%llX
				case TYPE_HEX_64_C:
					rtl_htoa(num_buf, *(u64*)args, true);
					args += 8;
					num_len = rtl_strlen(num_buf);

					//Prec
					if(num_len < prec) {
						rtl_memmove(
						    num_buf + (prec - num_len),
						    num_buf,
						    num_len + 1);

						for(i = 0; i < prec - num_len; i++) {
							num_buf[i] = '0';
						}

						num_len = prec;
					}

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							if(flag & FLAG_POUND) {
								//#
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[0] = '0';
								num_buf[1] = 'x';

								for(i = num_len + 2; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';

							} else {
								for(i = num_len; i < width; i++) {
									num_buf[i] = ' ';
								}

								num_buf[i] = '\0';
							}

						} else {
							if(width - num_len >= 2
							   || !(flag & FLAG_POUND)) {
								rtl_memmove(
								    num_buf + (width - num_len),
								    num_buf,
								    num_len + 1);

								if(flag & FLAG_ZERO) {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = '0';
									}

									if(flag & FLAG_POUND) {
										num_buf[0] = '0';
										num_buf[1] = 'x';
									}

								} else {
									for(i = 0; i < width - num_len; i++) {
										num_buf[i] = ' ';
									}

									if(flag & FLAG_POUND) {
										num_buf[i - 2] = '0';
										num_buf[i - 1] = 'x';
									}
								}

							} else {
								rtl_memmove(
								    num_buf + 2,
								    num_buf,
								    num_len + 1);
								num_buf[i - 2] = '0';
								num_buf[i - 1] = 'x';
							}
						}

					} else if(flag & FLAG_POUND) {
						//#
						rtl_memmove(
						    num_buf + 2,
						    num_buf,
						    num_len + 1);
						num_buf[0] = '0';
						num_buf[1] = 'x';
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%s
				case TYPE_STRING:
					data_str = va_arg(args, char*);

					if(data_str == NULL) {
						data_str = "<NULL>";
					}

					num_len = rtl_strlen(data_str);

					if(num_len < width) {
						for(i = 0; i < width - num_len; i++) {
							num_buf[i] = ' ';
						}

						num_buf[i] = '\0';

						if(flag & FLAG_LEFT_ALIGN) {
							write_buf(buf, buf_size, &p_output, data_str);
							write_buf(buf, buf_size, &p_output, num_buf);

						} else {
							write_buf(buf, buf_size, &p_output, num_buf);
							write_buf(buf, buf_size, &p_output, data_str);
						}

					} else {
						write_buf(buf, buf_size, &p_output, data_str);
					}

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				//%p
				case TYPE_POINTER:
					rtl_htoa(num_buf, va_arg(args, u32), true);
					num_len = rtl_strlen(num_buf);

					if(num_len < 10) {
						rtl_memmove(
						    num_buf + (10 - num_len),
						    num_buf,
						    num_len + 1);
					}

					for(i = 2; i < 10 - num_len; i++) {
						num_buf[i] = '0';
					}

					num_buf[0] = '0';
					num_buf[1] = 'x';
					num_len = 10;

					//Width
					if(num_len < width) {
						if(flag & FLAG_LEFT_ALIGN) {
							for(i = num_len; i < width; i++) {
								num_buf[i] = ' ';
							}

							num_buf[i] = '\0';

						} else {
							rtl_memmove(
							    num_buf + (width - num_len),
							    num_buf,
							    num_len + 1);

							for(i = 0; i < width - num_len; i++) {
								num_buf[i] = ' ';
							}
						}
					}

					write_buf(buf, buf_size, &p_output, num_buf);

					if((u32)(p_output - buf) > buf_size - 2) {
						*p_output = '\0';
						return p_output - buf;
					}

					break;

				default:
					continue;
				}
			}

		} else {
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

s32 rtl_atoi(char* str, int num_sys)
{
	char* p;
	u32 len;
	u32 ret;
	len = rtl_strlen(str);

	//Check arguments
	if(num_sys != 2
	   && num_sys != 8
	   && num_sys != 10
	   && num_sys != 16) {
		return 0;
	}

	//Convert string
	ret = 0;

	for(p = str; p < str + len; p++) {
		if(*p >= '0' && *p <= '9') {
			ret = ret * num_sys + (*p - '0');

		} else if(num_sys == 16) {
			if(*p >= 'a' && *p <= 'f') {
				ret = ret * 0x10 + (*p - 'a' + 0x0A);

			} else if(*p >= 'A' && *p <= 'F') {
				ret = ret * 0x10 + (*p - 'A' + 0x0A);

			} else {
				return ret;
			}

		} else {
			return ret;
		}
	}

	return ret;
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

	case 'e':
		if(h_flag) {
			ret = TYPE_E_FLOAT_32_L;

		} else if(ll_flag) {
			ret = TYPE_UNKNOW;

		} else {
			ret = TYPE_E_FLOAT_64_L;
		}

		break;

	case 'E':
		if(h_flag) {
			ret = TYPE_E_FLOAT_32_C;

		} else if(ll_flag) {
			ret = TYPE_UNKNOW;

		} else {
			ret = TYPE_E_FLOAT_64_C;
		}

		break;

	case 'f':
		if(h_flag) {
			ret = TYPE_FLOAT_32;

		} else if(ll_flag) {
			ret = TYPE_UNKNOW;

		} else {
			ret = TYPE_FLOAT_64;
		}

		break;

	case 'g':
		if(h_flag) {
			ret = TYPE_G_FLOAT_32_L;

		} else if(ll_flag) {
			ret = TYPE_UNKNOW;

		} else {
			ret = TYPE_G_FLOAT_64_L;
		}

		break;

	case 'G':
		if(h_flag) {
			ret = TYPE_G_FLOAT_32_C;

		} else if(ll_flag) {
			ret = TYPE_UNKNOW;

		} else {
			ret = TYPE_G_FLOAT_64_C;
		}

		break;

	case 'c':
		ret = TYPE_CHAR;
		break;

	case 's':
		ret = TYPE_STRING;
		break;

	case 'p':
		ret = TYPE_POINTER;
		break;

	case 'n':
		ret = TYPE_NUM;
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

char* rtl_itoa(char* buf, u64 num)
{
	char* p;
	u64 n;
	char* p1;
	char* p2;
	char t;

	if(num == 0) {
		buf[0] = '0';
		buf[1] = '\0';
		return buf;
	}

	for(n = num, p = buf;
	    n != 0;
	    n = rtl_div64(n, 10), p++) {
		*p = (u8)rtl_mod64(n, 10) + '0';
	}

	for(p1 = buf, p2 = p - 1;
	    p2 > p1;
	    p1++, p2--) {
		t = *p1;
		*p1 = *p2;
		*p2 = t;

	}

	*p = '\0';
	return buf;
}

char* rtl_htoa(char* buf, u64 num, bool capital_flag)
{
	char* p;
	u64 n;
	u8 t;
	char* p1;
	char* p2;

	if(num == 0) {
		buf[0] = '0';
		buf[1] = '\0';
		return buf;
	}

	for(n = num, p = buf;
	    n != 0;
	    n = rtl_div64(n, 0x10), p++) {
		t = (u8)rtl_mod64(n, 0x10);

		if(t < 0x0A) {
			*p = t + '0';

		} else {
			if(capital_flag) {
				*p = t - 0x0A + 'A';

			} else {
				*p = t - 0x0A + 'a';
			}
		}
	}

	for(p1 = buf, p2 = p - 1;
	    p2 > p1;
	    p1++, p2--) {
		t = *p1;
		*p1 = *p2;
		*p2 = t;

	}

	*p = '\0';
	return buf;
}

char* rtl_otoa(char* buf, u64 num)
{
	char* p;
	char* p1;
	char* p2;
	char t;
	u64 n;

	if(num == 0) {
		buf[0] = '0';
		buf[1] = '\0';
		return buf;
	}

	for(n = num, p = buf;
	    n != 0;
	    n = rtl_div64(n, 010), p++) {
		*p = (u8)rtl_mod64(n, 010) + '0';
	}

	for(p1 = buf, p2 = p - 1;
	    p2 > p1;
	    p1++, p2--) {
		t = *p1;
		*p1 = *p2;
		*p2 = t;

	}

	*p = '\0';
	return buf;
}
