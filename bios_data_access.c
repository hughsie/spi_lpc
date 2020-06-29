// SPDX-License-Identifier: GPL-2.0
/*
 * SPI LPC flash platform security driver
 *
 * Copyright 2020 (c) Daniel Gutson (daniel.gutson@eclypsium.com)
 *
 */
#include <linux/module.h>
#include "low_level_access.h"
#include "bios_data_access.h"

#define get_mask_from_bit_size(type, size)                                     \
	(((type) ~((type)0)) >> (sizeof(type) * 8 - size))

#define get_mask_from_bit_size_with_offset(type, size, offset)                 \
	(get_mask_from_bit_size(type, size) << (offset))

#define extract_bits(type, value, start, size)                                 \
	((value)&get_mask_from_bit_size_with_offset(type, size, start))

#define extract_bits_shifted(type, value, start, size)                         \
	(extract_bits(type, value, start, size) >> (start))

static int read_spibar(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch,
		       u64 *offset);

static int read_sbase_atom_avn_byt(struct sbase_atom_avn_byt *reg)
{
	u32 value;
	const int ret = pci_read_dword(&value, 0x0, 0x1f, 0x0, 0x54);

	if (ret != 0)
		return ret;

	reg->memi = extract_bits_shifted(u32, value, 0, 1);
	reg->enable = extract_bits_shifted(u32, value, 1, 1);
	reg->addrng = extract_bits_shifted(u32, value, 2, 1);
	reg->pref = extract_bits_shifted(u32, value, 3, 1);
	reg->base = extract_bits(u32, value, 9, 23);

	return 0;
}

int spi_read_sbase(enum PCH_Arch pch_arch __maybe_unused,
		   enum CPU_Arch cpu_arch, struct spi_sbase *reg)
{
	int ret = 0;

	reg->register_arch.source = RegSource_CPU;
	reg->register_arch.cpu_arch = cpu_arch;

	switch (cpu_arch) {
	case cpu_avn:
	case cpu_byt:
		ret = read_sbase_atom_avn_byt(&reg->cpu_byt);
		break;
	default:
		ret = -EIO;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(spi_read_sbase);

static int read_bc_pch_3xx_4xx_5xx(struct bc_pch_3xx_4xx_5xx *reg)
{
	u32 value;
	const int ret = pci_read_dword(&value, 0x0, 0x1f, 0x5, 0xdc);

	if (ret != 0)
		return ret;

	reg->bioswe = extract_bits_shifted(u32, value, 0, 1);
	reg->ble = extract_bits_shifted(u32, value, 1, 1);
	reg->src = extract_bits_shifted(u32, value, 2, 2);
	reg->tss = extract_bits_shifted(u32, value, 4, 1);
	reg->smm_bwp = extract_bits_shifted(u32, value, 5, 1);
	reg->bbs = extract_bits_shifted(u32, value, 6, 1);
	reg->bild = extract_bits_shifted(u32, value, 7, 1);
	reg->spi_sync_ss = extract_bits_shifted(u32, value, 8, 1);
	reg->spi_async_ss = extract_bits_shifted(u32, value, 10, 1);
	reg->ase_bwp = extract_bits_shifted(u32, value, 11, 1);

	return 0;
}

static int
read_bc_cpu_snb_jkt_ivb_ivt_bdx_hsx(struct bc_cpu_snb_jkt_ivb_ivt_bdx_hsx *reg)
{
	u32 value;
	const int ret = pci_read_dword(&value, 0x0, 0x1f, 0x5, 0xdc);

	if (ret != 0)
		return ret;

	reg->bioswe = extract_bits_shifted(u32, value, 0, 1);
	reg->ble = extract_bits_shifted(u32, value, 1, 1);
	reg->src = extract_bits_shifted(u32, value, 2, 2);
	reg->tss = extract_bits_shifted(u32, value, 4, 1);
	reg->smm_bwp = extract_bits_shifted(u32, value, 5, 1);

	return 0;
}

static int read_bc_cpu_skl_kbl_cfl(struct bc_cpu_skl_kbl_cfl *reg)
{
	u32 value;
	const int ret = pci_read_dword(&value, 0x0, 0x1f, 0x5, 0xdc);

	if (ret != 0)
		return ret;

	reg->bioswe = extract_bits_shifted(u32, value, 0, 1);
	reg->ble = extract_bits_shifted(u32, value, 1, 1);
	reg->src = extract_bits_shifted(u32, value, 2, 2);
	reg->tss = extract_bits_shifted(u32, value, 4, 1);
	reg->smm_bwp = extract_bits_shifted(u32, value, 5, 1);
	reg->bbs = extract_bits_shifted(u32, value, 6, 1);
	reg->bild = extract_bits_shifted(u32, value, 7, 1);

	return 0;
}

static int read_bc_cpu_apl_glk(struct bc_cpu_apl_glk *reg)
{
	u32 value;
	const int ret = pci_read_dword(&value, 0x0, 0xd, 0x2, 0xdc);

	if (ret != 0)
		return ret;

	reg->bioswe = extract_bits_shifted(u32, value, 0, 1);
	reg->ble = extract_bits_shifted(u32, value, 1, 1);
	reg->src = extract_bits_shifted(u32, value, 2, 2);
	reg->tss = extract_bits_shifted(u32, value, 4, 1);
	reg->smm_bwp = extract_bits_shifted(u32, value, 5, 1);
	reg->bbs = extract_bits_shifted(u32, value, 6, 1);
	reg->bild = extract_bits_shifted(u32, value, 7, 1);
	reg->spi_sync_ss = extract_bits_shifted(u32, value, 8, 1);
	reg->osfh = extract_bits_shifted(u32, value, 9, 1);
	reg->spi_async_ss = extract_bits_shifted(u32, value, 10, 1);
	reg->ase_bwp = extract_bits_shifted(u32, value, 11, 1);

	return 0;
}

static int read_bc_cpu_atom_avn(struct bc_cpu_atom_avn *reg,
				enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch)
{
	u8 value;
	int ret;
	u64 barOffset;

	ret = read_spibar(pch_arch, cpu_arch, &barOffset);
	if (ret != 0)
		return ret;

	ret = mmio_read_byte(barOffset + 0xfc, &value);
	if (ret != 0)
		return ret;

	reg->bioswe = extract_bits_shifted(u8, value, 0, 1);
	reg->ble = extract_bits_shifted(u8, value, 1, 1);
	reg->src = extract_bits_shifted(u8, value, 2, 2);
	reg->tss = extract_bits_shifted(u8, value, 4, 1);
	reg->smm_bwp = extract_bits_shifted(u8, value, 5, 1);

	return 0;
}

static int read_bc_cpu_atom_byt(struct bc_cpu_atom_byt *reg,
				enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch)
{
	u32 value;
	int ret;
	u64 barOffset;

	ret = read_spibar(pch_arch, cpu_arch, &barOffset);
	if (ret != 0)
		return ret;

	ret = mmio_read_dword(barOffset + 0xfc, &value);
	if (ret != 0)
		return ret;

	reg->bioswe = extract_bits_shifted(u32, value, 0, 1);
	reg->ble = extract_bits_shifted(u32, value, 1, 1);
	reg->src = extract_bits_shifted(u32, value, 2, 2);
	reg->smm_bwp = extract_bits_shifted(u32, value, 5, 1);

	return 0;
}

int spi_read_bc(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch,
		struct spi_bc *reg)
{
	int ret = 0;

	reg->register_arch.source = RegSource_PCH;
	reg->register_arch.pch_arch = pch_arch;

	switch (pch_arch) {
	case pch_3xx:
	case pch_4xx:
	case pch_495:
	case pch_5xx:
		ret = read_bc_pch_3xx_4xx_5xx(&reg->pch_5xx);
		break;
	default:
		reg->register_arch.source = RegSource_CPU;
		reg->register_arch.cpu_arch = cpu_arch;

		switch (cpu_arch) {
		case cpu_snb:
		case cpu_jkt:
		case cpu_ivb:
		case cpu_ivt:
		case cpu_bdw:
		case cpu_bdx:
		case cpu_hsx:
		case cpu_hsw:
			ret = read_bc_cpu_snb_jkt_ivb_ivt_bdx_hsx(
				&reg->cpu_hsw);
			break;
		case cpu_skl:
		case cpu_kbl:
		case cpu_cfl:
			ret = read_bc_cpu_skl_kbl_cfl(&reg->cpu_cfl);
			break;
		case cpu_apl:
		case cpu_glk:
			ret = read_bc_cpu_apl_glk(&reg->cpu_glk);
			break;
		case cpu_avn:
			ret = read_bc_cpu_atom_avn(&reg->cpu_avn, pch_arch,
						   cpu_arch);
			break;
		case cpu_byt:
			ret = read_bc_cpu_atom_byt(&reg->cpu_byt, pch_arch,
						   cpu_arch);
			break;
		default:
			ret = -EIO;
		}
	}
	return ret;
}
EXPORT_SYMBOL_GPL(spi_read_bc);

int read_spibar(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch, u64 *offset)
{
	int ret = 0;
	u64 field_offset;

	switch (cpu_arch) {
	case cpu_avn:
	case cpu_byt: {
		struct spi_sbase reg;

		ret = spi_read_sbase(pch_arch, cpu_arch, &reg);
		if (ret == 0) {
			ret = spi_read_sbase_base(&reg, &field_offset);
			*offset = field_offset + 0;
		}
	} break;
	default:
		ret = -EIO;
	}

	return ret;
}

int spi_read_bc_bioswe(const struct spi_bc *reg, u64 *value)
{
	int ret = 0;

	switch (reg->register_arch.source) {
	case RegSource_PCH:
		switch (reg->register_arch.pch_arch) {
		case pch_3xx:
			*value = reg->pch_3xx.bioswe;
			break;
		case pch_4xx:
			*value = reg->pch_4xx.bioswe;
			break;
		case pch_495:
			*value = reg->pch_495.bioswe;
			break;
		case pch_5xx:
			*value = reg->pch_5xx.bioswe;
			break;
		default:
			/* requested PCH arch hasn't field bioswe */
			ret = -EIO;
			*value = 0;
		}
		break;
	case RegSource_CPU:
		switch (reg->register_arch.cpu_arch) {
		case cpu_snb:
			*value = reg->cpu_snb.bioswe;
			break;
		case cpu_jkt:
			*value = reg->cpu_jkt.bioswe;
			break;
		case cpu_ivb:
			*value = reg->cpu_ivb.bioswe;
			break;
		case cpu_ivt:
			*value = reg->cpu_ivt.bioswe;
			break;
		case cpu_bdw:
			*value = reg->cpu_bdw.bioswe;
			break;
		case cpu_bdx:
			*value = reg->cpu_bdx.bioswe;
			break;
		case cpu_hsx:
			*value = reg->cpu_hsx.bioswe;
			break;
		case cpu_hsw:
			*value = reg->cpu_hsw.bioswe;
			break;
		case cpu_skl:
			*value = reg->cpu_skl.bioswe;
			break;
		case cpu_kbl:
			*value = reg->cpu_kbl.bioswe;
			break;
		case cpu_cfl:
			*value = reg->cpu_cfl.bioswe;
			break;
		case cpu_apl:
			*value = reg->cpu_apl.bioswe;
			break;
		case cpu_glk:
			*value = reg->cpu_glk.bioswe;
			break;
		case cpu_avn:
			*value = reg->cpu_avn.bioswe;
			break;
		case cpu_byt:
			*value = reg->cpu_byt.bioswe;
			break;
		default:
			/* requested CPU arch hasn't field bioswe */
			ret = -EIO;
			*value = 0;
		}
		break;
	default:
		ret = -EIO; /* should not reach here, it's a bug */
		*value = 0;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(spi_read_bc_bioswe);

int spi_read_bc_ble(const struct spi_bc *reg, u64 *value)
{
	int ret = 0;

	switch (reg->register_arch.source) {
	case RegSource_PCH:
		switch (reg->register_arch.pch_arch) {
		case pch_3xx:
			*value = reg->pch_3xx.ble;
			break;
		case pch_4xx:
			*value = reg->pch_4xx.ble;
			break;
		case pch_495:
			*value = reg->pch_495.ble;
			break;
		case pch_5xx:
			*value = reg->pch_5xx.ble;
			break;
		default:
			/* requested PCH arch hasn't field ble */
			ret = -EIO;
			*value = 0;
		}
		break;
	case RegSource_CPU:
		switch (reg->register_arch.cpu_arch) {
		case cpu_snb:
			*value = reg->cpu_snb.ble;
			break;
		case cpu_jkt:
			*value = reg->cpu_jkt.ble;
			break;
		case cpu_ivb:
			*value = reg->cpu_ivb.ble;
			break;
		case cpu_ivt:
			*value = reg->cpu_ivt.ble;
			break;
		case cpu_bdw:
			*value = reg->cpu_bdw.ble;
			break;
		case cpu_bdx:
			*value = reg->cpu_bdx.ble;
			break;
		case cpu_hsx:
			*value = reg->cpu_hsx.ble;
			break;
		case cpu_hsw:
			*value = reg->cpu_hsw.ble;
			break;
		case cpu_skl:
			*value = reg->cpu_skl.ble;
			break;
		case cpu_kbl:
			*value = reg->cpu_kbl.ble;
			break;
		case cpu_cfl:
			*value = reg->cpu_cfl.ble;
			break;
		case cpu_apl:
			*value = reg->cpu_apl.ble;
			break;
		case cpu_glk:
			*value = reg->cpu_glk.ble;
			break;
		case cpu_avn:
			*value = reg->cpu_avn.ble;
			break;
		case cpu_byt:
			*value = reg->cpu_byt.ble;
			break;
		default:
			/* requested CPU arch hasn't field ble */
			ret = -EIO;
			*value = 0;
		}
		break;
	default:
		ret = -EIO; /* should not reach here, it's a bug */
		*value = 0;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(spi_read_bc_ble);

int spi_read_bc_smm_bwp(const struct spi_bc *reg, u64 *value)
{
	int ret = 0;

	switch (reg->register_arch.source) {
	case RegSource_PCH:
		switch (reg->register_arch.pch_arch) {
		case pch_3xx:
			*value = reg->pch_3xx.smm_bwp;
			break;
		case pch_4xx:
			*value = reg->pch_4xx.smm_bwp;
			break;
		case pch_495:
			*value = reg->pch_495.smm_bwp;
			break;
		case pch_5xx:
			*value = reg->pch_5xx.smm_bwp;
			break;
		default:
			/* requested PCH arch hasn't field smm_bwp */
			ret = -EIO;
			*value = 0;
		}
		break;
	case RegSource_CPU:
		switch (reg->register_arch.cpu_arch) {
		case cpu_snb:
			*value = reg->cpu_snb.smm_bwp;
			break;
		case cpu_jkt:
			*value = reg->cpu_jkt.smm_bwp;
			break;
		case cpu_ivb:
			*value = reg->cpu_ivb.smm_bwp;
			break;
		case cpu_ivt:
			*value = reg->cpu_ivt.smm_bwp;
			break;
		case cpu_bdw:
			*value = reg->cpu_bdw.smm_bwp;
			break;
		case cpu_bdx:
			*value = reg->cpu_bdx.smm_bwp;
			break;
		case cpu_hsx:
			*value = reg->cpu_hsx.smm_bwp;
			break;
		case cpu_hsw:
			*value = reg->cpu_hsw.smm_bwp;
			break;
		case cpu_skl:
			*value = reg->cpu_skl.smm_bwp;
			break;
		case cpu_kbl:
			*value = reg->cpu_kbl.smm_bwp;
			break;
		case cpu_cfl:
			*value = reg->cpu_cfl.smm_bwp;
			break;
		case cpu_apl:
			*value = reg->cpu_apl.smm_bwp;
			break;
		case cpu_glk:
			*value = reg->cpu_glk.smm_bwp;
			break;
		case cpu_avn:
			*value = reg->cpu_avn.smm_bwp;
			break;
		case cpu_byt:
			*value = reg->cpu_byt.smm_bwp;
			break;
		default:
			/* requested CPU arch hasn't field smm_bwp */
			ret = -EIO;
			*value = 0;
		}
		break;
	default:
		ret = -EIO; /* should not reach here, it's a bug */
		*value = 0;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(spi_read_bc_smm_bwp);

int spi_read_sbase_base(const struct spi_sbase *reg, u64 *value)
{
	int ret = 0;

	switch (reg->register_arch.source) {
	case RegSource_PCH:
		ret = -EIO; /* no PCH archs have this field */
		*value = 0;
		break;
	case RegSource_CPU:
		switch (reg->register_arch.cpu_arch) {
		case cpu_avn:
			*value = reg->cpu_avn.base;
			break;
		case cpu_byt:
			*value = reg->cpu_byt.base;
			break;
		default:
			/* requested CPU arch hasn't field base */
			ret = -EIO;
			*value = 0;
		}
		break;
	default:
		ret = -EIO; /* should not reach here, it's a bug */
		*value = 0;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(spi_read_sbase_base);
