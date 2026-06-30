# yocto-embedded-linux

### Embedded Linux | Yocto Project | BitBake | Custom Layers | Cross-Compilation

**A personal lab repository for practicing and documenting Yocto Project / Embedded Linux skills.**

This repository is a collection of custom Yocto layers and recipes I build while learning embedded Linux development. Each subfolder is a self-contained Yocto layer that explores a different concept: native C/C++ recipes, custom BitBake tasks, fetching from Git, image customization, systemd services, and more.

The goal of this repo is not to show one finished product, but to show **how I learn and practice** — a structured, growing record of real Yocto work that a recruiter or hiring manager can browse and understand quickly.

---

## Why This Repo Exists

When applying for embedded Linux roles, it's hard to prove hands-on Yocto experience without a paper trail. This repo is that paper trail:

- Every layer is a real, working BitBake layer — not a copy-pasted tutorial.
- Every layer has its own `README.md` explaining what it does and what it taught me.
- The repo grows over time as I learn new parts of the Yocto build system.

---

## Repository Structure

```
yocto-embedded-linux/
├── README.md                  # This file — index of all layers
├── meta-student/               # Practice layer: native recipes, custom BitBake tasks, Git fetching
├── meta-openmotion/          # A custom OpenEmbedded layer that cross-compiles the host C application for the ARM Cortex-A35 inside the STM32MP257F-DK and includes it in the Linux image.

└── meta-<next-layer>/          # Future layer
```

Each `meta-*` folder is a complete, independent Yocto layer with its own `conf/layer.conf`, recipes, and `README.md`.

---

## Layers Index

| Layer | Focus | Status | Link |
|---|---|---|---|
| `meta-student` | Native C/C++ recipes, custom BitBake tasks (`addtask`), Git-fetched recipes, custom Makefile integration | Done | [meta-student/README.md](./meta-student/README.md) |
| `meta-meta-openmotion` | cross-compiles the host C application for the ARM Cortex-A35 inside the STM32MP257F-DK and includes it in the Linux image | Done | ./meta-openmotion/|

This table is updated every time a new layer is added.

---

## What Each Layer Generally Covers

As this repo grows, layers will progressively cover more of the Yocto/BitBake feature set, including:

- Local source recipes (`file://`) and native vs target recipes
- Fetching and building upstream projects (`git://`, `SRCREV`)
- Custom BitBake tasks and task ordering (`addtask`, `before`/`after`)
- Recipe variables, overrides, and soft assignment (`?=`, `:append`, `:remove`)
- Image customization (`.bbappend`, image recipes, systemd services)
- Packaging and `FILES:${PN}` management
- Cross-compilation for ARM targets (tested on STM32MP2 hardware)

---

## Hardware Used for Testing

| Component | Description |
|---|---|
| Target Board | STM32MP257F-DK (Cortex-A35, industrial Linux board) |
| Build Host | Ubuntu 24.04 LTS |
| Distro | ST OpenSTLinux (Yocto Scarthgap-based) |

---

## How to Use This Repo

Each layer can be added independently to a Yocto build using `bitbake-layers add-layer`. See the individual layer's `README.md` for exact build and test instructions.

```bash
git clone https://github.com/Abdelrahman-Abdellatif/yocto-embedded-linux
bitbake-layers add-layer /path/to/yocto-embedded-linux/meta-student
bitbake <recipe-name>
```

---

## Author

**Abdelrahman**
Embedded Linux & Firmware Engineer — open to opportunities

[LinkedIn](https://www.linkedin.com/in/abdelrahman-abdellatif-93371a405/) · [GitHub](#)
