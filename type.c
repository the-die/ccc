#include "chibicc.h"

// Compound literals
// Constructs an unnamed object of specified type (which may be struct, union, or even array type)
// in-place.
//
// Syntax
// ( storage-class-specifiers(since C23)(optional) type ) { initializer-list } (since C99)
// ( storage-class-specifiers(since C23)(optional) type ) { initializer-list , } (since C99)
//
// The compound literal expression constructs an unnamed object of the type specified by type and
// initializes it as specified by initializer-list. Designated initializers are accepted.
//
// The type of the compound literal is type (except when type is an array of unknown size; its size
// is deduced from the initializer-list as in array initialization).
//
// The value category of a compound literal is lvalue (its address can be taken).
Type *ty_int = &(Type){TY_INT};

bool is_integer(Type *ty) {
  return ty->kind == TY_INT;
}

Type *pointer_to(Type *base) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->base = base;
  return ty;
}

void add_type(Node *node) {
  if (!node || node->ty)
    return;

  add_type(node->lhs);
  add_type(node->rhs);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->inc);

  for (Node *n = node->body; n; n = n->next)
    add_type(n);

  switch (node->kind) {
  case ND_ADD:
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
  case ND_NEG:
  case ND_ASSIGN:
    node->ty = node->lhs->ty;
    return;
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_NUM:
    node->ty = ty_int;
    return;
  case ND_VAR:
    node->ty = node->var->ty;
    return;
  case ND_ADDR:
    node->ty = pointer_to(node->lhs->ty);
    return;
  case ND_DEREF:
    if (node->lhs->ty->kind != TY_PTR)
      error_tok(node->tok, "invalid pointer dereference");
    node->ty = node->lhs->ty->base;
    return;
  }
}
