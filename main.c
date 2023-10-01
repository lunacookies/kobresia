#include "all.h"

static void
work(imm i, void *arg)
{
	printf("[%td] doing work with arg %p\n", i, arg);

	struct timespec ts;
	ts.tv_sec = 1;
	ts.tv_nsec = 0;
	nanosleep(&ts, NULL);

	printf("[%td] done\n", i);
}

i32
main(i32 argc, char **argv)
{
	imm c = core_count();
	struct proc_mem pm;
	proc_mem_alloc(&pm, c);

#if DEVELOP
	if (argc == 2 && strcmp(argv[1], "--test") == 0) {
		run_tests(&pm.main);
		return 0;
	}
#endif

	struct diagnostics_store diagnostics_store;
	diagnostics_store_init(&diagnostics_store, &pm.main);

	struct project proj;
	project_search(&proj, &pm.main);

	printf("found %td packages\nfound %td files\n", proj.pkg_count,
	        proj.file_count);

	for (imm pkg_id = 0; pkg_id < proj.pkg_count; pkg_id++) {
		struct str name = project_pkg_name(&proj, pkg_id);
		struct str path = project_pkg_path(&proj, pkg_id);

		printf("\n");
		printf("id:   %td\n", pkg_id);
		printf("name: (%td) %.*s\n", name.n, cast(i32) name.n, name.p);
		printf("path: (%td) %.*s\n", path.n, cast(i32) path.n, path.p);

		imm file_count = proj.pkg_file_counts[pkg_id];
		imm first_file = proj.pkg_first_files[pkg_id];
		imm end = first_file + file_count;

		for (imm file_id = first_file; file_id < end; file_id++) {
			struct str file_name =
			        project_file_name(&proj, file_id);
			struct str file_path =
			        project_file_path(&proj, file_id);

			printf("\n");
			printf("\tid:   %td\n", file_id);
			printf("\tpkg:  %d\n", proj.file_pkgs[file_id]);
			printf("\tname: (%td) %.*s\n", file_name.n,
			        cast(i32) file_name.n, file_name.p);
			printf("\tpath: (%td) %.*s\n", file_path.n,
			        cast(i32) file_path.n, file_path.p);
		}
	}

	struct pool *p = pool_start(&pm.main, c, QOS_CLASS_UTILITY);
	void **args = alloc(&pm.main.perm, void *, c);
	for (imm i = 0; i < c; i++) {
		args[i] = cast(void *)(0xdeadbeef + i);
	}
	pool_sched(p, work, args);
	pool_sched(p, work, args);
}
