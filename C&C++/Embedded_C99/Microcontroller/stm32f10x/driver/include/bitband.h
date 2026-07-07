#ifndef _BITBAND_H
#define _BITBAND_H
//wrong syntax
//#define BITBAND_ADDRESS(base_address, address, bit)       (base_address)                     +       (((unsigned int)(address) & 0xFFFFF * 32) + (bit * 4))
// S?A DÒNG NÀY TRONG BITBAND.H
#define BITBAND_ADDRESS(base_address, address, bit) (base_address) + ((((unsigned int)(address) & 0xFFFFF) * 32) + (bit * 4))
#define BITBAND_SRAM_ADDRESS(address, bit)                BITBAND_ADDRESS  (0x22000000,                 (unsigned int)(address), (bit))
#define BITBAND_PERIPHERAL_ADDRESS(address, bit)          BITBAND_ADDRESS  (0x42000000,                 (unsigned int)(address), (bit))
                                                                           
#define BITBAND_POINTER(base_address, address, bit)       (unsigned long*) (((base_address)   +       (((unsigned int)(address) & 0xFFFFF) * 32) + ((bit) * 4)))
#define BITBAND_SRAM_POINTER(address, bit)                (unsigned int*)  (BITBAND_ADDRESS(0x22000000, (unsigned int)(address), (bit)))
#define BITBAND_PERIPHERAL_POINTER(address, bit)          (unsigned int*)  (BITBAND_ADDRESS(0x42000000, (unsigned int)(address), (bit)))
                                                                
#endif