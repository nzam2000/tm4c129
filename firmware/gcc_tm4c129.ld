MEMORY
{
    FLASH   (rx)    : ORIGIN = 0x00000000, LENGTH = 1024K
    RAM     (rwx)   : ORIGIN = 0x20000000, LENGTH = 256K
}

ENTRY(Reset_Handler)

SECTIONS
{
    .text :
    {
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
    } > FLASH

    __etext = ALIGN(4);

    .data : AT(__etext)
    {
        __sdata = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        __edata = .;
    } > RAM

    .bss :
    {
        . = ALIGN(4);
        __sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        __ebss = .;
    }

    . = (ORIGIN(RAM) + LENGTH(RAM)) - 0x10; /* save space for alignment */
    . = ALIGN(8);
    __stack_top = .;
}
