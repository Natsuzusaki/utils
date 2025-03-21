#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h> /* true, false, bool */
#include <stdbool.h>
#include <stdio.h> /*Includes the standard I/O library for functions like `printf`.*/
#include <stdlib.h> /*Includes the standard library for functions like `malloc`, `free`, and `realloc`.*/
#include <string.h> /*Includes the string manipulation library for functions like `memcpy`.*/

/*A list of ANSI color codes (including bright versions and
 * common styles) to use in C files:*/

/*Regular Colors*/
#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"

/*Bright Colors*/
#define BRIGHT_BLACK "\x1b[90m"
#define BRIGHT_RED "\x1b[91m"
#define BRIGHT_GREEN "\x1b[92m"
#define BRIGHT_YELLOW "\x1b[93m"
#define BRIGHT_BLUE "\x1b[94m"
#define BRIGHT_MAGENTA "\x1b[95m"
#define BRIGHT_CYAN "\x1b[96m"
#define BRIGHT_WHITE "\x1b[97m"

/*Background Colors*/
#define BG_BLACK "\x1b[40m"
#define BG_RED "\x1b[41m"
#define BG_GREEN "\x1b[42m"
#define BG_YELLOW "\x1b[43m"
#define BG_BLUE "\x1b[44m"
#define BG_MAGENTA "\x1b[45m"
#define BG_CYAN "\x1b[46m"
#define BG_WHITE "\x1b[47m"

/*Bright Background Colors*/
#define BG_BRIGHT_BLACK "\x1b[100m"
#define BG_BRIGHT_RED "\x1b[101m"
#define BG_BRIGHT_GREEN "\x1b[102m"
#define BG_BRIGHT_YELLOW "\x1b[103m"
#define BG_BRIGHT_BLUE "\x1b[104m"
#define BG_BRIGHT_MAGENTA "\x1b[105m"
#define BG_BRIGHT_CYAN "\x1b[106m"
#define BG_BRIGHT_WHITE "\x1b[107m"

/*Styles*/
#define RESET "\x1b[0m"
#define BOLD "\x1b[1m"
#define DIM "\x1b[2m"
#define ITALIC "\x1b[3m"
#define UNDERLINE "\x1b[4m"
#define INVERSE "\x1b[7m"
#define HIDDEN "\x1b[8m"
#define STRIKETHROUGH "\x1b[9m"

/*Here's a handy macro system for quickly combining foreground and background
 * colors, along with optional text styles:*/

/*Usage: COLOR(FG_COLOR, BG_COLOR, STYLE)*/
#define COLOR(fg, bg, style) fg bg style

/*Example usage:*/
/*printf(COLOR(RED, BG_YELLOW, BOLD) "Warning!" RESET "\n");*/
/*printf(COLOR(BRIGHT_GREEN, BG_BLACK, UNDERLINE) "Success!" RESET "\n");*/

/*If you want a more flexible macro that can accept just fg or fg + style:*/
#define FG_COLOR(fg) fg
#define FG_COLOR_STYLE(fg, style) fg style
#define FG_BG(fg, bg) fg bg
#define FG_BG_STYLE(fg, bg, style) fg bg style

/*Example usage:*/
/*printf(FG_COLOR(RED) "Error message\n" RESET);*/
/*printf(FG_COLOR_STYLE(BLUE, UNDERLINE) "Underlined blue text\n" RESET);*/
/*printf(FG_BG(YELLOW, BG_BLUE) "Yellow on blue\n" RESET);*/
/*printf(FG_BG_STYLE(BRIGHT_CYAN, BG_BLACK, BOLD) "Bright cyan bold on
 * black\n"*/
/*RESET);*/

/*a simple color_print helper function that takes foreground color, background
 * color, style, and your message, and automatically prints it with a reset at
 * the end:*/

/*Function that prints colored, styled text*/
static inline void color_print(const char *fg, const char *bg,
                               const char *style, const char *format, ...) {
  va_list args;
  va_start(args, format);
  printf("%s%s%s", fg, bg, style); // Apply colors and styles
  vprintf(format, args);           // Print the actual message
  printf(RESET);                   // Reset colors/styles after
  va_end(args);
}

// Example usage:
// color_print(RED, BG_YELLOW, BOLD, "Warning: %s!\n", "Disk almost full");
// color_print(BRIGHT_GREEN, BG_BLACK, UNDERLINE, "Success: %d files
// processed\n", 42); color_print(CYAN, BG_BLACK, "", "Normal cyan text without
// style\n");

void print_matrix_neighbor_coordinates_rules(void);
void debug_puts(char *str, int line, char *file);

#if !defined(NO_MEMORY_DEBUG)
/* turns on the memory debugging system */
/*#define MEMORY_DEBUG*/
#endif
#if !defined(EXIT_CRASH)
/* turns on the crash on exit */
/*#define EXIT_CRASH */
#endif

#ifdef MEMORY_DEBUG

/* ----- Debugging -----
If MEMORY_DEBUG  is enabled, the memory debugging system will create macros
that replace malloc, free and realloc and allows the system to keppt track of
and report where memory is beeing allocated, how much and if the memory is
beeing freed. This is very useful for finding memory leaks in large
applications. The system can also over allocate memory and fill it with a magic
number and can therfor detect if the application writes outside of the allocated
memory. if EXIT_CRASH is defined, then exit(void); will be replaced with a
funtion that writes to NULL. This will make it trivial ti find out where an
application exits using any debugger., */

extern void debug_memory_init(
    void (*lock)(void *mutex), void (*unlock)(void *mutex),
    void *mutex); /* Required for memory debugger to be thread safe */
extern void *
debug_mem_malloc(unsigned int size, char *file,
                 unsigned int line); /* Replaces malloc and records the c file
                                and line where it was called*/
extern void *
debug_mem_realloc(void *pointer, unsigned int size, char *file,
                  unsigned int line);  /* Replaces realloc and records the c
                                  file and  line where it was called*/
extern void debug_mem_free(void *buf); /* Replaces free and records the c file
                                            and line where it was called*/
extern void debug_mem_print(
    unsigned int
        min_allocs); /* Prints out a list of all allocations made, their
                location, how much memory each has allocated, freed,
                and how many allocations have been made. The min_allocs
                parameter can be set to avoid printing any allocations
                that have been made fewer times then min_allocs */
extern void
debug_mem_reset(void); /* debug_mem_reset allows you to clear all memory stored
                        in the debugging system if you only want to record
                        allocations after a specific point in your code*/
extern bool
debug_memory(void); /*debug_memory checks if any of the bounds of any allocation
                     has been over written and reports where to standard out.
                     The function returns true if any error was found*/

#define malloc(n)                                                              \
  debug_mem_malloc(n, __FILE__, __LINE__) /* Replaces malloc.                  \
                                           */
#define realloc(n, m)                                                          \
  debug_mem_realloc(n, m, __FILE__, __LINE__) /* Replaces realloc. */
#define free(n) debug_mem_free(n)             /* Replaces free. */

#endif

#ifdef EXIT_CRASH

extern void
exit_crash(unsigned int i); /* function guaranteed to crash (Writes to NULL).*/
#define exit(n)                                                                \
  exit_crash(n) /* over writing exit(0)  with a function guaraneed to crash.   \
                 */

#endif

#ifdef __cplusplus
}
#endif

#endif // __UTILS_H__
