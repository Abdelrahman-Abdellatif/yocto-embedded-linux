# Appended recipe for linux-stm32mp in meta-bootlinlabs
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://defconfig \
    file://0001-Add-nunchuk-driver.patch \
"

KERNEL_DEFCONFIG = ""
KERNEL_EXTERNAL_DEFCONFIG = "defconfig"