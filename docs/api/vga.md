# VGA helper API

Header: `kernel/include/kernel/vga.h`

`enum vga_color` defines the standard 16 VGA colors. `vga_entry_color(fg, bg)`
combines foreground and background into one attribute byte.
`vga_entry(character, color)` creates the 16-bit text-mode cell consumed by
the terminal's `0xB8000` VGA buffer.

These helpers do not perform I/O or update cursor state; use the text terminal
API for normal output.
