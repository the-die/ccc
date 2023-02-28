#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./chibicc "$input" > tmp.s || exit

  # -static
  #   On systems that support dynamic linking, this overrides -pie and prevents linking with the
  #   shared libraries. On other systems, this option has no effect.
  #
  # -o file
  #   Place the primary output in file file. This applies to whatever sort of output is being
  #   produced, whether it be an executable file, an object file, an assembler file or preprocessed
  #   C code.
  #
  #   If -o is not specified, the default is to put an executable file in a.out, the object file for
  #   source.suffix in source.o, its assembler file in source.s, a precompiled header file in
  #   source.suffix.gch, and all preprocessed C source on standard output.
  gcc -static -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42

echo OK
