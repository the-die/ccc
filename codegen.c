#include "chibicc.h"

static int depth;

static void push(void) {
  // PUSH—Push Word, Doubleword, or Quadword Onto the Stack
  // Decrements the stack pointer and then stores the source operand on the top of the stack.
  printf("  push %%rax\n");
  depth++;
}

static void pop(char *arg) {
  // POP—Pop a Value From the Stack
  // Loads the value from the top of the stack to the location specified with the destination
  // operand (or explicit opcode) and then increments the stack pointer. The destination operand can
  // be a general-purpose register, memory location, or segment register.
  printf("  pop %s\n", arg);
  depth--;
}

static void gen_expr(Node *node) {
  switch (node->kind) {
  case ND_NUM:
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

    // long int strtol (const char* str, char** endptr, int base);
    // Convert string to long integer
    // Parses the C-string str interpreting its content as an integral number of the specified base,
    // which is returned as a long int value. If endptr is not a null pointer, the function also
    // sets the value of endptr to point to the first character after the number.
    printf("  mov $%d, %%rax\n", node->val);
    return;
  case ND_NEG:
    gen_expr(node->lhs);
    // NEG—Two's Complement Negation
    // Replaces the value of operand (the destination operand) with its two's complement. (This
    // operation is equivalent to subtracting the operand from 0.) The destination operand is
    // located in a general-purpose register or a memory location.
    printf("  neg %%rax\n");
    return;
  }

  gen_expr(node->rhs);
  push();
  gen_expr(node->lhs);
  // %rdi: used to pass 1st argument to functions
  pop("%rdi");
  // now %rax is lhs value, %rdi is rhs value

  switch (node->kind) {
  case ND_ADD:
    // ADD—Add
    // Adds the destination operand (first operand) and the source operand (second operand) and
    // then stores the result in the destination operand. The destination operand can be a
    // register or a memory location; the source operand can be an immediate, a register, or a
    // memory location. (However, two memory operands cannot be used in one instruction.) When an
    // immediate value is used as an operand, it is sign-extended to the length of the destination
    // operand format.
    printf("  add %%rdi, %%rax\n");
    return;
  case ND_SUB:
    // SUB—Subtract
    // Subtracts the second operand (source operand) from the first operand (destination operand)
    // and stores the result in the destination operand. The destination operand can be a register
    // or a memory location; the source operand can be an immediate, register, or memory location.
    // (However, two memory operands cannot be used in one instruction.) When an immediate value is
    // used as an operand, it is sign-extended to the length of the destination operand format.
    printf("  sub %%rdi, %%rax\n");
    return;
  case ND_MUL:
    // IMUL—Signed Multiply
    // Performs a signed multiplication of two operands. This instruction has three forms, depending
    // on the number of operands.
    printf("  imul %%rdi, %%rax\n");
    return;
  case ND_DIV:
    // CWD/CDQ/CQO—Convert Word to Doubleword/Convert Doubleword to Quadword
    // Doubles the size of the operand in register AX, EAX, or RAX (depending on the operand size)
    // by means of sign extension and stores the result in registers DX:AX, EDX:EAX, or RDX:RAX,
    // respectively. The CWD instruction copies the sign (bit 15) of the value in the AX register
    // into every bit position in the DX register. The CDQ instruction copies the sign (bit 31) of
    // the value in the EAX register into every bit position in the EDX register. The CQO
    // instruction (available in 64-bit mode only) copies the sign (bit 63) of the value in the RAX
    // register into every bit position in the RDX register.
    printf("  cqo\n");
    // IDIV—Signed Divide
    // Divides the (signed) value in the AX, DX:AX, or EDX:EAX (dividend) by the source operand
    // (divisor) and stores the result in the AX (AH:AL), DX:AX, or EDX:EAX registers. The source
    // operand can be a general-purpose register or a memory location. The action of this
    // instruction depends on the operand size (dividend/divisor).
    //
    // In 64-bit mode, the instruction’s default operation size is 32 bits. Use of the REX.R prefix
    // permits access to additional registers (R8-R15). Use of the REX.W prefix promotes operation
    // to 64 bits. In 64-bit mode when REX.W is applied, the instruction divides the signed value in
    // RDX:RAX by the source operand. RAX contains a 64-bit quotient; RDX contains a 64-bit
    // remainder.
    printf("  idiv %%rdi\n");
    return;
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
    // CMP—Compare Two operands
    // Compares the first source operand with the second source operand and sets the status flags
    // in the EFLAGS register according to the results. The comparison is performed by subtracting
    // the second operand from the first operand and then setting the status flags in the same
    // manner as the SUB instruction. When an immediate value is used as an operand, it is
    // sign-extended to the length of the first operand.
    printf("  cmp %%rdi, %%rax\n");

    // SETcc—Set Byte on Condition
    // Sets the destination operand to 0 or 1 depending on the settings of the status flags (CF, SF,
    // OF, ZF, and PF) in the EFLAGS register. The destination operand points to a byte register or
    // a byte in memory. The condition code suffix(cc) indicates the condition being tested for.
    //
    // The terms “above” and “below” are associated with the CF flag and refer to the relationship
    // between two unsigned integer values. The terms “greater” and “less” are associated with the
    // SF and OF flags and refer to the relationship between two signed integer values.
    if (node->kind == ND_EQ)
      printf("  sete %%al\n");
    else if (node->kind == ND_NE)
      printf("  setne %%al\n");
    else if (node->kind == ND_LT)
      printf("  setl %%al\n");
    else if (node->kind == ND_LE)
      printf("  setle %%al\n");

    // MOVZX—Move With Zero-Extend
    // Copies the contents of the source operand (register or memory location) to the destination
    // operand (register) and zero extends the value. The size of the converted value depends on the
    // operand-size attribute.
    //
    // movzb?
    printf("  movzx %%al, %%rax\n");
    return;
  }

  error("invalid expression");
}

static void gen_stmt(Node *node) {
  if (node->kind == ND_EXPR_STMT) {
    gen_expr(node->lhs);
    return;
  }

  error("invalid statement");
}

void codegen(Node *node) {
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

  for (Node *n = node; n; n = n->next) {
    gen_stmt(n);
    assert(depth == 0);
  }

  // RET—Return From Procedure
  // Transfers program control to a return address located on the top of the
  // stack. The address is usually placed on the stack by a CALL instruction,
  // and the return is made to the instruction that follows the CALL
  // instruction.
  printf("  ret\n");
}
