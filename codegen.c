#include "chibicc.h"

static int depth;
// Register    Usage callee                                              saved
// %rbx     callee-saved register                                         Yes
// %rcx     used to pass 4th integer argument to functions                No
// %rdx     used to pass 3rd argument to functions; 2nd return register   No
// %rsp     stack pointer                                                 Yes
// %rbp     callee-saved register; optionally used as frame pointer       Yes
// %rsi     used to pass 2nd argument to functions                        No
// %rdi     used to pass 1st argument to functions                        No
// %r8      used to pass 5th argument to functions                        No
// %r9      used to pass 6th argument to functions                        No
static char *argreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static Function *current_fn;

static void gen_expr(Node *node);

static int count(void) {
  static int i = 1;
  return i++;
}

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

// Round up `n` to the nearest multiple of `align`. For instance,
// align_to(5, 8) returns 8 and align_to(11, 8) returns 16.
static int align_to(int n, int align) {
  return (n + align - 1) / align * align;
}

// Compute the absolute address of a given node.
// It's an error if a given node does not reside in memory.
static void gen_addr(Node *node) {
  switch (node->kind) {
  case ND_VAR:
    // LEA—Load Effective Address
    // Computes the effective address of the second operand (the source operand) and stores it in
    // the first operand (destination operand). The source operand is a memory address (offset part)
    // specified with one of the processors addressing modes; the destination operand is a
    // general-purpose register. The address-size and operand-size attributes affect the action
    // performed by this instruction, as shown in the following table. The operand-size attribute of
    // the instruction is determined by the chosen register; the address-size attribute is
    // determined by the attribute of the code segment.
    printf("  lea %d(%%rbp), %%rax\n", node->var->offset);
    return;
  case ND_DEREF:
    gen_expr(node->lhs);
    return;
  }

  error_tok(node->tok, "not an lvalue");
}

// Generate code for a given node.
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
  // The value of the var node is the address of the var.
  case ND_VAR:
    gen_addr(node);
    printf("  mov (%%rax), %%rax\n");
    return;
  // "deref" "var"
  case ND_DEREF:
    gen_expr(node->lhs);
    printf("  mov (%%rax), %%rax\n");
    return;
  // "addr" "var"
  case ND_ADDR:
    gen_addr(node->lhs);
    return;
  case ND_ASSIGN:
    gen_addr(node->lhs);
    push();
    gen_expr(node->rhs);
    pop("%rdi");
    printf("  mov %%rax, (%%rdi)\n");
    return;
  case ND_FUNCALL: {
    int nargs = 0;
    for (Node *arg = node->args; arg; arg = arg->next) {
      gen_expr(arg);
      push();
      nargs++;
    }

    for (int i = nargs - 1; i >= 0; i--)
      pop(argreg[i]);

    printf("  mov $0, %%rax\n");
    // CALL—Call Procedure
    // Saves procedure linking information on the stack and branches to the called procedure
    // specified using the target operand. The target operand specifies the address of the first
    // instruction in the called procedure. The operand can be an immediate value, a general-purpose
    // register, or a memory location.
    printf("  call %s\n", node->funcname);
    return;
  }
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

  error_tok(node->tok, "invalid expression");
}

static void gen_stmt(Node *node) {
  switch (node->kind) {
  case ND_IF: {
    int c = count();
    gen_expr(node->cond);
    printf("  cmp $0, %%rax\n");
    // Jcc—Jump if Condition Is Met
    // Checks the state of one or more of the status flags in the EFLAGS register (CF, OF, PF, SF,
    // and ZF) and, if the flags are in the specified state (condition), performs a jump to the
    // target instruction specified by the destination operand. A condition code (cc) is associated
    // with each instruction to indicate the condition being tested for. If the condition is not
    // satisfied, the jump is not performed and execution continues with the instruction following
    // the Jcc instruction.
    printf("  je  .L.else.%d\n", c);
    gen_stmt(node->then);
    printf("  jmp .L.end.%d\n", c);
    printf(".L.else.%d:\n", c);
    if (node->els)
      gen_stmt(node->els);
    printf(".L.end.%d:\n", c);
    return;
  }
  case ND_FOR: {
    int c = count();
    if (node->init)
      gen_stmt(node->init);
    printf(".L.begin.%d:\n", c);
    if (node->cond) {
      gen_expr(node->cond);
      printf("  cmp $0, %%rax\n");
      printf("  je  .L.end.%d\n", c);
    }
    gen_stmt(node->then);
    if (node->inc)
      gen_expr(node->inc);
    printf("  jmp .L.begin.%d\n", c);
    printf(".L.end.%d:\n", c);
    return;
  }
  case ND_BLOCK:
    for (Node *n = node->body; n; n = n->next)
      gen_stmt(n);
    return;
  case ND_RETURN:
    gen_expr(node->lhs);
    // JMP—Jump
    // Transfers program control to a different point in the instruction stream without recording
    // return information. The destination (target) operand specifies the address of the instruction
    // being jumped to. This operand can be an immediate value, a general-purpose register, or a
    // memory location.
    //
    // Symbol names begin with a letter or with one of ‘._’. On most machines, you can also use $
    // in symbol names; exceptions are noted in Machine Dependent Features. That character may be
    // followed by any string of digits, letters, dollar signs (unless otherwise noted for a
    // particular target machine), and underscores. These restrictions do not apply when quoting
    // symbol names by ‘"’, which is permitted for most targets. Escaping characters in quoted
    // symbol names with ‘\’ generally extends only to ‘\’ itself and ‘"’, at the time of writing.
    //
    // Case of letters is significant: foo is a different symbol name than Foo.
    //
    // Symbol names do not start with a digit. An exception to this rule is made for Local Labels.
    // See below.
    //
    // Local Symbol Names
    // A local symbol is any symbol beginning with certain local label prefixes. By default, the
    // local label prefix is ‘.L’ for ELF systems or ‘L’ for traditional a.out systems, but each
    // target may have its own set of local label prefixes. On the HPPA local symbols begin with
    // ‘L$’.
    //
    // Local symbols are defined and used within the assembler, but they are normally not saved in
    // object files. Thus, they are not visible when debugging. You may use the ‘-L’ option (see
    // Include Local Symbols) to retain the local symbols in the object files.
    printf("  jmp .L.return.%s\n", current_fn->name);
    return;
  case ND_EXPR_STMT:
    gen_expr(node->lhs);
    return;
  }

  error_tok(node->tok, "invalid statement");
}

// Assign offsets to local variables.
static void assign_lvar_offsets(Function *prog) {
  for (Function *fn = prog; fn; fn = fn->next) {
    int offset = 0;
    for (Obj *var = fn->locals; var; var = var->next) {
      offset += 8;
      var->offset = -offset;
    }
    // %rsp: The stack pointer holds the address of the byte with lowest address which is part of
    // the stack. It is guaranteed to be 16-byte aligned at process entry.
    fn->stack_size = align_to(offset, 16);
  }
}

void codegen(Function *prog) {
  assign_lvar_offsets(prog);

  // https://sourceware.org/binutils/docs/as.html
  // https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
  // https://gitlab.com/x86-psABIs/x86-64-ABI

  for (Function *fn = prog; fn; fn = fn->next) {
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
    printf("  .globl %s\n", fn->name);

    // A label is written as a symbol immediately followed by a colon ‘:’. The
    // symbol then represents the current value of the active location counter,
    // and is, for example, a suitable instruction operand. You are warned if you
    // use the same symbol to represent two different locations: the first
    // definition overrides any other definitions.
    printf("%s:\n", fn->name);
    current_fn = fn;

    // Prologue
    // %rbp: callee-saved register; optionally used as frame pointer
    printf("  push %%rbp\n");
    // %rsp: stack pointer
    printf("  mov %%rsp, %%rbp\n");
    printf("  sub $%d, %%rsp\n", fn->stack_size);

    // Save passed-by-register arguments to the stack
    int i = 0;
    for (Obj *var = fn->params; var; var = var->next)
      printf("  mov %s, %d(%%rbp)\n", argreg[i++], var->offset);

    // Emit code
    gen_stmt(fn->body);
    assert(depth == 0);

    // Epilogue
    printf(".L.return.%s:\n", fn->name);
    // restore %rbp and %rsp
    printf("  mov %%rbp, %%rsp\n");
    printf("  pop %%rbp\n");

    //   Position  |            Contents         |  Frame
    // ------------+-----------------------------+----------
    // 8n+16(%rbp) | memory argument eightbyte n |
    //             |             . . .           | Previous
    //   16(%rbp)  | memory argument eightbyte 0 |
    // ------------+-----------------------------+----------
    //   8(%rbp)   |       return address        |
    //             +-----------------------------+
    //   0(%rbp)   |     previous %rbp value     |
    //             +-----------------------------+
    //  -8(%rbp)   |         unspecified         | Current
    //             |             . . .           |
    //   0(%rsp)   |         variable size       |
    //             +-----------------------------+
    // -128(%rsp)  |           red zone          |

    // RET—Return From Procedure
    // Transfers program control to a return address located on the top of the
    // stack. The address is usually placed on the stack by a CALL instruction,
    // and the return is made to the instruction that follows the CALL
    // instruction.
    printf("  ret\n");
  }
}
