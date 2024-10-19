// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "log/log.h"
#include "utils.h"

#define NUM_THREADS 4

static int sum;
static os_graph_t *graph;
static os_threadpool_t *tp;
/* TODO: Define graph synchronization mechanisms. */
pthread_mutex_t mutex_graf;
/* TODO: Define graph task argument. */

pthread_mutex_t sum_mutex = PTHREAD_MUTEX_INITIALIZER;

void process_node(unsigned int idx)
{
	/* TODO: Implement thread-pool based processing of graph. */

	//printf("AICI 2 (thread) %u\n", idx);
	os_node_t *nod = graph->nodes[idx];
	//if (graph->visited[idx] == 0) {
	//printf("AICI AUX\n");
	//pthread_mutex_unlock(&mutex_graf);
	//return;
	//}
	//os_node_t *nod = graph->nodes[idx];

	//graph->visited[idx] = DONE;
	//sum += nod->info;
	//add_to_sum_and_mark_done(idx);

	unsigned int i;

	for (i = 0; i < nod->num_neighbours; i++) {
		if (graph->visited[nod->neighbours[i]] == 0) {
			//pthread_mutex_lock(&mutex_graf);
			//graph->visited[nod->neighbours[i]] = PROCESSING;

			//printf("vecin idx: %u si suma %d\n", nod->neighbours[i], sum);
			os_task_t *task = create_task(process_node, graph->nodes[idx]->neighbours[i], NULL);
			//process_node(graph->nodes[idx]->neighbours[i]);
			enqueue_task(tp, task); //CRAPA
									//process_node(nod->neighbours[i]);
									//printf("AICI3\n");
									//pthread_mutex_unlock(&mutex_graf);
		}
	}
	pthread_mutex_lock(&sum_mutex);
	if (graph->visited[idx] == 0) {
		graph->visited[idx] = 1;
		sum += nod->info;
	}
	pthread_mutex_unlock(&sum_mutex);
}

int main(int argc, char *argv[])
{
	FILE *input_file;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s input_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	input_file = fopen(argv[1], "r");
	DIE(input_file == NULL, "fopen");

	graph = create_graph_from_file(input_file);
	/* TODO: Initialize graph synchronization mechanisms. */
	pthread_mutex_init(&sum_mutex, NULL);
	pthread_mutex_init(&mutex_graf, NULL);

	tp = create_threadpool(NUM_THREADS);

	process_node(0); /// crapa
					 // printf("AICI 1\n");
	wait_for_completion(tp);
	destroy_threadpool(tp);
	pthread_mutex_destroy(&sum_mutex);
	pthread_mutex_destroy(&mutex_graf);
	printf("%d", sum);

	return 0;
}
