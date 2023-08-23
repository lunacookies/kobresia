#include "all.h"

enum {
	MAX_PKGS = 1 << 16,
	MAX_FILES = 1 << 18,
	NAME_LEN = 16,
	PATH_LEN = 128,
	MAX_ENTITIES = MAX_PKGS + MAX_FILES,
	MAX_NAMES_BYTES = NAME_LEN * (MAX_FILES + MAX_PKGS),
	MAX_PATHS_BYTES = PATH_LEN * (MAX_FILES + MAX_PKGS),
};

struct project
discover_project(struct mem *m)
{
	char *names = alloc(&m->temp, char, MAX_NAMES_BYTES);
	u32 *name_starts = alloc(&m->temp, u32, MAX_ENTITIES);

	char *paths = alloc(&m->temp, char, MAX_PATHS_BYTES);
	u32 *path_starts = alloc(&m->temp, u32, MAX_ENTITIES);

	u32 *pkg_first_files = alloc(&m->temp, u32, MAX_PKGS);
	u32 *pkg_file_counts = alloc(&m->temp, u32, MAX_PKGS);
	u32 *pkg_ids = alloc(&m->temp, u32, MAX_PKGS);

	usize entities_count = 0;
	usize pkg_count = 0;

	usize names_bytes = 0;
	usize paths_bytes = 0;

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
		u32 first_file = 0;
		u32 file_count = 0;

		while (true) {
			struct dirent *file_entry = readdir(pkg);
			if (file_entry == NULL) {
				break;
			}

			if (file_entry->d_type != DT_REG) {
				continue;
			}

			if (file_count == 0) {
				first_file = entities_count;
			}
			file_count++;

			name_starts[entities_count] = names_bytes;
			memcpy(&names[names_bytes], file_entry->d_name,
			        file_entry->d_namlen);
			names_bytes += file_entry->d_namlen;

			path_starts[entities_count] = paths_bytes;

			memcpy(&paths[paths_bytes], pkg_entry->d_name,
			        pkg_entry->d_namlen);
			paths_bytes += pkg_entry->d_namlen;

			paths[paths_bytes] = '/';
			paths_bytes++;

			memcpy(&paths[paths_bytes], file_entry->d_name,
			        file_entry->d_namlen);
			paths_bytes += file_entry->d_namlen;

			entities_count++;
		}

		if (file_count == 0) {
			continue;
		}

		name_starts[entities_count] = names_bytes;
		memcpy(&names[names_bytes], pkg_entry->d_name,
		        pkg_entry->d_namlen);
		names_bytes += pkg_entry->d_namlen;

		path_starts[entities_count] = paths_bytes;
		memcpy(&paths[paths_bytes], pkg_entry->d_name,
		        pkg_entry->d_namlen);
		paths_bytes += pkg_entry->d_namlen;

		pkg_first_files[pkg_count] = first_file;
		pkg_file_counts[pkg_count] = file_count;
		pkg_ids[pkg_count] = entities_count;

		entities_count++;
		pkg_count++;

		assert(entities_count <= MAX_ENTITIES);
		assert(pkg_count <= MAX_PKGS);
		assert(names_bytes < MAX_NAMES_BYTES);
		assert(paths_bytes < MAX_PATHS_BYTES);
	}

	name_starts[entities_count] = names_bytes;
	path_starts[entities_count] = paths_bytes;

	return (struct project){
		.names = names,
		.name_starts = name_starts,
		.paths = paths,
		.path_starts = path_starts,
		.pkg_first_files = pkg_first_files,
		.pkg_file_counts = pkg_file_counts,
		.pkg_ids = pkg_ids,
		.pkg_count = pkg_count,
	};
}
