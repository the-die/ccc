#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_PUNCT, // Punctuators
  TK_NUM,   // Numeric literals
  TK_EOF,   // End-of-file markers
} TokenKind;

// Token type
typedef struct Token Token;
struct Token {
  TokenKind kind; // Token kind
  Token *next;    // Next token
  int val;        // If kind is TK_NUM, its value
  char *loc;      // Token location
  int len;        // Token length
};

// Input string
static char *current_input;

// Reports an error and exit.
static void error(char *fmt, ...) {
  va_list ap;

  // void va_start (va_list ap, paramN);
  // Initialize a variable argument list
  // Initializes ap to retrieve the additional arguments after parameter paramN.
  // A function that invokes va_start, shall also invoke va_end before it returns.
  va_start(ap, fmt);

  // int vfprintf ( FILE * stream, const char * format, va_list arg );
  // Write formatted data from variable argument list to stream
  // Writes the C string pointed by format to the stream, replacing any format specifier in the same
  // way as printf does, but using the elements in the variable argument list identified by arg
  // instead of additional function arguments.
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");

  // void va_end (va_list ap);
  // End using variable argument list
  // Performs the appropriate actions to facilitate a normal return by a function that has used the
  // va_list object ap to retrieve its additional arguments.
  // This macro should be invoked before the function returns whenever va_start has been invoked
  // from that function.
  va_end(ap);
  exit(1);
}

// Reports an error location and exit.
static void verror_at(char *loc, char *fmt, va_list ap) {
  int pos = loc - current_input;
  fprintf(stderr, "%s\n", current_input);
  // int fprintf ( FILE * stream, const char * format, ... );
  //
  // Write formatted data to stream
  //
  // Writes the C string pointed by format to the stream. If format includes format specifiers
  // (subsequences beginning with %), the additional arguments following format are formatted and
  // inserted in the resulting string replacing their respective specifiers.
  //
  // After the format parameter, the function expects at least as many additional arguments as
  // specified by format.
  //
  // C string that contains the text to be written to the stream.
  // It can optionally contain embedded format specifiers that are replaced by the values specified
  // in subsequent additional arguments and formatted as requested.
  //
  // A format specifier follows this prototype:
  //
  // %[flags][width][.precision][length]specifier
  //
  // width: description
  //
  // (number): Minimum number of characters to be printed. If the value to be printed is shorter
  // than this number, the result is padded with blank spaces. The value is not truncated even if
  // the result is larger.
  //
  // *: The width is not specified in the format string, but as an additional integer value argument
  // preceding the argument that has to be formatted.
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

static void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
}

static void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}

// Consumes the current token if it matches `s`.
static bool equal(Token *tok, char *op) {
  // strcmp?
  return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

// Ensure that the current token is `s`.
static Token *skip(Token *tok, char *s) {
  if (!equal(tok, s))
    error_tok(tok, "expected '%s'", s);
  return tok->next;
}

// Ensure that the current token is TK_NUM.
static int get_number(Token *tok) {
  if (tok->kind != TK_NUM)
    error_tok(tok, "expected a number");
  return tok->val;
}

// Create a new token.
static Token *new_token(TokenKind kind, char *start, char *end) {
  // void* calloc (size_t num, size_t size);
  // Allocate and zero-initialize array
  // Allocates a block of memory for an array of num elements, each of them size bytes long, and
  // initializes all its bits to zero.
  // The effective result is the allocation of a zero-initialized memory block of (num*size) bytes.
  // If size is zero, the return value depends on the particular library implementation (it may or
  // may not be a null pointer), but the returned pointer shall not be dereferenced.
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = start;
  tok->len = end - start;
  return tok;
}

// Tokenize `current_input` and returns new tokens.
static Token *tokenize(void) {
  char *p = current_input;
  Token head = {};
  Token *cur = &head;

  while (*p) {
    // Skip whitespace characters.
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Numeric literal
    if (isdigit(*p)) {
      cur = cur->next = new_token(TK_NUM, p, p);
      char *q = p;
      // unsigned long int strtoul (const char* str, char** endptr, int base);
      // Convert string to unsigned long integer
      // Parses the C-string str, interpreting its content as an integral number of the specified
      // base, which is returned as an value of type unsigned long int.
      // This function operates like strtol to interpret the string, but produces numbers of type
      // unsigned long int (see strtol for details on the interpretation process).
      cur->val = strtoul(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    // Punctuator
    if (*p == '+' || *p == '-') {
      cur = cur->next = new_token(TK_PUNCT, p, p + 1);
      p++;
      continue;
    }

    error_at(p, "invalid token");
  }

  cur = cur->next = new_token(TK_EOF, p, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);

  current_input = argv[1];
  Token *tok = tokenize();

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

  // long int strtol (const char* str, char** endptr, int base);
  // Convert string to long integer
  // Parses the C-string str interpreting its content as an integral number of the specified base,
  // which is returned as a long int value. If endptr is not a null pointer, the function also sets
  // the value of endptr to point to the first character after the number.
  //
  // The first token must be a number
  printf("  mov $%d, %%rax\n", get_number(tok));
  tok = tok->next;

  // ... followed by either `+ <number>` or `- <number>`.
  while (tok->kind != TK_EOF) {
    if (equal(tok, "+")) {
      // ADD—Add
      // Adds the destination operand (first operand) and the source operand (second operand) and
      // then stores the result in the destination operand. The destination operand can be a
      // register or a memory location; the source operand can be an immediate, a register, or a
      // memory location. (However, two memory operands cannot be used in one instruction.) When an
      // immediate value is used as an operand, it is sign-extended to the length of the destination
      // operand format.
      printf("  add $%d, %%rax\n", get_number(tok->next));
      tok = tok->next->next;
      continue;
    }

    tok = skip(tok, "-");
    // SUB—Subtract
    // Subtracts the second operand (source operand) from the first operand (destination operand)
    // and stores the result in the destination operand. The destination operand can be a register
    // or a memory location; the source operand can be an immediate, register, or memory location.
    // (However, two memory operands cannot be used in one instruction.) When an immediate value is
    // used as an operand, it is sign-extended to the length of the destination operand format.
    printf("  sub $%d, %%rax\n", get_number(tok));
    tok = tok->next;
  }

  // RET—Return From Procedure
  // Transfers program control to a return address located on the top of the
  // stack. The address is usually placed on the stack by a CALL instruction,
  // and the return is made to the instruction that follows the CALL
  // instruction.
  printf("  ret\n");
  return 0;
}
