/*
 * board.c
 *
 * Board functions for TI AM335X based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <status_led.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <power/tps65217.h>
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;


static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

#ifdef CONFIG_SPL_BUILD

/* Micron MT41K256M8DA-125K */
#define MT41K256M8DA125K_EMIF_READ_LATENCY  0x100007
#define MT41K256M8DA125K_EMIF_TIM1          0x154BB99B
#define MT41K256M8DA125K_EMIF_TIM2          0x266B7FDA
#define MT41K256M8DA125K_EMIF_TIM3          0x501F867F
#define MT41K256M8DA125K_EMIF_SDCFG         0x61C05332
#define MT41K256M8DA125K_EMIF_SDREF         0xC30
#define MT41K256M8DA125K_ZQ_CFG             0x50074BE4
#define MT41K256M8DA125K_RATIO              0x80
#define MT41K256M8DA125K_INVERT_CLKOUT      0x0
#define MT41K256M8DA125K_RD_DQS             0x3A
#define MT41K256M8DA125K_WR_DQS             0x4C
#define MT41K256M8DA125K_PHY_WR_DATA        0x84
#define MT41K256M8DA125K_PHY_FIFO_WE        0xB2
#define MT41K256M8DA125K_IOCTRL_VALUE       0x18B


static const struct ddr_data ddr3_data = {
	.datardsratio0 = MT41K256M8DA125K_RD_DQS,
	.datawdsratio0 = MT41K256M8DA125K_WR_DQS,
	.datafwsratio0 = MT41K256M8DA125K_PHY_FIFO_WE,
	.datawrsratio0 = MT41K256M8DA125K_PHY_WR_DATA,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
	.cmd0csratio = MT41K256M8DA125K_RATIO,
	.cmd0iclkout = MT41K256M8DA125K_INVERT_CLKOUT,

	.cmd1csratio = MT41K256M8DA125K_RATIO,
	.cmd1iclkout = MT41K256M8DA125K_INVERT_CLKOUT,

	.cmd2csratio = MT41K256M8DA125K_RATIO,
	.cmd2iclkout = MT41K256M8DA125K_INVERT_CLKOUT,
};

static struct emif_regs ddr3_emif_reg_data = {
	.sdram_config = MT41K256M8DA125K_EMIF_SDCFG,
	.ref_ctrl = MT41K256M8DA125K_EMIF_SDREF,
	.sdram_tim1 = MT41K256M8DA125K_EMIF_TIM1,
	.sdram_tim2 = MT41K256M8DA125K_EMIF_TIM2,
	.sdram_tim3 = MT41K256M8DA125K_EMIF_TIM3,
	.zq_config = MT41K256M8DA125K_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41K256M8DA125K_EMIF_READ_LATENCY,
};

#define OSC	(V_OSCK/1000000)
const struct dpll_params dpll_ddr = {
		400, OSC-1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	int mpu_vdd;

	/* Get the frequency */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);

	/* PMIC Code */
	int usb_cur_lim;

	enable_i2c0_pin_mux();
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	if (i2c_probe(TPS65217_CHIP_PM))
		return;

	/* Override what we have detected since it supports 1GHz. */
	dpll_mpu_opp100.m = MPUPLL_M_1000;

	/*
	 * Increase USB current limit to 1300mA or 1800mA and set
	 * the MPU voltage controller as needed.
	 */
	usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1800MA;
	mpu_vdd = TPS65217_DCDC_VOLT_SEL_1325MV;

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
			       TPS65217_POWER_PATH,
			       usb_cur_lim,
			       TPS65217_USB_INPUT_CUR_LIMIT_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set DCDC3 (CORE) voltage to 1.125V */
	if (tps65217_voltage_update(TPS65217_DEFDCDC3,
				    TPS65217_DCDC_VOLT_SEL_1125MV)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set DCDC2 (MPU) voltage */
	if (tps65217_voltage_update(TPS65217_DEFDCDC2, mpu_vdd)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/*  Set LDO3 to 1.8V and LDO4 to 3.3V */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS1,
			       TPS65217_LDO_VOLTAGE_OUT_1_8,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS2,
			       TPS65217_LDO_VOLTAGE_OUT_3_3,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr;
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}

void sdram_init(void)
{
	config_ddr(400, MT41K256M8DA125K_IOCTRL_VALUE,
		   &ddr3_data,
		   &ddr3_cmd_ctrl_data,
		   &ddr3_emif_reg_data, 0);
}
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#if defined(CONFIG_STATUS_LED) && defined(STATUS_LED_BOOT)
	status_led_set (STATUS_LED_BOOT, STATUS_LED_ON);
	status_led_set (STATUS_LED_RED, STATUS_LED_OFF);
#endif
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	setenv("board_name", "EC3");
	setenv("board_rev", "1");
#endif
	return 0;
}
#endif

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD))
static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 1,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};
#endif

#ifdef CONFIG_DRIVER_TI_CPSW
int board_eth_init(bd_t *bis)
{	
#ifndef CONFIG_SPL_BUILD
	int rv, n = 0;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;

	/* try reading mac address from efuse */
	mac_lo = readl(&cdev->macid0l);
	mac_hi = readl(&cdev->macid0h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	if (!getenv("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC\n");

		if (is_valid_ether_addr(mac_addr))
			eth_setenv_enetaddr("ethaddr", mac_addr);
	}

	writel(MII_MODE_ENABLE, &cdev->miisel);
	cpsw_slaves[0].phy_if = PHY_INTERFACE_MODE_MII;

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;

	return n;
#else
	return 0;
#endif
}
#endif
