#ifndef __MEMDEBUG_H__
#define __MEMDEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h> /* true, false, bool */
#include <stdbool.h>
#include <stdio.h> /*Includes the standard I/O library for functions like `printf`.*/
#include <stdlib.h> /*Includes the standard library for functions like `malloc`, `free`, and `realloc`.*/
#include <string.h> /*Includes the string manipulation library for functions like `memcpy`.*/

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

#endif // __MEMDEBUG_H__
