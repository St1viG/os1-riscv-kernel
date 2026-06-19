# OS1 RISC-V — Docker Cheatsheet

Container = disposable toolchain. Code lives in `os1/` on the Mac and is mounted in.
Nothing in `os1/` is ever lost by exiting a container.

## One-time setup

```bash
# build the toolchain image (rerun only if you edit the Dockerfile)
cd ~/path/to/os1
docker build -t riscv-kernel .
```

## Daily loop

```bash
# 1. start Docker Desktop (whale icon in menu bar), then:
cd ~/path/to/os1
docker run -it --rm --name os1 -v "$(pwd)":/kernel riscv-kernel bash

# 2. inside the container shell:
make            # build
make qemu       # build + run in QEMU

# 3. quit QEMU console:   Ctrl-A then X
# 4. end session:         exit   (or Ctrl-D)
```

Edit `.c` / `.S` / Makefile in VS Code on the Mac — changes are live in the container instantly.

## Debugging (two shells)

```bash
# shell 1 (the running container): boot halted + open gdb port
make qemu-gdb

# shell 2 (new terminal tab): attach to the SAME container
docker exec -it os1 bash
gdb-multiarch kernel
# then in gdb:
target remote localhost:25000
```

## Housekeeping

```bash
docker build -t riscv-kernel .     # rebuild image (after Dockerfile change)
docker rm -f os1                   # clear a stuck container ("name in use" error)
docker ps                          # list running containers
docker images                      # list images
```

## .gitignore

```
build/
kernel
kernel.asm
.gdbinit
```

## What's preserved

- **Kept:** everything in `os1/` (source + build output), the `riscv-kernel` image.
- **Lost on exit:** anything done inside the container outside `/kernel`
  (manual `apt install`, files in `/root`, shell history).
  Need a tool permanently? Add it to the Dockerfile and rebuild.
