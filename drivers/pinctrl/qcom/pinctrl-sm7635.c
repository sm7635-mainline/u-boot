// SPDX-License-Identifier: BSD-3-Clause
/*
 * Pinctrl drivers for Qualcomm SM7635 (SM6650)
 *
 * Copyright (c) 2025, Danila Tikhonov <danila@jiaxyga.com>
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"gpio", 0},
};

#define SDC_QDSD_PINGROUP(pg_name, ctl, pull, drv)	\
	{						\
		.name = pg_name,			\
		.ctl_reg = ctl,				\
		.io_reg = 0,				\
		.pull_bit = pull,			\
		.drv_bit = drv,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = -1,				\
	}

#define UFS_RESET(pg_name, ctl, io)			\
	{						\
		.name = pg_name,			\
		.ctl_reg = ctl,				\
		.io_reg = io,				\
		.pull_bit = 3,				\
		.drv_bit = 0,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = 0,				\
	}

static const struct msm_special_pin_data msm_special_pins_data[] = {
	[0] = UFS_RESET("ufs_reset", 0xb4004, 0xb5000),
	[1] = SDC_QDSD_PINGROUP("sdc2_clk", 0xab000, 0, 6),
	[2] = SDC_QDSD_PINGROUP("sdc2_cmd", 0xab000, 12, 3),
	[3] = SDC_QDSD_PINGROUP("sdc2_data", 0xab000, 9, 0),
};

static const char *sm7635_get_function_name(struct udevice *dev,
						unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sm7635_get_pin_name(struct udevice *dev,
						unsigned int selector)
{
	if (selector >= 167 && selector <= 170)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 msm_special_pins_data[selector - 167].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static unsigned int sm7635_get_function_mux(__maybe_unused unsigned int pin,
						unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sm7635_data = {
	.pin_data = {
		.pin_count = 171,
		.special_pins_start = 167,
		.special_pins_data = msm_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sm7635_get_function_name,
	.get_function_mux = sm7635_get_function_mux,
	.get_pin_name = sm7635_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,sm7635-tlmm", .data = (ulong)&sm7635_data },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pinctrl_sm7635) = {
	.name		= "pinctrl_sm7635",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};
