// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm SM7635 (SM6650)
 *
 * Copyright (c) 2025, Danila Tikhonov <danila@jiaxyga.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,sm7635-gcc.h>

#include "clock-qcom.h"

static const struct freq_tbl ftbl_gcc_usb30_prim_master_clk_src[] = {
	F(66666667, CFG_CLK_SRC_GPLL0_EVEN, 4.5, 0, 0),
	F(133333333, CFG_CLK_SRC_GPLL0, 4.5, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	F(240000000, CFG_CLK_SRC_GPLL0, 2.5, 0, 0),
	{ }
};

static ulong sm7635_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	switch (clk->id) {
	/* TODO: Add PCIE, QUP (for UART), SDCC2, UFS and missing USB clocks */
	case GCC_USB30_PRIM_MASTER_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x3902c,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	default:
		return 0;
	}
}

static const struct gate_clk sm7635_clks[] = {
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK,		0x39018, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_AUX_CLK,		0x39060, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK,		0x39064, BIT(0)),
	GATE_CLK(GCC_AGGRE_USB3_PRIM_AXI_CLK,		0x39090, BIT(0)),
};

static int sm7635_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	/* TODO: Remove this debug code */
	if (priv->data->num_clks < clk->id) {
		debug("%s: unknown clk id %lu\n", __func__, clk->id);
		return 0;
	}

	debug("%s: clk %s\n", __func__, sm7635_clks[clk->id].name);

	switch (clk->id) {
	case GCC_AGGRE_USB3_PRIM_AXI_CLK:
		qcom_gate_clk_en(priv, GCC_USB30_PRIM_MASTER_CLK);
		fallthrough;
	case GCC_USB30_PRIM_MASTER_CLK:
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_AUX_CLK);
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_COM_AUX_CLK);
		break;
	}

	qcom_gate_clk_en(priv, clk->id);

	return 0;
}

static const struct qcom_reset_map sm7635_gcc_resets[] = {
	[GCC_CAMERA_BCR] = { 0x26000 },
	[GCC_DISPLAY_BCR] = { 0x27000 },
	[GCC_GPU_BCR] = { 0x71000 },
	[GCC_PCIE_0_BCR] = { 0x6b000 },
	[GCC_PCIE_0_LINK_DOWN_BCR] = { 0x6c014 },
	[GCC_PCIE_0_NOCSR_COM_PHY_BCR] = { 0x6c020 },
	[GCC_PCIE_0_PHY_BCR] = { 0x6c01c },
	[GCC_PCIE_0_PHY_NOCSR_COM_PHY_BCR] = { 0x6c028 },
	[GCC_PCIE_1_BCR] = { 0x90000 },
	[GCC_PCIE_1_LINK_DOWN_BCR] = { 0x8e014 },
	[GCC_PCIE_1_NOCSR_COM_PHY_BCR] = { 0x8e020 },
	[GCC_PCIE_1_PHY_BCR] = { 0x8e01c },
	[GCC_PCIE_1_PHY_NOCSR_COM_PHY_BCR] = { 0x8e024 },
	[GCC_PCIE_RSCC_BCR] = { 0x11000 },
	[GCC_PDM_BCR] = { 0x33000 },
	[GCC_QUPV3_WRAPPER_0_BCR] = { 0x18000 },
	[GCC_QUPV3_WRAPPER_1_BCR] = { 0x1e000 },
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x12000 },
	[GCC_QUSB2PHY_SEC_BCR] = { 0x12004 },
	[GCC_SDCC1_BCR] = { 0xa3000 },
	[GCC_SDCC2_BCR] = { 0x14000 },
	[GCC_UFS_PHY_BCR] = { 0x77000 },
	[GCC_USB30_PRIM_BCR] = { 0x39000 },
	[GCC_USB3_DP_PHY_PRIM_BCR] = { 0x50008 },
	[GCC_USB3_PHY_PRIM_BCR] = { 0x50000 },
	[GCC_USB3PHY_PHY_PRIM_BCR] = { 0x50004 },
	[GCC_VIDEO_AXI0_CLK_ARES] = { 0x32018, 2 },
	[GCC_VIDEO_BCR] = { 0x32000 },
};

static const struct qcom_power_map sm7635_gdscs[] = {
	[PCIE_0_GDSC] = { 0x6b004 },
	[PCIE_0_PHY_GDSC] = { 0x6c000 },
	[UFS_PHY_GDSC] = { 0x77004 },
	[UFS_MEM_PHY_GDSC] = { 0x9e000 },
	[USB30_PRIM_GDSC] = { 0x39004 },
	[USB3_PHY_GDSC] = { 0x5000c },
};

static struct msm_clk_data sm7635_gcc_data = {
	.resets = sm7635_gcc_resets,
	.num_resets = ARRAY_SIZE(sm7635_gcc_resets),
	.clks = sm7635_clks,
	.num_clks = ARRAY_SIZE(sm7635_clks),
	.power_domains = sm7635_gdscs,
	.num_power_domains = ARRAY_SIZE(sm7635_gdscs),

	.enable = sm7635_enable,
	.set_rate = sm7635_set_rate,
};

static const struct udevice_id gcc_sm7635_of_match[] = {
	{
		.compatible = "qcom,gcc-sm7635",
		.data = (ulong)&sm7635_gcc_data,
	},
	{}
};

U_BOOT_DRIVER(gcc_sm7635) = {
	.name = "gcc_sm7635",
	.id = UCLASS_NOP,
	.of_match = gcc_sm7635_of_match,
	.bind = qcom_cc_bind,
	.flags = DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
