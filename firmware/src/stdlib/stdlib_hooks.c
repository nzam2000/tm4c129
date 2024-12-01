//
// Created by nzamora on 9/22/2024.
//
#include <stdio.h>
#include <string.h>
#include "uart_comm.h"

int getchar(void) {
  char c = '\0';
  if (UartRead(uart_comm, &c)) {
    return c;
  }
  return EOF;
}

int putchar(int c) {
  if (c <= UINT8_MAX && UartWrite(uart_comm, (uint8_t)c)) {
    return c;
  }
  return EOF;
}

size_t strlen(const char* s) {
  size_t len = 0;
  while (*s++ != '\0') {
    ++len;
  }
  return len;
}

int puts(const char* s) {
  while (*s != '\0') {
    if (putchar(*s++) == EOF) {
      return EOF;
    }
  }
  if (putchar('\n') == EOF) {
    return EOF;
  }
  return 1;
}
