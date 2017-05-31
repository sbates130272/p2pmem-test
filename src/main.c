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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <argconfig/argconfig.h>
#include <argconfig/report.h>
#include <argconfig/suffix.h>

#include "version.h"

#define HOST_ACCESSES 32

const char *desc = "Perform p2pmem and NVMe CMB testing (ver=" VERSION ")";

static struct {
	int nvme_read_fd;
	const char *nvme_read_filename;
	int nvme_write_fd;
	const char *nvme_write_filename;
	int p2pmem_fd;
	const char *p2pmem_filename;
	void     *buffer;
	unsigned check;
	size_t   chunk_size;
	size_t   chunks;
	int      host_access;
	long     page_size;
	int      read_parity;
	int      seed;
	size_t   size;
	int      write_parity;
} cfg = {
	.check       = 0,
	.chunk_size  = 4096,
	.chunks      = 1024,
	.host_access = 0,
	.seed        = -1,
};

static int hosttest()
{
	unsigned char hostwrdata[abs(cfg.host_access)*HOST_ACCESSES],
		hostrddata[abs(cfg.host_access)*HOST_ACCESSES];
	size_t offset, offsets[HOST_ACCESSES],
		hostbytes = abs(cfg.host_access)*HOST_ACCESSES;
	struct hostaccess {
		char entry[abs(cfg.host_access)];
	};
	struct hostaccess *hostdataptr = (struct hostaccess *)&hostwrdata[0];

	if (hostbytes > cfg.size) {
		errno = ENOMEM;
		return -1;
	}

	if (cfg.host_access > 0) {

		for (size_t i=0; i<hostbytes; i++)
			hostwrdata[i] = (char)rand();

		for(size_t i=0; i<HOST_ACCESSES; i++) {

			offsets[i] = (size_t)(rand() % (cfg.chunk_size /
							abs(cfg.host_access)));
			*((struct hostaccess *)(cfg.buffer) + offsets[i]) =
				*(hostdataptr + i);
		}
	}

        hostdataptr = (struct hostaccess *)&hostrddata[0];
	for(size_t i=0; i<HOST_ACCESSES; i++) {

		if (cfg.host_access < 0 )
			offset = (size_t)(rand() % (cfg.chunk_size /
						    abs(cfg.host_access)));
		else
			offset = offsets[i];
		*(hostdataptr + i) =
			*((struct hostaccess *)(cfg.buffer) + offset);
	}

	if (cfg.check && (cfg.host_access > 0)) {

		for (size_t i=0; i<HOST_ACCESSES; i++) {

			unsigned compare = 1;
			for (size_t j=0; j<HOST_ACCESSES; j++) {
				if (offsets[i] == offsets[j]) {
					compare = 0; // Don't check double writes.
					break;
				}
			}
			if (compare && (hostwrdata[i] != hostrddata[i])) {
				fprintf(stdout, "MISMATCH on host_access %04zd "\
					": 0x%02x != 0x%02x\n", i, hostwrdata[i],
					hostrddata[i]);
				errno = EINVAL;
				return -1;
			}
		}
		fprintf(stdout, "MATCH on host_access.\n");
	}

	/*
	 * Reset RNG so writedata is consistent.
	 */

	srand(cfg.seed);
	return 0;
}

static int writedata()
{
	int *buffer;
	ssize_t count;
	int ret = 0;

	if (posix_memalign((void **)&buffer, cfg.page_size, cfg.size))
		return -1;

	cfg.write_parity = 0;
	for (size_t i=0; i<(cfg.size/sizeof(int)); i++) {
		buffer[i] = rand();
		cfg.write_parity ^= buffer[i];
	}
	count = write(cfg.nvme_read_fd, (void *)buffer, cfg.size);
	if (count == -1) {
		ret = -1;
		goto out;
	}

out:
	free(buffer);
	return ret;
}

static int readdata()
{
	int *buffer;
	ssize_t count;
	int ret = 0;

	if (posix_memalign((void **)&buffer, cfg.page_size, cfg.size))
		return -1;

	count = read(cfg.nvme_write_fd, (void *)buffer, cfg.size);
	if (count == -1) {
		ret = -1;
		goto out;
	}

	cfg.read_parity = 0;
	for (size_t i=0; i<(cfg.size/sizeof(int)); i++)
		cfg.read_parity ^= buffer[i];

out:
	free(buffer);
	return ret;
}


int main(int argc, char **argv)
{
	ssize_t count;
	struct timeval start_time, end_time;
	double val;
	const char *suf;

	const struct argconfig_options opts[] = {
		{"nvme-read", .cfg_type=CFG_FD_RDWR_DIRECT,
		 .value_addr=&cfg.nvme_read_fd,
		 .argument_type=required_positional,
		 .force_default="/dev/nvme0n1",
		 .help="NVMe device to read"},
		{"nvme-write", .cfg_type=CFG_FD_RDWR_DIRECT,
		 .value_addr=&cfg.nvme_write_fd,
		 .argument_type=required_positional,
		 .force_default="/dev/nvme1n1",
		 .help="NVMe device to write"},
		{"p2pmem", .cfg_type=CFG_FD_RDWR,
		 .value_addr=&cfg.p2pmem_fd,
		 .argument_type=optional_positional,
		 .help="p2pmem device to use as buffer (omit for sys memory)"},
		{"check", 0, "", CFG_NONE, &cfg.check, no_argument,
		 "perform checksum check on transfer (slow)"},
		{"chunks", 'c', "", CFG_LONG_SUFFIX, &cfg.chunks, required_argument,
		 "number of chunks to transfer"},
		{"host_access", 0, "", CFG_INT, &cfg.host_access, required_argument,
		 "alignment and size for host access test (0 = no test, <0 = read only test)"},
		{"seed", 0, "", CFG_INT, &cfg.seed, required_argument,
		 "seed to use for random data (-1 for time based)"},
		{"size", 's', "", CFG_LONG_SUFFIX, &cfg.chunk_size, required_argument,
		 "size of data chunk"},
		{NULL}
	};

	argconfig_parse(argc, argv, desc, opts, &cfg, sizeof(cfg));
	cfg.page_size = sysconf(_SC_PAGESIZE);
	cfg.size = cfg.chunk_size*cfg.chunks;

	if (cfg.p2pmem_fd) {
		cfg.buffer = mmap(NULL, cfg.chunk_size, PROT_READ | PROT_WRITE, MAP_SHARED,
				  cfg.p2pmem_fd, 0);
		if (cfg.buffer == MAP_FAILED) {
			perror("mmap");
			goto fail_out;
		}
	} else {
		if (posix_memalign(&cfg.buffer, cfg.page_size, cfg.chunk_size)) {
			perror("posix_memalign");
			goto fail_out;
		}
	}

	if ( cfg.seed == -1 )
		cfg.seed = time(NULL);
	srand(cfg.seed);

	fprintf(stdout,"Running p2pmem-test: reading %s : writing %s : "
		"p2pmem buffer %s.\n",cfg.nvme_read_filename, cfg.nvme_write_filename,
		cfg.p2pmem_filename);
	val = cfg.size;
	suf = suffix_si_get(&val);
	fprintf(stdout,"\tchunk size = %zd : number of chunks =  %zd: total = %g%sB.\n",
		cfg.chunk_size, cfg.chunks, val, suf);
	fprintf(stdout,"\tbuffer = %p (%s)\n", cfg.buffer,
		cfg.p2pmem_fd ? "p2pmem" : "system memory");
	fprintf(stdout,"\tPAGE_SIZE = %ldB\n", cfg.page_size);
	if (cfg.host_access)
		fprintf(stdout,"\tchecking host access %s: alignment and size = %dB\n",
			cfg.host_access < 0 ? "(read only) " : "", abs(cfg.host_access));
	if (cfg.check)
		fprintf(stdout,"\tchecking data with seed = %d\n", cfg.seed);

	if (cfg.host_access)
		if (hosttest(cfg)) {
			perror("hosttest");
			goto free_fail_out;
		}

	if (cfg.check)
		if (writedata(cfg)) {
			perror("writedata");
			goto free_fail_out;
		}

	if (lseek(cfg.nvme_read_fd, 0, SEEK_SET) == -1) {
		perror("writedata-lseek");
		goto free_fail_out;
	}

	gettimeofday(&start_time, NULL);
	for (size_t i=0; i<cfg.chunks; i++) {

		count = read(cfg.nvme_read_fd, cfg.buffer, cfg.chunk_size);

		if (count == -1) {
			perror("read");
			goto free_fail_out;
		}

		count = write(cfg.nvme_write_fd, cfg.buffer, cfg.chunk_size);

		if (count == -1) {
			perror("write");
			goto free_fail_out;
		}
	}
	gettimeofday(&end_time, NULL);

	if (cfg.check) {
		if (lseek(cfg.nvme_write_fd, 0, SEEK_SET) == -1) {
			perror("readdata-lseek");
			goto free_fail_out;
		}
		if (readdata(cfg)) {
			perror("readdata");
			goto free_fail_out;
		}
	}

	if (cfg.check)
		fprintf(stdout, "%s on data check, 0x%x %s 0x%x.\n",
			cfg.write_parity==cfg.read_parity ? "MATCH" : "MISMATCH",
			cfg.write_parity,
			cfg.write_parity==cfg.read_parity ? "=" : "!=",
			cfg.read_parity);

	fprintf(stdout, "Transfer:\n");
	report_transfer_rate(stdout, &start_time, &end_time, cfg.size);
	fprintf(stdout, "\n");

	if (cfg.p2pmem_fd)
		munmap(cfg.buffer, cfg.chunk_size);
	else
		free(cfg.buffer);

	return EXIT_SUCCESS;


free_fail_out:
	if (cfg.p2pmem_fd)
		munmap(cfg.buffer, cfg.chunk_size);
	else
		free(cfg.buffer);
fail_out:
	return EXIT_FAILURE;
}
