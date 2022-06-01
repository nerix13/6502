#include "mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/misc.h"

#define ZERO_PAGE		0x0000 
#define ZERO_PAGE_END	0x00ff
#define SYS_STACK		0x0100 
#define SYS_STACK_END	0x01ff 
#define ROM 			0x8000

/**
 * The memory:
 *
 *  - RESERVED: 256 bytes 0x0000 to 0x00FF -> Zero Page
 *  - RESERVED: 256 bytes 0x0100 to 0x01FF -> System Stack
 *  - PROGRAM DATA: 0x10000 - 0x206
 *  - RESERVED: last 6 bytes of memory
 *
 *  pages are split into different arrays
 *
 * */
struct mem memory;


static uint8_t write_mem(uint16_t addr, int8_t data) {
    debug_print("(write_mem) writing: 0x%X at addr: 0x%X\n", data, addr);
    // NOTE: this yields "warning: comparison is always true due to limited range of
    // data type" if (!(addr >= 0x0000 && addr <= 0xFFFF)) return 1;

    if (addr <= ZERO_PAGE_END) {
        memory.zero_page[addr] = data;
    } else if (addr >= SYS_STACK && addr <= SYS_STACK_END) {
        memory.stack[addr - 0x0100] = data;
    } else if (addr >= 0xFFFA) {
        memory.last_six[addr - 0xFDFA] = data;
    } else {
        memory.data[addr - 0x0200] = data;
    }

    return 0;
}

/**
 * load_example: Loads hard coded example program to program memory
 *               the program multiplies 10 by 3 and it's not optimized
 * @param void
 * @return void
 * */
void load_example(void) {
    const char* instructions[] = {
        "A2", "0A", "8E", "00", "00", "A2", "03", "8E", "01", "00",
        "AC", "00", "00", "A9", "00", "18", "6D", "01", "00", "88",
        "D0", "FA", "8D", "02", "00", "EA", "EA", "EA",
    };

    uint16_t off = ROM; // 0x8000
    for (uint8_t i = 0; i < 28; i++) {
        write_mem(off++, strtoul(instructions[i], NULL, 16));
    }

    write_mem(0xFFFC, (uint8_t) 0x00);
    write_mem(0xFFFD, (uint8_t) 0x80);
}

/**
 * @description: Save the bin file in ROM
 * @param path -> bin program file
 * @return void
 */
void load_bin(char *path) {
  uint16_t addr;
  FILE *fp;
  int opc;

  fp = fopen(path, "rb");

  if (!fp) {
	fprintf(stderr, "[ load_bin(*path) :error] -> the program doesn't exists!\n");
	exit(EXIT_FAILURE);
  }

  addr = ROM; // 0x8000

  printf("[ Reading ROM FILE ] ...\n\n");

  while ((opc = fgetc(fp)) != EOF) {
	printf("\t%04x: %02x\n", addr, opc);
	write_mem(addr, opc);
	addr++;
  }

  // im not really sure about this
  write_mem(0xFFFC, 0x00);
  write_mem(0xFFFD, 0x80);
  
  fclose(fp);
}

/**
 * mem_init: Initialize the memory to its initial state
 *
 * @param void
 * @return void
 * */
void mem_init(void) {
    memset(memory.zero_page, 0, sizeof(memory.zero_page));
    memset(memory.stack, 0, sizeof(memory.stack));
    memset(memory.data, 0, sizeof(memory.data));

    // im not really sure about this
    memory.last_six[0] = 0xA;
    memory.last_six[1] = 0xB;
    memory.last_six[2] = 0xC;
    memory.last_six[3] = 0xD;
    memory.last_six[4] = 0xE;
    memory.last_six[5] = 0xF;

    //load_example();
}

/**
 * mem_get_ptr: returns pointer to currently active memory struct
 * */
struct mem* mem_get_ptr(void) {
    struct mem* mp = &memory;
    return mp;
}

/**
 * mem_dump: Dumps the memory to a file called dump.bin
 *
 * @param void
 * @return 0 if success, 1 if fail
 * */
int mem_dump(void) {
    // 100% there's a better way to do this

    FILE* fp = fopen("dump.bin", "wb+");
    if (fp == NULL) return 1;

    size_t wb = fwrite(memory.zero_page, 1, sizeof(memory.zero_page), fp);
    if (wb != sizeof(memory.zero_page)) {
        printf("[FAILED] Errors while dumping the zero page.\n");
        fclose(fp);
        return 1;
    }
    wb = fwrite(memory.stack, 1, sizeof(memory.stack), fp);

    if (wb != sizeof(memory.stack)) {
        printf("[FAILED] Errors while dumping the system stack.\n");
        fclose(fp);
        return 1;
    }

    wb = fwrite(memory.data, 1, sizeof(memory.data), fp);

    if (wb != sizeof(memory.data)) {
        printf("[FAILED] Errors while dumping the program data.\n");
        fclose(fp);
        return 1;
    }

    wb = fwrite(memory.last_six, 1, sizeof(memory.last_six), fp);

    if (wb != sizeof(memory.last_six)) {
        printf("[FAILED] Errors while dumping the last six reserved bytes.\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}
