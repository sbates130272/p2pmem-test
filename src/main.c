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

#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

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
	size_t chunks;
} cfg = {
	.size   = 4096,
	.chunks = 1024,
};

int main(int argc, char **argv)
{
	void *p2pmem;
	ssize_t count;

	const struct argconfig_options opts[] = {
		{"nvme-read", .cfg_type=CFG_FD_RDD,
		 .value_addr=&cfg.nvme_read_fd,
		 .argument_type=required_positional,
		 .force_default="/dev/nvme0n1",
		 .help="NVMe device to read"},
		{"nvme-write", .cfg_type=CFG_FD_WRD,
		 .value_addr=&cfg.nvme_write_fd,
		 .argument_type=required_positional,
		 .force_default="/dev/nvme1n1",
		 .help="NVMe device to write"},
		{"p2pmem", .cfg_type=CFG_FD_RDWR,
		 .value_addr=&cfg.p2pmem_fd,
		 .argument_type=required_positional,
		 .force_default="/dev/p2pmem0",
		 .help="p2pmem device to use as buffer"},
		{"size", 's', "", CFG_SIZE, &cfg.size, required_argument,
		 "size of data transfer"},
		{"chunks", 'c', "", CFG_SIZE, &cfg.chunks, required_argument,
		 "number of chunks to transfer"},
		{NULL}
	};

	argconfig_parse(argc, argv, desc, opts, &cfg, sizeof(cfg));

	p2pmem = mmap(NULL, cfg.size, PROT_READ | PROT_WRITE, MAP_SHARED,
		      cfg.p2pmem_fd, 0);

	if (p2pmem == MAP_FAILED)
		perror("mmap");

	fprintf(stdout,"Running p2pmem-test: Reading %s : Writing %s : "
		"p2pmem Buffer %s.\n",cfg.nvme_read_filename, cfg.nvme_write_filename,
		cfg.p2pmem_filename);

	for (size_t i=0; i<cfg.chunks; i++) {

		count = read(cfg.nvme_read_fd, p2pmem, cfg.size);

		if (count == -1)
			perror("read");

		count = write(cfg.nvme_write_fd, p2pmem, cfg.size);

		if (count == -1)
			perror("write");
	}

	munmap(p2pmem, cfg.size);

	return 0;
}
