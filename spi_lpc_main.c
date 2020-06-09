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

#include <linux/module.h>
#include <linux/security.h>
#include <linux/pci.h>
#include "data_access.h"
#include "low_level_access.h"

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#define SIZE_BYTE sizeof(u8)
#define SIZE_WORD sizeof(u16)
#define SIZE_DWORD sizeof(u32)
#define SIZE_QWORD sizeof(u64)
#define BYTE_MASK 0xFFu
#define WORD_MASK 0xFFFFu
#define DWORD_MASK 0xFFFFFFFFu
#define HIGH_DWORD_MASK (0xFFFFFFFFull << (SIZE_DWORD * 8u))

enum PCH_Arch pch_arch;
enum CPU_Arch cpu_arch;

static int get_pci_vid_did(u8 bus, u8 dev, u8 fun, u16* vid, u16* did)
{
    u32 vid_did;
    int ret = pci_read_dword(&vid_did, bus, dev, fun, 0);
    if (ret == 0)
    {
        *vid = vid_did & WORD_MASK;
        *did = (vid_did >> (SIZE_WORD * 8)) & WORD_MASK;
    }
    return ret;
}

static int get_pch_arch(enum PCH_Arch* pch_arch)
{
    u16 pch_vid;
    u16 pch_did;
    int ret = get_pci_vid_did(0, 0x1f, 0, &pch_vid, &pch_did);
    if (ret == 0)
    {
        pr_info("PCH VID: %x - DID: %x\n", pch_vid, pch_did);
        ret = viddid2pch_arch(pch_vid, pch_did, pch_arch);
    }
    return ret;
}

static int get_cpu_arch(enum CPU_Arch* cpu_arch)
{
    u16 cpu_vid;
    u16 cpu_did;
    int ret = get_pci_vid_did(0, 0, 0, &cpu_vid, &cpu_did);
    if (ret == 0)
    {
        pr_info("CPU VID: %x - DID: %x\n", cpu_vid, cpu_did);
        ret = viddid2cpu_arch(cpu_vid, cpu_did, cpu_arch);
    }
    return ret;
}

static int get_pch_cpu(enum PCH_Arch* pch_arch, enum CPU_Arch* cpu_arch)
{
    const int cpu_res = get_cpu_arch(cpu_arch);
    const int pch_res = get_pch_arch(pch_arch);

    return cpu_res != 0 && pch_res != 0 ? -1 : 0;
}

struct dentry *spi_dir;
struct dentry *spi_bioswe;
struct dentry *spi_ble;
struct dentry *spi_smm_bwp;

// Buffer to return: always 3 because of the following chars: value \n \0
#define BUFFER_SIZE 3

static ssize_t bioswe_read(struct file *filp, char __user *buf,
               size_t count, loff_t *ppos)
{
    if (*ppos < BUFFER_SIZE)
    {
        char tmp[BUFFER_SIZE];
        ssize_t ret;
        u64 bioswe = 0;
        struct BC bc;

        ret = read_BC(pch_arch, cpu_arch, &bc);
        if (ret == 0)
            ret = read_BC_BIOSWE(&bc, &bioswe);

        if (ret == 0)
        {
            pr_debug("BIOSWE: %lld\n", bioswe);
            sprintf(tmp, "%d\n", (int) bioswe & 1);
            ret = simple_read_from_buffer(buf, count, ppos, tmp, sizeof(tmp));
        }
        else
        {
            pr_err("Error reading BIOSWE\n");
        }
        return ret;
    }
    else
    {
        return 0; // nothing else to read
    }
}

static const struct file_operations spi_bioswe_ops = {
    .read  = bioswe_read,
};

static ssize_t ble_read(struct file *filp, char __user *buf,
            size_t count, loff_t *ppos)
{
    if (*ppos < BUFFER_SIZE)
    {
        char tmp[BUFFER_SIZE];
        u64 ble = 0;
        ssize_t ret;
        struct BC bc;

        ret = read_BC(pch_arch, cpu_arch, &bc);
        if (ret == 0)
            ret = read_BC_BLE(&bc, &ble);

        if (ret == 0)
        {
            pr_debug("BLE: %lld\n", ble);
            sprintf(tmp, "%d\n", (int) ble & 1);
            ret = simple_read_from_buffer(buf, count, ppos, tmp, sizeof(tmp));
        }
        else
        {
            pr_err("Error reading BLE\n");
        }

        return ret;
    }
    else
    {
        return 0;
    }
}

static const struct file_operations spi_ble_ops = {
    .read  = ble_read,
};

static ssize_t smm_bwp_read(struct file *filp, char __user *buf,
                size_t count, loff_t *ppos)
{
    if (*ppos < BUFFER_SIZE)
    {
        char tmp[BUFFER_SIZE];
        u64 smm_bwp = 0;
        ssize_t ret;
        struct BC bc;

        ret = read_BC(pch_arch, cpu_arch, &bc);
        if (ret == 0)
            ret = read_BC_SMM_BWP(&bc, &smm_bwp);

        if (ret == 0)
        {
            pr_debug("SMM_BWP: %lld\n", smm_bwp);
            sprintf(tmp, "%d\n", (int)smm_bwp & 1);
            ret = simple_read_from_buffer(buf, count, ppos, tmp, sizeof(tmp));
        }
        else
        {
            pr_err("Error reading SMM_BWP\n");
        }

        return ret;
    }
    else
    {
        return 0;
    }
}

static const struct file_operations spi_smm_bwp_ops = {
    .read  = smm_bwp_read,
};

static int __init mod_init(void)
{
    if (get_pch_cpu(&pch_arch, &cpu_arch) != 0)
    {
        pr_err("Couldn't detect PCH or CPU\n");
        return -1;
    }

    spi_dir = securityfs_create_dir("firmware", NULL);
    if (IS_ERR(spi_dir))
    {
        pr_err("Couldn't create firmware sysfs dir\n");
        return -1;
    }
    else
    {
        pr_info("firmware securityfs dir creation successful\n");
    }

    spi_bioswe =
        securityfs_create_file("bioswe",
                   0600, spi_dir, NULL,
                   &spi_bioswe_ops);
    if (IS_ERR(spi_bioswe))
    {
        pr_err("Error creating sysfs file bioswe\n");
        goto out;
    }

    spi_ble =
        securityfs_create_file("ble",
                   0600, spi_dir, NULL,
                   &spi_ble_ops);
    if (IS_ERR(spi_ble))
    {
        pr_err("Error creating sysfs file ble\n");
        goto out;
    }

    spi_smm_bwp =
        securityfs_create_file("smm_bwp",
                   0600, spi_dir, NULL,
                   &spi_smm_bwp_ops);
    if (IS_ERR(spi_smm_bwp))
    {
        pr_err("Error creating sysfs file smm_bwp\n");
        goto out;
    }

    return 0;
out:
    securityfs_remove(spi_smm_bwp);
    securityfs_remove(spi_ble);
    securityfs_remove(spi_bioswe);
    securityfs_remove(spi_dir);
    return -1;
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
MODULE_LICENSE("GPL");
