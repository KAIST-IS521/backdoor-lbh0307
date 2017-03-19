// This is where you put your VM code.
// I am trying to follow the coding style of the original.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "minivm.h"

#define NUM_REGS   (256)
#define NUM_FUNCS  (256)
#define SIZE_RAM   (8092)

// Global variable that indicates if the process is running.
static bool is_running = true;

void usageExit() {
    // TODO: show usage
    exit(1);
}

void halt(struct VMContext* ctx, uint32_t instr) {
    is_running = false;
}

void load(struct VMContext* ctx, uint32_t instr) {
    Reg src, dst;
    dst = ctx->r[EXTRACT_B1(instr)];
    src = ctx->r[EXTRACT_B2(instr)];

    if (src.value >= ctx->memSize) {
        is_running = false;
        return;
    }

    dst.value = ctx->mem[src.value];
    ++ctx->pc;
}

void store(struct VMContext* ctx, uint32_t instr) {
    Reg src, dst;
    dst = ctx->r[EXTRACT_B1(instr)];
    src = ctx->r[EXTRACT_B2(instr)];

    if (dst.value >= ctx->memSize) {
        is_running = false;
        return;
    }

    ctx->mem[dst.value] = src.value;
    ++ctx->pc;
}

void move(struct VMContext* ctx, uint32_t instr) {
    Reg src, dst;
    dst = ctx->r[EXTRACT_B1(instr)];
    src = ctx->r[EXTRACT_B2(instr)];
    dst.value = src.value;
    ++ctx->pc;
}

void puti(struct VMContext* ctx, uint32_t instr) {
    ctx->r[EXTRACT_B1(instr)].value = EXTRACT_B2(instr);
    ++ctx->pc;
}

void add(struct VMContext* ctx, uint32_t instr) {
    Reg l, r, dst;
    dst = ctx->r[EXTRACT_B1(instr)];
      l = ctx->r[EXTRACT_B2(instr)];
      r = ctx->r[EXTRACT_B3(instr)];

    dst.value = l.value + r.value;
    ++ctx->pc;
}

void sub(struct VMContext* ctx, uint32_t instr) {
    Reg l, r, dst;
    dst = ctx->r[EXTRACT_B1(instr)];
      l = ctx->r[EXTRACT_B2(instr)];
      r = ctx->r[EXTRACT_B3(instr)];

    dst.value = l.value + r.value;
    ++ctx->pc;
}

void gt(struct VMContext* ctx, uint32_t instr) {
    Reg l, r, dst;
    dst = ctx->r[EXTRACT_B1(instr)];
      l = ctx->r[EXTRACT_B2(instr)];
      r = ctx->r[EXTRACT_B3(instr)];

    dst.value = l.value > r.value;
    ++ctx->pc;
}

void ge(struct VMContext* ctx, uint32_t instr) {
    Reg l, r, dst;
    dst = ctx->r[EXTRACT_B1(instr)];
      l = ctx->r[EXTRACT_B2(instr)];
      r = ctx->r[EXTRACT_B3(instr)];

    dst.value = l.value >= r.value;
    ++ctx->pc;
}

void eq(struct VMContext* ctx, uint32_t instr) {
    Reg l, r, dst;
    dst = ctx->r[EXTRACT_B1(instr)];
      l = ctx->r[EXTRACT_B2(instr)];
      r = ctx->r[EXTRACT_B3(instr)];

    dst.value = l.value == r.value;
    ++ctx->pc;
}

void ite(struct VMContext* ctx, uint32_t instr) {
}

void jmp(struct VMContext* ctx, uint32_t instr) {
}

void putstr(struct VMContext* ctx, uint32_t instr) {
}

void getstr(struct VMContext* ctx, uint32_t instr) {
}

void invalidInstError(struct VMContext* ctx, uint32_t instr) {
    perror("Invalid instruction");
    is_running = false;
}

void initFuncs(FunPtr *f, uint32_t cnt) {
    uint32_t i;
    for (i = 0; i < cnt; i++) {
        f[i] = invalidInstError;
    }

    f[0x00] = halt;
    f[0x10] = load;
    f[0x20] = store;
    f[0x30] = move;
    f[0x40] = puti;
    f[0x50] = add;
    f[0x60] = sub;
    f[0x70] = gt;
    f[0x80] = ge;
    f[0x90] = eq;
    f[0xa0] = ite;
    f[0xb0] = jmp;
    f[0xc0] = putstr;
    f[0xd0] = getstr;
}

void initRegs(Reg *r, uint32_t cnt) {
    uint32_t i;
    for (i = 0; i < cnt; i++) {
        r[i].value = 0;
    }
}

off_t readCode(const char* path, uint32_t** code) {
    off_t size;
    struct stat sb;
    int fd;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("fopen");
        exit(-1);
    }

    if (fstat (fd, &sb) == -1) {
        perror ("fstat");
        exit(-1);
    }

    *code = mmap (0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (*code == MAP_FAILED) {
        perror ("mmap");
        exit(-1);
    }

    if (close (fd) == -1) {
        perror ("close");
    }

    return sb.st_size;
}

int main(int argc, char** argv) {
    VMContext vm;
    Reg r[NUM_REGS];
    FunPtr f[NUM_FUNCS];
    uint8_t* mem;
    uint32_t* code;
    off_t codeSize;


    // There should be at least one argument.
    if (argc < 2) usageExit();

    // Initialize registers.
    initRegs(r, NUM_REGS);
    // Initialize interpretation functions.
    initFuncs(f, NUM_FUNCS);
    // Initialize memory.
    mem = (uint8_t *)calloc(1, SIZE_RAM);
    // readCode from bytecode file
    codeSize = readCode(argv[1], &code);
    // Initialize VM context.
    initVMContext(&vm, NUM_REGS, NUM_FUNCS, SIZE_RAM, codeSize,
            r, f, mem, code);

    while (is_running) {
        stepVMContext(&vm);
    }

    munmap(code, codeSize);

    // Zero indicates normal termination.
    return 0;
}
