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

#define cast(t) (t)

// str.c

struct str {
	u8 *p;
	usize n;
};
struct str str_make(void *p, usize n);
#define S(str) (str_make((str), sizeof(str) - 1))

// strbuilder.c

struct strbuilder {
	struct str buf;
	usize used;
};

void strbuilder_init(struct strbuilder *sb, struct str buf);
struct str strbuilder_done(struct strbuilder *sb);

void strbuilder_byte(struct strbuilder *sb, u8 b);
void strbuilder_push(struct strbuilder *sb, struct str s);
void strbuilder_printf(struct strbuilder *sb, char *fmt, ...);

// arena.c

struct arena {
	struct str buf;
	usize used, peak_used, temp_count;
};

void arena_init(struct arena *a, struct str buf);

void *_alloc(struct arena *a, usize size, usize align);
void *_alloc_u(struct arena *a, usize size, usize align);
#define alloc(a, t, n) (cast(t *) _alloc((a), sizeof(t) * (n), alignof(t)))
#define alloc_u(a, t, n) (cast(t *) _alloc_u((a), sizeof(t) * (n), alignof(t)))
struct str alloc_str(struct arena *a, usize size, usize align);
struct str alloc_str_u(struct arena *a, usize size, usize align);

void alloc_arena(struct arena *a, struct arena *out, usize size);

void *_alloc_copy(struct arena *a, void *data, usize size, usize align);
#define alloc_copy(a, t, v, n)                                                 \
	(cast(t *) _alloc_copy((a), (v), sizeof(t) * (n), alignof(t)))
struct str alloc_copy_str(struct arena *a, struct str from, usize align);

struct arena_temp {
	struct arena *a;
	usize used;
};

struct arena_temp arena_temp_begin(struct arena *a);
void arena_temp_end(struct arena_temp t);

// early_death.c

void early_death(struct str msg);

// mem.c

struct mem {
	struct arena perm;
	struct arena temp;
};

struct proc_mem {
	struct mem main;
	struct mem *workers;
};

struct str os_alloc(usize nbytes);
void proc_mem_alloc(struct proc_mem *pm, u32 core_count);

// thread.c

u32 core_count(void);

struct barrier {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	u32 n;
	u32 threads;
};

void barrier_init(struct barrier *b, u32 n);
void barrier_wait(struct barrier *b);

typedef void (*job)(u32, void *);
struct pool {
	job *jobs;
	void **args;
	struct barrier ready;
	struct barrier done;
	u32 count;
};

struct pool *pool_start(struct mem *m, u32 core_count, qos_class_t qos);
void pool_sched(struct pool *p, job j, void **args);

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

void diagnostics_store_init(struct diagnostics_store *d, struct mem *m);

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

void project_search(struct project *p, struct mem *m);

struct str project_file_name(struct project *p, u32 id);
struct str project_file_path(struct project *p, u32 id);
struct str project_pkg_name(struct project *p, u32 id);
struct str project_pkg_path(struct project *p, u32 id);

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

struct str token_kind_name(enum token_kind k);
void lex(struct tokens *t, struct mem *m, struct str input);

#if DEVELOP
struct str lex_test(struct mem *m, struct str input);
#endif

// test.c

#if DEVELOP
void run_tests(struct mem *m);
#endif
