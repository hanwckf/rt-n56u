# Script used in producing headers of assembly constants from C expressions.
# The input to this script looks like:
#	#cpp-directive ...
#	NAME1
#	NAME2 expression ...
# The output of this script is C code to be run through gcc -S and then
# massaged to extract the integer constant values of the given C expressions.
# A line giving just a name implies an expression consisting of just that name.

BEGIN { started = 0 }

# cpp directives go straight through.
/^#/ { print; next }

NF >= 1 && !started {
  printf "void dummy(void);\n";
  print "void dummy(void) {";
  started = 1;
}

# Separator.
$1 == "--" { next }

NF == 1 { sub(/^.*$/, "& &"); }

NF > 1 {
  name = $1;
  sub(/^[^ 	]+[ 	]+/, "");
  printf "__asm__ (\"@@@name@@@%s@@@value@@@%%0@@@end@@@\" : : \"i\" ((long) %s));\n",
    name, $0;
}

END { if (started) print "}" }
