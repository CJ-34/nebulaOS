# Console API

Header: `kernel/include/kernel/console.h`

`console_init()` resets the input line and writes the prompt. Call it after
initializing the VGA terminal.

`console_handle_char(character)` consumes one translated ASCII character. It
handles line editing and dispatches `help`, `clear`, `mem`, `heap`,
`heaptest`, `usertest`, `ticks`, and `panic`.

The console is driven by the main kernel loop, not directly from IRQ context.
