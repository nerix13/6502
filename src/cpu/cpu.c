#include <stdint.h>
#include <stdio.h>

#include "cpu.h"
#include "../mem/mem.h"
#include "instructions.h"
#include "../utils/misc.h"


/**
 * Little-endian 8-bit microprocessor that expects addresses
 * to be store in memory least significant byte first
 * */
struct central_processing_unit cpu;

// clock cycles, every fetch implies a clock cycle
uint32_t cycles = 0;

struct mem* mem_ptr = NULL;

/**
 * cpu_init: Initialize CPU by linking it to the memory
 * @param void
 * @return void
 */
void cpu_init(void) {
    mem_ptr = mem_get_ptr();
}

/**
 * cpu_extract_sr: Extract one of the 7 flags from the status reg.
 * @param flag The flag to be extracted
 * @return the bit of the wanted flag
 * */
uint8_t cpu_extract_sr(uint8_t flag) { return ((cpu.sr >> (flag % 8)) & 1); }

/**
 * cpu_mod_sr: Modify the sr register (flags)
 * @param flag The flag to set
 * @param val The value
 * @return 0 if success, 1 if failure
 */
uint8_t cpu_mod_sr(uint8_t flag, uint8_t val) {
    if (val != 0 && val != 1) return 1;

    if (flag > 0 && flag < 8 && flag != 5) {
        if (val == 1) {
            SET_BIT(cpu.sr, flag);
        } else {
            CLEAR_BIT(cpu.sr, flag);
        }
        return 0;
    } else {
        return 1;
    }
}

/**
 * cpu_reset: Reset the CPU to its initial state.
 *            There are different initial states for different
 *            machines, see like the reset process in the C64.
 *            At the moment I am setting everything to 0.
 *
 * @param void
 * @return void
 * */
void cpu_reset(void) {
    printf("(cpu_reset) called.\n");

    cpu.pc = 0x0200;
    cpu.sp = 0xFD;
    cpu.ac = 0x00;
    cpu.x = 0x00;
    cpu.y = 0x00;
    cpu.sr = 0x00;

    cycles = 8;
}

/**
 * get_mem: Wrapper to handle memory accessing, due to the pages being separated
 * @param addr The address we want to access
 * @return The retrieved data
 */
static int8_t get_mem(uint16_t addr) {
    // this yields "warning: comparison is always true due to limited range of data type"
    // if (!(addr >= 0x0000 && addr <= 0xFFFF)) return -1;

    uint8_t parsed;

    // no need to check >= 0x0000, it's unsigned
    if (addr <= 0x00FF) {
        return mem_ptr->zero_page[addr];
    } else if (addr >= 0x0100 && addr <= 0x01FF) {
        parsed = addr - 0x0100;
        return mem_ptr->stack[parsed];
    } else if (addr >= 0xFFFA) {
        parsed = addr - 0xFDFA;
        return mem_ptr->last_six[parsed];
    } else {
        parsed = addr - 0x0200;
        return mem_ptr->data[parsed];
    }
}

/**
 * write_mem: Write bytes to a given address
 * @param addr The location in memory where to write to
 * @param data The data to be written
 * @return 0 if success, 1 if failure
 */
static uint8_t write_mem(uint16_t addr, uint8_t data) {
    // this yields "warning: comparison is always true due to limited range of data type"
    // if (!(addr >= 0x0000 && addr <= 0xFFFF)) return 1;

    if (addr <= 0x00FF) {
        mem_ptr->zero_page[addr] = data;
    } else if (addr >= 0x0100 && addr <= 0x01FF) {
        mem_ptr->stack[addr - 0x0100] = data;
    } else if (addr >= 0xFFFA) {
        mem_ptr->last_six[addr - 0xFDFA] = data;
    } else {
        mem_ptr->data[addr - 0x0200] = data;
    }

    return 0;
}

/**
 * cpu_fetch: Fetch memory from a given address
 * @param void
 * @return void
 */
uint8_t cpu_fetch(uint16_t addr) {
    uint8_t data = get_mem(addr);
    if (addr == cpu.pc) cpu.pc++;

    return data;
}

/**
 * cpu_write: Wrapper for write_mem()
 * @param addr The address to be written to
 * @param data The data to be written
 * @return 0 if success, 1 if failure
 */
uint8_t cpu_write(uint16_t addr, uint8_t data) {
    return write_mem(addr, data) == 1 ? 1 : 0;
}

/**
 * cpu_exec: Execute fetched data (single stepping)
 * @param void
 * @return void
 */
void cpu_exec() {
    debug_print("(cpu_exec) cycles: %d, mem: %p\n", cycles, (void*)mem_ptr);

    int8_t fetched;
    do {
        debug_print("(loop) cycles: %d\n", cycles);
        // executing in a take
        if (cycles == 0) {
            fetched = cpu_fetch(cpu.pc);
            if (fetched == -1) goto error;

            printf("(cpu_exec) fetched: 0x%X\n", fetched);
            inst_exec(fetched, &cycles);
        }
        cycles--;
    } while (cycles != 0);

    error:
        printf("(FAILED) Couldn't fetch valid data!\n");
}
