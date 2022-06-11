#include "mem.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../utils/misc.h"

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


char *to_binary(int n) {
  /* from: https://www.programmingsimplified.com/c/source-code/c-program-convert-decimal-to-binary */
  int c, d, t;
  char *p;
  t = 0;
  p = (char*) malloc(8+1);
  
  if (p == NULL)
	exit(EXIT_FAILURE);

  for (c = 7; c >= 0 ; c--) {
	d = n >> c;

	if (d & 1)
	  *(p+t) = 1 + '0';
	else
	  *(p+t) = 0 + '0';
	t++;
  }

  *(p+t) = '\0';

  return  p;
}


static uint8_t write_mem(uint16_t addr, int8_t data) {
    debug_print("(write_mem) writing: 0x%X at addr: 0x%X\n", data, addr);
    // NOTE: this yields "warning: comparison is always true due to limited range of
    // data type" if (!(addr >= 0x0000 && addr <= 0xFFFF)) return 1;
  
    if (addr <= ZERO_PAGE + 0xff) {
        memory.zero_page[addr] = data;
    } else if (addr >= SYS_STACK && addr <= SYS_STACK + 0xff) {
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
static void load_example(void) {
    const char* instructions[] = {
        "A2", "0A", "8E", "00", "00", "A2", "03", "8E", "01", "00",
        "AC", "00", "00", "A9", "00", "18", "6D", "01", "00", "88",
        "D0", "FA", "8D", "02", "00", "EA", "EA", "EA",
    };

    uint16_t addr = ROM; // 0x8000
    for (uint8_t i = 0; i < 28; i++) {
        write_mem(addr++, strtoul(instructions[i], NULL, 16));
    }

    write_mem(0xFFFC, (uint8_t) 0x00);
    write_mem(0xFFFD, (uint8_t) 0x80);
}

/**
 * @description: Copy the content of bin file to 6502 ROM
 * @param path -> bin program file
 * @return void
 */
static void load_program(char *filename) {
  uint16_t addr;
  FILE *fp;
  int opc; // NOTE: changed to Integer type to verify if it's EOF while reading the bin file

  fp = fopen(filename, "rb");

  if (!fp) {
	fprintf(stderr, "[x] PROGRAM NOT FOUND -> the program doesn't exists!\n");
	exit(EXIT_FAILURE);
  }

  addr = ROM; // 0x8000

  while ((opc = fgetc(fp)) != EOF) write_mem(addr++, opc);

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
void mem_init(char *filename) {
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
	
	if (strlen(filename) > 0) {
	  load_program(filename);
	  printf("\n[-!-] Verifying program loaded... NAME: \"%s\"\n", filename);
	} else {
	  printf("[!] NO PROGRAM LOADED -> loading \"example.bin\"\n");
	  load_example();
	}
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
