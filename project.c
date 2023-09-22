#include "all.h"

enum {
	MAX_PKGS = 1 << 16,
	MAX_FILES = 1 << 18,
	NAME_LEN = 16,
	PATH_LEN = 128,
	MAX_ENTITIES = MAX_PKGS + MAX_FILES,
};

void
project_search(struct project *p, struct mem *m)
{
	struct arena_temp t = arena_temp_begin(&m->temp);

	struct arena file_names = { 0 };
	struct arena file_paths = { 0 };
	struct arena pkg_names = { 0 };
	struct arena pkg_paths = { 0 };

	alloc_arena(&m->temp, &file_names, NAME_LEN * MAX_FILES, 1);
	alloc_arena(&m->temp, &file_paths, PATH_LEN * MAX_FILES, 1);
	alloc_arena(&m->temp, &pkg_names, NAME_LEN * MAX_PKGS, 1);
	alloc_arena(&m->temp, &pkg_paths, PATH_LEN * MAX_PKGS, 1);

	u32 *file_name_starts = alloc(&m->temp, u32, MAX_FILES);
	u32 *file_path_starts = alloc(&m->temp, u32, MAX_FILES);
	u32 *pkg_name_starts = alloc(&m->temp, u32, MAX_PKGS);
	u32 *pkg_path_starts = alloc(&m->temp, u32, MAX_PKGS);

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

			file_name_starts[file_count] = (u32)file_names.used;
			alloc_copy(&file_names, u8, file_entry->d_name,
			        file_entry->d_namlen);

			file_path_starts[file_count] = (u32)file_paths.used;

			alloc_copy(&file_paths, u8, pkg_entry->d_name,
			        pkg_entry->d_namlen);

			u8 slash = '/';
			alloc_copy(&file_paths, u8, &slash, 1);

			alloc_copy(&file_paths, u8, file_entry->d_name,
			        file_entry->d_namlen);

			u8 null = 0;
			alloc_copy(&file_paths, u8, &null, 1);

			file_pkgs[file_count] = pkg_count;

			file_count++;
		}

		if (files_in_pkg == 0) {
			continue;
		}

		assert(pkg_count < MAX_PKGS);

		pkg_name_starts[pkg_count] = (u32)pkg_names.used;
		alloc_copy(
		        &pkg_names, u8, pkg_entry->d_name, pkg_entry->d_namlen);

		pkg_path_starts[pkg_count] = (u32)pkg_paths.used;
		alloc_copy(
		        &pkg_paths, u8, pkg_entry->d_name, pkg_entry->d_namlen);

		u8 null = 0;
		alloc_copy(&pkg_paths, u8, &null, 1);

		pkg_first_files[pkg_count] = first_file_in_pkg;
		pkg_file_counts[pkg_count] = files_in_pkg;

		pkg_count++;
	}

	file_name_starts[file_count] = (u32)file_names.used;
	file_path_starts[file_count] = (u32)file_paths.used;
	pkg_name_starts[pkg_count] = (u32)pkg_names.used;
	pkg_path_starts[pkg_count] = (u32)pkg_paths.used;

	p->file_count = file_count;
	p->pkg_count = pkg_count;

	p->file_names = alloc_copy_str(
	        &m->perm, str_make(file_names.buf.p, file_names.used), 1);
	p->file_paths = alloc_copy_str(
	        &m->perm, str_make(file_paths.buf.p, file_paths.used), 1);
	p->pkg_names = alloc_copy_str(
	        &m->perm, str_make(pkg_names.buf.p, pkg_names.used), 1);
	p->pkg_paths = alloc_copy_str(
	        &m->perm, str_make(pkg_paths.buf.p, pkg_paths.used), 1);

	// the “starts” arrays also include an end
	p->file_name_starts =
	        alloc_copy(&m->perm, u32, file_name_starts, file_count + 1);
	p->file_path_starts =
	        alloc_copy(&m->perm, u32, file_path_starts, file_count + 1);
	p->pkg_name_starts =
	        alloc_copy(&m->perm, u32, pkg_name_starts, pkg_count + 1);
	p->pkg_path_starts =
	        alloc_copy(&m->perm, u32, pkg_path_starts, pkg_count + 1);

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
