#include "vm.c"


int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "USAGE: ./bm <input.vasm> <output.vm>\n");
    fprintf(stderr, "ERROR: Expected input and output\n");
    exit(1);
  }
  
  const char *inputFilePath = argv[1];
  const char *outputFilePath = argv[2];
  
  StringView source_code = slurp_file(inputFilePath);
  
  vm.programSize = vm.programSize = vm_translate_source(
      source_code,
      vm.program,
      VM_PROGRAM_CAPACITY);
  
  vm_save_program_to_file(vm.program, vm.programSize, outputFilePath);
  
  return 0;
}
