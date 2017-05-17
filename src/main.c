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

const char *desc = "Perform p2pmem and NVMe CMB testing (ver=" VERSION ")";

static struct {
	int nvme_read_fd;
	const char *nvme_read_filename;
	int nvme_write_fd;
	const char *nvme_write_filename;
	int p2pmem_fd;
	const char *p2pmem_filename;
	size_t size;
	size_t chunk;
} cfg = {
	.size  = 1048576,
	.chunk = 1024,
};

int main(int argc, char **argv)
{
	const struct argconfig_options opts[] = {
		{"nvme-read", .cfg_type=CFG_FD_RD,
		 .value_addr=&cfg.nvme_read_fd,
		 .argument_type=required_positional,
		 .force_default="/dev/nvme0n1",
		 .help="NVMe device to read"},
		{"nvme-write", .cfg_type=CFG_FD_WR,
		 .value_addr=&cfg.nvme_write_fd,
		 .argument_type=required_positional,
		 .force_default="/dev/nvme1n1",
		 .help="NVMe device to write"},
		{"p2pmem", .cfg_type=CFG_FD_WR,
		 .value_addr=&cfg.p2pmem_fd,
		 .argument_type=optional_positional,
		 .force_default="/dev/p2pmem0",
		 .help="p2pmem device to use as buffer"},
		{"size", 's', "", CFG_SIZE, &cfg.size, required_argument,
		 "total size of data transfer"},
		{"chunk", 'c', "", CFG_SIZE, &cfg.chunk, required_argument,
		 "size of each single transfer (note OS might not honor this)"},
		{NULL}
	};

	argconfig_parse(argc, argv, desc, opts, &cfg, sizeof(cfg));

	return 0;
}
