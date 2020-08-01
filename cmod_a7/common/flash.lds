/*
 * Linker script which reserves the first 0xE0 bytes for the bootloader,
 * such that the user program always starts from a fixed address
 * packs program code in SPI flash memory
 */

MEMORY{
    blockRam (RWXAI) : ORIGIN = 0x00000000, LENGTH = BLOCK_RAM_SIZE
    FLASH    (RXI)   : ORIGIN = 0x01200000, LENGTH = 0x200000
}

ENTRY(start);

SECTIONS {
    /*
    .bootloader :
    {
        KEEP(*bootloader.o(.text));
        . = 0xE0;
    } >blockRam
    */

    /*
    .ram_code :
    {
        *(.ram_code*);
    } > blockRam
    */

    /* The program code and other data goes into FLASH */
    .text :
    {
        . = ALIGN(4);
        _startup_adr = .;
        *(.text)           /* .text sections (code) */
        *(.text*)          /* .text* sections (code) */
        *(.rodata)         /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
        *(.srodata)        /* .rodata sections (constants, strings, etc.) */
        *(.srodata*)       /* .rodata* sections (constants, strings, etc.) */
        . = ALIGN(4);
        _etext = .;        /* define a global symbol at end of code */
        _sidata = _etext;  /* This is used by the startup in order to initialize the .data secion */
    } >FLASH

    /* This is the initialized data section
    The program executes knowing that the data is in the blockRam
    but the loader puts the initial values in the FLASH (inidata).
    It is one task of the startup to copy the initial values from FLASH to blockRam. */
    .data : AT ( _sidata )
    {
        . = ALIGN(4);

        _sdata = .;  /* create a global symbol at data start; used by startup code in order to initialise the .data section in blockRam */
        _ram_start = .;    /* create a global symbol at blockRam start for garbage collector */
        . = ALIGN(4);
        *(.data)           /* .data sections */
        *(.data*)          /* .data* sections */
        *(.sdata)           /* .sdata sections */
        *(.sdata*)          /* .sdata* sections */
        . = ALIGN(4);
        _edata = .;        /* define a global symbol at data end; used by startup code in order to initialise the .data section in blockRam */
    } >blockRam

    /* Uninitialized data section */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;         /* define a global symbol at bss start; used by startup code */
        *(.bss)
        *(.bss*)
        *(.sbss)
        *(.sbss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;         /* define a global symbol at bss end; used by startup code */
    } >blockRam

    /* this is to define the start of the heap, and make sure we have a minimum size */
    .heap :
    {
        . = ALIGN(4);
        _heap_start = .;    /* define a global symbol at heap start */
    } >blockRam
}
