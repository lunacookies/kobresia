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
main(int argc, char **argv)
{
	u32 c = core_count();
	struct proc_mem pm = alloc_proc_mem(c);

#if DEVELOP
	if (argc == 2 && strcmp(argv[1], "--test") == 0) {
		run_tests(&pm.main);
		return 0;
	}
#endif

	struct diagnostics_store diagnostics_store =
	        create_diagnostics_store(&pm.main);

	struct project proj = discover_project(&pm.main);

	printf("found %d packages\nfound %d files\n", proj.pkg_count,
	        proj.file_count);

	for (u32 pkg_id = 0; pkg_id < proj.pkg_count; pkg_id++) {
		struct s name = project_pkg_name(&proj, pkg_id);
		struct s path = project_pkg_path(&proj, pkg_id);

		printf("\n");
		printf("id:   %d\n", pkg_id);
		printf("name: (%zu) %.*s\n", name.n, (int)name.n, name.p);
		printf("path: (%zu) %.*s\n", path.n, (int)path.n, path.p);

		u32 file_count = proj.pkg_file_counts[pkg_id];
		u32 first_file = proj.pkg_first_files[pkg_id];
		u32 end = first_file + file_count;

		for (u32 file_id = first_file; file_id < end; file_id++) {
			struct s file_name = project_file_name(&proj, file_id);
			struct s file_path = project_file_path(&proj, file_id);

			printf("\n");
			printf("\tid:   %d\n", file_id);
			printf("\tpkg:  %d\n", proj.file_pkgs[file_id]);
			printf("\tname: (%zu) %.*s\n", file_name.n,
			        (int)file_name.n, file_name.p);
			printf("\tpath: (%zu) %.*s\n", file_path.n,
			        (int)file_path.n, file_path.p);
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
