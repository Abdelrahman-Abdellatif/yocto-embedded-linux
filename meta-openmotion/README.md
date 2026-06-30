## Yocto Layer: `meta-openmotion`

### this layer is a part if the openmotion project, if you would like to know more please check: https://github.com/Abdelrahman-Abdellatif/OpenMotion
A custom OpenEmbedded layer that cross-compiles the host C application for the ARM Cortex-A35 inside the STM32MP257F-DK and includes it in the Linux image.

**Layer structure:**
```
meta-openmotion/
├── conf/layer.conf
├── recipes-openmotion/openmotion-host/
│   ├── openmotion-host_1.0.bb      # BitBake recipe
│   └── files/
│       ├── main.c
│       └── Makefile
└── recipes-st/
    └── images
	|___ st-image-weston.bbappend
```

---
