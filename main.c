#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#define VM_STACK_CAPACITY 1024

typedef enum {
    TRAP_OK=0,
    TRAP_STACK_OVERFLOW,
    TRAP_STACK_UNDERFLOW,
    TRAP_ILLEGAL_INSTRUCTION,
    TRAP_ILLEGAL_OPERAND_ZERO,
} Trap;

// Trap Textual Representation
const char *trap_as_cstr(Trap trap) {
    switch(trap) {
        case TRAP_OK:
            return "TRAP_OK";
        case TRAP_STACK_OVERFLOW:
            return "TRAP_STACK_OVERFLOW";
        case TRAP_STACK_UNDERFLOW:
            return "TRAP_STACK_UNDERFLOW";
        case TRAP_ILLEGAL_INSTRUCTION:
            return "TRAP_ILLEGAL_INSTRUCTION";
        default:
            assert(0 && "trap_as_cstr: Unreachable");
    }
}

typedef int64_t Word;

typedef struct {
    Word stack[VM_STACK_CAPACITY];
    size_t stack_size;
} VM;

typedef enum {
    INSTRUCTION_PUSH,
    INSTRUCTION_PLUS,
    INSTRUCTION_MINUS,
    INSTRUCTION_MULTIPLY,
    INSTRUCTION_DIVIDE,
} InstructionType;

typedef struct {
    InstructionType type;
    Word operand;
} Instruction; 

#define MAKE_INSTRUCTION_PUSH(value) {.type = INSTRUCTION_PUSH, .operand = (value)}
#define MAKE_INSTRUCTION_PLUS() {.type = INSTRUCTION_PLUS}
#define MAKE_INSTRUCTION_MINUS() {.type = INSTRUCTION_MINUS}
#define MAKE_INSTRUCTION_MULTIPLY() {.type = INSTRUCTION_MULTIPLY}
#define MAKE_INSTRUCTION_DIVIDE() {.type = INSTRUCTION_DIVIDE}

//static inline Instruction instruction_push(Word operand) {
//    return (Instruction) {
//        .type = INSTRUCTION_PUSH,
//        .operand = operand,
//    };
//}

//static inline Instruction instruction_plus(void) {
//    return (Instruction) {.type = INSTRUCTION_PLUS};
//}

Trap vm_execute_instruction(
    VM *vm, 
    Instruction instruction
) {
    switch(instruction.type) {

        case INSTRUCTION_PUSH:
            if (vm->stack_size >= VM_STACK_CAPACITY) return TRAP_STACK_OVERFLOW;
            vm->stack[vm->stack_size++] = instruction.operand;
            break;

        case INSTRUCTION_PLUS:
            if (vm->stack_size < 2) return TRAP_STACK_UNDERFLOW;
            vm->stack[vm->stack_size - 2] += vm->stack[vm->stack_size - 1];
            vm->stack_size -= 1;
            break;

        case INSTRUCTION_MINUS:
            if (vm->stack_size < 2) return TRAP_STACK_UNDERFLOW;
            vm->stack[vm->stack_size - 2] -= vm->stack[vm->stack_size - 1];
            vm->stack_size -= 1;
            break;
        
        case INSTRUCTION_MULTIPLY:
            if (vm->stack_size < 2) return TRAP_STACK_UNDERFLOW;
            vm->stack[vm->stack_size - 2] *= vm->stack[vm->stack_size - 1];
            vm->stack_size -= 1;
            break;

        case INSTRUCTION_DIVIDE:
            if (vm->stack_size < 2) return TRAP_STACK_UNDERFLOW;
            if (vm->stack[vm->stack_size - 2] == 0) return TRAP_ILLEGAL_OPERAND_ZERO;
            vm->stack[vm->stack_size - 2] /= vm->stack[vm->stack_size - 1];
            vm->stack_size -= 1;
            break;

        default:
            return TRAP_ILLEGAL_INSTRUCTION;
    }
    return TRAP_OK;
}

void vm_dump(FILE *stream, const VM *vm) {
    fprintf(stream, "Stack:\n");
    if (vm->stack_size > 0) {
        for (size_t i = 0; i < vm->stack_size; ++i) {
            fprintf(stream, "  %ld\n", vm->stack[i]);
        }
    } else {
        fprintf(stream, "  [Stack Empty]\n");
    }
}

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))

VM vm = {0};
Instruction program[]  = {
    MAKE_INSTRUCTION_PUSH(100),
    MAKE_INSTRUCTION_PUSH(50),
    MAKE_INSTRUCTION_DIVIDE(),
};

int main() {
   
    vm_dump(stdout, &vm);

    for (size_t i = 0; i < ARRAY_SIZE(program); ++i) {
        Trap trap = vm_execute_instruction(&vm, program[i]);
        if (trap != TRAP_OK) {
            fprintf(stderr, "Trap Activated: %s\n", trap_as_cstr(trap));
            vm_dump(stderr, &vm);
            exit(1);
        } 
    }
    vm_dump(stdout, &vm);
    return 0;
}
