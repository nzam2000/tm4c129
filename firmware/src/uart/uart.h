/**
 * @file uart.h
 * @author Nicholas Zamora
 * @date 11/30/24
 * @brief
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
 void (*Init)(void* drv_data);
 bool (*Write)(void* drv_data, uint8_t c_out);
 bool (*Read)(void* drv_data, uint8_t* c_in);
} UartDrv;

typedef struct {
 void* drv_data;
 UartDrv* drv;
} UartController;

#define UartInit(ctrl) ((ctrl)->drv->Init((ctrl)->drv_data))
#define UartWrite(ctrl, c_out) ((ctrl)->drv->Write((ctrl)->drv_data, (c_out)))
#define UartRead(ctrl, c_in) ((ctrl)->drv->Read((ctrl)->drv_data, (c_in)))

#endif //UART_H
