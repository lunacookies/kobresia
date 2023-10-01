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
typedef ptrdiff_t imm;
typedef size_t umm;

#define KIBIBYTE (1024)
#define MEBIBYTE (1024 * KIBIBYTE)
#define GIBIBYTE (1024 * MEBIBYTE)

#define cast(t) (t)
#define size_of(t) (cast(imm) sizeof(t))
#define align_of(t) (cast(imm) alignof(t))

// base.c

struct str {
	u8 *p;
	imm n;
};
struct str str_make(void *p, imm n);
#define S(str) (str_make((str), size_of(str) - 1))

void str_copy(struct str dst, struct str src);
void str_fill(struct str s, u8 b);
void str_zero(struct str s);

struct str str_slice(struct str s, imm start, imm end);

bool str_equal(struct str s1, struct str s2);
bool str_all(struct str s, u8 b);

void _assert_zero(struct str);
#define assert_zero(p) (_assert_zero(str_make((p), size_of(*(p)))))
#define zero_out(p) (str_zero(str_make((p), size_of(*(p)))))

// strbuilder.c

struct strbuilder {
	struct str buf;
	imm used;
};

void strbuilder_init(struct strbuilder *sb, struct str buf);
struct str strbuilder_done(struct strbuilder *sb);

void strbuilder_byte(struct strbuilder *sb, u8 b);
void strbuilder_push(struct strbuilder *sb, struct str s);
void strbuilder_printf(struct strbuilder *sb, char *fmt, ...);

// arena.c

struct arena {
	struct str buf;
	imm used, peak_used, temp_count;
};

void arena_init(struct arena *a, struct str buf);

struct str alloc_str(struct arena *a, imm size, imm align);
struct str alloc_str_u(struct arena *a, imm size, imm align);
#define alloc(a, t, n)                                                         \
	(cast(t *) alloc_str((a), size_of(t) * (n), align_of(t)).p)
#define alloc_u(a, t, n)                                                       \
	(cast(t *) alloc_str_u((a), size_of(t) * (n), align_of(t)).p)

struct str alloc_copy_str(struct arena *a, struct str src, imm align);
void *_alloc_copy(struct arena *a, void *data, imm size, imm align);
#define alloc_copy(a, t, v, n)                                                 \
	(cast(t *) alloc_copy_str(                                             \
	        (a), str_make((v), size_of(t) * (n)), align_of(t))             \
	                .p)

struct arena_temp {
	struct arena *a;
	imm used;
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

struct str os_alloc(imm nbytes);
void proc_mem_alloc(struct proc_mem *pm, imm core_count);

// thread.c

imm core_count(void);

struct barrier {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	imm n;
	imm threads;
};

void barrier_init(struct barrier *b, imm n);
void barrier_wait(struct barrier *b);

typedef void (*job)(imm, void *);
struct pool {
	job *jobs;
	void **args;
	struct barrier ready;
	struct barrier done;
	imm count;
};

struct pool *pool_start(struct mem *m, imm core_count, qos_class_t qos);
void pool_sched(struct pool *p, job j, void **args);

// diagnostics.c

enum severity {
	ERROR,
	WARNING,
};

struct diagnostics_store {
	enum severity *sev;
	char *msg;
	i32 *msglen;
	imm count;
};

void diagnostics_store_init(struct diagnostics_store *d, struct mem *m);

// project.c

struct project {
	imm file_count;
	imm pkg_count;

	char *file_names;
	char *file_paths;
	i32 *file_name_starts;
	i32 *file_path_starts;
	i32 *file_pkgs;

	char *pkg_names;
	char *pkg_paths;
	i32 *pkg_name_starts;
	i32 *pkg_path_starts;
	i32 *pkg_first_files;
	i32 *pkg_file_counts;
};

void project_search(struct project *p, struct mem *m);

struct str project_file_name(struct project *p, imm id);
struct str project_file_path(struct project *p, imm id);
struct str project_pkg_name(struct project *p, imm id);
struct str project_pkg_path(struct project *p, imm id);

// lexer.c

enum token_kind {
	T_INVALID,
	T_IDENT,
	T_NUMBER,
};

struct span {
	i32 start, end;
};

struct tokens {
	enum token_kind *kinds;
	struct span *spans;
	imm count;
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
