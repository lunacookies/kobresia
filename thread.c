#include "all.h"

smm
core_count(void)
{
	s32 value;
	umm size = cast(umm) size_of(s32);
	s32 code = sysctlbyname("hw.logicalcpu_max", &value, &size, NULL, 0);
	assert(code == 0);
	assert(size == cast(umm) size_of(s32));
	assert(value > 0);
	return value;
}

void
barrier_init(struct barrier *b, smm n)
{
	assert_zero(b);

	pthread_condattr_t condattr;
	pthread_condattr_init(&condattr);
	pthread_cond_init(&b->cond, &condattr);
	pthread_condattr_destroy(&condattr);

	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutex_init(&b->mutex, &mutexattr);
	pthread_mutexattr_destroy(&mutexattr);

	b->threads = n;
}

void
barrier_wait(struct barrier *b)
{
	pthread_mutex_lock(&b->mutex);
	b->n++;
	assert(b->n <= b->threads);

	if (b->n < b->threads) {
		// We haven’t yet reached the required number of waiting
		// threads, so wait until we do.
		pthread_cond_wait(&b->cond, &b->mutex);
	} else {
		// We reached the required number of waiting threads, so reset
		// the counter and wake up all waiting threads.
		assert(b->n == b->threads);
		b->n = 0;
		pthread_cond_broadcast(&b->cond);
	}

	pthread_mutex_unlock(&b->mutex);
}

struct worker_args {
	smm i;
	struct pool *p;
};

static void *
worker(void *a)
{
	struct worker_args *args = cast(struct worker_args *) a;
	smm i = args->i;
	struct pool *p = args->p;

	while (true) {
		barrier_wait(&p->ready);

		job j = p->jobs[i];
		void *job_arg = p->args[i];

		p->jobs[i] = NULL;
		p->args[i] = NULL;

		j(i, job_arg);

		barrier_wait(&p->done);
	}

	return NULL;
}

struct pool *
pool_start(struct mem *m, smm core_count, qos_class_t qos)
{
	assert(core_count > 0);
	struct pool *p = alloc(&m->perm, struct pool, 1);

	p->jobs = alloc(&m->perm, job, core_count);
	p->args = alloc(&m->perm, void *, core_count);

	// + 1 for main thread
	barrier_init(&p->ready, core_count + 1);
	barrier_init(&p->done, core_count + 1);

	p->count = core_count;

	for (smm i = 0; i < core_count; i++) {
		struct worker_args *args =
		        alloc(&m->perm, struct worker_args, 1);
		args->i = i;
		args->p = p;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_set_qos_class_np(&attr, qos, 0);

		pthread_t thread;
		pthread_create(&thread, &attr, worker, args);

		pthread_attr_destroy(&attr);
	}

	return p;
}

void
pool_sched(struct pool *p, job j, void **args)
{
	for (smm i = 0; i < p->count; i++) {
		void *arg = args[i];

		assert(p->jobs[i] == NULL);
		assert(p->args[i] == NULL);

		p->jobs[i] = j;
		p->args[i] = arg;
	}

	// Start all threads at the same time.
	barrier_wait(&p->ready);

	// Wait for all threads to complete.
	barrier_wait(&p->done);
}
