/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 * Modified by nzamora
 */
/*
 * @OSF_COPYRIGHT@
 */

/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

/*
 *  Common code for printf et al.
 *
 *  The calling routine typically takes a variable number of arguments,
 *  and passes the address of the first one.  This implementation
 *  assumes a straightforward, stack implementation, aligned to the
 *  machine's wordsize.  Increasing addresses are assumed to point to
 *  successive arguments (left-to-right), as is the case for a machine
 *  with a downward-growing stack with arguments pushed right-to-left.
 *
 *  To write, for example, fprintf() using this routine, the code
 *
 *	fprintf(fd, format, args)
 *	FILE *fd;
 *	char *format;
 *	{
 *	_doprnt(format, &args, fd);
 *	}
 *
 *  would suffice.  (This example does not handle the fprintf's "return
 *  value" correctly, but who looks at the return value of fprintf
 *  anyway?)
 *
 *  This version implements the following printf features:
 *
 *	%d	decimal conversion
 *	%u	unsigned conversion
 *	%x	hexadecimal conversion
 *	%X	hexadecimal conversion with capital letters
 *	%o	octal conversion
 *	%c	character
 *	%s	string
 *	%m.n	field width, precision
 *	%-m.n	left adjustment
 *	%0m.n	zero-padding
 *	%*.*	width and precision taken from arguments
 *
 *  This version does not implement %f, %e, or %g.  It accepts, but
 *  ignores, an `l' as in %ld, %lo, %lx, and %lu, and therefore will not
 *  work correctly on machines for which sizeof(long) != sizeof(int).
 *  It does not even parse %D, %O, or %U; you should be using %ld, %o and
 *  %lu if you mean long conversion.
 *
 *  As mentioned, this version does not return any reasonable value.
 *
 *  Permission is granted to use, modify, or propagate this code as
 *  long as this notice is incorporated.
 *
 *  Steve Summit 3/25/87
 */

/*
 * Added formats for decoding device registers:
 *
 * printf("reg = %b", regval, "<base><arg>*")
 *
 * where <base> is the output base expressed as a control character:
 * i.e. '\10' gives octal, '\20' gives hex.  Each <arg> is a sequence of
 * characters, the first of which gives the bit number to be inspected
 * (origin 1), and the rest (up to a control character (<= 32)) give the
 * name of the register.  Thus
 *	printf("reg = %b\n", 3, "\10\2BITTWO\1BITONE")
 * would produce
 *	reg = 3<BITTWO,BITONE>
 *
 * If the second character in <arg> is also a control character, it
 * indicates the last bit of a bit field.  In this case, printf will extract
 * bits <1> to <2> and print it.  Characters following the second control
 * character are printed before the bit field.
 *	printf("reg = %b\n", 0xb, "\10\4\3FIELD1=\2BITTWO\1BITONE")
 * would produce
 *	reg = b<FIELD1=2,BITONE>
 *
 * The %B format is like %b but the bits are numbered from the most
 * significant (the bit weighted 31), which is called 1, to the least
 * significant, called 32.
 */
/*
 * Added for general use:
 *	#	prefix for alternate format:
 *		0x (0X) for hex
 *		leading 0 for octal
 *	+	print '+' if positive
 *	blank	print ' ' if positive
 *
 *	z	signed hexadecimal
 *	r	signed, 'radix'
 *	n	unsigned, 'radix'
 *
 *	D,U,O,Z	same as corresponding lower-case versions
 *		(compatibility)
 */

#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

// integer division algorithm
static void int_div_mod(unsigned int a, unsigned int b, unsigned int* d,
                        unsigned int* m) {
  *d = 0;
  *m = 0;
  if (a < b) {
    *m = a;
    return;
  }
  unsigned int q = 0;
  unsigned int r = a;
  while (r >= b) {
    q += 1;
    r -= b;
  };
  *d = q;
  *m = r;
}

static unsigned int int_div(unsigned int a, unsigned int b) {
  unsigned int d = 0, m = 0;
  int_div_mod(a, b, &d, &m);
  return d;
}

static unsigned int int_mod(unsigned int a, unsigned int b) {
  unsigned int d = 0, m = 0;
  int_div_mod(a, b, &d, &m);
  return m;
}

/*
 * Forward declarations
 */
void printnum(register unsigned int u, register int base, int (*putc)(int));

#define isdigit(d) ((d) >= '0' && (d) <= '9')
#define Ctod(c) ((c) - '0')

#define MAXBUF (sizeof(long int) * 8) /* enough for binary */

void printnum(register unsigned int u, /* number to print */
              register int base, int (*putc)(int)) {
  char buf[MAXBUF]; /* build number here */
  register char* p = &buf[MAXBUF - 1];
  static char digs[] = "0123456789abcdef";

  do {
    *p-- = digs[int_mod(u, base)];
    u = int_div(u, base);
  } while (u != 0);

  while (++p != &buf[MAXBUF])
    (*putc)(*p);
}

static bool _doprnt_truncates = false;

static void _doprnt(register const char* fmt, va_list* argp,
                    /* character output routine */
                    int (*putc)(int), int radix) /* default radix - for '%r' */
{
  int length;
  int prec;
  bool ladjust;
  char padc;
  long n;
  unsigned long u;
  int plus_sign;
  int sign_char;
  bool altfmt, truncate;
  int base;
  register char c;
  int capitals;

  while ((c = *fmt) != '\0') {
    if (c != '%') {
      (*putc)(c);
      fmt++;
      continue;
    }

    fmt++;

    length = 0;
    prec = -1;
    ladjust = false;
    padc = ' ';
    plus_sign = 0;
    sign_char = 0;
    altfmt = false;

    while (true) {
      c = *fmt;
      if (c == '#') {
        altfmt = true;
      } else if (c == '-') {
        ladjust = true;
      } else if (c == '+') {
        plus_sign = '+';
      } else if (c == ' ') {
        if (plus_sign == 0)
          plus_sign = ' ';
      } else
        break;
      fmt++;
    }

    if (c == '0') {
      padc = '0';
      c = *++fmt;
    }

    if (isdigit(c)) {
      while (isdigit(c)) {
        length = 10 * length + Ctod(c);
        c = *++fmt;
      }
    } else if (c == '*') {
      length = va_arg(*argp, int);
      c = *++fmt;
      if (length < 0) {
        ladjust = !ladjust;
        length = -length;
      }
    }

    if (c == '.') {
      c = *++fmt;
      if (isdigit(c)) {
        prec = 0;
        while (isdigit(c)) {
          prec = 10 * prec + Ctod(c);
          c = *++fmt;
        }
      } else if (c == '*') {
        prec = va_arg(*argp, int);
        c = *++fmt;
      }
    }

    if (c == 'l')
      c = *++fmt; /* need it if sizeof(int) < sizeof(long) */

    truncate = false;
    capitals = 0; /* Assume lower case printing */

    switch (c) {
      case 'b':
      case 'B': {
        register char* p;
        bool any;
        register int i;

        u = va_arg(*argp, unsigned long);
        p = va_arg(*argp, char *);
        base = *p++;
        printnum(u, base, putc);

        if (u == 0)
          break;

        any = false;
        while ((i = *p++) != '\0') {
          if (*fmt == 'B')
            i = 33 - i;
          if (*p <= 32) {
            /*
 * Bit field
 */
            register int j;
            if (any)
              (*putc)(',');
            else {
              (*putc)('<');
              any = true;
            }
            j = *p++;
            if (*fmt == 'B')
              j = 32 - j;
            for (; (c = *p) > 32; p++)
              (*putc)(c);
            printnum((unsigned)((u >> (j - 1)) &
                                ((2 << (i - j)) -
                                 1)),
                     base, putc);
          } else if (u & (1 << (i - 1))) {
            if (any)
              (*putc)(',');
            else {
              (*putc)('<');
              any = true;
            }
            for (; (c = *p) > 32; p++)
              (*putc)(c);
          } else {
            for (; *p > 32; p++)
              continue;
          }
        }
        if (any)
          (*putc)('>');
        break;
      }

      case 'c':
        c = va_arg(*argp, int);
        (*putc)(c);
        break;

      case 's': {
        register char* p;
        register char* p2;

        if (prec == -1)
          prec = 0x7fffffff; /* MAXINT */

        p = va_arg(*argp, char *);

        if (p == (char*)0)
          p = "";

        if (length > 0 && !ladjust) {
          n = 0;
          p2 = p;

          for (; *p != '\0' && n < prec; p++)
            n++;

          p = p2;

          while (n < length) {
            (*putc)(' ');
            n++;
          }
        }

        n = 0;

        while (*p != '\0') {
          if (++n > prec || (length > 0 && n > length))
            break;

          (*putc)(*p++);
        }

        if (n < length && ladjust) {
          while (n < length) {
            (*putc)(' ');
            n++;
          }
        }

        break;
      }

      case 'o':
        truncate = _doprnt_truncates;
      case 'O':
        base = 8;
        goto print_unsigned;

      case 'd':
        truncate = _doprnt_truncates;
      case 'D':
        base = 10;
        goto print_signed;

      case 'u':
        truncate = _doprnt_truncates;
      case 'U':
        base = 10;
        goto print_unsigned;

      case 'p':
        altfmt = true;
      case 'x':
        truncate = _doprnt_truncates;
        base = 16;
        goto print_unsigned;

      case 'X':
        base = 16;
        capitals = 16; /* Print in upper case */
        goto print_unsigned;

      case 'z':
        truncate = _doprnt_truncates;
        base = 16;
        goto print_signed;

      case 'Z':
        base = 16;
        capitals = 16; /* Print in upper case */
        goto print_signed;

      case 'r':
        truncate = _doprnt_truncates;
      case 'R':
        base = radix;
        goto print_signed;

      case 'n':
        truncate = _doprnt_truncates;
      case 'N':
        base = radix;
        goto print_unsigned;

      print_signed:
        n = va_arg(*argp, long);
        if (n >= 0) {
          u = n;
          sign_char = plus_sign;
        } else {
          u = -n;
          sign_char = '-';
        }
        goto print_num;

      print_unsigned:
        u = va_arg(*argp, unsigned long);
        goto print_num;

      print_num: {
          char buf[MAXBUF]; /* build number here */
          register char* p = &buf[MAXBUF - 1];
          static char digits[] = "0123456789abcdef0123456789ABCDEF";
          char* prefix = 0;

          if (truncate)
            u = (long)((int)(u));

          if (u != 0 && altfmt) {
            if (base == 8)
              prefix = "0";
            else if (base == 16)
              prefix = "0x";
          }

          do {
            /* Print in the correct case */
            *p-- = digits[int_mod(u, base) + capitals];
            u = int_div(u, base);
          } while (u != 0);

          length -= (&buf[MAXBUF - 1] - p);
          if (sign_char)
            length--;
          if (prefix)
            length -= strlen((const char*)prefix);

          if (padc == ' ' && !ladjust) {
            /* blank padding goes before prefix */
            while (--length >= 0)
              (*putc)(' ');
          }
          if (sign_char)
            (*putc)(sign_char);
          if (prefix)
            while (*prefix)
              (*putc)(*prefix++);
          if (padc == '0') {
            /* zero padding goes after sign and prefix */
            while (--length >= 0)
              (*putc)('0');
          }
          while (++p != &buf[MAXBUF])
            (*putc)(*p);

          if (ladjust) {
            while (--length >= 0)
              (*putc)(' ');
          }
          break;
        }

      case '\0':
        fmt--;
        break;

      default:
        (*putc)(c);
    }
    fmt++;
  }
}

/* derived from boot_gets */
char* fgets(char* restrict s, int n, FILE* restrict stream) {
  register char* lp;
  register int c;
  char* strmax = s + n - 1; /* allow space for trailing 0 */

  lp = s;
  for (;;) {
    c = getchar();
    switch (c) {
      case '\n':
      case '\r':
      case EOF:
        printf("\n");
        *lp++ = 0;
        return c == EOF ? NULL : s;

      case '\b':
      case '#':
      case '\177':
        if (lp > s) {
          printf("\b \b");
          lp--;
        }
        continue;

      case '@':
      case 'u' & 037:
        lp = s;
        printf("\n\r");
        continue;

      default:
        if (c >= ' ' && c < '\177') {
          if (lp < strmax) {
            *lp++ = c;
            printf("%c", c);
          } else {
            printf("%c", '\007'); /* beep */
          }
        }
    }
  }
}

int printf(const char* fmt, ...) {
  va_list listp;

  va_start(listp, fmt);
  _doprnt(fmt, &listp, putchar, 16);
  va_end(listp);

  // supposed to return number of bytes transmitted
  return true;
}

static char* copybyte_str;

static int copybyte(int byte) {
  *copybyte_str++ = byte;
  *copybyte_str = '\0';
  return true;
}

int sprintf(char* buf, const char* fmt, ...) {
  va_list listp;

  va_start(listp, fmt);
  copybyte_str = buf;
  _doprnt(fmt, &listp, copybyte, 16);
  va_end(listp);
  return strlen(buf);
}
