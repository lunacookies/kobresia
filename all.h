#include <assert.h>
#include <dirent.h>
#include <pthread.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <unistd.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;

#define KIBIBYTE (1024)
#define MEBIBYTE (1024 * KIBIBYTE)
#define GIBIBYTE (1024 * MEBIBYTE)

// s.c

struct s {
	u8 *top, *p;
	usize n;
};
#define sstr(str)                                                              \
	((struct s){ .top = (u8 *)(str), .p = (u8 *)(str), .n = strlen(str) })

struct s create_s(u8 *p, usize n);
void split_off(struct s *s, usize n, struct s *fst, struct s *snd);

void *_alloc(struct s *s, usize size, usize align);
#define alloc(s, t, n) (_alloc((s), sizeof(t) * (n), alignof(t)))
struct s alloc_s(struct s *s, usize size);
void alloc_copy(struct s *s, void *data, usize size, usize align);
usize s_used(struct s *s);

// early_death.c

void early_death(struct s msg);

// mem.c

struct mem {
	struct s perm;
	struct s temp;
};

struct proc_mem {
	struct mem main;
	struct mem *workers;
};

struct s os_alloc(usize nbytes);
struct proc_mem alloc_proc_mem(u32 core_count);

// thread.c

u32 core_count(void);

struct barrier {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	u32 n;
	u32 threads;
};

struct barrier barrier_create(u32 n);
void barrier_wait(struct barrier *b);

typedef void (*job)(u32, void *);
struct pool {
	job *jobs;
	void **args;
	struct barrier ready;
	struct barrier done;
	u32 count;
};

struct pool *start_pool(struct mem *m, u32 core_count, qos_class_t qos);
void execute(struct pool *p, job j, void **args);

// diagnostics.c

enum severity {
	ERROR,
	WARNING,
};

struct diagnostics_store {
	enum severity *sev;
	char *msg;
	u32 *msglen;
	usize count;
};

struct diagnostics_store create_diagnostics_store(struct mem *m);

// project.c

struct project {
	char *names;
	u32 *name_starts;

	char *paths;
	u32 *path_starts;

	u32 *pkg_first_files;
	u32 *pkg_file_counts;
	u32 *pkg_ids;
	u32 pkg_count;
};

struct project discover_project(struct mem *m);
