/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated.
 * Sricharan R	  <r.sricharan@ti.com>
 *
 * Derived from OMAP4 done by:
 *	Aneesh V <aneesh@ti.com>
 *
 * TI OMAP5 AND DRA7XX common configuration settings
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * For more details, please see the technical documents listed at
 * http://www.ti.com/product/omap5432
 */

#ifndef __CONFIG_OMAP5_COMMON_H
#define __CONFIG_OMAP5_COMMON_H

#define CONFIG_OMAP54XX
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_MISC_INIT_R
#define CONFIG_ARCH_CPU_INIT

#define CONFIG_SYS_CACHELINE_SIZE	64

/* Use General purpose timer 1 */
#define CONFIG_SYS_TIMERBASE		GPT2_BASE

/*
 * For the DDR timing information we can either dynamically determine
 * the timings to use or use pre-determined timings (based on using the
 * dynamic method.  Default to the static timing infomation.
 */
#define CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS
#ifndef CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS
#define CONFIG_SYS_AUTOMATIC_SDRAM_DETECTION
#define CONFIG_SYS_DEFAULT_LPDDR2_TIMINGS
#endif

#ifndef CONFIG_SPL_BUILD
#define CONFIG_PALMAS_POWER
#endif

#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

#define CONFIG_ENV_SIZE			(128 << 10)

#include <configs/ti_armv7_common.h>

/*
 * Hardware drivers
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		48000000

/* Per-SoC commands */
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

/*
 * Environment setup
 */
#ifndef PARTS_DEFAULT
#define PARTS_DEFAULT
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x80200000\0" \
	"fdtaddr=0x80F80000\0" \
	"fdt_high=0xa0000000\0" \
	"rdaddr=0x81000000\0" \
	"console=" CONSOLEDEV ",115200n8\0" \
	"fdtfile=undefined\0" \
	"bootpart=0:2\0" \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"usbtty=cdc_acm\0" \
	"vram=16M \0" \
	"partitions=" PARTS_DEFAULT "\0" \
	"optargs=\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk1p2 rw\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"usbroot=/dev/sda2 rw\0" \
	"usbrootfstype=ext4 rootwait\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"vram=${vram} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"usbargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"vram=${vram}" \
		"root=${usbroot} " \
		"rootfstype=${usbrootfstype}\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
		"source ${loadaddr}\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} uEnv.txt\0" \
	"importbootenv=echo Importing environment from mmc${mmcdev} ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"loadimage=load mmc ${bootpart} ${loadaddr} ${bootdir}/${bootfile}\0" \
	"mmcboot=mmc dev ${mmcdev}; " \
		"if mmc rescan; then " \
			"echo SD/MMC found on device ${mmcdev};" \
			"if run loadbootenv; then " \
				"echo Loaded environment from ${bootenv};" \
				"run importbootenv;" \
			"fi;" \
			"if test -n $uenvcmd; then " \
				"echo Running uenvcmd ...;" \
				"run uenvcmd;" \
			"fi;" \
			"if run loadimage; then " \
				"run loadfdt; " \
				"echo Booting from mmc${mmcdev} ...; " \
				"run mmcargs; " \
				"bootz ${loadaddr} - ${fdtaddr}; " \
			"fi;" \
		"fi;\0" \
	"loadfdt=load ${devtype} ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile};\0" \
	FIND_FDT_FILE \
	BOOTCMD_COMMON \
	BOOTCMD_MMC \
	BOOTCMD_NAND \
	BOOTCMD_USB

/*
 * SPL related defines.  The Public RAM memory map the ROM defines the
 * area between 0x40300000 and 0x4031E000 as a download area for OMAP5
 * (dra7xx is larger, but we do not need to be larger at this time).  We
 * set CONFIG_SPL_DISPLAY_PRINT to have omap_rev_string() called and
 * print some information.
 */
#define CONFIG_SPL_TEXT_BASE		0x40300000
#define CONFIG_SPL_MAX_SIZE		(0x4031E000 - CONFIG_SPL_TEXT_BASE)
#define CONFIG_SPL_DISPLAY_PRINT
#define CONFIG_SPL_LDSCRIPT "$(CPUDIR)/omap-common/u-boot-spl.lds"

#endif /* __CONFIG_OMAP5_COMMON_H */
