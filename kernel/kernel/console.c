#include <stdbool.h>
#include <stddef.h>

#include <kernel/console.h>
#include <kernel/tty.h>

#include <kernel/pmm.h>
#include <kernel/heap.h>
#include <kernel/pit.h>

#include <kernel/panic.h>

#define CONSOLE_LINE_CAPACITY 64

static char line_buffer[CONSOLE_LINE_CAPACITY];
static size_t line_length;

static void console_prompt(void)
{
    terminal_writestring("> ");
}

static bool line_equals(const char *text)
{
    size_t index = 0;

    while (text[index] != '\0')
    {
        if (index >= line_length || line_buffer[index] != text[index])
        {
            return false;
        }

        index++;
    }

    return index == line_length;
}

static void console_write_uint(uint32_t value) {
    char buffer[10];
    size_t length = 0;

    if (value == 0) {
        terminal_putchar('0');
        return;
    }

    while(value != 0) {
        buffer[length++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (length != 0) {
        terminal_putchar(buffer[--length]);
    }
}

static void console_submit_line(void)
{
    terminal_putchar('\n');

    if (line_length == 0)
    {
        console_prompt();
        return;
    }

    if (line_equals("help"))
    {
        terminal_writestring("commands: help clear mem heap ticks panic\n");
    }
    else if (line_equals("clear"))
    {
        terminal_initialize();
    }
    else if (line_equals("mem"))
    {
        terminal_writestring("PMM frames: total ");
        console_write_uint(pmm_total_frames());
        terminal_writestring(", free ");
        console_write_uint(pmm_free_frames());
        terminal_putchar('\n');
    }
    else if (line_equals("heap"))
    {
        terminal_writestring("Heap: high water ");
        console_write_uint(heap_used_bytes());
        terminal_writestring(" bytes, mapped ");
        console_write_uint(heap_mapped_pages());
        terminal_writestring(" pages\n");
    }
    else if (line_equals("heaptest"))
    {
        terminal_writestring("heaptest is not implemented yet\n");
    }
    else if (line_equals("ticks"))
    {
        terminal_writestring("PIT ticks: ");
        console_write_uint(pit_ticks());
        terminal_putchar('\n');
    }
    else if (line_equals("panic"))
    {
        PANIC("Panic requested from kernel console");
    }
    else
    {
        terminal_writestring("unknown command\n");
    }

    line_length = 0;
    console_prompt();
}

void console_init(void)
{
    line_length = 0;
    console_prompt();
}

void console_handle_char(char character)
{
    if (character == '\b')
    {
        if (line_length != 0)
        {
            line_length--;
            terminal_backspace();
        }

        return;
    }

    if (character == '\n')
    {
        console_submit_line();
        return;
    }

    if (character < ' ' || character > '~')
    {
        return;
    }

    if (line_length >= CONSOLE_LINE_CAPACITY - 1)
    {
        return;
    }

    line_buffer[line_length++] = character;
    terminal_putchar(character);
}
