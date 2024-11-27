/**
 * @file startup_tm4c129.c
 * @author Nicholas Zamora
 * @date 11/27/24
 * @brief Startup routines for the tm4c129
 */
#include <stdint.h>

void Reset_Handler();
void Default_Handler();

/*
 * Vector table
 */
extern uint32_t __stack_top;

void (*vectors[])() __attribute__((section(".vectors"))) = {
    (void (*)(void))&__stack_top,
    Reset_Handler,
    Default_Handler, // The NMI handler
    Default_Handler, // The hard fault handler
    Default_Handler, // The MPU fault handler
    Default_Handler, // The bus fault handler
    Default_Handler, // The usage fault handler
    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // Reserved
    Default_Handler, // SVCall handler
    Default_Handler, // Debug monitor handler
    0, // Reserved
    Default_Handler, // The PendSV handler
    Default_Handler, // The SysTick handler

    Default_Handler, // GPIO Port A
    Default_Handler, // GPIO Port B
    Default_Handler, // GPIO Port C
    Default_Handler, // GPIO Port D
    Default_Handler, // GPIO Port E
    Default_Handler, // UART0 Rx and Tx
    Default_Handler, // UART1 Rx and Tx
    Default_Handler, // SSI0 Rx and Tx
    Default_Handler, // I2C0 Master and Slave
    Default_Handler, // PWM Fault
    Default_Handler, // PWM Generator 0
    Default_Handler, // PWM Generator 1
    Default_Handler, // PWM Generator 2
    Default_Handler, // Quadrature Encoder 0
    Default_Handler, // ADC Sequence 0
    Default_Handler, // ADC Sequence 1
    Default_Handler, // ADC Sequence 2
    Default_Handler, // ADC Sequence 3
    Default_Handler, // Watchdog timer
    Default_Handler, // Timer 0 subtimer A
    Default_Handler, // Timer 0 subtimer B
    Default_Handler, // Timer 1 subtimer A
    Default_Handler, // Timer 1 subtimer B
    Default_Handler, // Timer 2 subtimer A
    Default_Handler, // Timer 2 subtimer B
    Default_Handler, // Analog Comparator 0
    Default_Handler, // Analog Comparator 1
    Default_Handler, // Analog Comparator 2
    Default_Handler, // System Control (PLL, OSC, BO)
    Default_Handler, // FLASH Control
    Default_Handler, // GPIO Port F
    Default_Handler, // GPIO Port G
    Default_Handler, // GPIO Port H
    Default_Handler, // UART2 Rx and Tx
    Default_Handler, // SSI1 Rx and Tx
    Default_Handler, // Timer 3 subtimer A
    Default_Handler, // Timer 3 subtimer B
    Default_Handler, // I2C1 Master and Slave
    Default_Handler, // CAN0
    Default_Handler, // CAN1
    Default_Handler, // Ethernet
    Default_Handler, // Hibernate
    Default_Handler, // USB0
    Default_Handler, // PWM Generator 3
    Default_Handler, // uDMA Software Transfer
    Default_Handler, // uDMA Error
    Default_Handler, // ADC1 Sequence 0
    Default_Handler, // ADC1 Sequence 1
    Default_Handler, // ADC1 Sequence 2
    Default_Handler, // ADC1 Sequence 3
    Default_Handler, // External Bus Interface 0
    Default_Handler, // GPIO Port J
    Default_Handler, // GPIO Port K
    Default_Handler, // GPIO Port L
    Default_Handler, // SSI2 Rx and Tx
    Default_Handler, // SSI3 Rx and Tx
    Default_Handler, // UART3 Rx and Tx
    Default_Handler, // UART4 Rx and Tx
    Default_Handler, // UART5 Rx and Tx
    Default_Handler, // UART6 Rx and Tx
    Default_Handler, // UART7 Rx and Tx
    Default_Handler, // I2C2 Master and Slave
    Default_Handler, // I2C3 Master and Slave
    Default_Handler, // Timer 4 subtimer A
    Default_Handler, // Timer 4 subtimer B
    Default_Handler, // Timer 5 subtimer A
    Default_Handler, // Timer 5 subtimer B
    Default_Handler, // FPU
    0, // Reserved
    0, // Reserved
    Default_Handler, // I2C4 Master and Slave
    Default_Handler, // I2C5 Master and Slave
    Default_Handler, // GPIO Port M
    Default_Handler, // GPIO Port N
    0, // Reserved
    Default_Handler, // Tamper
    Default_Handler, // GPIO Port P (Summary or P0)
    Default_Handler, // GPIO Port P1
    Default_Handler, // GPIO Port P2
    Default_Handler, // GPIO Port P3
    Default_Handler, // GPIO Port P4
    Default_Handler, // GPIO Port P5
    Default_Handler, // GPIO Port P6
    Default_Handler, // GPIO Port P7
    Default_Handler, // GPIO Port Q (Summary or Q0)
    Default_Handler, // GPIO Port Q1
    Default_Handler, // GPIO Port Q2
    Default_Handler, // GPIO Port Q3
    Default_Handler, // GPIO Port Q4
    Default_Handler, // GPIO Port Q5
    Default_Handler, // GPIO Port Q6
    Default_Handler, // GPIO Port Q7
    Default_Handler, // GPIO Port R
    Default_Handler, // GPIO Port S
    Default_Handler, // SHA/MD5 0
    Default_Handler, // AES 0
    Default_Handler, // DES3DES 0
    Default_Handler, // LCD Controller 0
    Default_Handler, // Timer 6 subtimer A
    Default_Handler, // Timer 6 subtimer B
    Default_Handler, // Timer 7 subtimer A
    Default_Handler, // Timer 7 subtimer B
    Default_Handler, // I2C6 Master and Slave
    Default_Handler, // I2C7 Master and Slave
    Default_Handler, // HIM Scan Matrix Keyboard 0
    Default_Handler, // One Wire 0
    Default_Handler, // HIM PS/2 0
    Default_Handler, // HIM LED Sequencer 0
    Default_Handler, // HIM Consumer IR 0
    Default_Handler, // I2C8 Master and Slave
    Default_Handler, // I2C9 Master and Slave
    Default_Handler // GPIO Port T
};

/*
 * Handlers
 */
void Reset_Handler()
{
    // copy .data section from flash to RAM
    extern uint32_t __etext; // LMA
    extern uint32_t __sdata; // VMA
    extern uint32_t __edata;
    uint32_t* etext = &__etext;
    uint32_t* sdata = &__sdata;
    uint32_t* edata = &__edata;

    while (sdata <= edata)
    {
        *sdata++ = *etext++;
    }

    // zero .bss section
    extern uint32_t __sbss;
    extern uint32_t __ebss;
    uint32_t* sbss = &__sbss;
    uint32_t* ebss = &__ebss;

    while (sbss <= ebss)
    {
        *sbss++ = 0;
    }

    // call main
    extern int main();
    main();
    while (1); // we should never get here
    // could appy POR instead
    // or decode the return value from main
}

void Default_Handler()
{
    while (1)
    {
    }
}
