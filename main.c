#define MEMORY_DEBUG
#define EXIT_CRASH
#include "utils.h"

int main(int argc, char **argv) {

  // Initialize the memory debugging system
  debug_memory_init(NULL, NULL, NULL);
  debug_mem_print(0);

  printf("argc: %d\n", argc);
  int i;
  for (i = 0; i < argc; i++) {
    printf("argv[%d]: %s\n", i, argv[i]);
    puts(__FILE__);
    printf("%d\n", __LINE__);
  }

  size_t n = 5;
  Vector v = create_vector(n);
  debug_mem_print(0);
  for (size_t i = 0; i < n; ++i)
    set_index_vector(&v, i, i);
  debug_mem_print(0);

  print_vector(&v); /* 0 1 2 3 4 */
  right_rotate_n_times_vector(&v, 5 * 1000);
  print_vector(&v); // 0 1 2 3 4
  right_rotate_n_times_vector(&v, 2);
  print_vector(&v); // 3 4 0 1 2
  size_t s = get_size_vector(&v);
  printf("%lu\n", s);
  left_rotate_vector(&v);
  print_vector(&v); // 4 0 1 2 3
  int val = pop_vector(&v, 2);
  printf("%d\n", val); // 1
  print_vector(&v);    // 4 0 2 3
  val = find_transposition_vector(&v, 3);
  printf("%d\n", val); // 2
  print_vector(&v);    // 4 0 3 2
  destroy_vector(&v);

  // Allocate some memory
  int *ptr1 = (int *)malloc(100 * sizeof(int));
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
