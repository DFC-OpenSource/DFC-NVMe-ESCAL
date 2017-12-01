Driver for DFC card.


Currently, we have not modified the u-boot drivers for proper configuration of the PCIe RC. This is achieved by the UIOGEN2 driver module(that is a modification of uio_pci_generic). To create the driver module file -

    Copy the UIOGEN2 folder as it is in the <Yocto_Dir>/sources/meta-freescale/recipes-user directory.
    cd <YOCTO_DIR>
    source fsl-setup-env –m ls2088aissd
    bitbake uiogen2 (where uiogen2 is the bitbake recipe name)
    The module file uiogen2.ko should be created in <Yocto_Dir>/build_ls2088aissd/tmp/sysroots/ls2088aissd/lib/modules/4.1.8-rt8+g09ffce2/extra

It also contains RAM Mapping addresses that can be used if PCI BAR address space has to be mapped to the DFC RAM instead of the FPGA.
