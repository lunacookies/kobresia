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
		struct str entry_name =
		        str_make(entry->d_name, entry->d_namlen);

		// Copy together a path to this file.

		// + 1 for slash
		// + 1 for null terminator
		usize entry_path_length = path.n + 1 + entry->d_namlen + 1;
		struct str entry_path =
		        alloc_str(&m->temp, entry_path_length, 1);
		struct strbuilder entry_path_builder;
		strbuilder_init(&entry_path_builder, entry_path);
		strbuilder_push(&entry_path_builder, path);
		strbuilder_byte(&entry_path_builder, '/');
		strbuilder_push(&entry_path_builder, entry_name);
		strbuilder_byte(&entry_path_builder, 0);
		assert(entry_path_builder.used == entry_path_length);
		entry_path = strbuilder_done(&entry_path_builder);

		// Next. we read the contents of the file.

		i32 fd = open(cast(char *) entry_path.p, O_RDONLY);
		struct stat s;
		fstat(fd, &s);
		usize size = cast(usize) s.st_size;

		// + 1 for null terminator
		struct str content = alloc_str(&m->temp, size + 1, 1);
		read(fd, content.p, size);
		content = str_prefix(content, size); // cut off the null

		// Find where the divider between input source and expect
		// output is located in the file contents.

		u8 *divider_p = cast(u8 *)
		        strstr(cast(char *) content.p, cast(char *) DIVIDER.p);
		if (divider_p == NULL) {
			fprintf(stderr, "test %.*s has no divider.\n",
			        cast(int) entry_path.n, entry_path.p);
			continue;
		}

		usize divider_pos = cast(usize)(divider_p - content.p);
		struct str input = str_prefix(content, divider_pos);
		struct str expect_output =
		        str_suffix(content, divider_pos + DIVIDER.n);

		struct str actual_output = f(m, input);

		if (str_equal(expect_output, actual_output)) {
			printf("*** pass - %.*s\n", cast(int) entry_path.n,
			        entry_path.p);
			continue;
		}

		printf("*** fail - %.*s\n", cast(int) entry_path.n,
		        entry_path.p);
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
