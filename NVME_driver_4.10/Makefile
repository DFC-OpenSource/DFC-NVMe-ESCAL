obj-$(CONFIG_NVME_CORE)			+= nvme-core.o
obj-$(CONFIG_BLK_DEV_NVME)		+= nvme.o
obj-$(CONFIG_NVME_FABRICS)		+= nvme-fabrics.o
obj-$(CONFIG_NVME_RDMA)			+= nvme-rdma.o

nvme-core-y				:= core.o
nvme-core-$(CONFIG_BLK_DEV_NVME_SCSI)	+= scsi.o
nvme-core-$(CONFIG_NVM)			+= lightnvm.o

nvme-y					+= pci.o

nvme-fabrics-y				+= fabrics.o

nvme-rdma-y				+= rdma.o

#obj-m+=pci.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
