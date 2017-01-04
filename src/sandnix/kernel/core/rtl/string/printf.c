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

#include "../obj/obj.h"

#include "string.h"

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

#define	TYPE_KOBJ				0x80



#define	TYPE_UNKNOW				0xFF

static	u32 		get_flag(char** p_p_fmt);
static	u32			get_width(char** p_p_fmt);
static	u32			get_prec(char** p_p_fmt);
static	u32			get_type(char** p_p_fmt);
static	u32			write_buf(char* dest, size_t size, char** p_p_output, char* src);

char* core_rtl_snprintf(char* buf, size_t size, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    return core_rtl_vsnprintf(buf, size, fmt, ap);
}

char* core_rtl_vsnprintf(char* buf, size_t size, const char* fmt, va_list ap)
{
    char* p_fmt;
    char* p_output;
    char num_buf[128];
    u32 num_len;
    u32 flag;
    u32 width;
    u32 prec;
    u32 type;
    p_fmt = (char*)fmt;
    p_output = buf;
    u32 i;
    s32 sign;
    s16 data_s16;
    s32 data_s32;
    s64 data_s64;
    pkstring_obj_t p_kstr;
    pobj_t p_obj;
    char* data_str;

    while(*p_fmt != '\0') {
        if(*p_fmt == '%') {
            p_fmt++;

            //"%%"
            if(*p_fmt == '%') {
                *p_output = '%';
                p_output++;
                p_fmt++;

                if((u32)(p_output - buf) > size - 2) {
                    *p_output = '\0';
                    return buf;
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
                            num_buf[0] = va_arg(ap, int);
                            num_buf[1] = '\0';

                        } else if(flag & FLAG_LEFT_ALIGN) {
                            num_buf[0] = va_arg(ap, int);

                            for(i = 1; i < width; i++) {
                                num_buf[i] = ' ';
                            }

                            num_buf[i] = '\0';

                        } else {
                            width = width - 1;

                            for(i = 0; i < width; i++) {
                                num_buf[i] = ' ';
                            }

                            num_buf[i] = va_arg(ap, int);
                            i++;
                            num_buf[i] = '\0';
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //hd
                    case TYPE_INT_16:
                        data_s16 = va_arg(ap, s32);

                        if(data_s16 > 0) {
                            sign = 1;

                        } else {
                            sign = -1;
                        }

                        core_rtl_itoa(num_buf, data_s32 * sign);
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                core_rtl_memmove(
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
                                core_rtl_memmove(
                                    num_buf + 1,
                                    num_buf,
                                    num_len + 1);

                                if(sign > 0) {
                                    num_buf[0] = '+';

                                } else {
                                    num_buf[0] = '-';
                                }

                            } else if(sign < 0) {
                                core_rtl_memmove(
                                    num_buf + 1,
                                    num_buf,
                                    num_len + 1);
                                num_buf[0] = '-';
                            }
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //ld
                    case TYPE_INT_32:
                        data_s32 = va_arg(ap, s32);

                        if(data_s32 > 0) {
                            sign = 1;

                        } else {
                            sign = -1;
                        }

                        core_rtl_itoa(num_buf, data_s32 * sign);
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                core_rtl_memmove(
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
                                core_rtl_memmove(
                                    num_buf + 1,
                                    num_buf,
                                    num_len + 1);

                                if(sign > 0) {
                                    num_buf[0] = '+';

                                } else {
                                    num_buf[0] = '-';
                                }

                            } else if(sign < 0) {
                                core_rtl_memmove(
                                    num_buf + 1,
                                    num_buf,
                                    num_len + 1);
                                num_buf[0] = '-';
                            }
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //lld
                    case TYPE_INT_64:
                        data_s64 = va_arg(ap, s64);

                        if(data_s64 > 0) {
                            sign = 1;

                        } else {
                            sign = -1;
                        }

                        core_rtl_itoa(num_buf, data_s32 * sign);
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                core_rtl_memmove(
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
                                core_rtl_memmove(
                                    num_buf + 1,
                                    num_buf,
                                    num_len + 1);

                                if(sign > 0) {
                                    num_buf[0] = '+';

                                } else {
                                    num_buf[0] = '-';
                                }

                            } else if(sign < 0) {
                                core_rtl_memmove(
                                    num_buf + 1,
                                    num_buf,
                                    num_len + 1);
                                num_buf[0] = '-';
                            }
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%hu
                    case TYPE_UINT_16:
                        core_rtl_itoa(num_buf, va_arg(ap, u32));
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                core_rtl_memmove(
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

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%lu
                    case TYPE_UINT_32:
                        core_rtl_itoa(num_buf, va_arg(ap, u32));
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                core_rtl_memmove(
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

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%llu
                    case TYPE_UINT_64:
                        core_rtl_itoa(num_buf, va_arg(ap, u64));
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                core_rtl_memmove(
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

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%ho
                    case TYPE_OCTAL_16:
                        core_rtl_otoa(num_buf, va_arg(ap, u32));
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                core_rtl_memmove(
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
                            core_rtl_memmove(
                                num_buf + 1,
                                num_buf,
                                num_len + 1);
                            num_buf[0] = '0';
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%lo
                    case TYPE_OCTAL_32:
                        core_rtl_otoa(num_buf, va_arg(ap, u32));
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                core_rtl_memmove(
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
                            core_rtl_memmove(
                                num_buf + 1,
                                num_buf,
                                num_len + 1);
                            num_buf[0] = '0';
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%llo
                    case TYPE_OCTAL_64:
                        core_rtl_otoa(num_buf, va_arg(ap, u64));
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                core_rtl_memmove(
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
                            core_rtl_memmove(
                                num_buf + 1,
                                num_buf,
                                num_len + 1);
                            num_buf[0] = '0';
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%hx
                    case TYPE_HEX_16_L:
                        core_rtl_htoa(num_buf, va_arg(ap, u32), false);
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
                                        num_buf + 2,
                                        num_buf,
                                        num_len + 1);
                                    num_buf[i - 2] = '0';
                                    num_buf[i - 1] = 'x';
                                }
                            }

                        } else if(flag & FLAG_POUND) {
                            //#
                            core_rtl_memmove(
                                num_buf + 2,
                                num_buf,
                                num_len + 1);
                            num_buf[0] = '0';
                            num_buf[1] = 'x';
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%lx
                    case TYPE_HEX_32_L:
                        core_rtl_htoa(num_buf, va_arg(ap, u32), false);
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
                                        num_buf + 2,
                                        num_buf,
                                        num_len + 1);
                                    num_buf[i - 2] = '0';
                                    num_buf[i - 1] = 'x';
                                }
                            }

                        } else if(flag & FLAG_POUND) {
                            //#
                            core_rtl_memmove(
                                num_buf + 2,
                                num_buf,
                                num_len + 1);
                            num_buf[0] = '0';
                            num_buf[1] = 'x';
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%llx
                    case TYPE_HEX_64_L:
                        core_rtl_htoa(num_buf, va_arg(ap, u64), false);
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
                                        num_buf + 2,
                                        num_buf,
                                        num_len + 1);
                                    num_buf[i - 2] = '0';
                                    num_buf[i - 1] = 'x';
                                }
                            }

                        } else if(flag & FLAG_POUND) {
                            //#
                            core_rtl_memmove(
                                num_buf + 2,
                                num_buf,
                                num_len + 1);
                            num_buf[0] = '0';
                            num_buf[1] = 'x';
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%hX
                    case TYPE_HEX_16_C:
                        core_rtl_htoa(num_buf, va_arg(ap, u32), true);
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
                                        num_buf + 2,
                                        num_buf,
                                        num_len + 1);
                                    num_buf[i - 2] = '0';
                                    num_buf[i - 1] = 'x';
                                }
                            }

                        } else if(flag & FLAG_POUND) {
                            //#
                            core_rtl_memmove(
                                num_buf + 2,
                                num_buf,
                                num_len + 1);
                            num_buf[0] = '0';
                            num_buf[1] = 'x';
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%lX
                    case TYPE_HEX_32_C:
                        core_rtl_htoa(num_buf, va_arg(ap, u32), true);
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
                                        num_buf + 2,
                                        num_buf,
                                        num_len + 1);
                                    num_buf[i - 2] = '0';
                                    num_buf[i - 1] = 'x';
                                }
                            }

                        } else if(flag & FLAG_POUND) {
                            //#
                            core_rtl_memmove(
                                num_buf + 2,
                                num_buf,
                                num_len + 1);
                            num_buf[0] = '0';
                            num_buf[1] = 'x';
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%llX
                    case TYPE_HEX_64_C:
                        core_rtl_htoa(num_buf, va_arg(ap, u64), true);
                        num_len = core_rtl_strlen(num_buf);

                        //Prec
                        if(num_len < prec) {
                            core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
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
                                    core_rtl_memmove(
                                        num_buf + 2,
                                        num_buf,
                                        num_len + 1);
                                    num_buf[i - 2] = '0';
                                    num_buf[i - 1] = 'x';
                                }
                            }

                        } else if(flag & FLAG_POUND) {
                            //#
                            core_rtl_memmove(
                                num_buf + 2,
                                num_buf,
                                num_len + 1);
                            num_buf[0] = '0';
                            num_buf[1] = 'x';
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%s
                    case TYPE_STRING:
                        data_str = va_arg(ap, char*);
                        num_len = core_rtl_strlen(data_str);

                        if(num_len < width) {
                            if(flag & FLAG_LEFT_ALIGN) {
                                write_buf(buf, size, &p_output, data_str);

                                for(i = 0; i < width - num_len; i++) {
                                    write_buf(buf, size, &p_output, " ");
                                }

                            } else {
                                for(i = 0; i < width - num_len; i++) {
                                    write_buf(buf, size, &p_output, " ");
                                }

                                write_buf(buf, size, &p_output, data_str);
                            }

                        } else {
                            write_buf(buf, size, &p_output, data_str);
                        }

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%p
                    case TYPE_POINTER:
                        core_rtl_htoa(num_buf, va_arg(ap, u32), true);
                        num_len = core_rtl_strlen(num_buf);

                        if(num_len < 10) {
                            core_rtl_memmove(
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
                                core_rtl_memmove(
                                    num_buf + (width - num_len),
                                    num_buf,
                                    num_len + 1);

                                for(i = 0; i < width - num_len; i++) {
                                    num_buf[i] = ' ';
                                }
                            }
                        }

                        write_buf(buf, size, &p_output, num_buf);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
                        }

                        break;

                    //%k
                    case TYPE_KOBJ:
                        p_obj = va_arg(ap, pobj_t);
                        p_kstr = TO_STRING(p_obj);
                        num_len = p_kstr->len(p_kstr);
                        data_str = p_kstr->buf;

                        if(num_len < width) {
                            if(flag & FLAG_LEFT_ALIGN) {
                                write_buf(buf, size, &p_output, data_str);

                                for(i = 0; i < width - num_len; i++) {
                                    write_buf(buf, size, &p_output, " ");
                                }

                            } else {
                                for(i = 0; i < width - num_len; i++) {
                                    write_buf(buf, size, &p_output, " ");
                                }

                                write_buf(buf, size, &p_output, data_str);
                            }

                        } else {
                            write_buf(buf, size, &p_output, data_str);
                        }

                        DEC_REF(p_kstr);

                        if((u32)(p_output - buf) > size - 2) {
                            *p_output = '\0';
                            return buf;
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

            if((u32)(p_output - buf) > size - 2) {
                *p_output = '\0';
                return buf;
            }
        }
    }

    *p_output = '\0';
    return buf;
}

char* core_rtl_kprintf(const char* fmt, ...)
{
    UNREFERRED_PARAMETER(fmt);
    return NULL;
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

    ret = (s32)core_rtl_atoi(*p_p_fmt, 10);

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
    ret = (s32)core_rtl_atoi(*p_p_fmt, 10);

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

        case 'k':
            ret = TYPE_KOBJ;
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
    len = core_rtl_strlen(src);

    if(len + (*p_p_output - dest) > size - 2) {
        len = size - 2 - (*p_p_output - dest);
    }

    core_rtl_memcpy(*p_p_output, src, len);
    (*p_p_output) += len;
    return len;
}
