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

void initFuncs(FunPtr *f, uint32_t cnt) {
    uint32_t i;
    for (i = 0; i < cnt; i++) {
        f[i] = NULL;
    }

    // TODO: initialize function pointers
    // f[0x00] = halt;
    // f[0x10] = load;
}

void initRegs(Reg *r, uint32_t cnt) {
    uint32_t i;
    for (i = 0; i < cnt; i++) {
        r[i].type = 0;
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
