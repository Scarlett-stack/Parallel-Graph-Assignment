// SPDX-License-Identifier: BSD-3-Clause

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include "os_threadpool.h"
#include "../utils/log/log.h"
#include "utils.h"

/* Create a task that would be executed by a thread. */
os_task_t *create_task(void (*action)(void *), void *arg, void (*destroy_arg)(void *))
{
	os_task_t *t;

	t = malloc(sizeof(os_task_t));
	DIE(t == NULL, "malloc");

	t->action = action;
	t->argument = arg;
	t->destroy_arg = destroy_arg;

	return t;
}

/* Destroy task. */
void destroy_task(os_task_t *t)
{
	// printf("minune 2  %p\n", t);
	if (t == NULL)
		return;
	if (t->destroy_arg != NULL)
		t->destroy_arg(t->argument);
	free(t);
}

/* Put a new task to threadpool task queue. */
void enqueue_task(os_threadpool_t *tp, os_task_t *t)
{
	assert(tp != NULL);
	assert(t != NULL);
	// printf("SUNT IN ENQUUE\n");
	/* TODO: Enqueue task to the shared task queue. Use synchronization. */

	// list_add_tail(&tp->head, &t->list); // CRAPA
	// printf("AICI4\n");

	pthread_mutex_lock(&(tp->COADA_MUTEX));

	list_add(&(tp->head), &(t->list));

	pthread_cond_signal(&(tp->COADA_COND));

	pthread_mutex_unlock(&(tp->COADA_MUTEX));
}

/*
 * Check if queue is empty.
 * This function should be called in a synchronized manner.
 */
static int queue_is_empty(os_threadpool_t *tp)
{
	return list_empty(&tp->head);
}

/*
 * Get a task from threadpool task queue.
 * Block if no task is available.
 * Return NULL if work is complete, i.e. no task will become available,
 * i.e. all threads are going to block.
 */

os_task_t *dequeue_task(os_threadpool_t *tp)
{
	assert(tp != NULL);
	os_task_t *t;
	//aici ar trb un while si un flag in threadpool
	//pthread_mutex_lock(&(tp->queue_mutex));
	//while (queue_is_empty(tp) && tp->stop == 0)
	//{
	//pthread_cond_wait(&(tp->queue_not_empty))
	//}
	//dar pisici ei ca mi se blocheaza ??????!!!
	//if (tp->num_threads < 4)
	//{
	//pthread_cond_wait(&tp->COADA_COND, &tp->COADA_MUTEX);
	//}

	pthread_mutex_lock(&(tp->COADA_MUTEX));
	//tp->num_threads++;
	if (queue_is_empty(tp)) {
		pthread_mutex_unlock(&tp->COADA_MUTEX);
		//tp->num_threads--;
		return NULL;
	}
	//while (queue_is_empty(tp))
	//{
	//pthread_mutex_unlock(&tp->queue_mutex);
	//tp->stop = 0;
	//return NULL;
	//}

	/* TODO: Dequeue task from the shared task queue. Use synchronization. */
	//return NULL;
	if (!queue_is_empty(tp)) {
		t = list_entry(tp->head.prev, os_task_t, list);
		list_del(tp->head.prev);
	}
	pthread_mutex_unlock(&(tp->COADA_MUTEX));
	//tp->num_threads--;
	return t;
}

/* Loop function for threads */
static void *thread_loop_function(void *arg)
{
	os_threadpool_t *tp = (os_threadpool_t *)arg;
	// printf("DIN LOOP\n");
	while (1) {
		os_task_t *t;

		t = dequeue_task(tp);
		// printf("minune : %p\n", t);
		if (t == NULL)
			break;
		t->action(t->argument); // crapa
		// printf("HELLO??\n");
		destroy_task(t);
		// printf("ACTION\n");
	}

	return NULL;
}

/* Wait completion of all threads. This is to be called by the main thread. */
void wait_for_completion(os_threadpool_t *tp)
{

	/* --------OK DECI AM UN DISCLAIMER DE FACUT AICI----------
	 * L-am intrebat pe RD la curs si mi-a zis sa am grija la sincronizarea din dequeue
	 * dar daca nu imi iese si imi trece si asa sa il pun asa ca cica o sa se depuncteze
	 * anul viitor sau cv de genu.
	 * Si da daca fac cu wait ala mi se blocheaza, dar nu are probleme la signal
	 * Deci data viitoare anuntati si voi cand dati enuntul detalii de genu va rog
	 * ca sa am nervi destui pt si mai multe seg faulturi :C
	 */
	/* TODO: Wait for all worker threads. Use synchronization. */
	//pthread_mutex_lock(&(tp->queue_mutex));
	//while (!queue_is_empty(&tp->head))
	//{
	//pthread_cond_wait(&(tp->queue_not_empty), &(tp->queue_mutex));
	//}
	//pthread_cond_broadcast(&(tp->COADA_COND));
	//pthread_mutex_unlock(&(tp->queue_mutex));
	for (unsigned int i = 0; i < tp->num_threads; i++) {
		if (pthread_join(tp->threads[i], NULL) != 0) {
			perror("vezi pthread_join");
			exit(EXIT_FAILURE);
		}
	}
}

/* Create a new threadpool. */
os_threadpool_t *create_threadpool(unsigned int num_threads)
{
	os_threadpool_t *tp = NULL;
	int rc;

	tp = malloc(sizeof(*tp));
	DIE(tp == NULL, "malloc");

	list_init(&tp->head);

	/* TODO: Initialize synchronization data. */

	pthread_mutex_init(&tp->COADA_MUTEX, NULL);
	pthread_cond_init(&tp->COADA_COND, NULL);
	tp->stop = 0;
	tp->num_threads = num_threads;

	tp->threads = malloc(num_threads * sizeof(pthread_t));
	DIE(tp->threads == NULL, "malloc");
	for (unsigned int i = 0; i < num_threads; ++i) {
		rc = pthread_create(&tp->threads[i], NULL, &thread_loop_function, (void *)tp);
		DIE(rc < 0, "pthread_create");
	}

	return tp;
}

/* Destroy a threadpool. Assume all threads have been joined. */
void destroy_threadpool(os_threadpool_t *tp)
{
	os_list_node_t *n, *p;

	/* TODO: Cleanup synchronization data. */

	pthread_mutex_destroy(&(tp->COADA_MUTEX));
	pthread_cond_destroy(&(tp->COADA_COND));

	list_for_each_safe(n, p, &tp->head) {
		list_del(n);
		destroy_task(list_entry(n, os_task_t, list));
	}

	free(tp->threads);
	free(tp);
}
