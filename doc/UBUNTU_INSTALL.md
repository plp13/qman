# Reproducible compile instruction on Ubuntu 22.04 for qman

This guide provides step‑by‑step instructions to compile and install **qman** version 1.5.1 on a **Ubuntu 22.04 x86_64** system. All commands are intended to be run in a terminal and should produce a reproducible build.

## System version

- Ubuntu 22.04 (Jammy Jellyfish)
- Architecture: x86_64

## 1. Download the qman source code

```bash
curl -o qman-1.5.1.zip -Lk https://github.com/plp13/qman/archive/refs/tags/v1.5.1.zip
unzip qman-1.5.1.zip
cd qman-1.5.1
```

## 2. Install build dependencies

Install the required packages using `apt`. You may need to use `sudo` for these commands.

```bash
sudo apt update
sudo apt install cmake gcc pkg-config
sudo apt install libncurses5-dev libncursesw5-dev
sudo apt install zlib1g-dev libbz2-dev liblzma-dev
sudo apt install libcunit1-dev groff
sudo apt install libbsd-dev
```

Additionally, install `cogapp` using `pipx` (a Python tool for installing applications in isolated environments). If you don’t have `pipx` installed, you can install it first with `sudo apt install pipx`.

```bash
pipx install cogapp
```

> **Note:** Ensure `pipx` is in your PATH. If the `pipx` command is not found, add `~/.local/bin` to your PATH or log out and back in.

## 3. Build and install qman with Meson

Now configure the build directory, compile, and install.

```bash
# enable libbsd to ensure compatibility
meson setup build/ -Dlibbsd=enabled
cd build
meson compile
sudo meson install
```

After successful installation, the qman binaries will be placed in the appropriate system directories (e.g., `/usr/local/bin`).

## References

- [qman BUILDING.md](https://github.com/plp13/qman/blob/main/doc/BUILDING.md)
- [qman OS_SPECIFIC.md](https://github.com/plp13/qman/blob/main/doc/OS_SPECIFIC.md)
