#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


#include "cpu/cpu.h"
#include "mem/mem.h"
#include "peripherals/interface.h"
#include "peripherals/kinput.h"

uint8_t DEBUG = 0;

int main(int argc, char **argv) {

	mem_init();
    cpu_init();
    cpu_reset();

	if (argc > 1) {
	  load_bin(argv[1]);
	  printf("\n[Program loaded] -> \"%s\"\n", argv[1]);
	} else {
	  printf("[No program loaded] -> starting \"example.bin\"\n");
	  load_example();
	}

	sleep(2);
	
    WINDOW* win = newwin(WIN_ROWS, WIN_COLS, 0, 0);
    if ((win = initscr()) == NULL) {
        fprintf(stderr, "Error initialising ncurses.\n");
        exit(1);
    }

    curs_set(0);
    noecho();
    box(win, 0, 0);
    wrefresh(win);

    interface_display_header();
    wrefresh(win);

    while (1) {
        interface_display_cpu();
        interface_display_mem();
        wrefresh(win);

        kinput_listen();

        if (kinput_should_quit()) {
            break;
        }
    }

    delwin(win);
    endwin();

    mem_dump();
	
    return 0;
}
