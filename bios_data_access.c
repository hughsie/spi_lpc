// SPDX-License-Identifier: GPL-2.0
/*
 * SPI LPC flash platform security driver
 *
 * Copyright 2020 (c) Daniel Gutson (daniel.gutson@eclypsium.com)
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
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

static int read_SBASE_atom_avn_byt(struct SBASE_atom_avn_byt *reg)
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

int read_SBASE(enum PCH_Arch pch_arch __maybe_unused, enum CPU_Arch cpu_arch,
	       struct SBASE *reg)
{
	int ret = 0;
	reg->register_arch.source = RegSource_CPU;
	reg->register_arch.cpu_arch = cpu_arch;
	switch (cpu_arch) {
	case cpu_avn:
	case cpu_byt:
		ret = read_SBASE_atom_avn_byt(&reg->cpu_byt);
		break;
	default:
		ret = -EIO;
	}
	return ret;
}

static int read_BC_pch_3xx_4xx_5xx(struct BC_pch_3xx_4xx_5xx *reg)
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

static int
read_BC_cpu_snb_jkt_ivb_ivt_bdx_hsx(struct BC_cpu_snb_jkt_ivb_ivt_bdx_hsx *reg)
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

static int read_BC_cpu_skl_kbl_cfl(struct BC_cpu_skl_kbl_cfl *reg)
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

static int read_BC_cpu_apl_glk(struct BC_cpu_apl_glk *reg)
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

static int read_BC_cpu_atom_avn(struct BC_cpu_atom_avn *reg,
				enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch)
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

static int read_BC_cpu_atom_byt(struct BC_cpu_atom_byt *reg,
				enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch)
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

int read_BC(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch, struct BC *reg)
{
	int ret = 0;
	reg->register_arch.source = RegSource_PCH;
	reg->register_arch.pch_arch = pch_arch;
	switch (pch_arch) {
	case pch_3xx:
	case pch_4xx:
	case pch_495:
	case pch_5xx:
		ret = read_BC_pch_3xx_4xx_5xx(&reg->pch_5xx);
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
			ret = read_BC_cpu_snb_jkt_ivb_ivt_bdx_hsx(
				&reg->cpu_hsw);
			break;
		case cpu_skl:
		case cpu_kbl:
		case cpu_cfl:
			ret = read_BC_cpu_skl_kbl_cfl(&reg->cpu_cfl);
			break;
		case cpu_apl:
		case cpu_glk:
			ret = read_BC_cpu_apl_glk(&reg->cpu_glk);
			break;
		case cpu_avn:
			ret = read_BC_cpu_atom_avn(&reg->cpu_avn, pch_arch,
						   cpu_arch);
			break;
		case cpu_byt:
			ret = read_BC_cpu_atom_byt(&reg->cpu_byt, pch_arch,
						   cpu_arch);
			break;
		default:
			ret = -EIO;
		}
	}
	return ret;
}

int read_SPIBAR(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch, u64 *offset)
{
	int ret = 0;
	u64 field_offset;

	switch (cpu_arch) {
	case cpu_avn:
	case cpu_byt: {
		struct SBASE reg;
		ret = read_SBASE(pch_arch, cpu_arch, &reg);
		if (ret == 0) {
			ret = read_SBASE_Base(&reg, &field_offset);
			*offset = field_offset + 0;
		}
	} break;
	default:
		ret = -EIO;
	}

	return ret;
}

int read_BC_BIOSWE(const struct BC *reg, u64 *value)
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

int read_BC_BLE(const struct BC *reg, u64 *value)
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

int read_BC_SMM_BWP(const struct BC *reg, u64 *value)
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

int read_SBASE_Base(const struct SBASE *reg, u64 *value)
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

int viddid2pch_arch(u64 vid, u64 did, enum PCH_Arch *arch)
{
	switch (vid) {
	case 0x8086: /* INTEL */
		switch (did) {
		case 0x1c44:
		case 0x1c46:
		case 0x1c47:
		case 0x1c49:
		case 0x1c4a:
		case 0x1c4b:
		case 0x1c4c:
		case 0x1c4d:
		case 0x1c4e:
		case 0x1c4f:
		case 0x1c50:
		case 0x1c52:
		case 0x1c54:
		case 0x1c56:
		case 0x1c5c:
			*arch = pch_6_c200;
			return 0;
		case 0x1e47:
		case 0x1e48:
		case 0x1e49:
		case 0x1e44:
		case 0x1e46:
		case 0x1e4a:
		case 0x1e53:
		case 0x1e55:
		case 0x1e58:
		case 0x1e57:
		case 0x1e59:
		case 0x1e5d:
		case 0x1e5e:
		case 0x1e56:
			*arch = pch_7_c210;
			return 0;
		case 0x1d41:
			*arch = pch_c60x_x79;
			return 0;
		case 0x2390:
		case 0x2310:
			*arch = pch_communications_89xx;
			return 0;
		case 0x8c41:
		case 0x8c42:
		case 0x8c44:
		case 0x8c46:
		case 0x8c49:
		case 0x8c4a:
		case 0x8c4b:
		case 0x8c4c:
		case 0x8c4e:
		case 0x8c4f:
		case 0x8c50:
		case 0x8c52:
		case 0x8c54:
		case 0x8c56:
		case 0x8c5c:
		case 0x8cc1:
		case 0x8cc2:
		case 0x8cc3:
		case 0x8cc4:
		case 0x8cc6:
			*arch = pch_8_c220;
			return 0;
		case 0x8d40:
		case 0x8d44:
		case 0x8d47:
			*arch = pch_c61x_x99;
			return 0;
		case 0x9cc3:
		case 0x9cc5:
		case 0x9cc7:
		case 0x9cc9:
		case 0x9cc1:
		case 0x9cc2:
		case 0x9cc6:
			*arch = pch_5_mobile;
			return 0;
		case 0x9d41:
		case 0x9d43:
		case 0x9d46:
		case 0x9d48:
			*arch = pch_6_mobile;
			return 0;
		case 0x9d4b:
		case 0x9d4e:
		case 0x9d50:
		case 0x9d53:
		case 0x9d56:
		case 0x9d58:
			*arch = pch_7_8_mobile;
			return 0;
		case 0xa141:
		case 0xa142:
		case 0xa143:
		case 0xa144:
		case 0xa145:
		case 0xa146:
		case 0xa147:
		case 0xa148:
		case 0xa149:
		case 0xa14a:
		case 0xa14b:
		case 0xa14c:
		case 0xa14d:
		case 0xa14e:
		case 0xa14f:
		case 0xa150:
		case 0xa151:
		case 0xa152:
		case 0xa153:
		case 0xa154:
		case 0xa155:
		case 0xa156:
		case 0xa157:
		case 0xa158:
		case 0xa159:
		case 0xa15a:
		case 0xa15b:
		case 0xa15c:
		case 0xa15d:
		case 0xa15e:
		case 0xa15f:
			*arch = pch_1xx;
			return 0;
		case 0xa1c1:
		case 0xa1c2:
		case 0xa1c3:
		case 0xa1c4:
		case 0xa1c5:
		case 0xa1c6:
		case 0xa1c7:
		case 0xa242:
		case 0xa243:
		case 0xa244:
		case 0xa245:
		case 0xa246:
			*arch = pch_c620;
			return 0;
		case 0xa2c0:
		case 0xa2c1:
		case 0xa2c2:
		case 0xa2c3:
		case 0xa2c4:
		case 0xa2c5:
		case 0xa2c6:
		case 0xa2c7:
		case 0xa2c8:
		case 0xa2c9:
		case 0xa2ca:
		case 0xa2cb:
		case 0xa2cc:
		case 0xa2cd:
		case 0xa2ce:
		case 0xa2cf:
		case 0xa2d2:
		case 0xa2d3:
			*arch = pch_2xx;
			return 0;
		case 0xa300:
		case 0xa301:
		case 0xa302:
		case 0xa303:
		case 0xa304:
		case 0xa305:
		case 0xa306:
		case 0xa307:
		case 0xa308:
		case 0xa309:
		case 0xa30a:
		case 0xa30b:
		case 0xa30c:
		case 0xa30d:
		case 0xa30e:
		case 0xa30f:
		case 0xa310:
		case 0xa311:
		case 0xa312:
		case 0xa313:
		case 0xa314:
		case 0xa315:
		case 0xa316:
		case 0xa317:
		case 0xa318:
		case 0xa319:
		case 0xa31a:
		case 0xa31b:
		case 0xa31c:
		case 0xa31d:
		case 0xa31e:
		case 0xa31f:
		case 0x9d81:
		case 0x9d83:
		case 0x9d84:
		case 0x9d85:
		case 0x9d86:
			*arch = pch_3xx;
			return 0;
		case 0x280:
		case 0x281:
		case 0x282:
		case 0x283:
		case 0x284:
		case 0x285:
		case 0x286:
		case 0x287:
		case 0x288:
		case 0x289:
		case 0x28a:
		case 0x28b:
		case 0x28c:
		case 0x28d:
		case 0x28e:
		case 0x28f:
		case 0x290:
		case 0x291:
		case 0x292:
		case 0x293:
		case 0x294:
		case 0x295:
		case 0x296:
		case 0x297:
		case 0x298:
		case 0x299:
		case 0x29a:
		case 0x29b:
		case 0x29c:
		case 0x29d:
		case 0x29e:
		case 0x29f:
		case 0x680:
		case 0x681:
		case 0x682:
		case 0x683:
		case 0x684:
		case 0x685:
		case 0x686:
		case 0x687:
		case 0x688:
		case 0x689:
		case 0x68a:
		case 0x68b:
		case 0x68c:
		case 0x68d:
		case 0x68e:
		case 0x68f:
		case 0x690:
		case 0x691:
		case 0x692:
		case 0x693:
		case 0x694:
		case 0x695:
		case 0x696:
		case 0x697:
		case 0x698:
		case 0x699:
		case 0x69a:
		case 0x69b:
		case 0x69c:
		case 0x69d:
		case 0x69e:
		case 0x69f:
		case 0xa3c1:
		case 0xa3c8:
		case 0xa3da:
			*arch = pch_4xx;
			return 0;
		case 0x3480:
		case 0x3481:
		case 0x3482:
		case 0x3483:
		case 0x3484:
		case 0x3485:
		case 0x3486:
		case 0x3487:
		case 0x3488:
		case 0x3489:
		case 0x348a:
		case 0x348b:
		case 0x348c:
		case 0x348d:
		case 0x348e:
		case 0x348f:
		case 0x3490:
		case 0x3491:
		case 0x3492:
		case 0x3493:
		case 0x3494:
		case 0x3495:
		case 0x3496:
		case 0x3497:
		case 0x3498:
		case 0x3499:
		case 0x349a:
		case 0x349b:
		case 0x349c:
		case 0x349d:
		case 0x349e:
		case 0x349f:
		case 0x3887:
			*arch = pch_495;
			return 0;
		case 0x4380:
		case 0x4381:
		case 0x4382:
		case 0x4383:
		case 0x4384:
		case 0x4385:
		case 0x4386:
		case 0x4387:
		case 0x4388:
		case 0x4389:
		case 0x438a:
		case 0x438b:
		case 0x438c:
		case 0x438d:
		case 0x438e:
		case 0x438f:
		case 0x4390:
		case 0x4391:
		case 0x4392:
		case 0x4393:
		case 0x4394:
		case 0x4395:
		case 0x4396:
		case 0x4397:
		case 0x4398:
		case 0x4399:
		case 0x439a:
		case 0x439b:
		case 0x439c:
		case 0x439d:
		case 0x439e:
		case 0x439f:
		case 0xa080:
		case 0xa081:
		case 0xa082:
		case 0xa083:
		case 0xa084:
		case 0xa085:
		case 0xa086:
		case 0xa087:
		case 0xa088:
		case 0xa089:
		case 0xa08a:
		case 0xa08b:
		case 0xa08c:
		case 0xa08d:
		case 0xa08e:
		case 0xa08f:
		case 0xa090:
		case 0xa091:
		case 0xa092:
		case 0xa093:
		case 0xa094:
		case 0xa095:
		case 0xa096:
		case 0xa097:
		case 0xa098:
		case 0xa099:
		case 0xa09a:
		case 0xa09b:
		case 0xa09c:
		case 0xa09d:
		case 0xa09e:
		case 0xa09f:
			*arch = pch_5xx;
			return 0;
		default:
			*arch = pch_none;
			return -EIO; /* DID not found */
		}
	case 0x1022: /* AMD */
		switch (did) {
		default:
			*arch = pch_none;
			return -EIO; /* DID not found */
		}
	default:
		return -EIO; /* VID not supported */
	}
}

int viddid2cpu_arch(u64 vid, u64 did, enum CPU_Arch *arch)
{
	switch (vid) {
	case 0x8086: /* INTEL */
		switch (did) {
		case 0x1600:
		case 0x1604:
		case 0x1610:
		case 0x1614:
		case 0x1618:
			*arch = cpu_bdw;
			return 0;
		case 0x6f00:
			*arch = cpu_bdx;
			return 0;
		case 0xa00:
		case 0xa04:
		case 0xa08:
		case 0xc00:
		case 0xc04:
		case 0xc08:
		case 0xd00:
		case 0xd04:
		case 0xd08:
			*arch = cpu_hsw;
			return 0;
		case 0x2f00:
			*arch = cpu_hsx;
			return 0;
		case 0xe00:
			*arch = cpu_ivt;
			return 0;
		case 0x3c00:
			*arch = cpu_jkt;
			return 0;
		case 0x5900:
		case 0x5904:
		case 0x590c:
		case 0x590f:
		case 0x5910:
		case 0x5914:
		case 0x5918:
		case 0x591f:
			*arch = cpu_kbl;
			return 0;
		case 0x1900:
		case 0x1904:
		case 0x190c:
		case 0x190f:
		case 0x1910:
		case 0x191f:
		case 0x1918:
		case 0x2020:
			*arch = cpu_skl;
			return 0;
		case 0x150:
		case 0x154:
		case 0x158:
			*arch = cpu_ivb;
			return 0;
		case 0x100:
		case 0x104:
		case 0x108:
			*arch = cpu_snb;
			return 0;
		case 0x1f00:
		case 0x1f01:
		case 0x1f02:
		case 0x1f03:
		case 0x1f04:
		case 0x1f05:
		case 0x1f06:
		case 0x1f07:
		case 0x1f08:
		case 0x1f09:
		case 0x1f0a:
		case 0x1f0b:
		case 0x1f0c:
		case 0x1f0d:
		case 0x1f0e:
		case 0x1f0f:
			*arch = cpu_avn;
			return 0;
		case 0x3e0f:
		case 0x3e10:
		case 0x3e18:
		case 0x3e1f:
		case 0x3e30:
		case 0x3e31:
		case 0x3e32:
		case 0x3e33:
		case 0x3ec2:
		case 0x3ec4:
		case 0x3ec6:
		case 0x3eca:
		case 0x3ecc:
		case 0x3ed0:
			*arch = cpu_cfl;
			return 0;
		case 0xf00:
			*arch = cpu_byt;
			return 0;
		case 0x3e34:
		case 0x3e35:
			*arch = cpu_whl;
			return 0;
		case 0x3e20:
		case 0x9b33:
		case 0x9b43:
		case 0x9b44:
		case 0x9b51:
		case 0x9b53:
		case 0x9b54:
		case 0x9b61:
		case 0x9b63:
		case 0x9b64:
		case 0x9b71:
		case 0x9b73:
			*arch = cpu_cml;
			return 0;
		case 0x8a00:
		case 0x8a02:
		case 0x8a10:
		case 0x8a12:
		case 0x8a16:
			*arch = cpu_icl;
			return 0;
		case 0x5af0:
			*arch = cpu_apl;
			return 0;
		case 0x3180:
		case 0x31f0:
			*arch = cpu_glk;
			return 0;
		case 0x9a02:
		case 0x9a04:
		case 0x9a12:
		case 0x9a14:
		case 0x9a26:
		case 0x9a36:
			*arch = cpu_tgl;
			return 0;
		default:
			*arch = cpu_none;
			return -EIO; /* DID not found */
		}
	case 0x1022: /* AMD */
		switch (did) {
		case 0x1410:
		case 0x1422:
		case 0x1576:
		case 0x1510:
		case 0x1514:
		case 0x1536:
		case 0x1566:
		case 0x1450:
		case 0x1480:
		case 0x15d0:
			*arch = cpu_amd;
			return 0;
		default:
			*arch = cpu_none;
			return -EIO; /* DID not found */
		}
	default:
		return -EIO; /* VID not supported */
	}
}
