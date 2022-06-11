#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "cpu/cpu.h"
#include "mem/mem.h"
#include "peripherals/interface.h"
#include "peripherals/kinput.h"

#define AUTO_MODE		1
#define MANUAL_MODE		2

uint8_t DEBUG = 0;
// 6502 PROGRAMS EXECUTION MODES
// 1 -> automatic exec (no key listening) 
// (X or 2) -> default mode (manual) (need press ENTER to go to next instruction) (key listening)
uint8_t MODE = MANUAL_MODE; 
// 6502 DISPLAYS (coming soon)
// 	1 -> display (32x32) pixels
//uint8_t DISPLAY	= 1; 


int main(int argc, char **argv) {

	mem_init(argv[1]); // first program argument always will be the binary program
    cpu_init();
    cpu_reset();

	// program arguments settings
	for (int i = 1; i < argc; i++) {
	  if (strcmp(argv[i], "--auto-exec") == 0) {
		MODE = 1; // enable auto program exec
	  }
	}
	
    WINDOW* win = newwin(WIN_ROWS, WIN_COLS, 0, 0);
    if ((win = initscr()) == NULL) {
        fprintf(stderr, "Error initialising ncurses.\n");
        exit(1);
    }

	if (has_colors() == FALSE) {
	  endwin(); // close ncurses
	  fprintf(stderr, "Your terminal doesn't support colors...\n");
	  exit(EXIT_FAILURE);
	}

	start_color();
	init_pair(ZEROPAGE_PAIR, COLOR_WHITE, COLOR_BLUE);
	init_pair(HEADER_PAIR, COLOR_BLACK, COLOR_WHITE);
	init_pair(STACK_PAIR, COLOR_WHITE, COLOR_RED);
	init_pair(ROM_PAIR, COLOR_BLACK, COLOR_WHITE);
	init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
	init_pair(RED, COLOR_RED, COLOR_BLACK);

    curs_set(0);
    noecho();
    box(win, 0, 0);
    wrefresh(win);

	// program loop
    while (1) {
		// draw the app header
		attron(COLOR_PAIR(HEADER_PAIR));
		  FILL_ROW();
		  CENTER_TEXT(0, "6502 Emulator");
		attroff(COLOR_PAIR(HEADER_PAIR));
		
		// interface
		interface_display_cpu(3, 6);
		interface_show_status(60, 6);
		interface_show_zeropage(3, 8);
		interface_show_ROM(3, 28);
        interface_show_stack(60, 28);
		wrefresh(win);

		if (MODE == AUTO_MODE) {
		  // draw mode at top left
		  attron(COLOR_PAIR(RED));
			mvprintw(2, 3, "[EXEC MODE]: AUTO");
		  attroff(COLOR_PAIR(RED));
		  
		  if (cpu_extract_sr(I) & 1) {
			// show assembler program status
			attron(COLOR_PAIR(YELLOW));
			  mvprintw(2, 25, "[PROGRAM STATUS]: STOPPED");
			attroff(COLOR_PAIR(YELLOW));
			// show help commands
			interface_show_help(3, 4);
			kinput_listen();
		  } else { 
			// show assembler program status
			attron(COLOR_PAIR(GREEN));
			  mvprintw(2, 25, "[PROGRAM STATUS]: RUNNING");
			attroff(COLOR_PAIR(GREEN));
			cpu_exec();
		  }
		  
		  usleep(1000); // 10000 microseconds
		} else {
		  // draw mode at top left
		  attron(COLOR_PAIR(GREEN));
			mvprintw(2, 3, "[EXEC MODE]: DEFAULT (MANUAL/DEBUG)");
		  attroff(COLOR_PAIR(GREEN));
		  
		  interface_show_help(3, 4);
		  kinput_listen();
		}

		if (kinput_should_quit()) {
		  break;
		}
    }

    delwin(win);
    endwin();

    mem_dump();

    return 0;
}
