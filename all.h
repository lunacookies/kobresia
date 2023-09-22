#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
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
	u8 *p;
	usize n;
};
struct s create_s(void *p, usize n);
#define sstr(str) (create_s((str), sizeof(str) - 1))

// arena.c

struct arena {
	struct s buf;
	usize used, temp_count;
};

void init_arena(struct arena *a, struct s buf);

void *_alloc(struct arena *a, usize size, usize align);
#define alloc(a, t, n) ((t *)_alloc((a), sizeof(t) * (n), alignof(t)))
struct s alloc_s(struct arena *a, usize size, usize align);
void alloc_arena(struct arena *a, struct arena *out, usize size, usize align);
void *_alloc_copy(struct arena *a, void *data, usize size, usize align);
#define alloc_copy(a, t, v, n)                                                 \
	((t *)_alloc_copy((a), (v), sizeof(t) * (n), alignof(t)))
void *alloc_copy_s(struct arena *a, struct s from, usize align);
void *alloc_copy_arena(struct arena *a, struct arena *from, usize align);
void arena_printf(struct arena *a, char *fmt, ...);

struct arena_temp {
	struct arena *a;
	usize used;
};

struct arena_temp arena_temp_begin(struct arena *a);
void arena_temp_end(struct arena_temp t);

// early_death.c

void early_death(struct s msg);

// mem.c

struct mem {
	struct arena perm;
	struct arena temp;
};

struct proc_mem {
	struct mem main;
	struct mem *workers;
};

struct s os_alloc(usize nbytes);
void alloc_proc_mem(struct proc_mem *pm, u32 core_count);

// thread.c

u32 core_count(void);

struct barrier {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	u32 n;
	u32 threads;
};

void barrier_create(struct barrier *b, u32 n);
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

void init_diagnostics_store(struct diagnostics_store *d, struct mem *m);

// project.c

struct project {
	u32 file_count;
	u32 pkg_count;

	char *file_names;
	char *file_paths;
	u32 *file_name_starts;
	u32 *file_path_starts;
	u32 *file_pkgs;

	char *pkg_names;
	char *pkg_paths;
	u32 *pkg_name_starts;
	u32 *pkg_path_starts;
	u32 *pkg_first_files;
	u32 *pkg_file_counts;
};

void discover_project(struct project *p, struct mem *m);

struct s project_file_name(struct project *p, u32 id);
struct s project_file_path(struct project *p, u32 id);
struct s project_pkg_name(struct project *p, u32 id);
struct s project_pkg_path(struct project *p, u32 id);

// lexer.c

enum token_kind {
	T_INVALID,
	T_IDENT,
	T_NUMBER,
};

struct span {
	u32 start, end;
};

struct tokens {
	enum token_kind *kinds;
	struct span *spans;
	usize count;
};

struct s token_kind_name(enum token_kind k);
void lex(struct tokens *t, struct mem *m, struct s input);

#if DEVELOP
struct s lex_test(struct mem *m, struct s input);
#endif

// test.c

#if DEVELOP
void run_tests(struct mem *m);
#endif
