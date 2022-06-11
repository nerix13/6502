#include "interface.h"

#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdint.h>

#include "../cpu/cpu.h"
#include "../mem/mem.h"

// style methods
void CENTER_TEXT(int row, char *str) {
  mvprintw(row, (COLS / 2) - strlen(str) + (strlen(str)/2), str);
}

void FILL_ROW(void) {
  for (unsigned int i=0; i < COLS; i++) mvprintw(0, i, " ");
}

void interface_show_help(uint8_t start_x, uint8_t start_y) {
    mvprintw(start_y, start_x, "Commands -> Enter: Execute new instruction, r: Resets the CPU, q: Quits");
}

/**
 * interface_display_cpu: prints CPU status to the screen using ncurses
 * @param void
 * @return void
 * */
void interface_display_cpu(uint8_t start_x, uint8_t start_y) {
    mvprintw(start_y, start_x, "[CPU STATUS] A: $%02X PC: $%04X SP: $%02X X: $%02X Y: $%02X",
             cpu.ac, cpu.pc, cpu.sp, cpu.x, cpu.y);
}

void interface_show_status(uint8_t start_x, uint8_t start_y) {
  uint8_t x = start_x;
  uint8_t y = start_y;

  mvprintw(y, x, " Status ");
  y += 1;
  mvprintw(y, x, "NV--DIZC");
  y += 1;
  mvprintw(y, x, "--------");
  y += 1;
  mvprintw(y, x, "%s", to_binary(cpu.sr));
}

/**
 * @description: Print the ROM memory in screen
 * Example:
 *
 * 		$8000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 		$8010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 		$8020: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 *		[...]
 * 		$80f0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 *		
 *		prints 16 addresses values per line...
 *
 * @param start_x Start position X to print it
 * @param start_y Start position Y to print it
 * */
void interface_show_ROM(uint8_t start_x, uint8_t start_y) {
  struct mem* mp = mem_get_ptr();

  uint16_t count_addr = ROM;
  
  uint8_t x = start_x, 
		  y = start_y, 
		  cell_pos = 0;

  mvprintw(y, x, "Read Only Memory (ROM):");
  
  y += 2;

  mvprintw(y, x, "$%04X:", count_addr);

  x += 7;

  for (uint16_t i = ROM - 0x0200; i < (ROM - 0x0200) + 0xff; i++) {
	
	// highlights the current instruction.
	(i + 0x200 == cpu.pc) ? attron(COLOR_PAIR(ROM_PAIR)) : attroff(COLOR_PAIR(ROM_PAIR));

	mvprintw(y, x, "%02X", mp->data[i]);

	if (cell_pos == 0xf) {
	  cell_pos = 0;					
	  count_addr += 0x10;		
	  y += 1;
	  x = start_x;
	  attroff(COLOR_PAIR(ROM_PAIR));
	  mvprintw(y, x, "$%04X:", count_addr);
	  x += 7;
	} else {
	  cell_pos++;
	  x += 3;
	}
  }
}

/**
 * @description: Print the Zero Page in screen
 * @param start_x Start position X to print it
 * @param start_y Start position Y to print it
 * */
void interface_show_zeropage(uint8_t start_x, uint8_t start_y) {
  struct mem* mp = mem_get_ptr();

  uint16_t count_addr = ZERO_PAGE;

  uint8_t x = start_x, 
		  y = start_y, 
		  cell_pos = 0;

  mvprintw(y, x, "Zero Page:");
  
  y += 2;

  mvprintw(y, x, "$%04X:", count_addr);

  x += 7;

  for (uint8_t i = 0; i < 0xff; i++) {
	mvprintw(y, x, "%02X", mp->zero_page[i]); // prints the memory location ($XX cell) 

	if (cell_pos == 0xf) {
	  cell_pos = 0;			// resets memory cell position counter 
	  count_addr += 0x10;	// Address in screen increment by 16, in hex (0x10)
	  y += 1;				// new line...
	  x = start_x;			// back to start x position
	  mvprintw(y, x, "$%04X:", count_addr);	// print address position again... ($0000)
	  x += 7;				// do some space
	} else {
	  cell_pos++;			
	  x += 3;
	}
  }
}

/**
 * @description: Print the System Stack in screen
 * @param start_x Start position X to print it
 * @param start_y Start position Y to print it
 * */
void interface_show_stack(uint8_t start_x, uint8_t start_y) {
  struct mem* mp = mem_get_ptr();

  uint16_t count_addr = SYS_STACK;

  uint8_t x = start_x, 
		  y = start_y, 
		  cell_pos = 0;

  mvprintw(y, x, "System Stack:");
  
  y += 2;

  mvprintw(y, x, "$%04X:", count_addr);

  x += 7;

  for (uint8_t i = 0; i < 0xff; i++) {

	mvprintw(y, x, "%02X", mp->stack[i]); // prints the memory location ($XX cell) 

	if (cell_pos == 0xf) {
	  cell_pos = 0;			// resets memory cell position counter 
	  count_addr += 0x10;	// Address in screen increment by 16, in hex (0x10)
	  y += 1;				// new line...
	  x = start_x;			// back to start x position
	  attroff(COLOR_PAIR(STACK_PAIR));
	  mvprintw(y, x, "$%04X:", count_addr);	// print address position again... ($0000)
	  x += 7;				// do some space
	} else {
	  cell_pos++;			
	  x += 3;
	}
  }
}
