# Install systemd as init manager
DISTRO_FEATURES:append = " systemd usrmerge"
# Select systemd as init manager
VIRTUAL-RUNTIME_init_manager = "systemd"

# Prevent sysvinit from being silently backfilled into DISTRO_FEATURES
DISTRO_FEATURES_BACKFILL_CONSIDERED += "sysvinit"