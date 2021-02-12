#include <stdint.h>
#include <stdio.h>

#include "include/cpu.h"
#include "include/mem.h"
#include "include/instructions.h"
#include "utils/misc.h"

/**
 * Little-endian 8-bit microprocessor that expects addresses
 * to be store in memory least significant byte first
 * */
struct central_processing_unit {
    uint16_t pc;
    uint8_t sp;
    uint8_t ac;
    uint8_t x;
    uint8_t y;

    /*
     * Status Register:
     *
     * bit 0: Carry
     * bit 1: Zero
     * bit 2: Interrupt
     * bit 3: Decimal
     * bit 4: Break
     * bit 5: 0
     * bit 6: Overflow (V)
     * bit 7: Negative
     * */
    uint8_t sr;
} cpu;

uint32_t cycles = 0;
struct mem* mem_ptr = NULL;

/**
 * cpu_extract_sr: Extract one of the 7 flags from the status reg.
 *
 * @param the flag to be extracted
 * @return the bit of the wanted flag
 * */
uint8_t cpu_extract_sr(uint8_t flag) { return ((cpu.sr >> (flag % 8)) & 1); }

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
    cpu.pc = 0x200;
    cpu.sp = 0xFD;
    cpu.ac = 0;
    cpu.x = 0;
    cpu.y = 0;
    cpu.sr = 0x00;

    cycles = 8;
}

void cpu_set_reg(struct regs regs) {
    printf("(cpu_set_reg) pc: %d, sp: %d, ac: %d, x: %d, y: %d\n", regs.pc, regs.sp, regs.ac, regs.x, regs.y);
    if (regs.pc >= 0) cpu.pc = regs.pc;
    if (regs.sp >= 0) cpu.sp = regs.sp;
    if (regs.ac >= 0) {
        cpu.ac = regs.ac;
    };
    if (regs.x) cpu.x = regs.x;
    if (regs.y) cpu.y = regs.y;

    if (cpu.ac == 0) {
        cpu_mod_sr(1, 1);
    }

    if ((cpu.ac & 0x80) > 0) {
        cpu_mod_sr(7, 1);
    }
}

static uint8_t get_mem(uint32_t addr) {
    uint8_t parsed = 0;

    // no need to check >= 0x0000, it's unsigned
    if (addr <= 0x00FF) {
        return mem_ptr->zero_page[addr];
    } else if (addr >= 0x0100 && addr <= 0x01FF) {
        parsed = addr - 0x0100;
        return mem_ptr->stack[parsed];
    } else if (addr >= 0xFFFA && addr <= 0xFFFF) {
        parsed = addr - 0xFDFA;
        return mem_ptr->last_six[parsed];
    } else {
        parsed = addr - 0x0200;
        return mem_ptr->data[parsed];
    }
}

uint8_t cpu_fetch() {
    uint8_t data = get_mem(cpu.pc);
    cpu.pc++;

    return data;
}

void cpu_exec() {
    mem_ptr = mem_get_ptr();
    printf("(cpu_exec) cycles: %d, mem: %p\n", cycles, (void*)mem_ptr);

    uint8_t fetched = 0x00;
    do {
        // TODO: actual instructions don't get executed
        fetched = cpu_fetch(mem_ptr);
        printf("(cpu_exec) fetched: 0x%X\n", fetched);
        inst_exec(fetched, &cycles);
    } while (cycles != 0);

}
