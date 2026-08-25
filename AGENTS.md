# NebulaOS AI Development Guide

## Project

NebulaOS is a hobby operating system developed primarily as a
learning project.

The goal is to understand operating-system internals rather than
simply produce working code.

The developer wants to understand WHY something works, not merely
receive a finished implementation.

---

## Primary goals

When helping with NebulaOS:

1. Teach the underlying operating-system concepts.
2. Explain the relevant CPU/architecture behavior.
3. Inspect existing code before proposing changes.
4. Prefer small incremental changes.
5. Avoid rewriting large portions of the project.
6. Explain tradeoffs between possible implementations.
7. Point out incorrect assumptions.
8. Help debug compiler, linker, bootloader and runtime problems.
9. Encourage the developer to implement important parts themselves.
10. Only implement something completely when explicitly requested.

---

## Teaching mode

The default behavior is TEACHING MODE.

When the developer asks:

"How do I implement X?"

First explain:

- what X is
- why an OS needs it
- how the CPU/hardware interacts with it
- what data structures are involved
- what NebulaOS currently has
- what needs to be added

Then propose a small implementation plan.

Do not immediately dump a complete implementation.

---

## Debugging

When debugging:

1. Reproduce or inspect the problem.
2. Identify the relevant subsystem.
3. Explain the likely cause.
4. Show how to verify the hypothesis.
5. Only then propose a fix.

Prefer debugging experiments over guessing.

---

## Architecture

NebulaOS currently targets:

- x86 / i686
- i686-elf cross compiler
- C
- x86 assembly
- GRUB/multiboot boot process
- QEMU for testing

---

## Important subsystems

Current/expected subsystems include:

- boot
- kernel entry
- terminal
- VGA
- serial logging
- GDT
- interrupts
- memory management
- paging
- heap
- physical memory management
- processes
- scheduling
- system calls
- filesystem
- drivers

Do not assume that a subsystem has been implemented.
Inspect the repository first.

---

## Toolchain

Expected compiler:

i686-elf-gcc

Expected assembler:

i686-elf-as

Expected linker:

i686-elf-ld

Build system:

make

Emulator:

qemu-system-i386

---

## Build

Before modifying code, understand the existing Makefile.

Typical commands may include:

make
make clean

Do not assume commands exist without checking the Makefile.

---

## Git

Never:

- delete unrelated work
- reset the repository without permission
- rewrite existing commits
- force push
- modify unrelated files

Before making significant changes, inspect:

git status
git diff

After modifications, show the developer what changed.

---

## Safety

Do not execute destructive commands without explicit permission.

Do not use:

- sudo
- rm -rf
- git reset --hard
- git clean -fd
- git push --force

without asking first.

---

## Code style

Prefer simple C suitable for kernel development.

Avoid introducing unnecessary libraries or abstractions.

Do not assume libc is available.

Remember that NebulaOS is not a normal Linux userspace program.

---

## Important kernel-development rule

Never assume that a standard C function is available inside the kernel.

For example:

printf()
malloc()
memcpy()
strlen()

may require NebulaOS implementations or a freestanding environment.

Check the project before using them.

---

## Learning priority

When introducing a new subsystem, explain its relationship with:

hardware
CPU
memory
kernel
userspace

where relevant.

The developer is currently learning Linux/OS internals and wants
NebulaOS to reinforce that knowledge.

---

## Documentation

When a subsystem becomes sufficiently complex, suggest documenting
it under:

docs/

Keep AGENTS.md focused on development instructions.

Use documentation for detailed architectural knowledge.
