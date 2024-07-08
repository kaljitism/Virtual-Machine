#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))
#define VM_STACK_CAPACITY 1024
#define VM_PROGRAM_CAPACITY 1024
#define VM_EXECUTION_LIMIT 69

typedef enum {
  TRAP_OK = 0,
  TRAP_STACK_OVERFLOW,
  TRAP_STACK_UNDERFLOW,
  TRAP_ILLEGAL_INSTRUCTION,
  TRAP_ILLEGAL_INSTRUCTION_ACCESS,
  TRAP_ILLEGAL_OPERAND_ZERO,
  TRAP_ILLEGAL_OPERAND
} Trap;

// Trap Textual Representation
const char *trap_as_cstr(Trap trap) {
  switch (trap) {
    case TRAP_OK:
      return "TRAP_OK";
    case TRAP_STACK_OVERFLOW:
      return "TRAP_STACK_OVERFLOW";
    case TRAP_STACK_UNDERFLOW:
      return "TRAP_STACK_UNDERFLOW";
    case TRAP_ILLEGAL_INSTRUCTION:
      return "TRAP_ILLEGAL_INSTRUCTION";
    case TRAP_ILLEGAL_INSTRUCTION_ACCESS:
      return "TRAP_ILLEGAL_INSTRUCTION_ACCESS";
    case TRAP_ILLEGAL_OPERAND_ZERO:
      return "TRAP_ILLEGAL_OPERAND_ZERO";
    case TRAP_ILLEGAL_OPERAND:
      return "TRAP_ILLEGAL_OPERAND";
    default:
      assert(0 && "trap_as_cstr: Unreachable");
  }
}

typedef enum {
  INSTRUCTION_PUSH,
  INSTRUCTION_DUPLICATE,
  INSTRUCTION_PLUS,
  INSTRUCTION_MINUS,
  INSTRUCTION_MULTIPLY,
  INSTRUCTION_DIVIDE,
  INSTRUCTION_JUMP,
  INSTRUCTION_JUMP_IF, // impl
  INSTRUCTION_EQUALITY, // impl
  INSTRUCTION_HALT,
  INSTRUCTION_PRINT_DEBUG, // impl
} InstructionType;

const char *instruction_type_as_cstr(InstructionType type) {
  switch (type) {
    case INSTRUCTION_PUSH:
      return "INSTRUCTION_PUSH";
    case INSTRUCTION_PLUS:
      return "INSTRUCTION_PLUS";
    case INSTRUCTION_MINUS:
      return "INSTRUCTION_MINUS";
    case INSTRUCTION_MULTIPLY:
      return "INSTRUCTION_MULTIPLY";
    case INSTRUCTION_DIVIDE:
      return "INSTRUCTION_DIVIDE";
    case INSTRUCTION_JUMP:
      return "INSTRUCTION_JUMP";
    case INSTRUCTION_JUMP_IF:
      return "INSTRUCTION_JUMP_IF";
    case INSTRUCTION_EQUALITY:
      return "INSTRUCTION_EQUALITY";
    case INSTRUCTION_HALT:
      return "INSTRUCTION_HALT";
    case  INSTRUCTION_PRINT_DEBUG:
      return " INSTRUCTION_PRINT_DEBUG";
    case INSTRUCTION_DUPLICATE:
      return "INSTRUCTION_DUPLICATE";
    default:
      assert(0 && "instruction_type_as_cstr: Unreachable");
  }
}

typedef int64_t Word;

typedef struct {
  InstructionType type;
  Word operand;
} Instruction;


typedef struct {
  Word stack[VM_STACK_CAPACITY];
  Word stackSize;
  
  Instruction program[VM_PROGRAM_CAPACITY];
  size_t programSize;
  Word instructionPointer;
  
  int halt;
} VM;

#define MAKE_INSTRUCTION_PUSH(value) {.type = INSTRUCTION_PUSH, .operand = (value)}
#define MAKE_INSTRUCTION_DUPLICATE(address) {.type = INSTRUCTION_DUPLICATE, \
.operand = (address)}
#define MAKE_INSTRUCTION_PLUS() {.type = INSTRUCTION_PLUS}
#define MAKE_INSTRUCTION_MINUS() {.type = INSTRUCTION_MINUS}
#define MAKE_INSTRUCTION_MULTIPLY() {.type = INSTRUCTION_MULTIPLY}
#define MAKE_INSTRUCTION_DIVIDE() {.type = INSTRUCTION_DIVIDE}
#define MAKE_INSTRUCTION_JUMP(address) {.type = INSTRUCTION_JUMP, .operand = \
(address)}
#define MAKE_INSTRUCTION_HALT() {.type = INSTRUCTION_HALT}

//static inline Instruction instruction_push(Word operand) {
//    return (Instruction) {
//        .type = INSTRUCTION_PUSH,
//        .operand = operand,
//    };
//}

//static inline Instruction instruction_plus(void) {
//    return (Instruction) {.type = INSTRUCTION_PLUS};
//}

Trap vm_execute_instruction(VM *vm) {
  if(vm->instructionPointer < 0 || vm->instructionPointer >= vm->programSize)
    return TRAP_ILLEGAL_INSTRUCTION_ACCESS;
  
  Instruction instruction = vm->program[vm->instructionPointer];
  
  switch (instruction.type) {
    
    case INSTRUCTION_PUSH:
      if (vm->stackSize >= VM_STACK_CAPACITY) return TRAP_STACK_OVERFLOW;
      
      vm->stack[vm->stackSize++] = instruction.operand;
      vm->instructionPointer += 1;
      break;
    
    case INSTRUCTION_PLUS:
      if (vm->stackSize < 2) return TRAP_STACK_UNDERFLOW;
      
      vm->stack[vm->stackSize - 2] += vm->stack[vm->stackSize - 1];
      vm->stackSize -= 1;
      vm->instructionPointer += 1;
      break;
    
    case INSTRUCTION_MINUS:
      if (vm->stackSize < 2) return TRAP_STACK_UNDERFLOW;
      vm->stack[vm->stackSize - 2] -= vm->stack[vm->stackSize - 1];
      vm->stackSize -= 1;
      vm->instructionPointer += 1;
      break;
    
    case INSTRUCTION_MULTIPLY:
      if (vm->stackSize < 2) return TRAP_STACK_UNDERFLOW;
      
      vm->stack[vm->stackSize - 2] *= vm->stack[vm->stackSize - 1];
      vm->stackSize -= 1;
      vm->instructionPointer += 1;
      break;
    
    case INSTRUCTION_DIVIDE:
      if (vm->stackSize < 2) return TRAP_STACK_UNDERFLOW;
      if (vm->stack[vm->stackSize - 2] == 0) return TRAP_ILLEGAL_OPERAND_ZERO;
      
      vm->stack[vm->stackSize - 2] /= vm->stack[vm->stackSize - 1];
      vm->stackSize -= 1;
      vm->instructionPointer += 1;
      break;
    
    case INSTRUCTION_JUMP:
      vm->instructionPointer = instruction.operand;
      break;
    
    case INSTRUCTION_JUMP_IF:
      if (vm->stackSize < 1) return TRAP_STACK_UNDERFLOW;
      
      if (vm->stack[vm->stackSize - 1] == 1) {
        vm->stackSize -= 1;
        vm->instructionPointer = instruction.operand;
      } else {
        vm->instructionPointer += 1;
      }
      break;
    
    case INSTRUCTION_HALT:
      vm->halt = 1;
      break;
      
    case INSTRUCTION_EQUALITY:
      if (vm->stackSize < 2) return TRAP_STACK_UNDERFLOW;
      
      vm->stack[vm->stackSize - 2] = vm->stack[vm->stackSize - 1] ==
          vm->stack[vm->stackSize - 2];
      vm->stackSize -= 1;
      vm->instructionPointer += 1;
      break;
      
    case INSTRUCTION_PRINT_DEBUG:
      if (vm->stackSize < 1) return TRAP_STACK_UNDERFLOW;
      
      printf("%ld\n", vm->stack[vm->stackSize - 1]);
      vm->stackSize -= 1;
      vm->instructionPointer += 1;
      break;
      
    case INSTRUCTION_DUPLICATE:
      if (vm->stackSize >= VM_STACK_CAPACITY) return TRAP_STACK_OVERFLOW;
      if (vm->stackSize - instruction.operand <= 0) return TRAP_STACK_UNDERFLOW;
      if (instruction.operand < 0) return TRAP_ILLEGAL_OPERAND;
      
      vm->stack[vm->stackSize] = vm->stack[vm->stackSize - 1 - instruction
      .operand];
      vm->stackSize += 1;
      vm->instructionPointer += 1;
      break;
    
    default:
      return TRAP_ILLEGAL_INSTRUCTION;
  }
  return TRAP_OK;
}

void vm_dump_stack(FILE *stream, const VM *vm) {
  fprintf(stream, "Stack:\n");
  if (vm->stackSize > 0) {
    for (Word i = 0; i < vm->stackSize; ++i) {
      fprintf(stream, "%ld ", vm->stack[i]);
    }
    printf("\n");
  } else {
    fprintf(stream, "  [Stack Empty]\n");
  }
}

void vm_load_program_from_memory(
    VM *vm,
    Instruction *program,
    size_t programSize) {
  assert(programSize < VM_PROGRAM_CAPACITY);
  memcpy(vm->program, program, sizeof(program[0]) * programSize);
  vm->programSize = programSize;
}


void vm_load_program_from_file() {
}

int main() {
  vm_load_program_from_memory(&vm, program, ARRAY_SIZE(program));
  for(int i = 0; i < VM_EXECUTION_LIMIT && !vm.halt; ++i) {

    printf("%s\n",
           instruction_type_as_cstr(program[vm.instructionPointer].type));
    Trap trap = vm_execute_instruction(&vm);
    vm_dump_stack(stdout, &vm);
    
    if (trap != TRAP_OK) {
      fprintf(stderr, "Trap Activated: %s\n", trap_as_cstr(trap));
      vm_dump_stack(stderr, &vm);
      exit(1);
    }
  }
  vm_dump_stack(stdout, &vm);
  return 0;
}
