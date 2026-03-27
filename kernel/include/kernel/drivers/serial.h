/* 
 * File: serial.h
 * Module: Serial Port Driver
 * Layer: architecture specific / x86
 *
 * Purpose:
 *  provides basic COM1 serial initialization and byte/string output
 *  for kernel debugging
 *
 * Responsibilities:
 *  - Initialize COM1
 *  - Check transmit readness
 *  - Send characters and strings
*/
#ifndef _KERNEL_SERIAL_H
#define _KERNEL_SERIAL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define COM1 0x3F8

void serial_init(void);
bool serial_is_initialized(void);

void serial_write_char(char c);
void serial_write_string(const char* s);
void serial_write_buffer(const char* data, size_t len);

#endif

