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
	usize n, total, temp_count;
};
#define sstr(str) (create_s_full((u8 *)(str), sizeof(str) - 1))

struct s create_s(u8 *p, usize n);
struct s create_s_full(u8 *p, usize n);

void *_alloc(struct s *s, usize size, usize align);
#define alloc(s, t, n) ((t *)_alloc((s), sizeof(t) * (n), alignof(t)))
struct s alloc_s(struct s *s, usize size, usize align);
void *_alloc_copy(struct s *s, void *data, usize size, usize align);
#define alloc_copy(s, t, v, n)                                                 \
	((t *)_alloc_copy((s), (v), sizeof(t) * (n), alignof(t)))
void *alloc_copy_s(struct s *s, struct s from, usize align);
void s_printf(struct s *s, char *fmt, ...);

struct s_temp {
	struct s *s;
	usize n;
};

struct s_temp s_temp_begin(struct s *s);
void s_temp_end(struct s_temp t);

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

struct project discover_project(struct mem *m);

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
struct tokens lex(struct mem *m, struct s input);

#if DEVELOP
struct s lex_test(struct mem *m, struct s input);
#endif

// test.c

#if DEVELOP
void run_tests(struct mem *m);
#endif
