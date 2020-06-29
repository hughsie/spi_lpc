/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SPI LPC flash platform security driver
 *
 * Copyright 2020 (c) Daniel Gutson (daniel.gutson@eclypsium.com)
 *
 */

#ifndef VIDDID_ARCH_MAP_H
#define VIDDID_ARCH_MAP_H

#include "bios_data_access.h"

int viddid2pch_arch(u64 vid, u64 did, enum PCH_Arch *arch);
int viddid2cpu_arch(u64 vid, u64 did, enum CPU_Arch *arch);

#endif /* VIDDID_ARCH_MAP_H */
