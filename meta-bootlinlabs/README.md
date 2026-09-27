# meta-bootlinlabs

**Yocto Project / OpenEmbedded custom layer — Bootlin "Yocto Project and OpenEmbedded system development" training, self-studied and adapted to the STM32MP257F-DK (Cortex-A35).**

> Bootlin's official labs target the STM32MP157 Discovery Kit (Cortex-A7). I went through every lab, rebuilt each step, and re-validated it on my own board, the **STM32MP257F-DK** (STM32MP2 series, Cortex-A35 + Cortex-M33 coprocessor). This repo is the resulting layer plus the full record of what each part does and why.

I did not take Bootlin's paid certification for this course — this repository is my own proof of work: a real, buildable, boot-tested layer, not a copy of slides.

---

## Hardware

| | |
|---|---|
| **Target board** | STM32MP257F-DK (Cortex-A35, dual-core, industrial Linux board) |
| **Original lab target** | STM32MP157 Discovery Kit (Cortex-A7) — *not what I used* |
| **Build host** | Ubuntu, x86_64 |
| **Distro** | ST OpenSTLinux (Yocto, `wrynose`/`scarthgap`-based branches) |
| **Base layers** | `poky`, `meta-openembedded`, `meta-st-stm32mp` |

Because the STM32MP2 series is architecturally different from the STM32MP1 (different core, different `MACHINE`, different DT files, different tuning), following the STM32MP1-only training material and getting it to actually boot on STM32MP257F-DK required real debugging, not just copy-pasting commands — see [Adapting to STM32MP2](#adapting-the-labs-to-stm32mp257f-dk) below.

---

## What this layer contains

```
meta-bootlinlabs/
├── conf/
│   ├── layer.conf
│   ├── machine/
│   │   └── my-bootlin-board.conf          # custom BSP machine (Lab 6)
│   └── templates/bootlinlabs/conf-notes.txt
├── recipes-core/
│   ├── ninvaders/                          # custom application recipe (Lab 3/4)
│   │   ├── files/joystick-support.patch    # Nunchuk joystick support (Lab 5)
│   │   ├── ninvaders_0.1.1.bb
│   │   └── ninvaders-0.1.1.tar.gz
│   └── packagegroups/
│       └── packagegroup-bootlinlabs-games.bb   # custom package group (Lab 7)
├── recipes-example/
│   └── example/example_0.1.bb              # bitbake-layers create-layer template recipe
├── recipes-image/
│   └── images/
│       ├── bootlinlabs-image-minimal.bb        # production image (Lab 7)
│       └── bootlinlabs-image-minimal-dbg.bb    # debug variant, shares the base recipe
├── recipes-kernel/
│   └── linux/
│       ├── files/
│       │   ├── 0001-Add-nunchuk-driver.patch  # kernel patch: Nunchuk as joystick (Lab 5)
│       │   └── defconfig
│       └── linux-stm32mp_%.bbappend        # extends the ST kernel recipe
└── recipes-utils/
    └── hello/
        ├── hello/0001-Change-the-greeting-message.patch
        └── hello_2.10.bb                    # devtool-managed recipe (Lab 9)
```

Every recipe here is one I wrote or extended myself while working through the labs — none of it is copy-pasted from the training slides verbatim.

---

## Skills covered, lab by lab

| Lab | Topic | What I built / did |
|---|---|---|
| 1 | First Yocto build | Set up `bitbake` + `openembedded-core` + `meta-yocto` + `meta-openembedded` + `meta-st-stm32mp`, configured `MACHINE`/`DISTRO`, built `core-image-minimal`, flashed an SD card, got serial console + boot working on real hardware |
| 2 | Advanced configuration | NFS-mounted rootfs over Ethernet, `IMAGE_INSTALL` customization (added Dropbear SSH), `PREFERRED_PROVIDER` / `virtual/kernel` resolution, BitBake task inspection (`listtasks`, `-c`, `-f`, `-s`) |
| 3 | Custom application recipe | Wrote a from-scratch recipe for **nInvaders** (terminal space-invaders game, `ncurses`-based): fetch, checksum, license, cross-compilation flags (`-std=gnu17`, `-fcommon` to work around old-codebase issues with modern GCC), fixed a `void(void)` vs `void(int)` conflicting-types build error |
| 4 | Custom layer | Created `meta-bootlinlabs` with `bitbake-layers create-layer`, registered it in `bblayers.conf`, migrated the nInvaders recipe out of `meta-*` core layers and into my own layer, the *correct* way per Yocto conventions |
| 5 | Extending a recipe | Wrote a `.bbappend` for `linux-stm32mp`, applied out-of-tree kernel patches via `do_patch` to add **Wii Nunchuk** support as a joystick input device (`/dev/input/js0`) over I2C, wired the Nunchuk to the board's I2C1 pins, patched **nInvaders** itself to accept joystick input, and played the game controlled by the Nunchuk |
| 6 | Custom machine | Wrote a from-scratch `MACHINE` configuration (`bootlinlabs`) reusing ST's `st-machine-common-stm32mp` / `st-machine-providers-stm32mp` includes, set `DEFAULTTUNE`, `MACHINEOVERRIDES`, `UBOOT_CONFIG`, `STM32MP_DT_FILES_SDCARD`, and `MACHINE_FEATURES` |
| 7 | Custom image | Wrote `bootlinlabs-image-minimal.bb` from scratch (inheriting `core-image`), explored and selected `packagegroup-*` bundles, created my own `packagegroup-bootlinlabs-games`, and derived a `-dbg` debug image variant without duplicating the whole recipe |
| 8 | Poky SDK | Built and installed the full (not toolchain-only) SDK, cross-compiled a third-party app (**ctris**) outside of Yocto's build system, fixed native-vs-cross-compiler Makefile issues, `-Werror=format-security` and `-fcommon` build errors, verified the ELF target with `file`, deployed over `scp` |
| 9 | `devtool` | Used `devtool add` / `build` / `deploy-target` / `edit-recipe` / `modify` / `update-recipe` / `upgrade` / `finish` to generate, iterate on, patch, and version-bump a recipe (GNU Hello 2.10 → 2.12.3) without hand-writing `SRC_URI` checksums or patch boilerplate |

---

## Adapting the labs to STM32MP257F-DK

The official Bootlin material is written for the **STM32MP157** (Cortex-A7, STM32MP1 family). I ran the same labs against my **STM32MP257F-DK** (Cortex-A35, STM32MP2 family), which meant working out several things not covered in the guide, including:

- Correct `MACHINE` selection and DT/board-variant naming for the STM32MP2 family instead of `stm32mp157a-dk1` / `stm32mp157d-dk1` I used `stm32mp257f-dk` for testing and later on I created the custom mahcine configuration and i used the `my-bootlin-board` 
- Adjusting the SD card flash layout / `create_sdcard_from_flashlayout.sh` invocation for the STM32MP2 image outputs
- Re-validating the kernel `.bbappend` and Nunchuk driver patch against the STM32MP2 kernel tree [I have not used the device tree file which bootlin provided becouse it does not suit my baord, i just uised the patch only for testing the method]
- also worth mentioning that if your device is lagging and stopping after starting to build the image it is better to reduce the CUP and memory used by bitbake, for exmaple these were the values i used to be able to build without any probelms, you should add it in the local.conf file inside your build directory. `BB_NUMBER_THREADS = "8"` `PARALLEL_MAKE = "-j 8"` 
---

### Network boot: NFS + TFTP
 
One of the most valuable parts of this training is setting up **NFS** and **TFTP**, and I'd strongly recommend anyone going through these labs not to skip it. Once it's working, you stop reflashing the SD card for every single change — you just rebuild the rootfs/kernel and reboot the board, which makes iteration dramatically faster.
 
- **NFS (Network File System)**: the board's rootfs is exported from the workstation and mounted by the target over the network at boot (`root=/dev/nfs`), instead of living on the SD card. Verified working on my setup:
```
  $ sudo ls /nfs/
  bin  boot  dev  etc  home  lib  media  mnt  proc  run  sbin  sys  tmp  usr  var
```
 
- **TFTP (Trivial File Transfer Protocol)**: the kernel image and device tree are served to U-Boot over the network at boot time, instead of being copied to the SD card's boot partition by hand. Verified working on my setup:
```
  $ sudo ls /srv/tftp/
  Image.gz  stm32mp257f-dk.dtb
```
 
With both in place, the full development loop becomes: rebuild with `bitbake`, reboot the board, done — no SD card handling at all. This is the setup I'd recommend to anyone doing serious iteration on real hardware rather than just following the labs once and moving on.

---


## Building it

```bash
# Base layers
git clone https://git.openembedded.org/bitbake -b yocto-6.0.2
git clone https://git.openembedded.org/openembedded-core -b yocto-6.0.2
git clone https://git.yoctoproject.org/meta-yocto -b yocto-6.0.2
git clone -b wrynose https://git.openembedded.org/meta-openembedded
git clone https://github.com/STMicroelectronics/meta-st-stm32mp

# This layer
git clone https://github.com/Abdelrahman-Abdellatif/yocto-embedded-linux/tree/main/meta-bootlinlabs
# meta-bootlinlabs lives at yocto-embedded-linux/meta-bootlinlabs

# Set up the build
source openembedded-core/oe-init-build-env
# edit conf/local.conf: MACHINE, DISTRO
# edit conf/bblayers.conf: add meta-yocto/meta-poky, meta-openembedded/meta-oe,
#                          meta-openembedded/meta-python, meta-st-stm32mp,
#                          and this meta-bootlinlabs layer

bitbake bootlinlabs-image-minimal
```

Output images land in `$BUILDDIR/tmp/deploy/images/<machine>/`.

---

## License

`meta-bootlinlabs` is released under the MIT license (see `COPYING.MIT`). Individual recipe/patch content follows the upstream license of the software it packages (see each recipe's `LICENSE` / `LIC_FILES_CHKSUM`).

---

## Author

**Abdelrahman Abdellatif** — Embedded Linux & Firmware Engineer

Part of my [yocto-embedded-linux](https://github.com/Abdelrahman-Abdellatif/yocto-embedded-linux) collection of Yocto Project practice layers.

[LinkedIn] · [GitHub](https://github.com/Abdelrahman-Abdellatif)