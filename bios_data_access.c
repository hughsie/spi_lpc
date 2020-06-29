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

static int read_SPIBAR(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch,
		       u64 *offset);

static int
read_sbase_register_atom_avn_byt(struct sbase_register_atom_avn_byt *reg)
{
	u32 value;
	const int ret = pci_read_dword(&value, 0x0, 0x1f, 0x0, 0x54);

	if (ret != 0)
		return ret;

	reg->MEMI = extract_bits_shifted(u32, value, 0, 1);
	reg->Enable = extract_bits_shifted(u32, value, 1, 1);
	reg->ADDRNG = extract_bits_shifted(u32, value, 2, 1);
	reg->PREF = extract_bits_shifted(u32, value, 3, 1);
	reg->Base = extract_bits(u32, value, 9, 23);

	return 0;
}

int read_sbase_register(enum PCH_Arch pch_arch __maybe_unused,
			enum CPU_Arch cpu_arch, struct sbase_register *reg)
{
	int ret = 0;

	reg->register_arch.source = RegSource_CPU;
	reg->register_arch.cpu_arch = cpu_arch;

	switch (cpu_arch) {
	case cpu_avn:
	case cpu_byt:
		ret = read_sbase_register_atom_avn_byt(&reg->cpu_byt);
		break;
	default:
		ret = -EIO;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(read_sbase_register);

static int read_bios_control_register_pch_3xx_4xx_5xx(
	struct bios_control_register_pch_3xx_4xx_5xx *reg)
{
	u32 value;
	const int ret = pci_read_dword(&value, 0x0, 0x1f, 0x5, 0xdc);

	if (ret != 0)
		return ret;

	reg->BIOSWE = extract_bits_shifted(u32, value, 0, 1);
	reg->BLE = extract_bits_shifted(u32, value, 1, 1);
	reg->SRC = extract_bits_shifted(u32, value, 2, 2);
	reg->TSS = extract_bits_shifted(u32, value, 4, 1);
	reg->SMM_BWP = extract_bits_shifted(u32, value, 5, 1);
	reg->BBS = extract_bits_shifted(u32, value, 6, 1);
	reg->BILD = extract_bits_shifted(u32, value, 7, 1);
	reg->SPI_SYNC_SS = extract_bits_shifted(u32, value, 8, 1);
	reg->SPI_ASYNC_SS = extract_bits_shifted(u32, value, 10, 1);
	reg->ASE_BWP = extract_bits_shifted(u32, value, 11, 1);

	return 0;
}

static int read_bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx(
	struct bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx *reg)
{
	u32 value;
	const int ret = pci_read_dword(&value, 0x0, 0x1f, 0x5, 0xdc);

	if (ret != 0)
		return ret;

	reg->BIOSWE = extract_bits_shifted(u32, value, 0, 1);
	reg->BLE = extract_bits_shifted(u32, value, 1, 1);
	reg->SRC = extract_bits_shifted(u32, value, 2, 2);
	reg->TSS = extract_bits_shifted(u32, value, 4, 1);
	reg->SMM_BWP = extract_bits_shifted(u32, value, 5, 1);

	return 0;
}

static int read_bios_control_register_cpu_skl_kbl_cfl(
	struct bios_control_register_cpu_skl_kbl_cfl *reg)
{
	u32 value;
	const int ret = pci_read_dword(&value, 0x0, 0x1f, 0x5, 0xdc);

	if (ret != 0)
		return ret;

	reg->BIOSWE = extract_bits_shifted(u32, value, 0, 1);
	reg->BLE = extract_bits_shifted(u32, value, 1, 1);
	reg->SRC = extract_bits_shifted(u32, value, 2, 2);
	reg->TSS = extract_bits_shifted(u32, value, 4, 1);
	reg->SMM_BWP = extract_bits_shifted(u32, value, 5, 1);
	reg->BBS = extract_bits_shifted(u32, value, 6, 1);
	reg->BILD = extract_bits_shifted(u32, value, 7, 1);

	return 0;
}

static int read_bios_control_register_cpu_apl_glk(
	struct bios_control_register_cpu_apl_glk *reg)
{
	u32 value;
	const int ret = pci_read_dword(&value, 0x0, 0xd, 0x2, 0xdc);

	if (ret != 0)
		return ret;

	reg->BIOSWE = extract_bits_shifted(u32, value, 0, 1);
	reg->BLE = extract_bits_shifted(u32, value, 1, 1);
	reg->SRC = extract_bits_shifted(u32, value, 2, 2);
	reg->TSS = extract_bits_shifted(u32, value, 4, 1);
	reg->SMM_BWP = extract_bits_shifted(u32, value, 5, 1);
	reg->BBS = extract_bits_shifted(u32, value, 6, 1);
	reg->BILD = extract_bits_shifted(u32, value, 7, 1);
	reg->SPI_SYNC_SS = extract_bits_shifted(u32, value, 8, 1);
	reg->OSFH = extract_bits_shifted(u32, value, 9, 1);
	reg->SPI_ASYNC_SS = extract_bits_shifted(u32, value, 10, 1);
	reg->ASE_BWP = extract_bits_shifted(u32, value, 11, 1);

	return 0;
}

static int read_bios_control_register_cpu_atom_avn(
	struct bios_control_register_cpu_atom_avn *reg, enum PCH_Arch pch_arch,
	enum CPU_Arch cpu_arch)
{
	u8 value;
	int ret;
	u64 barOffset;

	ret = read_SPIBAR(pch_arch, cpu_arch, &barOffset);
	if (ret != 0)
		return ret;

	ret = mmio_read_byte(barOffset + 0xfc, &value);
	if (ret != 0)
		return ret;

	reg->BIOSWE = extract_bits_shifted(u8, value, 0, 1);
	reg->BLE = extract_bits_shifted(u8, value, 1, 1);
	reg->SRC = extract_bits_shifted(u8, value, 2, 2);
	reg->TSS = extract_bits_shifted(u8, value, 4, 1);
	reg->SMM_BWP = extract_bits_shifted(u8, value, 5, 1);

	return 0;
}

static int read_bios_control_register_cpu_atom_byt(
	struct bios_control_register_cpu_atom_byt *reg, enum PCH_Arch pch_arch,
	enum CPU_Arch cpu_arch)
{
	u32 value;
	int ret;
	u64 barOffset;

	ret = read_SPIBAR(pch_arch, cpu_arch, &barOffset);
	if (ret != 0)
		return ret;

	ret = mmio_read_dword(barOffset + 0xfc, &value);
	if (ret != 0)
		return ret;

	reg->BIOSWE = extract_bits_shifted(u32, value, 0, 1);
	reg->BLE = extract_bits_shifted(u32, value, 1, 1);
	reg->SRC = extract_bits_shifted(u32, value, 2, 2);
	reg->SMM_BWP = extract_bits_shifted(u32, value, 5, 1);

	return 0;
}

int read_bios_control_register(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch,
			       struct bios_control_register *reg)
{
	int ret = 0;

	reg->register_arch.source = RegSource_PCH;
	reg->register_arch.pch_arch = pch_arch;

	switch (pch_arch) {
	case pch_3xx:
	case pch_4xx:
	case pch_495:
	case pch_5xx:
		ret = read_bios_control_register_pch_3xx_4xx_5xx(&reg->pch_5xx);
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
			ret = read_bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx(
				&reg->cpu_hsw);
			break;
		case cpu_skl:
		case cpu_kbl:
		case cpu_cfl:
			ret = read_bios_control_register_cpu_skl_kbl_cfl(
				&reg->cpu_cfl);
			break;
		case cpu_apl:
		case cpu_glk:
			ret = read_bios_control_register_cpu_apl_glk(
				&reg->cpu_glk);
			break;
		case cpu_avn:
			ret = read_bios_control_register_cpu_atom_avn(
				&reg->cpu_avn, pch_arch, cpu_arch);
			break;
		case cpu_byt:
			ret = read_bios_control_register_cpu_atom_byt(
				&reg->cpu_byt, pch_arch, cpu_arch);
			break;
		default:
			ret = -EIO;
		}
	}
	return ret;
}
EXPORT_SYMBOL_GPL(read_bios_control_register);

int read_SPIBAR(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch, u64 *offset)
{
	int ret = 0;
	u64 field_offset;

	switch (cpu_arch) {
	case cpu_avn:
	case cpu_byt: {
		struct sbase_register reg;

		ret = read_sbase_register(pch_arch, cpu_arch, &reg);
		if (ret == 0) {
			ret = read_sbase_register_Base(&reg, &field_offset);
			*offset = field_offset + 0;
		}
	} break;
	default:
		ret = -EIO;
	}

	return ret;
}

int read_bios_control_register_BIOSWE(const struct bios_control_register *reg,
				      u64 *value)
{
	int ret = 0;

	switch (reg->register_arch.source) {
	case RegSource_PCH:
		switch (reg->register_arch.pch_arch) {
		case pch_3xx:
			*value = reg->pch_3xx.BIOSWE;
			break;
		case pch_4xx:
			*value = reg->pch_4xx.BIOSWE;
			break;
		case pch_495:
			*value = reg->pch_495.BIOSWE;
			break;
		case pch_5xx:
			*value = reg->pch_5xx.BIOSWE;
			break;
		default:
			/* requested PCH arch hasn't field BIOSWE */
			ret = -EIO;
			*value = 0;
		}
		break;
	case RegSource_CPU:
		switch (reg->register_arch.cpu_arch) {
		case cpu_snb:
			*value = reg->cpu_snb.BIOSWE;
			break;
		case cpu_jkt:
			*value = reg->cpu_jkt.BIOSWE;
			break;
		case cpu_ivb:
			*value = reg->cpu_ivb.BIOSWE;
			break;
		case cpu_ivt:
			*value = reg->cpu_ivt.BIOSWE;
			break;
		case cpu_bdw:
			*value = reg->cpu_bdw.BIOSWE;
			break;
		case cpu_bdx:
			*value = reg->cpu_bdx.BIOSWE;
			break;
		case cpu_hsx:
			*value = reg->cpu_hsx.BIOSWE;
			break;
		case cpu_hsw:
			*value = reg->cpu_hsw.BIOSWE;
			break;
		case cpu_skl:
			*value = reg->cpu_skl.BIOSWE;
			break;
		case cpu_kbl:
			*value = reg->cpu_kbl.BIOSWE;
			break;
		case cpu_cfl:
			*value = reg->cpu_cfl.BIOSWE;
			break;
		case cpu_apl:
			*value = reg->cpu_apl.BIOSWE;
			break;
		case cpu_glk:
			*value = reg->cpu_glk.BIOSWE;
			break;
		case cpu_avn:
			*value = reg->cpu_avn.BIOSWE;
			break;
		case cpu_byt:
			*value = reg->cpu_byt.BIOSWE;
			break;
		default:
			/* requested CPU arch hasn't field BIOSWE */
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
EXPORT_SYMBOL_GPL(read_bios_control_register_BIOSWE);

int read_bios_control_register_BLE(const struct bios_control_register *reg,
				   u64 *value)
{
	int ret = 0;

	switch (reg->register_arch.source) {
	case RegSource_PCH:
		switch (reg->register_arch.pch_arch) {
		case pch_3xx:
			*value = reg->pch_3xx.BLE;
			break;
		case pch_4xx:
			*value = reg->pch_4xx.BLE;
			break;
		case pch_495:
			*value = reg->pch_495.BLE;
			break;
		case pch_5xx:
			*value = reg->pch_5xx.BLE;
			break;
		default:
			/* requested PCH arch hasn't field BLE */
			ret = -EIO;
			*value = 0;
		}
		break;
	case RegSource_CPU:
		switch (reg->register_arch.cpu_arch) {
		case cpu_snb:
			*value = reg->cpu_snb.BLE;
			break;
		case cpu_jkt:
			*value = reg->cpu_jkt.BLE;
			break;
		case cpu_ivb:
			*value = reg->cpu_ivb.BLE;
			break;
		case cpu_ivt:
			*value = reg->cpu_ivt.BLE;
			break;
		case cpu_bdw:
			*value = reg->cpu_bdw.BLE;
			break;
		case cpu_bdx:
			*value = reg->cpu_bdx.BLE;
			break;
		case cpu_hsx:
			*value = reg->cpu_hsx.BLE;
			break;
		case cpu_hsw:
			*value = reg->cpu_hsw.BLE;
			break;
		case cpu_skl:
			*value = reg->cpu_skl.BLE;
			break;
		case cpu_kbl:
			*value = reg->cpu_kbl.BLE;
			break;
		case cpu_cfl:
			*value = reg->cpu_cfl.BLE;
			break;
		case cpu_apl:
			*value = reg->cpu_apl.BLE;
			break;
		case cpu_glk:
			*value = reg->cpu_glk.BLE;
			break;
		case cpu_avn:
			*value = reg->cpu_avn.BLE;
			break;
		case cpu_byt:
			*value = reg->cpu_byt.BLE;
			break;
		default:
			/* requested CPU arch hasn't field BLE */
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
EXPORT_SYMBOL_GPL(read_bios_control_register_BLE);

int read_bios_control_register_SMM_BWP(const struct bios_control_register *reg,
				       u64 *value)
{
	int ret = 0;

	switch (reg->register_arch.source) {
	case RegSource_PCH:
		switch (reg->register_arch.pch_arch) {
		case pch_3xx:
			*value = reg->pch_3xx.SMM_BWP;
			break;
		case pch_4xx:
			*value = reg->pch_4xx.SMM_BWP;
			break;
		case pch_495:
			*value = reg->pch_495.SMM_BWP;
			break;
		case pch_5xx:
			*value = reg->pch_5xx.SMM_BWP;
			break;
		default:
			/* requested PCH arch hasn't field SMM_BWP */
			ret = -EIO;
			*value = 0;
		}
		break;
	case RegSource_CPU:
		switch (reg->register_arch.cpu_arch) {
		case cpu_snb:
			*value = reg->cpu_snb.SMM_BWP;
			break;
		case cpu_jkt:
			*value = reg->cpu_jkt.SMM_BWP;
			break;
		case cpu_ivb:
			*value = reg->cpu_ivb.SMM_BWP;
			break;
		case cpu_ivt:
			*value = reg->cpu_ivt.SMM_BWP;
			break;
		case cpu_bdw:
			*value = reg->cpu_bdw.SMM_BWP;
			break;
		case cpu_bdx:
			*value = reg->cpu_bdx.SMM_BWP;
			break;
		case cpu_hsx:
			*value = reg->cpu_hsx.SMM_BWP;
			break;
		case cpu_hsw:
			*value = reg->cpu_hsw.SMM_BWP;
			break;
		case cpu_skl:
			*value = reg->cpu_skl.SMM_BWP;
			break;
		case cpu_kbl:
			*value = reg->cpu_kbl.SMM_BWP;
			break;
		case cpu_cfl:
			*value = reg->cpu_cfl.SMM_BWP;
			break;
		case cpu_apl:
			*value = reg->cpu_apl.SMM_BWP;
			break;
		case cpu_glk:
			*value = reg->cpu_glk.SMM_BWP;
			break;
		case cpu_avn:
			*value = reg->cpu_avn.SMM_BWP;
			break;
		case cpu_byt:
			*value = reg->cpu_byt.SMM_BWP;
			break;
		default:
			/* requested CPU arch hasn't field SMM_BWP */
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
EXPORT_SYMBOL_GPL(read_bios_control_register_SMM_BWP);

int read_sbase_register_Base(const struct sbase_register *reg, u64 *value)
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
			*value = reg->cpu_avn.Base;
			break;
		case cpu_byt:
			*value = reg->cpu_byt.Base;
			break;
		default:
			/* requested CPU arch hasn't field Base */
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
EXPORT_SYMBOL_GPL(read_sbase_register_Base);
