SUMMARY = "A custom image for bootlin labs."

IMAGE_INSTALL = "packagegroup-core-boot \
                 packagegroup-core-ssh-dropbear \
                 packagegroup-core-full-cmdline \
                 packagegroup-bootlinlabs-games \
                 "

inherit core-image

IMAGE_ROOTFS_EXTRA_SPACE = "100 * 1000"