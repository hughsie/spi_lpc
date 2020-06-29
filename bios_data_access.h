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

struct SBASE_atom_avn_byt {
	u64 MEMI;
	u64 Enable;
	u64 ADDRNG;
	u64 PREF;
	u64 Base;
};

struct SBASE {
	struct RegisterArch register_arch;

	union {
		struct SBASE_atom_avn_byt cpu_avn;
		struct SBASE_atom_avn_byt cpu_byt;
	};
};

struct BC_pch_3xx_4xx_5xx {
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

struct BC_cpu_snb_jkt_ivb_ivt_bdx_hsx {
	u64 BIOSWE;
	u64 BLE;
	u64 SRC;
	u64 TSS;
	u64 SMM_BWP;
};

struct BC_cpu_skl_kbl_cfl {
	u64 BIOSWE;
	u64 BLE;
	u64 SRC;
	u64 TSS;
	u64 SMM_BWP;
	u64 BBS;
	u64 BILD;
};

struct BC_cpu_apl_glk {
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

struct BC_cpu_atom_avn {
	u64 BIOSWE;
	u64 BLE;
	u64 SRC;
	u64 TSS;
	u64 SMM_BWP;
};

struct BC_cpu_atom_byt {
	u64 BIOSWE;
	u64 BLE;
	u64 SRC;
	u64 SMM_BWP;
};

struct BC {
	struct RegisterArch register_arch;

	union {
		struct BC_pch_3xx_4xx_5xx pch_3xx;
		struct BC_pch_3xx_4xx_5xx pch_4xx;
		struct BC_pch_3xx_4xx_5xx pch_495;
		struct BC_pch_3xx_4xx_5xx pch_5xx;
		struct BC_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_snb;
		struct BC_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_jkt;
		struct BC_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_ivb;
		struct BC_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_ivt;
		struct BC_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_bdw;
		struct BC_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_bdx;
		struct BC_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_hsx;
		struct BC_cpu_snb_jkt_ivb_ivt_bdx_hsx cpu_hsw;
		struct BC_cpu_skl_kbl_cfl cpu_skl;
		struct BC_cpu_skl_kbl_cfl cpu_kbl;
		struct BC_cpu_skl_kbl_cfl cpu_cfl;
		struct BC_cpu_apl_glk cpu_apl;
		struct BC_cpu_apl_glk cpu_glk;
		struct BC_cpu_atom_avn cpu_avn;
		struct BC_cpu_atom_byt cpu_byt;
	};
};

extern int read_SBASE(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch,
		      struct SBASE *reg);
extern int read_BC(enum PCH_Arch pch_arch, enum CPU_Arch cpu_arch,
		   struct BC *reg);
extern int read_BC_BIOSWE(const struct BC *reg, u64 *value);
extern int read_BC_BLE(const struct BC *reg, u64 *value);
extern int read_BC_SMM_BWP(const struct BC *reg, u64 *value);
extern int read_SBASE_Base(const struct SBASE *reg, u64 *value);
#endif /* BIOS_DATA_ACCESS_H */
