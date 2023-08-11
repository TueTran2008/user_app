#include "app_debug.h"
#include <stdarg.h>
#include "stdio.h"
#include <string.h>
#include <ctype.h>

//static char m_debug_buffer[APP_DEBUG_PRINTF_BUFFER_SIZE];
app_debug_callback_t m_cb =
{
	.get_ms = 0,
	.put_cb = 0,
};

unsigned int log_get_sys_ms(void)
{
	if (m_cb.get_ms)
	{
		return m_cb.get_ms();
	}
	return 0;
}

void app_debug_set_callback(app_debug_callback_t debug_callback)
{
	m_cb = debug_callback;
}

int32_t app_debug_print_nothing(const char *fmt,...)
{
    return -1;
}

#include "drivers.h"
//#include "putchar.h"

static void printchar(char **str, int c)
{
	if (str)
	{
		**str = c;
		++(*str);
	}
	else
	{
        if (m_cb.put_cb)
        {
        	m_cb.put_cb(&c, 1);
        }
	}
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad) {
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr)
			++len;
		if (len >= width)
			width = 0;
		else
			width -= len;
		if (pad & PAD_ZERO)
			padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for (; width > 0; --width) {
			printchar(out, padchar);
			++pc;
		}
	}
	for (; *string; ++string) {
		printchar(out, *string);
		++pc;
	}
	for (; width > 0; --width) {
		printchar(out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad,
		int letbase) {
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints(out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN - 1;
	*s = '\0';

	while (u) {
		t = u % b;
		if (t >= 10)
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if (width && (pad & PAD_ZERO)) {
			printchar(out, '-');
			++pc;
			--width;
		} else {
			*--s = '-';
		}
	}

	return pc + prints(out, s, width, pad);
}

static int print(char **out, const char *format, va_list args) {
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0')
				break;
			if (*format == '%')
				goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for (; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if (*format == 's') {
				register char *s = (char *) va_arg( args, int );
				pc += prints(out, s ? s : "(null)", width, pad);
				continue;
			}
			if (*format == 'd') {
				pc += printi(out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			if (*format == 'x') {
				pc += printi(out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if (*format == 'X') {
				pc += printi(out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if (*format == 'u') {
				pc += printi(out, va_arg( args, int ), 10, 0, width, pad, 'a');
				continue;
			}
			if (*format == 'c') {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char) va_arg( args, int );
				scr[1] = '\0';
				pc += prints(out, scr, width, pad);
				continue;
			}
		} else {
			out: printchar(out, *format);
			++pc;
		}
	}
	if (out)
		**out = '\0';
//	va_end( args );
	return pc;
}

int32_t app_debug_print_raw(const char *format, ...) {
	int ret;
	va_list args;

	va_start( args, format );
	ret = print(0, format, args);
	va_end( args );

	return ret;
}


unsigned char is_printable(unsigned char data)
{
	if (data >= 'A' && data <= 'Z')
	{
		return 1;
	}

	if (data >= 'a' && data <= 'z')
	{
		return 1;
	}

	if (data >= '0' && data <= '9')
	{
		return 1;
	}

	if (data == '!'
		|| data == '"'
		|| data == '#'
		|| data == '$'
		|| data == '%'
		|| data == '\''
		|| data == '('
		|| data == ')'
		|| data == '*'
		|| data == ','
		|| data == '+'
		|| data == '-'
		|| data == '*'
		|| data == '/'
		|| data == ':'
		|| data == ';'
		|| data == '<'
		|| data == '>'
		|| data == '='
		|| data == '?'
		|| data == '@'
		|| data == '['
		|| data == ']'
		|| data == '\\'
		|| data == '^'
		|| data == '_'
		|| data == '`'
		|| data == '{'
		|| data == '|'
		|| data == '}'
		|| data == '~')
	{
		return 1;
	}

	return 0;
}

void app_debug_dump(const void* data, int len, const char* string, ...)
{
	unsigned char* p = (unsigned char*)data;
	unsigned char  buffer[16];
	int i_len;
	int i;

    DEBUG_RAW("%s %u bytes\r\n", string, len);
    while (len > 0)
    {
        i_len = (len > 16) ? 16 : len;
        for (int j = 0; j < 16; j++)
        {
        	buffer[j] = 0;
        }

        for (int j = 0; j < i_len; j++)
        {
        	buffer[j] = p[j];
        }

        for (i = 0; i < 16; i++)
        {
            if (i < i_len)
            {
                DEBUG_RAW("%02X ", buffer[i]);
            }
            else
            {
                DEBUG_RAW("   ");
            }
        }
        DEBUG_RAW("\t");
        for (i = 0; i < 16; i++)
        {
            if (i < i_len)
            {
                if (is_printable(buffer[i]))
				{
                    DEBUG_RAW("%c", (char)buffer[i]);
				}
                else
				{
                    DEBUG_RAW(".");
				}
            }
            else
            {
                DEBUG_RAW(" ");
            }
        }
        DEBUG_RAW("\r\n");
        len -= i_len;
        p += i_len;
    }
}
