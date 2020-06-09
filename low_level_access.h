#ifndef LOW_LEVEL_H
#define LOW_LEVEL_H

#include <linux/types.h>

int pci_read_byte(u8 *value, u64 bus, u64 device, u64 function, u64 offset);
int pci_read_word(u16 *value, u64 bus, u64 device, u64 function, u64 offset);
int pci_read_dword(u32 *value, u64 bus, u64 device, u64 function, u64 offset);

int mmio_read_byte(u64 phys_address, u8 *value);
int mmio_read_word(u64 phys_address, u16 *value);
int mmio_read_dword(u64 phys_address, u32 *value);

#endif /* LOW_LEVEL_H */
