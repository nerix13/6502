#include <stdint.h>

#ifndef INC_6502_INTERFACE_H
#define INC_6502_INTERFACE_H

#define WIN_ROWS 		35
#define WIN_COLS 		50

#define ROM_PAIR			1
#define ZEROPAGE_PAIR		2
#define HEADER_PAIR			3
#define AUTO_EXEC_PAIR		4
#define STACK_PAIR			5

// text colors
#define RED					10
#define GREEN				11
#define BLUE				12
#define YELLOW				13
#define MAGENTA				14

void CENTER_TEXT(int row, char *str);
void FILL_ROW(void);

void interface_display_cpu(uint8_t start_x, uint8_t start_y);
void interface_display_mem(void);
void interface_show_zeropage(uint8_t start_x, uint8_t start_y);
void interface_show_ROM(uint8_t start_x, uint8_t start_y);
void interface_show_stack(uint8_t start_x, uint8_t start_y);
void interface_show_help(uint8_t start_x, uint8_t start_y);
void interface_show_status(uint8_t start_x, uint8_t start_y);

#endif
