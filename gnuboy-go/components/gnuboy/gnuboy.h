#pragma once

#include <rg_system.h>
#include <stdbool.h>
#include <stdint.h>

#define MESSAGE_ERROR(x, ...) printf("!! %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_INFO(x, ...) printf("%s: " x, __func__, ## __VA_ARGS__)
// #define MESSAGE_DEBUG(x, ...) printf("> %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_DEBUG(x...) {}

typedef uint8_t byte;
typedef uint8_t un8;
typedef uint16_t un16;
typedef uint32_t un32;
typedef int8_t n8;
typedef int16_t n16;
typedef int32_t n32;
typedef uint16_t word;
typedef unsigned int addr_t; // Most efficient but at least 16 bits

int  gnuboy_init(void);
int  gnuboy_load_bios(const char *file);
void gnuboy_free_bios(void);
int  gnuboy_load_rom(const char *file);
void gnuboy_free_rom(void);
void gnuboy_reset(bool hard);
void gnuboy_run(bool draw);
void gnuboy_die(const char *fmt, ...);

void gnuboy_get_time(int *day, int *hour, int *minute, int *second);
void gnuboy_set_time(int day, int hour, int minute, int second);

void gnuboy_load_bank(int);
