objres is a utility that "compiles" arbitrary binary data as object
files. Given a file such as a png image, objres will create a .o file
that exports an object initialized to that file's contents, allowing
it to be accessed inside a program as an extern variable. (For
convenience, objres also produces a simple .h file.) The variable name
can be set on the command-line.

Occasionally the need arises to compile binary data into a program
directly, such as an icon image. Frequently this is done either via
gas, with a short assembly source file that uses the .incbin
directive, or else by using a short Perl script to translate the
binary file into C, as a char array initialization. Both of these
approaches are a bit roundabout, given that it would be far more
straightforward to just wrap an object file around the binary data
directly. If Linux had historically had a standard tool to do this, it
might look something like objres.

Since objres doesn't produce any .text segments, all it needs in the
way of ELF structures is a symbol table and string table. Well, that
plus the usual ELF header, and section header table with its own
string table, which every object file needs to have.

Even ELF files as simple as the ones objres creates have
interdependencies. The string table has to be laid out before the
symbol table can be filled in, and the number of local and global
symbols needs to be known before the section header table can be
completed. (And of course the section header table has its own string
table, which needs to be laid out beforehand as well.) And finally the
ELF header needs to know the location of the section header table, not
to mention the index of its string table.

objres removes some of this interdependency by always laying out the
ELF file in the same fashion, with the constant-sized ELF header and
section header table at the top of the file, and the variably-sized
string tables and .data section coming at the end. Once the program
has examined the input files, it can determine how big each part of
the ELF image will be, and thus can calculate all the necessary
offsets and indexes to fill in the contents of all the structures.
