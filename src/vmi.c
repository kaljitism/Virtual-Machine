#include "vm.c"

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "USAGE: ./vmi <input.vm>\n");
    fprintf(stderr, "ERROR: Expected input.\n");
    exit(1);
  }
  
  
  vm_load_program_from_file(&vm, argv[1]);
  vm_dump_stack(stdout, &vm);
  for ( int i = 0; i < VM_EXECUTION_LIMIT && !vm.halt; ++i ) {
    Trap trap = vm_execute_instruction(&vm);
    vm_dump_stack(stdout, &vm);
    
    if ( trap != TRAP_OK ) {
      fprintf(stderr, "Trap Activated: %s\n", trap_as_cstr(trap));
      vm_dump_stack(stderr, &vm);
      exit(1);
    }
  }

  return 0;
}
