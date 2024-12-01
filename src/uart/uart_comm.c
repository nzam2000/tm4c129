/**
 * @file uart_comm.c
 * @author Nicholas Zamora
 * @date 11/30/24
 * @brief UART driver for serial PC communications. Initializes on PA1-0.
 *  Assumes use of 16 MHz PIOSC to clock peripheral. Sets to 115200 baud rate.
 *  Only one singleton UART comm.
 */

#include <stdbool.h>
#include "uart.h"
#include "tm4c1294ncpdt.h"

typedef struct
{
} UartCommDrvData;

static void UartComm_Init(void* drv_data) {
  // configure clock
  SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0; // activate clock for UART0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0; // activate clock for Port A
  while ((SYSCTL_PRUART_R & SYSCTL_PRUART_R0) == 0) {
  }; // allow time for clock to stabilize
  // configure baud rate
  UART0_CTL_R &= ~UART_CTL_UARTEN; // disable UART
  UART0_IBRD_R = 8; // IBRD = int(16,000,000 / (16 * 115,200)) = int(8.681)
  UART0_FBRD_R = 44; // FBRD = round(0.6806 * 64) = 44
  // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_LCRH_R = (UART_LCRH_WLEN_8 | UART_LCRH_FEN);
  UART0_CC_R = (UART0_CC_R & ~UART_CC_CS_M) + UART_CC_CS_PIOSC;
  // the alternate clock source is the PIOSC (default)
  SYSCTL_ALTCLKCFG_R = (SYSCTL_ALTCLKCFG_R & ~SYSCTL_ALTCLKCFG_ALTCLK_M) +
                       SYSCTL_ALTCLKCFG_ALTCLK_PIOSC;
  UART0_CTL_R &= ~UART_CTL_HSE;
  // high-speed disable; divide clock by 16 rather than 8 (default)
  UART0_CTL_R |= UART_CTL_UARTEN; // enable UART
  while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R0) == 0) {
  }; // allow time for clock to stabilize
  GPIO_PORTA_AFSEL_R |= 0x03; // enable alt funct on PA1-0
  GPIO_PORTA_DEN_R |= 0x03; // enable digital I/O on PA1-0
  // configure PA1-0 as UART
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFFFF00) + 0x00000011;
  GPIO_PORTA_AMSEL_R &= ~0x03; // disable analog functionality on PA
}

static bool UartComm_Write(void* drv_data, const uint8_t c_out) {
  while ((UART0_FR_R & UART_FR_TXFF) != 0) {
  }; // allow hardware to process existing FIFO elements
  UART0_DR_R = c_out;
  return true;
}

static bool UartComm_Read(void* drv_data, uint8_t* c_in) {
  while ((UART0_FR_R & UART_FR_RXFE) != 0) {
  }; // block until data available
  if (c_in) {
    *c_in = UART0_DR_R;
  }
  return true;
}

UartDrv uart_comm_drv = {
    .Init = UartComm_Init,
    .Write = UartComm_Write,
    .Read = UartComm_Read,
};

// singleton, keep controller declaration non-static
#define DECLARE_UART_COMM(name)                 \
  static UartCommDrvData name##_drv_data;       \
  static UartController name##_inst = {         \
  .drv_data = &name##_drv_data,                 \
  .drv = &uart_comm_drv,                        \
  };                                            \
  UartController* name = &name##_inst;

DECLARE_UART_COMM(uart_comm);
