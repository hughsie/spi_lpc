#include <linux/version.h>
#include <linux/pci.h>
#include "low_level_access.h"

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
/* 'ioremap_nocache' was deprecated in kernels >= 5.6, so instead we
     use 'ioremap' which is no-cache by default since kernels 2.6.25. */
#define IOREMAP_NO_CACHE(address, size) ioremap(address, size)
#else /* KERNEL_VERSION < 2.6.25 */
#define IOREMAP_NO_CACHE(address, size) ioremap_nocache(address, size)
#endif

#define GENERIC_MMIO_READ(Type, Suffix, function)                              \
	int mmio_read_##Suffix(u64 phys_address, Type *value)                  \
	{                                                                      \
		int ret = 0;                                                   \
		void __iomem *mapped_address =                                 \
			IOREMAP_NO_CACHE(phys_address, sizeof(Type));          \
		pr_info("Reading MMIO 0x%llx 0x%lx\n", phys_address,           \
			sizeof(Type));                                         \
		if (mapped_address != NULL) {                                  \
			*value = function(mapped_address);                     \
			iounmap(mapped_address);                               \
		} else {                                                       \
			pr_err("Failed to MAP IO memory: 0x%llx\n",            \
			       phys_address);                                  \
			ret = -1;                                              \
		}                                                              \
		return ret;                                                    \
	}
GENERIC_MMIO_READ(u8, byte, readb)
GENERIC_MMIO_READ(u16, word, readw)
GENERIC_MMIO_READ(u32, dword, readl)
#undef GENERIC_MMIO_READ

#define GENERIC_PCI_READ(Suffix, Type)                                         \
	int pci_read_##Suffix(Type *value, u64 bus, u64 device, u64 function,  \
			      u64 offset)                                      \
	{                                                                      \
		int ret;                                                       \
		struct pci_bus *found_bus = pci_find_bus(0, bus);              \
		pr_info("Reading PCI 0x%llx 0x%llx 0x%llx 0x%llx \n", bus,     \
			device, function, offset);                             \
		if (found_bus != NULL) {                                       \
			ret = pci_bus_read_config_##Suffix(                    \
				found_bus, PCI_DEVFN(device, function),        \
				offset, value);                                \
		} else {                                                       \
			pr_err("Couldn't find Bus 0x%lld\n", bus);             \
			ret = -1;                                              \
		}                                                              \
		return ret;                                                    \
	}

GENERIC_PCI_READ(byte, u8)
GENERIC_PCI_READ(word, u16)
GENERIC_PCI_READ(dword, u32)

#undef GENERIC_PCI_READ
