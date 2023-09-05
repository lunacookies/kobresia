#include "all.h"

#if DEVELOP

#define DIVIDER (sstr("===\n"))

typedef struct s (*test_fn)(struct mem *m, struct s input);

static void
run_component_tests(struct mem *m, struct s path, test_fn f)
{
	DIR *d = opendir((char *)path.p);

	while (true) {
		struct dirent *entry = readdir(d);
		if (entry == NULL) {
			break;
		}
		if (entry->d_type != DT_REG) {
			continue;
		}

		struct s_temp t = s_temp_begin(&m->temp);

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
		usize size = (usize)s.st_size;

		u8 *content = alloc(&m->temp, u8, size + 1);
		read(fd, content, size);
		content[size] = '\0';

		// Find where the divider between input source and expect
		// output is located in the file contents.

		char *divider_p = strstr((char *)content, (char *)DIVIDER.p);
		if (divider_p == NULL) {
			fprintf(stderr, "test %s has no divider.\n",
			        entry_path);
			continue;
		}

		usize divider_pos = (usize)((u8 *)divider_p - content);
		struct s input = create_s_full(content, divider_pos);

		u8 *expect_start = content + divider_pos + DIVIDER.n;
		usize expect_length = size - divider_pos - DIVIDER.n;
		struct s expect_output =
		        create_s_full(expect_start, expect_length);

		struct s actual_output = f(m, input);

		if (expect_output.n == actual_output.n &&
		        memcmp(expect_output.p, actual_output.p,
		                expect_output.n) == 0) {
			printf("*** pass - %s\n", entry_path);
			continue;
		}

		printf("*** fail - %s\n", entry_path);
		printf("expect (%zu):\n%.*s\n", expect_output.n,
		        (int)expect_output.n, expect_output.p);
		printf("actual (%zu):\n%.*s\n", actual_output.n,
		        (int)actual_output.n, actual_output.p);

		s_temp_end(t);
	}
}

void
run_tests(struct mem *m)
{
	run_component_tests(m, sstr("test_data_lexer"), lex_test);
}

#endif