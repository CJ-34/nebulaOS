#include <kernel/drivers/serial.h>

static bool g_serial_ready = false;

// temporary method - later implementation per arch
static inline void outb(uint16_t port, uint8_t value) {
  __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static int serial_received(void) {
  return inb(COM1 + 5) & 1;
}

static int serial_transmit_empty(void) {
  return inb(COM1 + 5) & 0x20;
}

static void serial_wait_tx(void) {
  while (!serial_transmit_empty()) {
    __asm__ volatile ("pause");
  }
}

void serial_init(void) {
  /* 
   * Disable interrupts 
   */
  outb(COM1 + 1, 0x00);

  /*
  * Enable DLAB so divisor registers are accessible
  */
  outb(COM1 + 3, 0x80);

  /*
  * Set divisor = 1 => 115200 baud
  * low byte at +0, high byte at +1 while DLAB=1
  */
  outb(COM1 + 0, 0x01);
  outb(COM1 + 1, 0x00);

  /*
  * 8 bits, no parity, one stop bit, and clear DLAB
  */
  outb(COM1 + 3, 0x03);

  /*
  * Enable FIFO, clear them, 14 byte threshold
  */
  outb(COM1 + 2, 0xC7);

  /*
  * IRQs disabled, RTS/DSR set
  */
  outb(COM1 + 4, 0x03);

  /*
  * Optional loopback self-test:
  * Set loopback mode, send test byte, read it back.
  */
  outb(COM1 + 4, 0x1E);
  outb(COM1 + 0, 0xAE);

  if (inb(COM1 + 0) != 0xAE) {
    g_serial_ready = false;
    return;
  }

  /*
  * Back to normal mode
  */
  outb(COM1 + 4, 0x0F);
  
  g_serial_ready = true;
}

bool serial_is_initialized(void) {
  return g_serial_ready;
}

void serial_write_char(char c) {
  if (!g_serial_ready) {
    return;
  }

  if (c == '\n') {
    serial_wait_tx();
    outb(COM1 + 0, '\r');
  }

  serial_wait_tx();
  outb(COM1 + 0, (uint8_t)c);
}

void serial_write_string(const char* s) {
  if (!g_serial_ready || !s) {
    return;
  }

  while(*s) {
    serial_write_char(*s++);
  }
}

void serial_write_buffer(const char* data, size_t len) {
  if (!g_serial_ready || !data) {
    return;
  }

  for (size_t i = 0; i < len; i++) {
    serial_write_char(data[i]);
  }
}
