#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
    return 1;
  }

  // https://sourceware.org/binutils/docs/as.html
  // https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
  // https://gitlab.com/x86-psABIs/x86-64-ABI

  // Symbols are a central concept: the programmer uses symbols to name things,
  // the linker uses symbols to link, and the debugger uses symbols to debug.
  // Warning: as does not place symbols in the object file in the same order
  // they were declared. This may break some debuggers.

  // .global symbol, .globl symbol
  // .global makes the symbol visible to ld. If you define symbol in your
  // partial program, its value is made available to other partial programs that
  // are linked with it. Otherwise, symbol takes its attributes from a symbol of
  // the same name from another file linked into the same program. Both
  // spellings (‘.globl’ and ‘.global’) are accepted, for compatibility with
  // other assemblers.
  printf("  .globl main\n");

  // A label is written as a symbol immediately followed by a colon ‘:’. The
  // symbol then represents the current value of the active location counter,
  // and is, for example, a suitable instruction operand. You are warned if you
  // use the same symbol to represent two different locations: the first
  // definition overrides any other definitions.
  printf("main:\n");

  // T&T immediate operands are preceded by ‘$’; Intel immediate operands are
  // undelimited (Intel ‘push 4’ is AT&T ‘pushl $4’). AT&T register operands are
  // preceded by ‘%’; Intel register operands are undelimited.
  //
  // AT&T and Intel syntax use the opposite order for source and destination
  // operands. Intel ‘add eax, 4’ is ‘addl $4, %eax’. The ‘source, dest’
  // convention is maintained for compatibility with previous Unix assemblers.
  //
  // System V AMD64 ABI
  // %rax: temporary register; with variable arguments passes information about
  // the number of vector registers used; 1st return register
  printf("  mov $%d, %%rax\n", atoi(argv[1]));

  // RET—Return From Procedure
  // Transfers program control to a return address located on the top of the
  // stack. The address is usually placed on the stack by a CALL instruction,
  // and the return is made to the instruction that follows the CALL
  // instruction.
  printf("  ret\n");
  return 0;
}
