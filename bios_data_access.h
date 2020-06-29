/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SPI LPC flash platform security driver
 *
 * Copyright 2020 (c) Daniel Gutson (daniel.gutson@eclypsium.com)
 *
 */
#ifndef BIOS_DATA_ACCESS_H
#define BIOS_DATA_ACCESS_H

#include <linux/types.h>

enum PCH_Arch {
	pch_none,
	pch_6_c200,
	pch_7_c210,
	pch_c60x_x79,
	pch_communications_89xx,
	pch_8_c220,
	pch_c61x_x99,
	pch_5_mobile,
	pch_6_mobile,
	pch_7_8_mobile,
	pch_1xx,
	pch_c620,
	pch_2xx,
	pch_3xx,
	pch_4xx,
	pch_495,
	pch_5xx,
	PCH_Archs_count
};

enum CPU_Arch {
	cpu_none,
	cpu_bdw,
	cpu_bdx,
	cpu_hsw,
	cpu_hsx,
	cpu_ivt,
	cpu_jkt,
	cpu_kbl,
	cpu_skl,
	cpu_ivb,
	cpu_snb,
	cpu_avn,
	cpu_cfl,
	cpu_byt,
	cpu_whl,
	cpu_cml,
	cpu_icl,
	cpu_apl,
	cpu_glk,
	cpu_tgl,
	cpu_amd,
	CPU_Archs_count
};

enum RegisterSource { RegSource_PCH, RegSource_CPU };

struct RegisterArch {
	enum RegisterSource source;

	union {
		enum PCH_Arch pch_arch;
		enum CPU_Arch cpu_arch;
	};
};

struct sbase_atom_avn_byt {
	u64 memi;
	u64 enable;
	u64 addrng;
	u64 pref;
	u64 base;
};

struct spi_sbase {
	struct RegisterArch register_arch;

	union {
		struct sbase_atom_avn_byt cpu_avn;
		struct sbase_atom_avn_byt cpu_byt;
	};
};

struct bc_pch_3xx_4xx_5xx {
	u64 bioswe;
	u64 ble;
	u64 src;
	u64 tss;
	u64 smm_bwp;
	u64 bbs;
	u64 bild;
	u64 spi_sync_ss;
	u64 spi_async_ss;
	u64 ase_bwp;
};

struct bc_cpu_snb_jkt_ivb_ivt_bdx_hsx {
	u64 bioswe;
	u64 ble;
	u64 src;
	u64 tss;
	u64 smm_bwp;
};

struct bc_cpu_skl_kbl_cfl {
	u64 bioswe;
	u64 ble;
	u64 src;
	u64 tss;
	u64 smm_bwp;
	u64 bbs;
	u64 bild;
};

struct bc_cpu_apl_glk {
	u64 bioswe;
	u64 ble;
	u64 src;
	u64 tss;
	u64 smm_bwp;
	u64 bbs;
	u64 bild;
	u64 spi_sync_ss;
	u64 osfh;
	u64 spi_async_ss;
	u64 ase_bwp;
};

struct bc_cpu_atom_avn {
	u64 bioswe;
	u64 ble;
	u64 src;
	u64 tss;
	u64 smm_bwp;
};

struct bc_cpu_atom_byt {
	u64 bioswe;
	u64 ble;
	u64 src;
	u64 smm_bwp;
};

struct spi_bc {
	struct RegisterArch register_arch;

	union {
		struct bc_pch_3xx_4xx_5xx pch_3xx;
		struct bc_pch_3xx_4xx_5xx pch_4xx;
		struct bc_pch_3xx_4xx_5xx pch_495;
		struct bc_pch_3xx_4xx_5xx pch_5xx;
		struct bc_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_snb;
		struct bc_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_jkt;
		struct bc_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_ivb;
		struct bc_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_ivt;
		struct bc_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_bdw;
		struct bc_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_bdx;
		struct bc_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_hsx;
		struct bc_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_hsw;
		struct bc_cpu_skl_kbl_cfl cpu_skl;
		struct bc_cpu_skl_kbl_cfl cpu_kbl;
		struct bc_cpu_skl_kbl_cfl cpu_cfl;
		struct bc_cpu_apl_glk cpu_apl;
		struct bc_cpu_apl_glk cpu_glk;
		struct bc_cpu_atom_avn cpu_avn;
		struct bc_cpu_atom_byt cpu_byt;
	};
};

extern int spi_read_sbase(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch,
			  struct spi_sbase *reg);
extern int spi_read_bc(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch,
		       struct spi_bc *reg);
extern int spi_read_bc_bioswe(const struct spi_bc *reg, u64 *value);
extern int spi_read_bc_ble(const struct spi_bc *reg, u64 *value);
extern int spi_read_bc_smm_bwp(const struct spi_bc *reg, u64 *value);
extern int spi_read_sbase_base(const struct spi_sbase *reg, u64 *value);
#endif /* BIOS_DATA_ACCESS_H */
