#include "chibicc.h"

// Input string
static char *current_input;

// Reports an error and exit.
void error(char *fmt, ...) {
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

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}

// Consumes the current token if it matches `op`.
bool equal(Token *tok, char *op) {
  // strcmp?
  return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

// Ensure that the current token is `op`.
Token *skip(Token *tok, char *op) {
  if (!equal(tok, op))
    error_tok(tok, "expected '%s'", op);
  return tok->next;
}

// The return value indicates whether the tok consumes the str.
bool consume(Token **rest, Token *tok, char *str) {
  if (equal(tok, str)) {
    *rest = tok->next;
    return true;
  }
  *rest = tok;
  return false;
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

static bool startswith(char *p, char *q) {
  // int strncmp ( const char * str1, const char * str2, size_t num );
  // Compare characters of two strings
  // Compares up to num characters of the C string str1 to those of the C string str2.
  // This function starts comparing the first character of each string. If they are equal to each
  // other, it continues with the following pairs until the characters differ, until a terminating
  // null-character is reached, or until num characters match in both strings, whichever happens
  // first.
  return strncmp(p, q, strlen(q)) == 0;
}

// Returns true if c is valid as the first character of an identifier.
static bool is_ident1(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// Returns true if c is valid as a non-first character of an identifier.
static bool is_ident2(char c) {
  return is_ident1(c) || ('0' <= c && c <= '9');
}

// Read a punctuator token from p and returns its length.
static int read_punct(char *p) {
  if (startswith(p, "==") || startswith(p, "!=") ||
      startswith(p, "<=") || startswith(p, ">="))
    return 2;

  // int ispunct ( int c );
  // Check if character is a punctuation character
  // Checks whether c is a punctuation character.
  return ispunct(*p) ? 1 : 0;
}

static bool is_keyword(Token *tok) {
  static char *kw[] = {"return", "if", "else", "for", "while", "int"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    if (equal(tok, kw[i]))
      return true;
  return false;
}

static void convert_keywords(Token *tok) {
  for (Token *t = tok; t->kind != TK_EOF; t = t->next)
    if (is_keyword(t))
      t->kind = TK_KEYWORD;
}

// Tokenize a given string and returns new tokens.
Token *tokenize(char *p) {
  current_input = p;
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

    // Identifier or keyword
    if (is_ident1(*p)) {
      char *start = p;
      do {
        p++;
      } while (is_ident2(*p));
      cur = cur->next = new_token(TK_IDENT, start, p);
      continue;
    }

    // Punctuators
    int punct_len = read_punct(p);
    if (punct_len) {
      cur = cur->next = new_token(TK_PUNCT, p, p + punct_len);
      p += cur->len;
      continue;
    }

    error_at(p, "invalid token");
  }

  cur = cur->next = new_token(TK_EOF, p, p);
  convert_keywords(head.next);
  return head.next;
}
