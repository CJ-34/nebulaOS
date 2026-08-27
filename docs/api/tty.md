# Text terminal API

Header: `kernel/include/kernel/tty.h`

`terminal_initialize()` clears the 80×25 VGA text terminal. Use
`terminal_putchar`, `terminal_write`, or `terminal_writestring` for output,
and `terminal_setcolor` to choose the VGA color attribute.

`terminal_backspace()` erases one character when the cursor is not in column
zero. Output scrolls at the bottom row by moving rows 1–24 upward and clearing
row 24. There is no cursor-positioning or terminal escape-sequence API yet.
