/*
 * frontend/console.h
 *
 * This
 *
 *
 */

#ifndef LAME_CONSOLE_H
#define LAME_CONSOLE_H

#if defined(_WIN32)  &&  !defined(__CYGWIN__)
# include <windows.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct console_io_struct {
    unsigned long ClassID;
    unsigned long ClassProt;
    FILE   *Console_fp;      /* filepointer to stream reporting information */
    FILE   *Error_fp;        /* filepointer to stream fatal error reporting information */
    FILE   *Report_fp;       /* filepointer to stream reports (normally a text file or /dev/null) */
#if defined(_WIN32)  &&  !defined(__CYGWIN__)
    HANDLE  Console_Handle;
#endif
    int     disp_width;
    int     disp_height;
    char    str_up[10];
    char    str_clreoln[10];
    char    str_emph[10];
    char    str_norm[10];
    char    Console_buff[2048];
    int     Console_file_type;
} Console_IO_t;

extern Console_IO_t Console_IO;
int frontend_open_console(void) __attribute__((constructor));
void frontend_close_console(void) __attribute__((destructor));

extern void frontend_msgf(const char *format, va_list ap);
extern void frontend_debugf(const char *format, va_list ap);
extern void frontend_errorf(const char *format, va_list ap);
extern void frontend_print_null(const char *format, va_list ap);

int     console_printf(const char *format, ...);
int     error_printf(const char *format, ...);
int     report_printf(const char *format, ...);

void    console_flush(void);
void    error_flush(void);
void    report_flush(void);

void    console_up(int n_lines);

void    set_debug_file(const char *fn);


#define LOG(fmt, ...) console_printf("<%s:%d>@sun " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)

#include <ctype.h>

#define dump_buffer(b, n) do {\
  const unsigned char *__b = (const unsigned char *)b;\
  int __n = (int)n;\
  char s[2048], *p;\
  p = s;\
  while (__n--){\
    *p++ = isprint((int)*__b++)?(console_printf("** "),__b[-1]):(console_printf("%.2x ", __b[-1]), '.');\
  }\
  console_printf("\n\n");\
  LOG("@buffer %.*s", n, s);\
} while (0)

#if defined(__cplusplus)
}
#endif

#endif /* LAME_CONSOLE_H */

/* end of console.h */
