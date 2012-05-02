#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void)
{
	FILE *lmm;					/* mmap-able file */
	FILE *lso;					/* static object */
	struct stat fd_stat;
	int c;
	size_t i;

	if (!(lmm = fopen("locale.mmap", "r"))) {
		printf("can't open locale.mmap!\n");
		return EXIT_FAILURE;
	}

	if (fstat(fileno(lmm), &fd_stat)) {
		printf("can't stat locale.mmap!\n");
		fclose(lmm);
		return EXIT_FAILURE;
	}

	if (!(lso = fopen("locale_data.c", "w"))) {
		printf("can't open locale_data.c!\n");
		fclose(lmm);
		return EXIT_FAILURE;
	}

	fprintf(lso,
			"#include <stddef.h>\n"
			"#include <stdint.h>\n"
			"#include \"lt_defines.h\"\n"
			"#include \"locale_mmap.h\"\n\n"
			"typedef union {\n"
			"\tunsigned char buf[%zu];\n"
			"\t__locale_mmap_t lmm;\n"
			"} locale_union_t;\n\n"
			"static const locale_union_t locale_union = { {",
			(size_t) fd_stat.st_size
			);

	i = 0;
	while ((c = getc(lmm)) != EOF) {
		if (!(i & 0x7)) {
			fprintf(lso, "\n\t");
		}
		fprintf(lso, "%#04x, ", c);
		++i;
	}
	fprintf(lso,
			"\n} };\n\n"
			"const __locale_mmap_t *__locale_mmap = &locale_union.lmm;\n\n"
			);

	if (ferror(lmm)) {
		printf("error reading!\n");
		return EXIT_FAILURE;
	}

	if (ferror(lso) || fclose(lso)) {
		printf("error writing!\n");
		return EXIT_FAILURE;
	}

	fclose(lmm);

	return EXIT_SUCCESS;
}
