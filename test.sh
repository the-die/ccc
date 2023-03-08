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


# Here Documents
#
# This type of redirection instructs the shell to read input from the current source until a line
# containing only word (with no trailing blanks) is seen. All of the lines read up to that point are
# then used as the standard input (or file descriptor n if n is specified) for a command.
#
# The format of here-documents is:
#
# [n]<<[-]word
#         here-document
# delimiter
#
#
# https://pubs.opengroup.org/onlinepubs/9699919799.2018edition/nframe.html
#
# 2.7.4 Here-Document
#
#
# No parameter and variable expansion, command substitution, arithmetic expansion, or filename
# expansion is performed on word. If any part of word is quoted, the delimiter is the result of
# quote removal on word, and the lines in the here-document are not expanded. If word is unquoted,
# all lines of the here-document are subjected to parameter expansion, command substitution, and
# arithmetic expansion, the character sequence \newline is ignored, and ‘\’ must be used to quote
# the characters ‘\’, ‘$’, and ‘`’.
#
# If the redirection operator is ‘<<-’, then all leading tab characters are stripped from input
# lines and the line containing delimiter. This allows here-documents within shell scripts to be
# indented in a natural fashion.
#
#
# -x language
# Specify explicitly the language for the following input files (rather than letting the compiler
# choose a default based on the file name suffix). This option applies to all following input files
# until the next -x option.
#
# -c
# Compile or assemble the source files, but do not link. The linking stage simply is not done. The
# ultimate output is in the form of an object file for each source file.
# By default, the object file name for a source file is made by replacing the suffix ‘.c’, ‘.i’,
# ‘.s’, etc., with ‘.o’.
# Unrecognized input files, not requiring compilation or assembly, are ignored.
#
#
# POSIX
#
# https://pubs.opengroup.org/onlinepubs/9699919799.2018edition/
#
# For utilities that use operands to represent files to be opened for either reading or writing, the
# '-' operand should be used to mean only standard input (or standard output when it is clear from
# context that an output file is being specified) or a file named -.
cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
EOF

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
  gcc -static -o tmp tmp.s tmp2.o
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

assert 3 '{ int a; a=3; return a; }'
assert 3 '{ int a=3; return a; }'
assert 8 '{ int a=3; int z=5; return a+z; }'

assert 3 '{ int a=3; return a; }'
assert 8 '{ int a=3; int z=5; return a+z; }'
assert 6 '{ int a; int b; a=b=3; return a+b; }'
assert 3 '{ int foo=3; return foo; }'
assert 8 '{ int foo123=3; int bar=5; return foo123+bar; }'

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

assert 55 '{ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 '{ for (;;) return 3; return 5; }'

assert 10 '{ int i=0; while(i<10) i=i+1; return i; }'

assert 3 '{ {1; {2;} return 3;} }'

assert 10 '{ int i=0; while(i<10) i=i+1; return i; }'
assert 55 '{ int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} return j; }'

assert 3 '{ int x=3; return *&x; }'
assert 3 '{ int x=3; int *y=&x; int **z=&y; return **z; }'
assert 5 '{ int x=3; int y=5; return *(&x+1); }'
assert 3 '{ int x=3; int y=5; return *(&y-1); }'
assert 5 '{ int x=3; int y=5; return *(&x-(-1)); }'
assert 5 '{ int x=3; int *y=&x; *y=5; return x; }'
assert 7 '{ int x=3; int y=5; *(&x+1)=7; return y; }'
assert 7 '{ int x=3; int y=5; *(&y-2+1)=7; return x; }'
assert 5 '{ int x=3; return (&x+2)-&x+3; }'
assert 8 '{ int x, y; x=3; y=5; return x+y; }'
assert 8 '{ int x=3, y=5; return x+y; }'

assert 3 '{ return ret3(); }'
assert 5 '{ return ret5(); }'

echo OK
