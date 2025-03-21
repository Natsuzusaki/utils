#define MEMORY_DEBUG
#define EXIT_CRASH
#include "utils.h"
#include <stdlib.h>

int main(int argc, char **argv) {

  printf("argc: %d\n", argc);
  int i;
  for (i = 0; i < argc; i++) {
    printf("argv[%d]: %s\n", i, argv[i]);
    puts(__FILE__);
    printf("%d\n", __LINE__);
  }

  debug_puts("hello", __LINE__, __FILE__);
  puts("print_matrix_neighbor_coordinates_rules()");
  print_matrix_neighbor_coordinates_rules();
  puts("hello");

  puts("All tests passed!");

  // Initialize the memory debugging system
  debug_memory_init(NULL, NULL, NULL);

  // Allocate some memory
  /*int *ptr1 = (int *)debug_mem_malloc(100, __FILE__, __LINE__);*/
  /*int *ptr2 = (int *)debug_mem_malloc(200, __FILE__, __LINE__);*/
  int *ptr1 = (int *)malloc(100 * sizeof(int));
  debug_mem_print(0);
  int *ptr2 = (int *)malloc(200 * sizeof(int));

  // Free some memory
  debug_mem_free(ptr1);

  // Print memory report
  debug_mem_print(0);

  // Free remaining memory
  debug_mem_free(ptr2);

  debug_mem_print(0);

  int *ptr3 = (int *)malloc(100 * sizeof(int));
  debug_mem_print(0);
  int *ptr4 = (int *)malloc(200 * sizeof(int));
  ptr3 = realloc(ptr3, 700);
  debug_mem_print(0);
  debug_mem_free(ptr3);
  debug_mem_free(ptr4);
  debug_mem_print(0);
  debug_mem_reset();

  exit(0);

  return 0;
}
