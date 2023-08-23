#include "all.h"

static void
work(u32 i, void *arg)
{
	printf("[%d] doing work with arg %p\n", i, arg);

	struct timespec ts = { .tv_sec = 1, .tv_nsec = 0 };
	nanosleep(&ts, NULL);

	printf("[%d] done\n", i);
}

int
main(void)
{
	u32 c = core_count();
	struct proc_mem pm = alloc_proc_mem(c);
	struct diagnostics_store diagnostics_store =
	        create_diagnostics_store(&pm.main);

	struct project proj = discover_project(&pm.main);

	for (u32 i = 0; i < proj.pkg_count; i++) {
		u32 pkg_id = proj.pkg_ids[i];

		u32 name_start = proj.name_starts[pkg_id];
		u32 name_end = proj.name_starts[pkg_id + 1];
		u32 name_length = name_end - name_start;
		char *name = &proj.names[name_start];

		u32 path_start = proj.path_starts[pkg_id];
		u32 path_end = proj.path_starts[pkg_id + 1];
		u32 path_length = path_end - path_start;
		char *path = &proj.paths[path_start];

		printf("\nname: %.*s\n", name_length, name);
		printf("path: %.*s\n", path_length, path);

		u32 file_count = proj.pkg_file_counts[i];
		u32 first_file = proj.pkg_first_files[i];
		for (u32 j = 0; j < file_count; j++) {
			u32 file_id = first_file + j;

			u32 file_name_start = proj.name_starts[file_id];
			u32 file_name_end = proj.name_starts[file_id + 1];
			u32 file_name_length = file_name_end - file_name_start;
			char *file_name = &proj.names[file_name_start];

			u32 file_path_start = proj.path_starts[file_id];
			u32 file_path_end = proj.path_starts[file_id + 1];
			u32 file_path_length = file_path_end - file_path_start;
			char *file_path = &proj.paths[file_path_start];

			printf("\n\tname: %.*s\n", file_name_length, file_name);
			printf("\tpath: %.*s\n", file_path_length, file_path);
		}
	}

	struct pool *p = start_pool(&pm.main, c, QOS_CLASS_UTILITY);
	void **args = alloc(&pm.main.perm, void *, c);
	for (u32 i = 0; i < c; i++) {
		args[i] = (void *)(0xdeadbeef + i);
	}
	// execute(p, work, args);
	// execute(p, work, args);
}
