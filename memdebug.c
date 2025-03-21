#define NO_MEMORY_DEBUG /*A preprocessor directive that disables debug         \
                           features for this file*/
#include "memdebug.h"

/*from external*/

extern void debug_mem_print(unsigned int min_allocs);

/*- **`MEMORY_OVER_ALLOC`**: Defines the number of extra bytes allocated for
 * overflow detection.*/
/*- **`MEMORY_MAGIC_NUMBER`**: A special value used to detect memory
 * corruption.*/
#define MEMORY_OVER_ALLOC 32
#define MEMORY_MAGIC_NUMBER 132

/*- **`STMemAllocBuf`**: A structure to store the size and pointer of a memory
 * allocation.*/
typedef struct {
  unsigned int size;
  void *buf;
} STMemAllocBuf;

/*- **`STMemAllocLine`**: A structure to track memory allocations for a specific
 * line in a file.*/
/*  - **`line`**: The line number in the source file where the allocation
 * occurred.*/
/*  - **`file`**: The name of the source file.*/
/*  - **`allocs`**: A pointer to an array of `STMemAllocBuf` structures.*/
/*  - **`alloc_count`**: The number of allocations for this line.*/
/*  - **`alloc_allocated`**: The allocated size of the `allocs` array.*/
/*  - **`size`**: Total size of memory allocated for this line.*/
/*  - **`allocated`**: Total number of allocations for this line.*/
/*  - **`freed`**: Total number of deallocations for this line.*/
/**/
typedef struct {
  unsigned int line;
  char file[256];
  STMemAllocBuf *allocs;
  unsigned int alloc_count;
  unsigned int alloc_allocated;
  unsigned int size;
  unsigned int allocated;
  unsigned int freed;
} STMemAllocLine;

/*- **`alloc_lines`**: An array to store memory allocation data for up to 1024
 * lines.*/
/*- **`alloc_line_count`**: Tracks the number of lines with allocations.*/
/*- **`alloc_mutex`**: A mutex for thread safety (initialized to `NULL`).*/
/*- **`alloc_mutex_lock`**: A function pointer for locking the mutex.*/
/*- **`alloc_mutex_unlock`**: A function pointer for unlocking the mutex.*/
STMemAllocLine alloc_lines[1024];
unsigned int alloc_line_count = 0;
void *alloc_mutex = NULL;
void (*alloc_mutex_lock)(void *mutex) = NULL;
void (*alloc_mutex_unlock)(void *mutex) = NULL;

/*- Initializes the memory debugging system with a mutex and its lock/unlock
 * functions.*/
void debug_memory_init(void (*lock)(void *mutex), void (*unlock)(void *mutex),
                       void *mutex) {
  alloc_mutex = mutex;
  alloc_mutex_lock = lock;
  alloc_mutex_unlock = unlock;
}

/*- Checks for memory overflows by verifying the magic number in the
 * over-allocated region.*/
/*- If an overflow is detected, it prints an error message and triggers a crash
 * (via `X[0] = 0`).*/
/**/
bool debug_memory(void) {
  bool output = false;
  unsigned int i, j, k;
  if (alloc_mutex != NULL)
    alloc_mutex_lock(alloc_mutex);
  for (i = 0; i < alloc_line_count; i++) {
    for (j = 0; j < alloc_lines[i].alloc_count; j++) {
      unsigned char *buf;
      unsigned int size;
      buf = alloc_lines[i].allocs[j].buf;
      size = alloc_lines[i].allocs[j].size;
      for (k = 0; k < MEMORY_OVER_ALLOC; k++)
        if (buf[size + k] != MEMORY_MAGIC_NUMBER)
          break;
      if (k < MEMORY_OVER_ALLOC) {
        printf("MEM ERROR: Overshoot at line %u in file %s\n",
               alloc_lines[i].line, alloc_lines[i].file);
        {
          unsigned int *X = NULL;
          X[0] = 0;
        }
        output = true;
      }
    }
  }
  if (alloc_mutex != NULL)
    alloc_mutex_unlock(alloc_mutex);
  return output;
}

/*- Adds a memory allocation to the tracking system.*/
/*- Initializes the over-allocated region with the magic number.*/
/*- Updates the allocation data for the corresponding file and line.*/
void debug_mem_add(void *pointer, unsigned int size, char *file,
                   unsigned int line) {
  unsigned int i, j;
  for (i = 0; i < MEMORY_OVER_ALLOC; i++)
    ((unsigned char *)pointer)[size + i] = MEMORY_MAGIC_NUMBER;

  for (i = 0; i < alloc_line_count; i++) {
    if (line == alloc_lines[i].line) {
      for (j = 0; file[j] != 0 && file[j] == alloc_lines[i].file[j]; j++)
        ;
      if (file[j] == alloc_lines[i].file[j])
        break;
    }
  }
  if (i < alloc_line_count) {
    if (alloc_lines[i].alloc_allocated == alloc_lines[i].alloc_count) {
      alloc_lines[i].alloc_allocated += 1024;
      alloc_lines[i].allocs =
          realloc(alloc_lines[i].allocs, (sizeof *alloc_lines[i].allocs) *
                                             alloc_lines[i].alloc_allocated);
    }
    alloc_lines[i].allocs[alloc_lines[i].alloc_count].size = size;
    alloc_lines[i].allocs[alloc_lines[i].alloc_count++].buf = pointer;
    alloc_lines[i].size += size;
    alloc_lines[i].allocated++;
  } else {
    if (i < 1024) {
      alloc_lines[i].line = line;
      for (j = 0; j < 255 && file[j] != 0; j++)
        alloc_lines[i].file[j] = file[j];
      alloc_lines[i].file[j] = 0;
      alloc_lines[i].alloc_allocated = 256;
      alloc_lines[i].allocs = malloc((sizeof *alloc_lines[i].allocs) *
                                     alloc_lines[i].alloc_allocated);
      alloc_lines[i].allocs[0].size = size;
      alloc_lines[i].allocs[0].buf = pointer;
      alloc_lines[i].alloc_count = 1;
      alloc_lines[i].size = size;
      alloc_lines[i].freed = 0;
      alloc_lines[i].allocated++;
      alloc_line_count++;
    }
  }
}

/*- **`debug_mem_malloc`**: Allocates memory and tracks it.*/
/*- **`debug_mem_remove`**: Removes a memory allocation from tracking.*/
/*- **`debug_mem_free`**: Frees memory and removes it from tracking.*/
/*- **`debug_mem_realloc`**: Reallocates memory and updates tracking.*/
void *debug_mem_malloc(unsigned int size, char *file, unsigned int line) {
  void *pointer;
  unsigned int i;
  if (alloc_mutex != NULL)
    alloc_mutex_lock(alloc_mutex);
  pointer = malloc(size + MEMORY_OVER_ALLOC);

  if (pointer == NULL) {
    printf("MEM ERROR: Malloc returns NULL when trying to allocate %u bytes at "
           "line %u in file %s\n",
           size, line, file);
    if (alloc_mutex != NULL)
      alloc_mutex_unlock(alloc_mutex);
    debug_mem_print(0);
    exit(0);
  }
  for (i = 0; i < size + MEMORY_OVER_ALLOC; i++)
    ((unsigned char *)pointer)[i] = MEMORY_MAGIC_NUMBER + 1;
  debug_mem_add(pointer, size, file, line);
  if (alloc_mutex != NULL)
    alloc_mutex_unlock(alloc_mutex);
  return pointer;
}

bool debug_mem_remove(void *buf) {
  unsigned int i, j, k;
  for (i = 0; i < alloc_line_count; i++) {
    for (j = 0; j < alloc_lines[i].alloc_count; j++) {
      if (alloc_lines[i].allocs[j].buf == buf) {
        for (k = 0; k < MEMORY_OVER_ALLOC; k++)
          if (((unsigned char *)buf)[alloc_lines[i].allocs[j].size + k] !=
              MEMORY_MAGIC_NUMBER)
            break;
        if (k < MEMORY_OVER_ALLOC)
          printf("MEM ERROR: Overshoot at line %u in file %s\n",
                 alloc_lines[i].line, alloc_lines[i].file);
        alloc_lines[i].size -= alloc_lines[i].allocs[j].size;
        alloc_lines[i].allocs[j] =
            alloc_lines[i].allocs[--alloc_lines[i].alloc_count];
        alloc_lines[i].freed++;
        return true;
      }
    }
  }
  return false;
}

void debug_mem_free(void *buf) {
  if (alloc_mutex != NULL)
    alloc_mutex_lock(alloc_mutex);
  if (!debug_mem_remove(buf)) {
    unsigned int *X = NULL;
    X[0] = 0;
  }
  free(buf);
  if (alloc_mutex != NULL)
    alloc_mutex_unlock(alloc_mutex);
}

void *debug_mem_realloc(void *pointer, unsigned int size, char *file,
                        unsigned int line) {
  unsigned int i, j, k, move;
  void *pointer2;
  if (pointer == NULL)
    return debug_mem_malloc(size, file, line);

  if (alloc_mutex != NULL)
    alloc_mutex_lock(alloc_mutex);
  for (i = 0; i < alloc_line_count; i++) {
    for (j = 0; j < alloc_lines[i].alloc_count; j++)
      if (alloc_lines[i].allocs[j].buf == pointer)
        break;
    if (j < alloc_lines[i].alloc_count)
      break;
  }
  if (i == alloc_line_count) {
    printf(" Mem debugger error. Trying to reallocate pointer %p in %s "
           "line %u. Pointer has never beein allocated\n",
           pointer, file, line);
    for (i = 0; i < alloc_line_count; i++) {
      for (j = 0; j < alloc_lines[i].alloc_count; j++) {
        unsigned int *buf;
        buf = alloc_lines[i].allocs[j].buf;
        for (k = 0; k < alloc_lines[i].allocs[j].size; k++) {
          if (&buf[k] == pointer) {
            printf("Trying to reallocate pointer %u bytes (out of %u) in to "
                   "allocation made in %s on line %u.\n",
                   k, alloc_lines[i].allocs[j].size, alloc_lines[i].file,
                   alloc_lines[i].line);
          }
        }
      }
    }
    exit(0);
  }
  move = alloc_lines[i].allocs[j].size;

  if (move > size)
    move = size;

  pointer2 = malloc(size + MEMORY_OVER_ALLOC);
  if (pointer2 == NULL) {
    printf("MEM ERROR: Realloc returns NULL when trying to allocate %u bytes "
           "at line %u in file %s\n",
           size, line, file);
    if (alloc_mutex != NULL)
      alloc_mutex_unlock(alloc_mutex);
    debug_mem_print(0);
    exit(0);
  }
  for (i = 0; i < size + MEMORY_OVER_ALLOC; i++)
    ((unsigned char *)pointer2)[i] = MEMORY_MAGIC_NUMBER + 1;
  memcpy(pointer2, pointer, move);

  debug_mem_add(pointer2, size, file, line);
  debug_mem_remove(pointer);
  free(pointer);

  if (alloc_mutex != NULL)
    alloc_mutex_unlock(alloc_mutex);
  return pointer2;
}

/*- **`debug_mem_print`**: Prints a report of memory allocations.*/
/*- **`debug_mem_consumption`**: Returns the total memory consumption.*/
/*- **`debug_mem_reset`**: Resets the memory tracking system.*/
void debug_mem_print(unsigned int min_allocs) {
  unsigned int i;
  if (alloc_mutex != NULL)
    alloc_mutex_lock(alloc_mutex);
  printf("Memory repport:\n----------------------------------------------\n");
  for (i = 0; i < alloc_line_count; i++) {
    if (min_allocs < alloc_lines[i].allocated) {
      printf("%s line: %u\n", alloc_lines[i].file, alloc_lines[i].line);
      printf(" - Bytes allocated: %u\n - Allocations: %u\n - Frees: %u\n\n",
             alloc_lines[i].size, alloc_lines[i].allocated,
             alloc_lines[i].freed);
    }
  }
  printf("----------------------------------------------\n");
  if (alloc_mutex != NULL)
    alloc_mutex_unlock(alloc_mutex);
}

unsigned int debug_mem_consumption(void) {
  unsigned int i, sum = 0;

  if (alloc_mutex != NULL)
    alloc_mutex_lock(alloc_mutex);
  for (i = 0; i < alloc_line_count; i++)
    sum += alloc_lines[i].size;
  if (alloc_mutex != NULL)
    alloc_mutex_unlock(alloc_mutex);
  return sum;
}

void debug_mem_reset(void) {
  unsigned int i;
  if (alloc_mutex != NULL)
    alloc_mutex_lock(alloc_mutex);
  for (i = 0; i < alloc_line_count; i++)
    free(alloc_lines[i].allocs);
  alloc_line_count = 0;

  if (alloc_mutex != NULL)
    alloc_mutex_unlock(alloc_mutex);
}

/*- Triggers a crash by dereferencing a null pointer.*/
void exit_crash(unsigned int i) {
  unsigned int *a = NULL;
  i = 0;
  a[0] = i;
}

/*This code is a robust memory debugging system that helps detect memory
 * overflows, track allocations, and provide detailed reports. It is thread-safe
 * (using mutexes) and can be integrated into larger projects for debugging
 * purposes.*/
/**/
/*The memory debugging system is a powerful tool for tracking memory
 * allocations, detecting memory leaks, and identifying common programming
 * mistakes such as buffer overflows and uninitialized memory usage. Below is a
 * detailed explanation of how the system works and why it is useful:*/

/*

### **Key Features of the Memory Debugging System**

1. **Replacement of Standard Memory Functions**:

   - By defining the `MEMORY_DEBUG` macro, the standard memory allocation
functions (`malloc`, `free`, and `realloc`) are replaced with custom debug
versions (`debug_mem_malloc`, `debug_mem_free`, and `debug_mem_realloc`).
   - These custom functions track every memory allocation, recording the file
name, line number, size, and pointer for each allocation.

2. **Memory Leak Detection**:

   - The system keeps track of all allocations and deallocations.
   - By calling `debug_mem_print`, it generates a report showing:
     - The file name and line number where each allocation was made.
     - The number of allocations and deallocations for each location.
     - The total amount of memory allocated and freed.
   - This makes it easy to identify memory leaks (allocations that were never
freed).

3. **Buffer Overflow Detection**:

   - Each allocation is over-allocated by 1024 bytes (or another specified
amount, e.g., `MEMORY_OVER_ALLOC`).
   - The over-allocated region is filled with a "magic number" (e.g.,
`MEMORY_MAGIC_NUMBER`).
   - The system periodically checks whether the magic number in the
over-allocated region has been overwritten.
   - If the magic number is corrupted, it indicates a buffer overflow (e.g.,
writing past the end of an allocated block).

4. **Detection of Uninitialized Memory Usage**:

   - Some operating systems (e.g., Linux) zero out memory when it is allocated
to prevent sensitive data from being exposed.
   - While this is a security feature, it can hide programming mistakes, such as
failing to initialize variables or pointers.
   - The memory debugging system fills allocated memory with non-zero "garbage"
values to make it easier to detect uninitialized memory usage.
   - This ensures that any reliance on zero-initialized memory will be caught
during debugging.

5. **Thread Safety**:
   - The system uses mutexes (`alloc_mutex`) to ensure thread safety when
tracking allocations and deallocations in a multi-threaded environment.

---

### **How the System Works**

1. **Initialization**:

   - The `debug_memory_init` function initializes the memory debugging system
with a mutex and its lock/unlock functions.
   - This ensures that the system is thread-safe.

2. **Memory Allocation**:

   - When `debug_mem_malloc` is called:
     - It allocates memory with additional space for the over-allocated region.
     - It fills the over-allocated region with the magic number.
     - It records the allocation in the `alloc_lines` array, including the file
name, line number, size, and pointer.

3. **Memory Deallocation**:

   - When `debug_mem_free` is called:
     - It checks whether the over-allocated region has been corrupted.
     - It removes the allocation from the tracking system.
     - It frees the memory.

4. **Memory Reallocation**:

   - When `debug_mem_realloc` is called:
     - It checks whether the original allocation is valid.
     - It allocates new memory, copies the data, and updates the tracking
system.
     - It frees the original memory.

5. **Memory Reporting**:

   - The `debug_mem_print` function generates a detailed report of all memory
allocations, including:
     - File names and line numbers.
     - Number of allocations and deallocations.
     - Total memory usage.

6. **Memory Consumption**:

   - The `debug_mem_consumption` function calculates the total amount of memory
currently allocated.

7. **Memory Reset**:
   - The `debug_mem_reset` function clears all tracked allocations, freeing any
internal memory used by the debugging system.

---

### **Why This System is Useful**

1. **Memory Leak Detection**:

   - By tracking all allocations and deallocations, the system makes it easy to
identify memory leaks.

2. **Buffer Overflow Detection**:

   - The over-allocated region and magic number help detect buffer overflows,
which are a common source of bugs and security vulnerabilities.

3. **Uninitialized Memory Detection**:

   - Filling allocated memory with non-zero values helps catch uninitialized
memory usage, which can lead to undefined behavior.

4. **Detailed Reporting**:

   - The system provides detailed reports that make it easy to pinpoint the
source of memory-related issues.

5. **Thread Safety**:

   - The use of mutexes ensures that the system works correctly in
multi-threaded applications.

6. **Cross-Platform Compatibility**:
   - The system works on any platform, regardless of whether the operating
system zeroes out memory.
 */
