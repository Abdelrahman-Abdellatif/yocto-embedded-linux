# meta-student

### Yocto Project | Embedded Linux Layer | BitBake | C/C++ | STM32MP2 Cross-Compilation

**A custom Yocto layer for practicing core BitBake/recipe mechanics, built and tested on the STM32MP257F-DK.**

This layer is part of my [yocto-embedded-linux](../README.md) practice repository. The goal of `meta-student` is not to ship one finished application — it's to deliberately write each recipe in a **different way**, so I'm forced to practice a different part of the BitBake/Yocto mechanics every time: local source vs. Git-fetched source, `native` recipes vs. target recipes, custom task injection with `addtask`, variable overriding, and wrapping an external Makefile through `oe_runmake`.

Each recipe below was written as its own small experiment, on purpose, to try something new rather than repeat the same pattern.

---

## What This Layer Covers

| Concept | Where it's practiced |
|---|---|
| Local source recipe (`file://`), basic `do_compile`/`do_install` | `hello-native` |
| `native` recipe class, soft-assigned variables (`?=`), custom `do_run` task | `student-score-native` |
| Git-fetched source (`git://`) combined with local files, `SRCREV` pinning | `iot-gateway` |
| Wrapping an external Makefile with `oe_runmake` | `iot-gateway` |
| Custom BitBake task injection with `addtask ... after ... before ...` | `student-score-native`, `iot-gateway` |
| Custom QA/validation task (`do_check`) | `iot-gateway` |
| Explicit package output control (`FILES:${PN}`) | `iot-gateway` |

---

## Layer Structure

```
meta-student/
├── conf/
│   └── layer.conf                  # Layer priority and recipe pattern config
├── COPYING.MIT                     # License
├── README.md                       # This file
├── recipes-example/
│   └── example/
│       └── example_0.1.bb          # Default Yocto template recipe
└── recipes-student/
    ├── hello-native/
    │   ├── files/
    │   │   └── main.cpp            # Simple native C++ app
    │   └── hello-native_1.0.bb     # Local recipe + custom task ordering practice
    ├── iot-gateway/
    │   ├── files/
    │   │   ├── main.cpp            # App entry point
    │   │   ├── Makefile            # Standalone build pipeline used via oe_runmake
    │   │   ├── sender.cpp          # Telemetry/network payload logic
    │   │   └── sender.hpp          # Headers
    │   └── iot-gateway_1.0.bb      # Git-fetched recipe + custom Makefile + do_check task
    └── student-score-native/
        ├── files/
        │   └── score.cpp           # Local file, variable scope practice
        └── student-score-native_1.0.bb
```

---

## Recipe Breakdown

### `hello-native` — the baseline pattern
The simplest recipe in the layer, written first to nail down the basic recipe lifecycle before adding any complexity. It compiles a single local `main.cpp` directly with `${CXX} ${CXXFLAGS} ${LDFLAGS}` inside a custom `do_compile()`, then installs the resulting binary into `${D}${bindir}` in `do_install()`. `S` is set to `${WORKDIR}` directly since there's no upstream source tree to unpack — just one local file.

This recipe exists to answer: *what's the minimum a recipe needs to compile and install something?*

### `student-score-native` — variables, the `native` class, and a custom task
Builds on `hello-native` but practices three new things at once:

- **`inherit native`** — the recipe is built to run *on the build host*, not the target, which changes how Yocto resolves the toolchain.
- **Soft-assigned variables** (`QUIZ_SCORE ?= "80"`, etc.) — practicing the difference between `?=` (only applies if unset) and `=`, and how a `.bbappend` or `local.conf` could override these later.
- **A fully custom task, `do_run`**, inserted into the task graph with:
  ```bitbake
  addtask run after do_compile before do_install
  ```
  `do_run` executes the freshly compiled binary with the score variables as arguments and redirects its output to a file (`score-result.txt`) — practicing that BitBake tasks aren't limited to fetch/compile/install, they can run arbitrary logic anywhere in the graph.

### `iot-gateway` — the most advanced experiment in the layer
This recipe combines almost everything practiced in the other two, plus new concepts:

- **Mixed `SRC_URI`**: fetches the third-party `cpp-httplib` library straight from GitHub (`git://`) *and* pulls in local application files (`file://main.cpp`, `sender.cpp`, `sender.hpp`, `Makefile`) in the same recipe — practicing how Yocto merges multiple fetch types into one `${WORKDIR}`.
- **`SRCREV`** pinned to a tag (`v0.18.1`) for a reproducible, version-locked build instead of always pulling the latest commit.
- **`S = "${WORKDIR}/git"`** — pointing the build directory at the cloned repo instead of `${WORKDIR}` directly, then manually copying the local files into it inside `do_compile()` before building.
- **`oe_runmake`** instead of calling the compiler directly — the actual compilation logic lives in a separate, standalone `Makefile`, and `oe_runmake` makes sure Yocto's cross-compilation variables (`CXX`, `CXXFLAGS`, `LDFLAGS`) flow into it correctly.
- **A custom QA task, `do_check`**, again inserted with `addtask ... after do_compile before do_install`, which fails the build (`bberror` + `exit 1`) if the binary wasn't produced or if an expected dependency string isn't found in the source — practicing that `addtask` can be used as an enforcement gate, not just an extra step.
- **`oe_runmake install DESTDIR=${D}`** in `do_install` — letting the Makefile itself own the install step instead of writing `install -m` commands by hand like the other two recipes.
- **`FILES:${PN}`** explicitly set to make sure the binary is packaged into the final image output.

This recipe was the deliberate "do it the hard/real way" exercise — closer to how a recipe for a real third-party dependency would actually look.

---

## Why Each Recipe Is Written Differently

| Recipe | Source type | Toolchain target | Task style | New thing practiced |
|---|---|---|---|---|
| `hello-native` | Local file | Target | Manual compile/install | Minimal recipe lifecycle |
| `student-score-native` | Local file | Host (`native`) | Custom `do_run` task | `inherit native`, soft variables, output redirection |
| `iot-gateway` | Git + local files | Target | Makefile via `oe_runmake` + QA gate | Mixed fetch, `SRCREV`, external Makefile, `addtask` as a quality gate, `FILES:${PN}` |

The point of laying it out this way is to show progression: each recipe intentionally avoids reusing the previous one's pattern.

---

## Target Hardware

| Component | Specification |
|---|---|
| Target board | STM32MP257F-DK |
| Architecture | 64-bit ARM Cortex-A35 |
| Distro | ST OpenSTLinux (Scarthgap) |
| Cross-compiler | `aarch64-ostl-linux-g++` |

---

## How to Build and Run It Yourself

### 1. Set up the ST OpenSTLinux Yocto environment

Follow ST's official setup guide to get the OpenSTLinux Yocto distribution for the STM32MP257F-DK, then source the environment:

```bash
source layers/meta-st/scripts/envsetup.sh
```

Select the `stm32mp25-disco` build configuration from the menu; it sets up and switches into the build directory automatically.

### 2. Add this layer to your build

```bash
bitbake-layers add-layer /path/to/meta-student
```

### 3. Build a recipe

```bash
bitbake iot-gateway
```

### 4. Run the custom QA task manually (optional)

```bash
bitbake -c check -f iot-gateway
```

### 5. Find the cross-compiled binary

```bash
cd tmp-glibc/work/cortexa35-ostl-linux/iot-gateway/1.0/git/
file iot-gateway
```

Expected output:
```
iot-gateway: ELF 64-bit LSB executable, ARM aarch64, version 1 (SYSV)...
```

### 6. Deploy to the board

```bash
scp iot-gateway root@<BOARD_IP>:/usr/bin/
ssh root@<BOARD_IP>
chmod +x /usr/bin/iot-gateway
iot-gateway
```

---

## What I Learned

- How BitBake task graphs work and how to insert custom tasks with `addtask`
- The difference between local (`file://`) and Git-fetched (`git://`) recipes, and `SRCREV` pinning
- How `oe_runmake` passes Yocto's cross-compilation environment into a normal Makefile
- Variable scope and override mechanics (`?=`, `bitbake -e`)
- Writing a basic automated QA/validation step as part of a recipe build
- Cross-compiling and deploying real ARM binaries to the STM32MP257F-DK

---

## Author

**Abdelrahman**
Embedded Linux & Firmware Engineer — open to opportunities

[LinkedIn](https://www.linkedin.com/in/abdelrahman-abdellatif-93371a405/) · [GitHub](#)
