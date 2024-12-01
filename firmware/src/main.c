/**
 * @file main.c
 * @author Nicholas Zamora
 * @date 11/27/24
 * @brief Main function
 */
#include <stdbool.h>
#include "uart_comm.h"

void SystemInit();

int main() {
  SystemInit();

  while (true) {
  }
  return 0;
}

void SystemInit() {
  UartInit(uart_comm);
}
