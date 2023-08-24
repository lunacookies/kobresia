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

		struct s name = project_entity_name(&proj, pkg_id);
		struct s path = project_entity_path(&proj, pkg_id);

		printf("\nname: (%zu) %.*s\n", name.n, (int)name.n, name.p);
		printf("path: (%zu) %.*s\n", path.n, (int)path.n, path.p);

		u32 file_count = proj.pkg_file_counts[i];
		u32 first_file = proj.pkg_first_files[i];
		u32 end = first_file + file_count;

		for (u32 file_id = first_file; file_id < end; file_id++) {
			struct s file_name =
			        project_entity_name(&proj, file_id);
			struct s file_path =
			        project_entity_path(&proj, file_id);

			printf("\n\tname: (%zu) %.*s\n", file_name.n,
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
