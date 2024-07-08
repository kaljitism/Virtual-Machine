#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

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
  INSTRUCTION_NOPE = 0,
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
    case INSTRUCTION_NOPE:
      return "INSTRUCTION_NOPE";
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

#define MAKE_INSTRUCTION_NOPE() {}
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
    
    case INSTRUCTION_NOPE:
      vm->instructionPointer += 1;
      break;
    
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

void vm_load_program_from_file(VM *vm, const char *filePath) {
  FILE *file = fopen(filePath, "rb");
  if (file == NULL) {
    fprintf(
        stderr,
        "ERROR: Could not open file %s : %s\n",
        filePath, strerror(errno));
    exit(1);
  }
  
  if (fseek(file, 0, SEEK_END) < 0) {
    fprintf(
        stderr,
        "ERROR: Could not read file %s : %s\n",
        filePath, strerror(errno));
    exit(1);
  }
  
  long currentPosition = ftell(file);
  if (currentPosition < 0) {
    fprintf(
        stderr,
        "ERROR: Could not read file %s : %s\n",
        filePath, strerror(errno));
    exit(1);
  }
  
  assert(currentPosition % sizeof(vm->program[0]) == 0);
  assert((size_t) currentPosition <= VM_PROGRAM_CAPACITY * sizeof
      (vm->program[0]));
  
  if (fseek(file, 0, SEEK_SET) < 0) {
    fprintf(
        stderr,
        "ERROR: Could not read file %s : %s\n",
        filePath, strerror(errno));
    exit(1);
  }
  
  vm->programSize = fread(vm->program, sizeof(vm->program[0]), currentPosition
                                                               / sizeof
                                                                   (vm->program[0]), file);
  if (ferror(file)) {
    fprintf(
        stderr,
        "ERROR: Could not read file %s : %s\n",
        filePath, strerror(errno));
    exit(1);
  }
  
  fclose(file);
}

void vm_save_program_to_file(
    Instruction *program,
    size_t programSize,
    const char *filePath) {
  FILE *file = fopen(filePath, "wb");
  if (file == NULL) {
    fprintf(
        stderr,
        "ERROR: Could not open file %s : %s\n",
        filePath, strerror(errno));
    exit(1);
  }
  
  fwrite(program, sizeof(program[0]), programSize, file);
  if (ferror(file)) {
    fprintf(
        stderr,
        "ERROR: Could not write to the file %s : %s\n",
        filePath, strerror(errno));
    exit(1);
  }
  
  fclose(file);
}


typedef struct {
  size_t count;
  const char *data;
} StringView;

StringView cstr_as_stringView(const char *cstr) {
  return (StringView) {
    .count = strlen(cstr),
    .data = cstr,
  };
}

StringView stringView_trim_left(StringView stringView) {
  assert(0 && "SV_TRIM_LEFT: NOT IMPLEMENTED");
}

StringView stringView_trim_right(StringView stringView) {
  assert(0 && "SV_TRIM_RIGHT: NOT IMPLEMENTED");
}


StringView stringView_chop_by_delimeter(StringView *stringView, char
delimeter) {
  assert(0 && "SV_CHOP_BY_DELIMETER: NOT IMPLEMENTED");
}

int stringView_equal(StringView x, StringView y) {
  assert(0 && "SV_EQUAL: NOT IMPLEMENTED");
}

int stringView_toInteger(StringView line) {
  assert(0 && "SV_CHOP_INTEGER: NOT IMPLEMENTED");
}

Instruction vm_translate_line(StringView line) {
 line = stringView_trim_left(line);
 StringView instructionName = stringView_chop_by_delimeter(
     &line,
     ' ');
 
 if (stringView_equal(instructionName, cstr_as_stringView("push"))) {
   line = stringView_trim_left(line);
     int operand = stringView_toInteger(line);
     return (Instruction) {.type = INSTRUCTION_PUSH, .operand = operand};
 } else {
   fprintf(stderr, "ERROR: `%*.s` is not a number", (int) instructionName.count,
           instructionName.data);
   exit(1);
 }
}

size_t vm_translate_source(
    StringView sourceCode,
    Instruction *program,
    size_t programCapacity) {
  while(sourceCode.count > 0) {
    StringView line = stringView_chop_by_delimeter(
        &sourceCode,
        '\n');
    printf("#%.*s#\n", (int) line.count, line.data);
  }
  return 0;
}


VM vm = {0};
char *sourceCode =
    "push 0\n"
    "push 1\n"
    "duplicate 1\n"
    "duplicate 1\n"
    "plus\n"
    "jump 2\n";

Instruction program[] = {
    MAKE_INSTRUCTION_PUSH(0),
    MAKE_INSTRUCTION_PUSH(1),
    MAKE_INSTRUCTION_DUPLICATE(1),
    MAKE_INSTRUCTION_DUPLICATE(1),
    MAKE_INSTRUCTION_PLUS(),
    MAKE_INSTRUCTION_JUMP(2),
};


int main(void) {
  vm.programSize = vm_translate_source(cstr_as_stringView(sourceCode),
                                       vm.program,
                                       VM_PROGRAM_CAPACITY);
  
  return 0;
}

int main2() {
  vm_load_program_from_file(&vm, "fibonacci.vm");
  vm_dump_stack(stdout, &vm);
  for(int i = 0; i < VM_EXECUTION_LIMIT && !vm.halt; ++i) {
    printf("%s %ld\n",
           instruction_type_as_cstr(program[vm.instructionPointer].type),
           program[vm.instructionPointer].operand);
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
