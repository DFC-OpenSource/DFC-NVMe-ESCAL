/* uio_pci_generic - generic UIO driver for PCI 2.3 devices
 *
 * Copyright (C) 2009 Red Hat, Inc.
 * Author: Michael S. Tsirkin <mst@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.
 *
 * Since the driver does not declare any device ids, you must allocate
 * id and bind the device to the driver yourself.  For example:
 *
 * # echo "8086 10f5" > /sys/bus/pci/drivers/uio_pci_generic/new_id
 * # echo -n 0000:00:19.0 > /sys/bus/pci/drivers/e1000e/unbind
 * # echo -n 0000:00:19.0 > /sys/bus/pci/drivers/uio_pci_generic/bind
 * # ls -l /sys/bus/pci/devices/0000:00:19.0/driver

 * # modprobe uio_pci_generic
 * # ////%%%///echo "1665 3140" > /sys/bus/pci/drivers/uio_pci_generic/new_id
 * # echo -n 0000:01:00.0 > /sys/bus/pci/drivers/ls2_rc/unbind
 * # echo -n 0000:01:00.0 > /sys/bus/pci/drivers/uio_pci_generic/bind
 * # ls -l /sys/bus/pci/devices/0000\:01\:00.0/driver
 * .../0000:00:19.0/driver -> ../../../bus/pci/drivers/uio_pci_generic
 *
 * Driver won't bind to devices which do not support the Interrupt Disable Bit
 * in the command register. All devices compliant to PCI 2.3 (circa 2002) and
 * all compliant PCI Express devices should support this bit.
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/uio_driver.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/hardirq.h>
#include <linux/irqdomain.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>

#define DRIVER_VERSION	"0.02.0"
#define DRIVER_AUTHOR	"Michael S. Tsirkin <mst@redhat.com>"
#define DRIVER_DESC	"Generic UIO driver for PCI 2.3 devices"
//#define UIO_MSI
#define BAR0	0

#define VENDOR_ID 0x1957
#define DEVICE_ID 0x0953
#define CLASS_ID  0x010802

#define PCIE_REGS_START         0x3600000
#define LUT_REG_START		0x3680000		//ML - WHAT???
#define PCIE_REGS_SIZE          16*1024UL

#define DBI_REG                 0x8bc
#define LUT_CTRL_REG            0x7f8
#define INDEX_REG               0x900
#define CONTROL1_REG            0x904
#define CONTROL2_REG            0x908
#define BASE_ADD_LOW            0x90c
#define BASE_ADD_HIGH           0x910
#define ADD_RANGE_REG           0x914
#define TARGET_ADD_LOW          0x918
#define TARGET_ADD_HIGH         0x91c

static char pci_num = 0;

static struct pci_device_id pci_drv_ids[]  = {
	{ PCI_DEVICE(VENDOR_ID, DEVICE_ID), },
	{ 0, }
};

struct uio_pci_generic_dev {
	struct uio_info info;
	struct pci_dev *pdev;
#ifdef UIO_MSI
	bool have_msi;
#endif
};

static int no_interrupts = 1;
#ifdef UIO_MULTIPLE_MSI
static struct platform_device *pldev[14];
#endif    

static int inbound_config(int bar_no, uint64_t bar_address)
{
	uint32_t address = bar_address & 0xffffffff;
	uint32_t bar_low = address;
	uint32_t bar_high = (bar_address >> 32);
	uint32_t control2 = 0xc0000000 + (bar_no << 8);
	void __iomem *reg_vir;

	reg_vir = ioremap_nocache(PCIE_REGS_START, PCIE_REGS_SIZE);
	if (!reg_vir) {
		printk("Register remapping failed\n");
		return -1;
	}
	//Modified here murtuza - 	
		//writel(0x00000001, 0x36008bc); //DBI register
		//writel(0x0,0x3601014);		//Bar 1 mask
		//writel(0x0,0x3601018);		//Bar 2 mask
		//writel(0x0,0x360101c);		//Bar 3 mask
		//writel(0x0,0x3601020);		//Bar 4 mask
		//writel(0x0,0x3601030);		//Exapnsion rom
		//printk("PrintK here\n");
		writel(1, reg_vir + DBI_REG);
		//for 16 MB use below - 
		//writel(0x00FFFFFF,reg_vir + 0x1010);		//BAR 0 mask
		

		//for controller stack use below
		writel(0x1FFF,reg_vir + 0x1010);		//BAR 0 mask
		
		writel(0x003FFFFF,reg_vir + 0x1014);		//BAR 1 mask
		
		/*
		writel(0x0,0x360119c);		
		writel(0x0,0x36011a0);
		writel(0x0,0x36011a4);
		writel(0x0,0x36011a8);
		writel(0x0,0x36011ac);
		writel(0x0,0x36011b0);
		*/
		/*writel(0x0,0x3600180);		//SRIOV control register
		writel(0x00000001,0x3601010);	//Bar 0 mask
		writel(0x0,0x3600010);		//PCI BAR0
		writel(0x00001FFF,0x3601010);	//Bar0 mask
		writel(0x00000001,0x3601014);	//BAR 1 Mask
		writel(0x0,0x3601014);		//Bar 1 Mask
		writel(0x00003FFF,0x3601014);	//Bar 1 Mask
		*/
		/*
		writel(0x0,0x3621014);
		writel(0x0,0x3621018);
		writel(0x0,0x362101c);
		writel(0x0,0x3621020);
		writel(0x0,0x3621030);
		writel(0x0,0x362119c);
		writel(0x0,0x36211a0);
		writel(0x0,0x36211a4);
		writel(0x0,0x36211a8);
		writel(0x0,0x36211ac);
		writel(0x0,0x36211b0);
		writel(0x0,0x3620180);
		writel(0x00000001,0x3621010);
		writel(0x0,0x3621010);
		writel(0x003FFFFF,0x3621010);
		writel(0x00000001,0x3621014);
		writel(0x0,0x3620014);
		writel(0x00003FFF,0x3621014);
		writel(0x00000001,0x3621018);
		writel(0x0,0x3620018);
		writel(0x00FFFFFF,0x3621018);
		*/

		//writel(0x00000000, 0x36008bc);
	//Modification ends murtuza
	//writel(1, reg_vir + DBI_REG);
	writel(0x80000000 + bar_no, reg_vir + INDEX_REG);

	//below for FPGA
	writel(bar_low, reg_vir + TARGET_ADD_LOW);
	writel(bar_high, reg_vir + TARGET_ADD_HIGH);
	
	//use below for RAM mapping - 
	//writel(0x80000000, reg_vir + TARGET_ADD_LOW);
	//writel(0, reg_vir + TARGET_ADD_HIGH);
	

	writel(0, reg_vir + CONTROL1_REG);
	writel(control2, reg_vir + CONTROL2_REG);
	//writel(0, reg_vir + DBI_REG);

	//writel(1, reg_vir + DBI_REG);
	writel(0x80000001, reg_vir + INDEX_REG);
	writel(0, reg_vir + CONTROL2_REG);

	writel(0x80000003, reg_vir + INDEX_REG);
	writel(0, reg_vir + CONTROL2_REG);

	writel(0, reg_vir + DBI_REG);
	iounmap(reg_vir);
	return 0;
}

//Mur

//static int outbound_config(void)
//{
//	void __iomem *reg_vir;
//	reg_vir = ioremap_nocache(PCIE_REGS_START, PCIE_REGS_SIZE);
//	if (!reg_vir) {
//		printk(KERN_ERR "Register remapping failed\n");
//		return -1;
//	}
//
//	writel(1, reg_vir + DBI_REG);
//	writel(0x00000002, reg_vir + INDEX_REG);
//	writel(0xffffffff, reg_vir + ADD_RANGE_REG);
//	writel(0x00000000, reg_vir + TARGET_ADD_LOW);
//	writel(0x00000000, reg_vir + TARGET_ADD_HIGH);
//	writel(0x00000000, reg_vir + BASE_ADD_LOW);
//	writel(0x14, reg_vir + BASE_ADD_HIGH);
//	writel(0, reg_vir + CONTROL1_REG);
//	writel(0x80000000, reg_vir + CONTROL2_REG);
//	writel(0, reg_vir + DBI_REG);
//
//	iounmap(reg_vir);
//	return 0;
//}


static int outbound_config(void)
{
	void __iomem *reg_vir;
	uint32_t i;
	reg_vir = ioremap_nocache(PCIE_REGS_START, PCIE_REGS_SIZE);
	if (!reg_vir) {
		printk(KERN_ERR "Register remapping failed\n");
		return -1;
	}
	for(i = 0;i < 8; i++) {
		writel(1, reg_vir + DBI_REG);
		writel(0x00000002 + i, reg_vir + INDEX_REG); //ML - WHAT?
		writel(0xffffffff, reg_vir + ADD_RANGE_REG);
		writel(0x00000000, reg_vir + TARGET_ADD_LOW);
		//writel(0x00000000, reg_vir + TARGET_ADD_HIGH);
		writel(0x0 + i, reg_vir + TARGET_ADD_HIGH);
		writel(0x00000000, reg_vir + BASE_ADD_LOW);
		writel(0x30 + i, reg_vir + BASE_ADD_HIGH);
		writel(0, reg_vir + CONTROL1_REG);
		writel(0x80000000, reg_vir + CONTROL2_REG);
		writel(0, reg_vir + DBI_REG);
	}
	iounmap(reg_vir);
	return 0;
}

static int end_point_configuration(uint64_t bar0)
{
	int ret = 0;
	printk("Debug MUR BAR ep_config 0x%x\n", (uint64_t)bar0);
	

		

	if ((ret = inbound_config(BAR0, bar0)) < 0) {
		return ret;
	}
	if ((ret = outbound_config()) < 0) {
		return ret;
	}
	return ret;
} 

static inline struct uio_pci_generic_dev *to_uio_pci_generic_dev(struct uio_info *info)
{
	return container_of(info, struct uio_pci_generic_dev, info);
}

/* Interrupt handler. Read/modify/write the command register to disable
 * the interrupt. */
static irqreturn_t irqhandler(int irq, struct uio_info *info)
{
		struct uio_pci_generic_dev *gdev = to_uio_pci_generic_dev(info);

#ifdef UIO_MSI
		if (!gdev->have_msi && !pci_check_and_mask_intx(gdev->pdev))
#else
			if (!pci_check_and_mask_intx(gdev->pdev))
#endif    
				return IRQ_NONE;

		/* UIO core will signal the user process. */
		return IRQ_HANDLED;
}

static int probe(struct pci_dev *pdev,
			 const struct pci_device_id *id)
{
		struct uio_pci_generic_dev *gdev;
		int err;
		int i;
		unsigned char bar_num=0;
		int val;
		struct uio_info *uioinfo = NULL;
		char *name = "uio_pdrv_genirq";
		void __iomem *reg_vir;
#ifdef UIO_MSI
		bool have_msi = true;
#endif


		if (!pci_num) {
			reg_vir = ioremap_nocache(LUT_REG_START, 4096);
			printk("Debug Mur - Chk1\n");
			if (!reg_vir) {
				printk("Register remapping failed\n");
				return 0;
			}
			writel(0, reg_vir + LUT_CTRL_REG);
			iounmap(reg_vir);
		}
		pci_num++;	//pdev should be pointing to one of the FPGA links
		printk("UIO_PCI_GENERIC Device 0x%x has been found at"
		       " bus %d dev %d func %d pci_num %d\n",
		       pdev->device, pdev->bus->number, PCI_SLOT(pdev->devfn),
		       PCI_FUNC(pdev->devfn), pci_num);

		err = pci_enable_device(pdev);
		if (err) {
			dev_err(&pdev->dev, "%s: pci_enable_device failed: %d\n",
				__func__, err);
			printk("%s %d err : %d\n",__func__,__LINE__,err);
			return err;
		}	/* Enable Message Signaled Interrupt */
		/* set master */
		pci_set_master(pdev); //ML - device can also become bus master

		gdev = kzalloc(sizeof(struct uio_pci_generic_dev), GFP_KERNEL); //ML - Allocate space in kernel for driver structure
		if (!gdev) {
			err = -ENOMEM;
			printk("%s %d err : %d\n",__func__,__LINE__,err);
			goto err_alloc;
		}
		// ML - puts all the required info into the Kernel space for driver to use - also puts resource info.
		for (i = 0; i < PCI_NUM_RESOURCES; i++) {
			//ML - for IO BAR's
			if (pdev->resource[i].flags &&
			    (pdev->resource[i].flags & IORESOURCE_IO)) {
				gdev->info.port[i].size = 0;
				gdev->info.port[i].porttype = UIO_PORT_OTHER;
#ifdef CONFIG_X86
				gdev->info.port[i].porttype = UIO_PORT_X86;
#endif
			}
			//ML - for memory BAR's
			if (pdev->resource[i].flags &&
			    (pdev->resource[i].flags & IORESOURCE_MEM)) {
				gdev->info.mem[i].addr = pci_resource_start(pdev, i);
				printk("UIO_PCI_GENERIC pcie: BAR%d --> 0x%x\n", i, (unsigned int)gdev->info.mem[i].addr);
				gdev->info.mem[i].size = pci_resource_len(pdev, i);
				gdev->info.mem[i].internal_addr = NULL;
				gdev->info.mem[i].memtype = UIO_MEM_PHYS;
			}
		}

#ifdef UIO_MSI
		printk("Debug MUR - UIO_MSI present \n");
		pci_disable_msi(pdev);
#ifdef UIO_MULTIPLE_MSI
		printk("Debug MUR - UIO_MULTIPLE_MSI present \n");
		no_interrupts = 8;
#endif
		err = pci_enable_msi_range(pdev, 1, no_interrupts);
		if (err) {
			have_msi = true;
			printk("MSI UIO is enabled %d\n", err);
		}else{
			printk("MSI UIO failed to enable err: %d\n", err);
			if (!pci_intx_mask_supported(pdev)) {
#else
			if (!pci_intx_mask_supported(pdev)) {
#endif    
					err = -ENODEV;
					printk("%s %d !msi err : %d\n",__func__,__LINE__,err);
					goto err_verify;
			}
#ifdef UIO_MSI
		}
#endif

		printk("UIO_PCI_GENERIC IRQ assigned to device: %d \n",pdev->irq);

#ifdef UIO_MSI
		gdev->have_msi = have_msi;
#endif    
		gdev->info.name = "uio_pci_generic";
		gdev->info.version = DRIVER_VERSION;
		gdev->info.irq = pdev->irq;
#ifdef UIO_MSI
		gdev->info.irq_flags = have_msi ? 0 : IRQF_SHARED;
#else
		gdev->info.irq_flags = IRQF_SHARED;
#endif    
		gdev->info.handler = irqhandler;
		gdev->pdev = pdev;
		/* request regions */
		err = pci_request_regions(pdev, "uio_pci_generic");
		if (err) {
			dev_err(&pdev->dev, "Couldn't get PCI resources, aborting\n");
			printk("%s %d err : %d\n",__func__,__LINE__,err);
			return err;
		}

		err = uio_register_device(&pdev->dev, &gdev->info);
		if (err){
			printk("UIO PCI GENERIC driver registered is failed\n");
			goto err_register;
		}
		pci_set_drvdata(pdev, &gdev->info);
		pr_info("UIO_PCI_GENERIC : initialized new device (%x %x)\n",pdev->vendor, pdev->device);
		if(pci_num == 1) {	/*End point configuration for the PEX3 after getting PEX2 details*/
			if ((err = end_point_configuration(gdev->info.mem[2].addr)) < 0) {
				printk("END POINT CONFIGURATION is failed\n");
			}
		}

#ifdef UIO_MULTIPLE_MSI
		if (pci_num == 1) {
			struct uio_info uioinfoarr[no_interrupts];
			for(i = 0; i < (no_interrupts - 1); i++) {
				val = i+4;	/*7+i ==> (num_of_interrupt_in_pex2 -1) + i*/
				pldev[val] = devm_kzalloc(&pdev->dev, sizeof(struct platform_device), GFP_KERNEL);
				pldev[val] = platform_device_alloc(name, val);
				if (pldev[val]) {
					uioinfo = &uioinfoarr[i];
					uioinfo = dev_get_platdata(&pldev[i]->dev);
					uioinfo = devm_kzalloc(&pdev->dev, sizeof(*uioinfo), GFP_KERNEL);
					uioinfo->name = "uio_platform";
					uioinfo->version = "0";
					uioinfo->irq = pdev->irq + 1 + i;
					pldev[val]->dev.platform_data = uioinfo;
					err = platform_device_add_resources(pldev[val], NULL, 0);
					if (err == 0){
						err = platform_device_add(pldev[val]);
						if(err)
							dev_err(&(pldev[val]->dev), "unable to register platform device\n");
					}
				} else {
						err = -ENOMEM;
				}
			}
		}
#endif
	return 0;
err_register:
	kfree(gdev);
err_alloc:
#ifdef UIO_MSI
	if (have_msi){
		pci_disable_msi(pdev);
	}
#endif
	err_verify:
	pci_disable_device(pdev);
	printk("%s %d err : %d\n",__func__,__LINE__,err);
	return err;
}

static void remove(struct pci_dev *pdev)
{
	int i, val;
	struct uio_pci_generic_dev *gdev = pci_get_drvdata(pdev);

#ifdef UIO_MULTIPLE_MSI
	if(pci_num){
		for(i = 0; i < (no_interrupts - 1); i++) {
			val = i+4;
			if (pldev[val]) {
				platform_device_del(pldev[val]);
				kfree(pldev[val]);
				}
		}
			kfree(pdev->dev.platform_data);
			pci_num=0;
	}
#endif
#ifdef UIO_MSI
	if (gdev->have_msi){
		//pci_disable_msi(pdev);
	}
#endif
	uio_unregister_device(&gdev->info);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
	kfree(gdev);

}

static struct pci_driver uio_pci_driver = {
	.name = "uio_pci_generic",
	.id_table = pci_drv_ids, /* only dynamic id's */
	.probe = probe,
	.remove = remove,
};

module_pci_driver(uio_pci_driver);
MODULE_VERSION(DRIVER_VERSION);
MODULE_DEVICE_TABLE(uio,pci_drv_ids);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
