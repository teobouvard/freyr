#include <stdio.h>
#include <unistd.h>

void app_main(void) {
  for (int i = 0;;i++) {
    printf("i = %d\n", i);
    sleep(1);
  }
}
