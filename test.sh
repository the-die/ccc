#! /bin/bash

# https://www.gnu.org/software/bash/manual/bash.html

# In a non-interactive shell, or an interactive shell in which the interactive_comments option to
# the shopt builtin is enabled (see The Shopt Builtin), a word beginning with ‘#’ causes that word
# and all remaining characters on that line to be ignored. An interactive shell without the
# interactive_comments option enabled does not allow comments. The interactive_comments option is on
# by default in interactive shells. See Interactive Shells, for a description of what makes a shell
# interactive.

# Most versions of Unix make this a part of the operating system’s command execution mechanism. If
# the first line of a script begins with the two characters ‘#!’, the remainder of the line
# specifies an interpreter for the program and, depending on the operating system, one or more
# optional arguments for that interpreter. Thus, you can specify Bash, awk, Perl, or some other
# interpreter and write the rest of the script file in that language.

# Bash scripts often begin with #! /bin/bash (assuming that Bash has been installed in /bin), since
# this ensures that Bash will be used to interpret the script, even if it is executed under another
# shell. It’s a common idiom to use env to find bash even if it’s been installed in another
# directory: #!/usr/bin/env bash will find the first occurrence of bash in $PATH.

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

assert 0 '{ return 0; }'
assert 42 '{ return 42; }'
assert 21 '{ return 5+20-4; }'
assert 41 '{ return  12 + 34 - 5 ; }'
assert 47 '{ return 5+6*7; }'
assert 15 '{ return 5*(9-6); }'
assert 4 '{ return (3+5)/2; }'
assert 10 '{ return -10+20; }'
assert 10 '{ return - -10; }'
assert 10 '{ return - - +10; }'

assert 0 '{ return 0==1; }'
assert 1 '{ return 42==42; }'
assert 1 '{ return 0!=1; }'
assert 0 '{ return 42!=42; }'

assert 1 '{ return 0<1; }'
assert 0 '{ return 1<1; }'
assert 0 '{ return 2<1; }'
assert 1 '{ return 0<=1; }'
assert 1 '{ return 1<=1; }'
assert 0 '{ return 2<=1; }'

assert 1 '{ return 1>0; }'
assert 0 '{ return 1>1; }'
assert 0 '{ return 1>2; }'
assert 1 '{ return 1>=0; }'
assert 1 '{ return 1>=1; }'
assert 0 '{ return 1>=2; }'

assert 3 '{ a=3; return a; }'
assert 8 '{ a=3; z=5; return a+z; }'

assert 3 '{ a=3; return a; }'
assert 8 '{ a=3; z=5; return a+z; }'
assert 6 '{ a=b=3; return a+b; }'
assert 3 '{ foo=3; return foo; }'
assert 8 '{ foo123=3; bar=5; return foo123+bar; }'

assert 1 '{ return 1; 2; 3; }'
assert 2 '{ 1; return 2; 3; }'
assert 3 '{ 1; 2; return 3; }'

assert 3 '{ {1; {2;} return 3;} }'
assert 5 '{ ;;; return 5; }'

assert 3 '{ if (0) return 2; return 3; }'
assert 3 '{ if (1-1) return 2; return 3; }'
assert 2 '{ if (1) return 2; return 3; }'
assert 2 '{ if (2-1) return 2; return 3; }'
assert 4 '{ if (0) { 1; 2; return 3; } else { return 4; } }'
assert 3 '{ if (1) { 1; 2; return 3; } else { return 4; } }'

assert 55 '{ i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 '{ for (;;) {return 3;} return 5; }'

echo OK
