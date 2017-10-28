# p2pmem-test

Welcome to p2pmem-test, a utility for testing PCI Peer-2-Peer (P2P)
communication between p2pmem and NVMe devices. This utility becomes
very interesting as NVMe devices become p2pmem devices via the
Controller Memory Buffer (CMB) and Persistent Memory Region (PMR)
features.

## Contributing

p2pmem-test is an active project. We will happily consider Pull
Requests (PRs) submitted against the offical repo at
www.github.com/sbates130272/p2pmem-test.git.

## Getting Started

0. Make sure the (Linux based) OS supports p2pmem [1]. p2pmem is not
upstream (yet) so you will need to grab it and build you own kernel.

0. Make sure your system has at least one p2pmem capable
device. Examples include an Eideticom IOMEM device, a Microsemi NVRAM 
card or a CMB enabled NVMe SSD that supports the WDS and RDS features
(e.g. The Everspin NVNitro card or the Eideticom NoLoad
HDK). Basically this is any PCI EP capable of exposing a BAR with a
driver that ties into the p2pmem framework.

0. In addition to the p2pmem capable device you need at least one
other NVMe SSD. This does not have to be CMB enabled (but it is OK if
it does support CMB), any standard NVMe SSD will do. Perferably you
will have more than one NVMe SSD but its not a requirement.

0. Ideally place the two (or more) devices noted in the two previous
points behind a PCIe switch (for example the Microsemi Switchtec or a
PLX switch). If you do not have a switch you can connect both devices
to the Root Complex (RC) on the CPU but two things *may* happen:
  0. Performance may drop. Many RCs are ineffecient at routing P2P
  traffic.
  0. It might not work at all. Many RCs block P2P traffic.

0. You almost certainly want to disable the IOMMU in your system via
both the BIOS and the kernel. Even with these steps some BIOS still
activate PCIe Access Control Services (ACS) and part of ACS includes
TLP redirection which kills p2pmem. If this is happening on your setup
you can try and disable ACS or use this patch [2] to get the kernel to
turn it off. Use with care as it does bad things to IOVA sharing.

0. Boot your system. Assuming your p2pmem driver (which might be the
NVMe driver in your case) bound to the PCI EP exposing the p2pmem you
should see some lines in dmesg regarding p2pmem. You should also see a
device for each p2pmem in your system (e.g. /dev/p2pmemN where N is a
numnber). If you don't check all the steps above. Note there is also a
debugsfs entry for each p2pmem device in <debugfs mount
point>/p2pmem/p2pmemN.

0. Compile p2pmem-test by runnning make. Note p2pmem-test using
libargconfig as a submodule so make sure you have run submodule init
to pull that code.

0. You can now run p2pmem-test. Use p2pmem-test -h to get a list of
options.

## NVMe CMBs vs PCI Bounce Buffering

When one of the NVMe devices and the p2pmem device are the same PCI EP
then CMB should be automatically used. This means the NVMe device
should detect the data is in its CMB and do an internal data movement
rather than an external DMA. If the p2pmem device is neither of the
NVMe devices then two external DMAs will occur. We refer to this
second option as a bounce buffer for obvious reasons.

## Examples

Some simple examples:

./p2pmem-test /dev/nvme0n1 /dev/nvme1n1 /dev/p2pmem0 -s 1 -4k --check

Copy one 4KB chunk from /dev/nvme0n1 to /dev/nvme0n1 via the memory
exposed by /dev/p2pmem0. Perform a check on the data (i.e. write know
data to /dev/nvme0n1 and validate that by reading /dev/nvme1n1 after
the p2pmem based transfer).

## References

[1] https://github.com/sbates130272/linux-p2pmem
[2] https://lkml.org/lkml/2017/10/26/678
