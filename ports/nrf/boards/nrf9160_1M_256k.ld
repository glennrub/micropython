/*
    GNU linker script for NRF9160 NS
*/

_flash_size = 1M;
_ram_size   = 256K;
_sd_size = 0x00040000;
_sd_ram  = 0x00020000;
_fs_size = 20K;

/* produce a link error if there is not this amount of RAM for these sections */
_stack_size = 16K;
_minimum_heap_size = 64K;
