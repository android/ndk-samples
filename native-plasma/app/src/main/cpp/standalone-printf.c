/* gcc -m32 -Wno-builtin-declaration-mismatch printf.c -o printf */
/* clang -m32 -Wno-incompatible-library-redeclaration printf.c -Wno-implicit-function-declaration -o printf */

#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef int32_t WORD;
typedef uint32_t UWORD;
typedef int64_t DWORD;
typedef uint64_t UDWORD;

static void reverse(char *str) {
  int i, j;
  for (i = 0, j = strlen(str) - 1; i < j; i++, j--) {
    char ch;
    ch = str[i];
    str[i] = str[j];
    str[j] = ch;
  }
}

#define dword_mod(num, base) ((num)%(base))
#define dword_div(num, base) ((num)/(base))

static void utoa(uint64_t num, unsigned int base, char *str) {
  int i = 0;
  do {
    unsigned int d = dword_mod(num, base);
    str[i++] = d + (d < 10 ? '0' : 'a' - 10);
    num = dword_div(num, base);
  }
  while (num > 0);
  str[i] = '\0';
  reverse(str);
}

static void itoa(int64_t num, char *str) {
  if (num < 0) {
    num = -num;
    *(str++) = '-';
  }
  utoa(num, 10, str);
}

#define FLAGS_ZERO  (1)
#define FLAGS_LEFT  (2)
#define FLAGS_SPACE (4)
#define FLAGS_UPPER (8)
#define FLAGS_LONG  (16)

typedef void (*putc_func) (void *, char);

static int format_str(void *ctx, putc_func putc, char *str, int width,
 int flags) {
  int len;
  int i;
  int written = 0;
  len = strlen(str);
  if (!(flags & FLAGS_LEFT)) {
    for (i = width - len; i-- > 0; ) {
      ++written;
      putc(ctx, flags & FLAGS_ZERO ? '0' : ' ');
    }
  }
  for (i = 0; str[i] != '\0'; i++) {
    ++written;
    putc(ctx, flags & FLAGS_UPPER ? toupper(str[i]) : str[i]);
  }
  if (flags & FLAGS_LEFT) {
    for (i = width - len; i-- > 0; ) {
      ++written;
      putc(ctx, ' ');
    }
  }
  return written;
}

static int local_atoi(const char* str) {
  int i = 0;
  while (*str >= '0' && *str <= '9') {
    i = 10 * i + *str - '0';
  }
  return i;
}

static int format(void *ctx, putc_func putc, const char *fmt, va_list va) {
  char ch;
  char buf[8*sizeof(DWORD) + 1];
  int written = 0;
  while ((ch = *(fmt++))) {
    if (ch != '%') {
      ++written;
      putc(ctx, ch);
    }
    else {
      int flags;
      int width;
      for (flags = 0; ; fmt++) {
        switch (*fmt) {
          case '0' : flags |= FLAGS_ZERO; continue;
          case '-' : flags |= FLAGS_LEFT; continue;
          case ' ' : flags |= FLAGS_SPACE; continue;
        }
        break;
      }
      /* The '0' flag is ignored with '-' flag */
      if (flags & FLAGS_LEFT) flags &= ~FLAGS_ZERO;
      width = local_atoi(fmt);
      while (isdigit(*fmt)) fmt++;
      switch (*fmt) {
        case 'l' : flags |= FLAGS_LONG; fmt++; break;
      }
      ch = *(fmt++);
      switch (ch) {
        case 'c' : {
          ++written;
          putc(ctx, va_arg(va, int));
          break;
        }
        case 's' : {
          char* arg = va_arg(va, char*);
          if (arg == NULL) {
            written += format_str(ctx, putc, "(null)", width, flags);
          } else {
            written += format_str(ctx, putc, arg, width, flags);
          }
          break;
        }
        case 'i' :
        case 'd' : {
          DWORD v = flags & FLAGS_LONG ? va_arg(va, DWORD) : va_arg(va, WORD);
          itoa(v, buf);
          if (flags & FLAGS_SPACE) {
            if (strlen(buf) >= width || flags & FLAGS_ZERO) {
              ++written;
              putc(ctx, v < 0 ? '-' : ' ');
              width--;
              if (v < 0) utoa(-v, 10, buf);
            }
          }
          written += format_str(ctx, putc, buf, width, flags);
          break;
        }
        case 'u' : {
          utoa(flags & FLAGS_LONG ? va_arg(va, UDWORD) : va_arg(va, UWORD), 10,
           buf);
          written += format_str(ctx, putc, buf, width, flags);
          break;
        }
        case 'X' : flags |= FLAGS_UPPER;
        case 'x' : {
          utoa(flags & FLAGS_LONG ? va_arg(va, UDWORD) : va_arg(va, UWORD), 16,
           buf);
          written += format_str(ctx, putc, buf, width, flags);
          break;
        }
        case '%' : {
          ++written;
          putc(ctx, ch);
          break;
        }
        default : {
          written += format_str(ctx, putc, "%WRONG FORMAT CHARACTER%", width, flags);
          /* Bail out now since the rest of the arguments can't be trusted. */
          return written;
        }
      }
    }
  }
  return written;
}

int putchar(int c);

static void putc_stdout(void *ctx, char c) {
  (void)ctx;
  putchar(c);
}

int vprintf(const char *fmt, va_list va) {
  return format(0, putc_stdout, fmt, va);
}

int printf(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  int written = vprintf(fmt, va);
  va_end(va);
  return written;
}

static void putc_string(void *ctx, char c) {
  char **p;
  p = (char **)ctx;
  *p[0] = c;
  (*p)++;
}

int vsprintf(char *str, const char *fmt, va_list va) {
  int written = format((void *)&str, putc_string, fmt, va);
  *str = '\0';
  return written;
}

int sprintf(char *str, const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  int written = vsprintf(str, fmt, va);
  va_end(va);
  return written;
}

int fputc(int c, FILE* stream);

static void putc_file(void *ctx, char c) {
  fputc(c, (FILE*)ctx);
}

int vfprintf(FILE *file, const char *fmt, va_list va) {
  return format((void *)file, putc_file, fmt, va);
}

int fprintf(FILE *file, const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  int written = vfprintf(file, fmt, va);
  va_end(va);
  return written;
}
