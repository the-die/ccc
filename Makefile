# ‘#’ in a line of a makefile starts a comment. It and the rest of the line are ignored, except that
# a trailing backslash not escaped by another backslash will continue the comment across multiple
# lines. A line containing just a comment (with perhaps spaces before it) is effectively blank, and
# is ignored. If you want a literal #, escape it with a backslash (e.g., \#). Comments may appear on
# any line in the makefile, although they are treated specially in certain situations.

# The recipes in built-in implicit rules make liberal use of certain predefined variables. You can
# alter the values of these variables in the makefile, with arguments to make, or in the environment
# to alter how the implicit rules work without redefining the rules themselves. You can cancel all
# variables used by implicit rules with the ‘-R’ or ‘--no-builtin-variables’ option.
#
# For example, the recipe used to compile a C source file actually says
# ‘$(CC) -c $(CFLAGS) $(CPPFLAGS)’. The default values of the variables used are ‘cc’ and nothing,
# resulting in the command ‘cc -c’. By redefining ‘CC’ to ‘ncc’, you could cause ‘ncc’ to be used
# for all C compilations performed by the implicit rule. By redefining ‘CFLAGS’ to be ‘-g’, you
# could pass the ‘-g’ option to each compilation. All implicit rules that do C compilation use
# ‘$(CC)’ to get the program name for the compiler and all include ‘$(CFLAGS)’ among the arguments
# given to the compiler.

# CFLAGS
#   Extra flags to give to the C compiler

# https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html#Option-Summary
# -std=
#   Determine the language standard. See Language Standards Supported by GCC, for details of these
#   standard versions. This option is currently only supported when compiling C or C++.
# -g
#   Produce debugging information in the operating system’s native format (stabs, COFF, XCOFF, or
#   DWARF). GDB can work with this debugging information.
# -fcommon
#   In C code, this option controls the placement of global variables defined without an initializer,
#   known as tentative definitions in the C standard. Tentative definitions are distinct from
#   declarations of a variable with the extern keyword, which do not allocate storage.
#
#   The default is -fno-common, which specifies that the compiler places uninitialized global
#   variables in the BSS section of the object file. This inhibits the merging of tentative
#   definitions by the linker so you get a multiple-definition error if the same variable is
#   accidentally defined in more than one compilation unit.
#
#   The -fcommon places uninitialized global variables in a common block. This allows the linker to
#   resolve all tentative definitions of the same variable in different compilation units to the
#   same object, or to a non-tentative definition. This behavior is inconsistent with C++, and on
#   many targets implies a speed and code size penalty on global variable references. It is mainly
#   useful to enable legacy code to link without errors.
CFLAGS=-std=c11 -g -fno-common

# LDFLAGS
#   Extra flags to give to compilers when they are supposed to invoke the linker, ‘ld’, such as -L.
#   Libraries (-lfoo) should be added to the LDLIBS variable instead.
# CC
#   Program for compiling C programs; default ‘cc’.
chibicc: main.o
	$(CC) -o chibicc main.o $(LDFLAGS)

test: chibicc
	./test.sh

clean:
	rm -f chibicc *.o *~ tmp*

# A phony target is one that is not really the name of a file; rather it is just a name for a recipe
# to be executed when you make an explicit request. There are two reasons to use a phony target: to
# avoid a conflict with a file of the same name, and to improve performance.
#
# .PHONY
#   The prerequisites of the special target .PHONY are considered to be phony targets. When it is
#   time to consider such a target, make will run its recipe unconditionally, regardless of whether
#   a file with that name exists or what its last-modification time is.
.PHONY: test clean
