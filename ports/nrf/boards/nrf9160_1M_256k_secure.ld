/* Specify the memory areas */
MEMORY
{
    FLASH_TEXT (rx) : ORIGIN = 0x0000c200,     LENGTH = 0x3e00
    RAM (xrw)       : ORIGIN = 0x20000000,     LENGTH = 64K
}
