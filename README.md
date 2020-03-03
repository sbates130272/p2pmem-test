# p2pmem-test: A tool for userspace testing of the p2pdma kernel framework

Welcome to p2pmem-test, a utility for testing PCI Peer-2-Peer (P2P)
communication between p2pmem and NVMe devices. This utility becomes
very interesting as NVMe devices become p2pmem devices via the
Controller Memory Buffer (CMB) and Persistent Memory Region (PMR)
features.

## Contributing

p2pmem-test is an active project. We will happily consider Pull
Requests (PRs) submitted against the [official repo][1].

## Getting Started
1. In order to run any P2P traffic, you'll need to have a (Linux based) OS
that supports [p2pdma][2]. This framework is available in all Linux kernels 4.20
or newer, however it is NOT compiled in by default. You will almost certainly
have to compile a kernel from source and install that. The instructions on how
to do this are beyond the scope of this document but there is a tool that can
help [here][3].

1. You'll need install the separate [p2pmem-pci][5] module installed to expose
the device to userspace. Once installed, you should see the device exposed as
a /dev/p2pmemX.

1. Make sure your system has at least one p2pdma capable
device. Examples include an Eideticom IOMEM device, a Microsemi NVRAM
card or a CMB enabled NVMe SSD that supports the WDS and RDS features
(e.g. The Everspin NVNitro card or the [Eideticom NoLoad<sup>TM</sup> device][4]).
Basically this is any PCI EP capable of exposing a BAR with a driver that ties
into the p2pdma framework.

1. In addition to the p2pmem capable device you need at least one
other NVMe SSD. This does not have to be CMB enabled (but it is OK if
it does support CMB), any standard NVMe SSD will do. Preferably you
will have more than one NVMe SSD but its not a requirement. Ideally place
the two (or more) devices noted in the two previous points behind a PCIe
switch (for example the Microsemi Switchtec or a PLX switch). If you do not
have a switch you can connect both devices to the Root Complex (RC) on the CPU
but two things *may* happen:
   * Performance may drop. Many RCs are inefficient at routing P2P traffic.
   * It might not work at all. Many RCs block P2P traffic.

1. You'll almost certainly want to disable the IOMMU in your system
via either the BIOS or the kernel.

1. You may also need to disable the PCIe Access Control Services (ACS)
by either the BIOS and/or the kernel because TLP redirection (activated as part
of ACS) kills P2P traffic.

1. Finally, p2pmem-test requires the libargconfig submodule. You'll need to
either clone recursively or via `git submodule update --init`.

## NVMe CMBs vs PCI Bounce Buffering

When one of the NVMe devices and the p2pmem device are the same PCI EP
then CMB should be automatically used. This means the NVMe device
should detect the data is in its CMB and do an internal data movement
rather than an external DMA. If the p2pmem device is neither of the
NVMe devices then two external DMAs will occur. We refer to this
second option as a bounce buffer for obvious reasons.

## Examples

Some simple examples:
./p2pmem-test /dev/nvme0n1 /dev/nvme1n1 /dev/p2pmem0 -c 1 -s 4k --check

Copy one 4KB chunk from /dev/nvme0n1 to /dev/nvme0n1 via the memory
exposed by /dev/p2pmem0. Perform a check on the data (i.e. write know
data to /dev/nvme0n1 and validate that by reading /dev/nvme1n1 after
the p2pmem based transfer).

[1]: www.github.com/sbates130272/p2pmem-test.git
[2]: https://www.kernel.org/doc/html/latest/driver-api/pci/p2pdma.html
[3]: https://github.com/sbates130272/kernel-tools
[4]: https://www.eideticom.com/products.html
[5]: https://github.com/Eideticom/p2pmem-pci
