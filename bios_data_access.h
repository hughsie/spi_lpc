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

struct sbase_register_atom_avn_byt {
	u64 MEMI;
	u64 Enable;
	u64 ADDRNG;
	u64 PREF;
	u64 Base;
};

struct sbase_register {
	struct RegisterArch register_arch;

	union {
		struct sbase_register_atom_avn_byt cpu_avn;
		struct sbase_register_atom_avn_byt cpu_byt;
	};
};

struct bios_control_register_pch_3xx_4xx_5xx {
	u64 BIOSWE;
	u64 BLE;
	u64 SRC;
	u64 TSS;
	u64 SMM_BWP;
	u64 BBS;
	u64 BILD;
	u64 SPI_SYNC_SS;
	u64 SPI_ASYNC_SS;
	u64 ASE_BWP;
};

struct bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx {
	u64 BIOSWE;
	u64 BLE;
	u64 SRC;
	u64 TSS;
	u64 SMM_BWP;
};

struct bios_control_register_cpu_skl_kbl_cfl {
	u64 BIOSWE;
	u64 BLE;
	u64 SRC;
	u64 TSS;
	u64 SMM_BWP;
	u64 BBS;
	u64 BILD;
};

struct bios_control_register_cpu_apl_glk {
	u64 BIOSWE;
	u64 BLE;
	u64 SRC;
	u64 TSS;
	u64 SMM_BWP;
	u64 BBS;
	u64 BILD;
	u64 SPI_SYNC_SS;
	u64 OSFH;
	u64 SPI_ASYNC_SS;
	u64 ASE_BWP;
};

struct bios_control_register_cpu_atom_avn {
	u64 BIOSWE;
	u64 BLE;
	u64 SRC;
	u64 TSS;
	u64 SMM_BWP;
};

struct bios_control_register_cpu_atom_byt {
	u64 BIOSWE;
	u64 BLE;
	u64 SRC;
	u64 SMM_BWP;
};

struct bios_control_register {
	struct RegisterArch register_arch;

	union {
		struct bios_control_register_pch_3xx_4xx_5xx pch_3xx;
		struct bios_control_register_pch_3xx_4xx_5xx pch_4xx;
		struct bios_control_register_pch_3xx_4xx_5xx pch_495;
		struct bios_control_register_pch_3xx_4xx_5xx pch_5xx;
		struct bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_snb;
		struct bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_jkt;
		struct bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_ivb;
		struct bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_ivt;
		struct bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_bdw;
		struct bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_bdx;
		struct bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_hsx;
		struct bios_control_register_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_hsw;
		struct bios_control_register_cpu_skl_kbl_cfl cpu_skl;
		struct bios_control_register_cpu_skl_kbl_cfl cpu_kbl;
		struct bios_control_register_cpu_skl_kbl_cfl cpu_cfl;
		struct bios_control_register_cpu_apl_glk cpu_apl;
		struct bios_control_register_cpu_apl_glk cpu_glk;
		struct bios_control_register_cpu_atom_avn cpu_avn;
		struct bios_control_register_cpu_atom_byt cpu_byt;
	};
};

extern int read_sbase_register(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch,
			       struct sbase_register *reg);
extern int read_bios_control_register(enum PCH_Arch pch_arch,
				      enum CPU_Arch cpu_arch,
				      struct bios_control_register *reg);
extern int
read_bios_control_register_BIOSWE(const struct bios_control_register *reg,
				  u64 *value);
extern int
read_bios_control_register_BLE(const struct bios_control_register *reg,
			       u64 *value);
extern int
read_bios_control_register_SMM_BWP(const struct bios_control_register *reg,
				   u64 *value);
extern int read_sbase_register_Base(const struct sbase_register *reg,
				    u64 *value);
#endif /* BIOS_DATA_ACCESS_H */
