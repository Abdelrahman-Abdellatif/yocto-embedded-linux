SUMMARY = "A custom debug image for bootlin labs, including debugging tools."

require bootlinlabs-image-minimal.bb

IMAGE_FEATURES += "dbg-pkgs tools-debug"

IMAGE_INSTALL:append = " packagegroup-core-tools-debug"