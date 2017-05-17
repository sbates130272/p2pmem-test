/*
 * Raithlin Consulting Inc. p2pmem test suite
 * Copyright (c) 2017, Raithlin Consulting Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#include <argconfig/argconfig.h>
#include "version.h"

struct nvme_dev;
struct p2pmem_dev;

int nvme_read_handler(const char *optarg, void *value_addr,
		      const struct argconfig_options *opt)
{
	return 0;
}

int nvme_write_handler(const char *optarg, void *value_addr,
		       const struct argconfig_options *opt)
{
	return 0;
}

int p2pmem_handler(const char *optarg, void *value_addr,
		   const struct argconfig_options *opt)
{
	return 0;
}

const char *desc = "Perform p2pmem and NVMe CMB testing (ver=" VERSION ")";
static struct {
	struct nvme_dev *nvme_read;
	struct nvme_dev *nvme_write;
	struct p2pmem_dev *p2pmem;
	unsigned reset_bytes;
	unsigned refresh;
	int duration;
} cfg = {
	.refresh  = 1,
	.duration = -1,
};

#define NVME_READ_OPTION {"nvme-read", .cfg_type=CFG_CUSTOM,		\
			.value_addr=&cfg.nvme_read,			\
			.argument_type=required_positional,		\
			.custom_handler=nvme_read_handler,		\
			.complete="/dev/nvme*",				\
			.help="NVMe device to read"}

#define NVME_WRITE_OPTION {"nvme-write", .cfg_type=CFG_CUSTOM,		\
			.value_addr=&cfg.nvme_write,			\
			.argument_type=required_positional,		\
			.custom_handler=nvme_write_handler,		\
			.complete="/dev/nvme*",				\
			.help="NVMe device to write"}

#define P2PMEM_OPTION {"p2pmem", .cfg_type=CFG_CUSTOM,			\
			.value_addr=&cfg.p2pmem,			\
			.argument_type=required_positional,		\
			.custom_handler=p2pmem_handler,			\
			.complete="/dev/p2pmem*",			\
			.help="p2pmem buffer to use"}

int main(int argc, char **argv)
{
	const struct argconfig_options opts[] = {
		NVME_READ_OPTION,
		NVME_WRITE_OPTION,
		P2PMEM_OPTION,
		{"reset", 'r', "", CFG_NONE, &cfg.reset_bytes, no_argument,
		 "reset byte counters"},
		{"refresh", 'f', "", CFG_POSITIVE, &cfg.refresh, required_argument,
		 "gui refresh period in seconds (default: 1 second)"},
		{"duration", 'd', "", CFG_INT, &cfg.duration, required_argument,
		 "gui duration in seconds (-1 forever)"},
		{NULL}
	};

	argconfig_parse(argc, argv, desc, opts, &cfg, sizeof(cfg));

	return 0;
}
