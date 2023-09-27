#include "all.h"

enum {
	MAX_PKGS = 1 << 16,
	MAX_FILES = 1 << 18,
	NAME_LEN = 16,
	PATH_LEN = 128,
	MAX_ENTITIES = MAX_PKGS + MAX_FILES,
};

// Dense append-only storage for many strings.
struct dense {
	struct strbuilder data;
	u32 *starts;
	usize count;
};

static void
dense_init(struct dense *d, struct mem *m, usize count, usize elem_len)
{
	struct str data_buf = alloc_str(&m->temp, elem_len * count, 1);
	strbuilder_init(&d->data, data_buf);
	d->starts = alloc(&m->temp, u32, count + 1);
	d->count = 0;
}

static struct strbuilder *
dense_push(struct dense *d)
{
	d->starts[d->count] = cast(u32) d->data.used;
	d->count++;
	return &d->data;
}

static void
dense_finish(struct dense *d, struct mem *m, char **data, u32 **starts)
{
	d->starts[d->count] = cast(u32) d->data.used;

	// Copy data from temp memory into permanent memory.
	struct str in_temp = strbuilder_done(&d->data);
	struct str in_perm = alloc_copy_str(&m->perm, in_temp, 1);
	*data = cast(char *) in_perm.p;

	*starts = alloc_copy(&m->perm, u32, d->starts, d->count + 1);
}

void
project_search(struct project *p, struct mem *m)
{
	struct arena_temp t = arena_temp_begin(&m->temp);

	struct dense file_names = { 0 };
	struct dense file_paths = { 0 };
	struct dense pkg_names = { 0 };
	struct dense pkg_paths = { 0 };

	dense_init(&file_names, m, MAX_FILES, NAME_LEN);
	dense_init(&file_paths, m, MAX_FILES, PATH_LEN);
	dense_init(&pkg_names, m, MAX_PKGS, NAME_LEN);
	dense_init(&pkg_paths, m, MAX_PKGS, PATH_LEN);

	u32 *file_pkgs = alloc(&m->temp, u32, MAX_FILES);

	u32 *pkg_first_files = alloc(&m->temp, u32, MAX_PKGS);
	u32 *pkg_file_counts = alloc(&m->temp, u32, MAX_PKGS);

	u32 file_count = 0;
	u32 pkg_count = 0;

	DIR *root = opendir(".");

	while (true) {
		struct dirent *pkg_entry = readdir(root);
		if (pkg_entry == NULL) {
			break;
		}

		if (pkg_entry->d_type != DT_DIR) {
			continue;
		}

		struct str pkg_name =
		        str_make(pkg_entry->d_name, pkg_entry->d_namlen);

		DIR *pkg = opendir(pkg_entry->d_name);
		u32 first_file_in_pkg = 0;
		u32 files_in_pkg = 0;

		while (true) {
			struct dirent *file_entry = readdir(pkg);
			if (file_entry == NULL) {
				break;
			}

			if (file_entry->d_type != DT_REG) {
				continue;
			}

			if (file_entry->d_namlen < 4) {
				continue;
			}

			char expected_ext[3] = ".kb";
			char *actual_ext =
			        file_entry->d_name + file_entry->d_namlen - 3;

			if (memcmp(expected_ext, actual_ext, 3) != 0) {
				continue;
			}

			if (files_in_pkg == 0) {
				first_file_in_pkg = file_count;
			}
			files_in_pkg++;

			assert(file_count < MAX_FILES);

			struct str file_name = str_make(
			        file_entry->d_name, file_entry->d_namlen);

			struct strbuilder *name = dense_push(&file_names);
			strbuilder_push(name, file_name);

			struct strbuilder *path = dense_push(&file_paths);
			strbuilder_push(path, pkg_name);
			strbuilder_byte(path, '/');
			strbuilder_push(path, file_name);
			strbuilder_byte(path, 0);

			file_pkgs[file_count] = pkg_count;

			file_count++;
		}

		if (files_in_pkg == 0) {
			continue;
		}

		assert(pkg_count < MAX_PKGS);

		struct strbuilder *name = dense_push(&pkg_names);
		strbuilder_push(name, pkg_name);

		struct strbuilder *path = dense_push(&pkg_paths);
		strbuilder_push(path, pkg_name);
		strbuilder_byte(path, 0);

		pkg_first_files[pkg_count] = first_file_in_pkg;
		pkg_file_counts[pkg_count] = files_in_pkg;

		pkg_count++;
	}

	dense_finish(&file_names, m, &p->file_names, &p->file_name_starts);
	dense_finish(&file_paths, m, &p->file_paths, &p->file_path_starts);
	dense_finish(&pkg_names, m, &p->pkg_names, &p->pkg_name_starts);
	dense_finish(&pkg_paths, m, &p->pkg_paths, &p->pkg_path_starts);

	p->file_count = file_count;
	p->pkg_count = pkg_count;

	p->file_pkgs = alloc_copy(&m->perm, u32, file_pkgs, file_count);

	p->pkg_first_files =
	        alloc_copy(&m->perm, u32, pkg_first_files, pkg_count);
	p->pkg_file_counts =
	        alloc_copy(&m->perm, u32, pkg_file_counts, pkg_count);

	arena_temp_end(t);
}

struct str
project_file_name(struct project *p, u32 id)
{
	assert(id < p->file_count);
	u32 start = p->file_name_starts[id];
	u32 end = p->file_name_starts[id + 1];
	u32 length = end - start;
	return str_make(&p->file_names[start], length);
}

struct str
project_file_path(struct project *p, u32 id)
{
	assert(id < p->file_count);
	u32 start = p->file_path_starts[id];
	u32 end = p->file_path_starts[id + 1] - 1; // remove null terminator
	u32 length = end - start;
	return str_make(&p->file_paths[start], length);
}

struct str
project_pkg_name(struct project *p, u32 id)
{
	assert(id < p->pkg_count);
	u32 start = p->pkg_name_starts[id];
	u32 end = p->pkg_name_starts[id + 1];
	u32 length = end - start;
	return str_make(&p->pkg_names[start], length);
}

struct str
project_pkg_path(struct project *p, u32 id)
{
	assert(id < p->pkg_count);
	u32 start = p->pkg_path_starts[id];
	u32 end = p->pkg_path_starts[id + 1] - 1; // remove null terminator
	u32 length = end - start;
	return str_make(&p->pkg_paths[start], length);
}
