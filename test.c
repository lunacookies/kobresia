#include "all.h"

#if DEVELOP

#define DIVIDER (S("===\n"))

typedef struct str (*test_fn)(struct mem *m, struct str input);

static void
run_component_tests(struct mem *m, struct str path, test_fn f)
{
	DIR *d = opendir(cast(char *) path.p);

	while (true) {
		struct dirent *entry = readdir(d);
		if (entry == NULL) {
			break;
		}
		if (entry->d_type != DT_REG) {
			continue;
		}

		struct arena_temp t = arena_temp_begin(&m->temp);

		// Copy together a path to this file.

		// + 1 for slash
		// + 1 for null terminator
		usize entry_path_length = path.n + 1 + entry->d_namlen + 1;
		char *entry_path = alloc(&m->temp, char, entry_path_length);

		memcpy(entry_path, path.p, path.n);
		entry_path[path.n] = '/';
		memcpy(entry_path + path.n + 1, entry->d_name, entry->d_namlen);
		entry_path[path.n + 1 + entry->d_namlen] = '\0';

		// Next. we read the contents of the file.

		i32 fd = open(entry_path, O_RDONLY);
		struct stat s = { 0 };
		fstat(fd, &s);
		usize size = cast(usize) s.st_size;

		u8 *content = alloc(&m->temp, u8, size + 1);
		read(fd, content, size);
		content[size] = '\0';

		// Find where the divider between input source and expect
		// output is located in the file contents.

		char *divider_p =
		        strstr(cast(char *) content, cast(char *) DIVIDER.p);
		if (divider_p == NULL) {
			fprintf(stderr, "test %s has no divider.\n",
			        entry_path);
			continue;
		}

		usize divider_pos = cast(usize)(cast(u8 *) divider_p - content);
		struct str input = str_make(content, divider_pos);

		u8 *expect_start = content + divider_pos + DIVIDER.n;
		usize expect_length = size - divider_pos - DIVIDER.n;
		struct str expect_output =
		        str_make(expect_start, expect_length);

		struct str actual_output = f(m, input);

		if (expect_output.n == actual_output.n &&
		        memcmp(expect_output.p, actual_output.p,
		                expect_output.n) == 0) {
			printf("*** pass - %s\n", entry_path);
			continue;
		}

		printf("*** fail - %s\n", entry_path);
		printf("expect (%zu):\n%.*s\n", expect_output.n,
		        cast(int) expect_output.n, expect_output.p);
		printf("actual (%zu):\n%.*s\n", actual_output.n,
		        cast(int) actual_output.n, actual_output.p);

		arena_temp_end(t);
	}
}

void
run_tests(struct mem *m)
{
	run_component_tests(m, S("test_data_lexer"), lex_test);
}

#endif
