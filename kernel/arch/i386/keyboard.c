#include <kernel/keyboard.h>
#include <kernel/pic.h>

#include <stddef.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_BUFFER_SIZE 64

static volatile uint8_t scancode_buffer[KEYBOARD_BUFFER_SIZE];
static volatile uint32_t buffer_head;
static volatile uint32_t buffer_tail;

static bool left_shift;
static bool right_shift;
static bool extended_prefix;

static const char set1_keymap[128] = {
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',

    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',

    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',

    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',

    [0x1C] = '\n',
    [0x39] = ' ',
    [0x0C] = '-',
    [0x0D] = '=',
    [0x1A] = '[',
    [0x1B] = ']',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',
    [0x2B] = '\\',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
    [0x0E] = '\b',

};

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;

    __asm__ volatile(
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port));

    return value;
}

static char apply_shift(char character)
{
    if (character >= 'a' && character <= 'z')
    {
        return character - 'a' + 'A';
    }

    switch (character)
    {
    case '1':
        return '!';
    case '2':
        return '@';
    case '3':
        return '#';
    case '4':
        return '$';
    case '5':
        return '%';
    case '6':
        return '^';
    case '7':
        return '&';
    case '8':
        return '*';
    case '9':
        return '(';
    case '0':
        return ')';
    case '-':
        return '_';
    case '=':
        return '+';
    case '[':
        return '{';
    case ']':
        return '}';
    case ';':
        return ':';
    case '\'':
        return '"';
    case '`':
        return '~';
    case '\\':
        return '|';
    case ',':
        return '<';
    case '.':
        return '>';
    case '/':
        return '?';
    default:
        return character;
    }
}

static bool shift_is_held(void)
{
    return left_shift || right_shift;
}

void keyboard_init(void)
{
    buffer_head = 0;
    buffer_tail = 0;

    left_shift = false;
    right_shift = false;
    extended_prefix = false;
}

void keyboard_handle_irq(void)
{
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    uint32_t next_head = (buffer_head + 1) & (KEYBOARD_BUFFER_SIZE - 1);

    if (next_head != buffer_tail)
    {
        scancode_buffer[buffer_head] = scancode;
        buffer_head = next_head;
    }

    pic_send_eoi(1);
}

bool keyboard_pop_scancode(uint8_t *scancode)
{
    if (scancode == NULL || buffer_tail == buffer_head)
    {
        return false;
    }

    *scancode = scancode_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) & (KEYBOARD_BUFFER_SIZE - 1);

    return true;
}

bool keyboard_pop_char(char *character)
{
    uint8_t scancode;

    if (character == NULL)
    {
        return false;
    }

    while (keyboard_pop_scancode(&scancode))
    {
        if (scancode == 0xE0)
        {
            extended_prefix = true;
            continue;
        }

        if (extended_prefix)
        {
            extended_prefix = false;
            continue;
        }

        if (scancode == 0x2A)
        {
            left_shift = true;
            continue;
        }

        if (scancode == 0x36)
        {
            right_shift = true;
            continue;
        }

        if (scancode == 0xAA)
        {
            left_shift = false;
            continue;
        }

        if (scancode == 0xB6)
        {
            right_shift = false;
            continue;
        }

        if ((scancode & 0x80) != 0)
        {
            continue;
        }

        if (set1_keymap[scancode] != '\0')
        {
            *character = set1_keymap[scancode];

            if (shift_is_held())
            {
                *character = apply_shift(*character);
            }

            return true;
        }
    }

    return false;
}