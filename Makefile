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

# Wildcard expansion happens automatically in rules. But wildcard expansion does not normally take
# place when a variable is set, or inside the arguments of a function. If you want to do wildcard
# expansion in such places, you need to use the wildcard function, like this:
#
#   $(wildcard pattern…)
#
# This string, used anywhere in a makefile, is replaced by a space-separated list of names of
# existing files that match one of the given file name patterns. If no existing file name matches a
# pattern, then that pattern is omitted from the output of the wildcard function. Note that this is
# different from how unmatched wildcards behave in rules, where they are used verbatim rather than
# ignored (see Pitfalls of Using Wildcards).
#
# As with wildcard expansion in rules, the results of the wildcard function are sorted. But again,
# each individual expression is sorted separately, so ‘$(wildcard *.c *.h)’ will expand to all files
# matching ‘.c’, sorted, followed by all files matching ‘.h’, sorted.
SRCS=$(wildcard *.c)

# A substitution reference substitutes the value of a variable with alterations that you specify. It
# has the form ‘$(var:a=b)’ (or ‘${var:a=b}’) and its meaning is to take the value of the variable
# var, replace every a at the end of a word with b in that value, and substitute the resulting
# string.
#
# When we say “at the end of a word”, we mean that a must appear either followed by whitespace or at
# the end of the value in order to be replaced; other occurrences of a in the value are unaltered.
# For example:
#
#   foo := a.o b.o l.a c.o
#   bar := $(foo:.o=.c)
#
# sets ‘bar’ to ‘a.c b.c l.a c.c’. See Setting Variables.
#
# A substitution reference is shorthand for the patsubst expansion function (see Functions for
# String Substitution and Analysis): ‘$(var:a=b)’ is equivalent to ‘$(patsubst %a,%b,var)’. We
# provide substitution references as well as patsubst for compatibility with other implementations
# of make.
OBJS=$(SRCS:.c=.o)

# LDFLAGS
#   Extra flags to give to compilers when they are supposed to invoke the linker, ‘ld’, such as -L.
#   Libraries (-lfoo) should be added to the LDLIBS variable instead.
# CC
#   Program for compiling C programs; default ‘cc’.

# $@
# The file name of the target of the rule. If the target is an archive member, then ‘$@’ is the name
# of the archive file. In a pattern rule that has multiple targets (see Introduction to Pattern
# Rules), ‘$@’ is the name of whichever target caused the rule’s recipe to be run.
#
# $^
# The names of all the prerequisites, with spaces between them. For prerequisites which are archive
# members, only the named member is used (see Using make to Update Archive Files). A target has only
# one prerequisite on each other file it depends on, no matter how many times each file is listed as
# a prerequisite. So if you list a prerequisite more than once for a target, the value of $^
# contains just one copy of the name. This list does not contain any of the order-only
# prerequisites; for those see the ‘$|’ variable, below.
chibicc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJS): chibicc.h

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
