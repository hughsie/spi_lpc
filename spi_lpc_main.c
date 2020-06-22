// SPDX-License-Identifier: GPL-2.0
/*
 * SPI LPC flash platform security driver
 *
 * Copyright 2020 (c) Richard Hughes (richard@hughsie.com)
 *                    Daniel Gutson (daniel.gutson@eclypsium.com)
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/security.h>
#include "bios_data_access.h"
#include "low_level_access.h"

#define SIZE_WORD sizeof(u16)
#define WORD_MASK 0xFFFFu
#define LOW_WORD(x) ((x)&WORD_MASK)
#define HIGH_WORD(x) ((x) >> ((SIZE_WORD * 8)) & WORD_MASK)

static enum PCH_Arch pch_arch;
static enum CPU_Arch cpu_arch;

static struct dentry *spi_dir;
static struct dentry *spi_bioswe;
static struct dentry *spi_ble;
static struct dentry *spi_smm_bwp;

typedef int Read_BC_Flag_Fn(struct BC *bc, u64 *value);

static int get_pci_vid_did(u8 bus, u8 dev, u8 fun, u16 *vid, u16 *did)
{
	u32 vid_did;
	int ret = pci_read_dword(&vid_did, bus, dev, fun, 0);
	if (ret == 0) {
		*vid = LOW_WORD(vid_did);
		*did = HIGH_WORD(vid_did);
	}
	return ret;
}

static int get_pch_arch(enum PCH_Arch *pch_arch)
{
	u16 pch_vid;
	u16 pch_did;
	int ret = get_pci_vid_did(0, 0x1f, 0, &pch_vid, &pch_did);
	if (ret != 0)
		return ret;

	pr_debug("PCH VID: %x - DID: %x\n", pch_vid, pch_did);
	ret = viddid2pch_arch(pch_vid, pch_did, pch_arch);

	return ret;
}

static int get_cpu_arch(enum CPU_Arch *cpu_arch)
{
	u16 cpu_vid;
	u16 cpu_did;
	int ret = get_pci_vid_did(0, 0, 0, &cpu_vid, &cpu_did);
	if (ret != 0)
		return ret;

	pr_debug("CPU VID: %x - DID: %x\n", cpu_vid, cpu_did);
	ret = viddid2cpu_arch(cpu_vid, cpu_did, cpu_arch);

	return ret;
}

static int get_pch_cpu(enum PCH_Arch *pch_arch, enum CPU_Arch *cpu_arch)
{
	const int cpu_res = get_cpu_arch(cpu_arch);
	const int pch_res = get_pch_arch(pch_arch);

	return cpu_res != 0 && pch_res != 0 ? -EIO : 0;
}

/* Buffer to return: always 3 because of the following chars:
 *     value \n \0
 */
#define BUFFER_SIZE 3

static ssize_t bc_flag_read(struct file *filp, char __user *buf, size_t count,
			    loff_t *ppos)
{
	char tmp[BUFFER_SIZE];
	ssize_t ret;
	u64 value = 0;
	struct BC bc;

	if (*ppos == BUFFER_SIZE)
		return 0; /* nothing else to read */

	if (file_inode(filp)->i_private == NULL)
		return -EIO;

	ret = read_BC(pch_arch, cpu_arch, &bc);

	if (ret == 0)
		ret = ((Read_BC_Flag_Fn *)file_inode(filp)->i_private)(&bc,
								       &value);

	if (ret != 0)
		return ret;

	sprintf(tmp, "%d\n", (int)value & 1);
	ret = simple_read_from_buffer(buf, count, ppos, tmp, sizeof(tmp));

	return ret;
}

static const struct file_operations bc_flags_ops = {
	.read = bc_flag_read,
};

static int __init mod_init(void)
{
	int ret = 0;
	if (get_pch_cpu(&pch_arch, &cpu_arch) != 0) {
		pr_err("Couldn't detect PCH or CPU\n");
		return -EIO;
	}

	spi_dir = securityfs_create_dir("firmware", NULL);
	if (IS_ERR(spi_dir)) {
		pr_err("Couldn't create firmware securityfs dir\n");
		return PTR_ERR(spi_dir);
	}

#define create_file(name, function)                                            \
	do {                                                                   \
		spi_##name = securityfs_create_file(#name, 0600, spi_dir,      \
						    &function, &bc_flags_ops); \
		if (IS_ERR(spi_##name)) {                                      \
			pr_err("Error creating securityfs file " #name "\n");  \
			ret = PTR_ERR(spi_##name);                             \
			goto out_##name;                                       \
		}                                                              \
	} while (0)

	create_file(bioswe, read_BC_BIOSWE);
	create_file(ble, read_BC_BLE);
	create_file(smm_bwp, read_BC_SMM_BWP);

	return 0;

out_smm_bwp:
	securityfs_remove(spi_smm_bwp);
out_ble:
	securityfs_remove(spi_ble);
out_bioswe:
	securityfs_remove(spi_bioswe);
	securityfs_remove(spi_dir);
	return ret;
}

static void __exit mod_exit(void)
{
	securityfs_remove(spi_smm_bwp);
	securityfs_remove(spi_ble);
	securityfs_remove(spi_bioswe);
	securityfs_remove(spi_dir);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_DESCRIPTION("SPI LPC flash platform security driver");
MODULE_AUTHOR("Richard Hughes <richard@hughsie.com>");
MODULE_AUTHOR("Daniel Gutson <daniel.gutson@eclypsium.com>");
MODULE_LICENSE("GPL");
