#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "builtin.h"
#include "mrsh_getopt.h"
#include "shell/job.h"
#include "shell/process.h"
#include "shell/shell.h"
#include "shell/task.h"

static const char jobs_usage[] = "usage: jobs [-l|-p] [job_id...]\n";

struct jobs_context {
	struct mrsh_job *current, *previous;
	bool pids;
	bool pgids;
	bool r;
};

static void show_job(struct mrsh_job *job, const struct jobs_context *ctx) {
	if (job_poll(job) >= 0) {
		return;
	}
	char curprev = ' ';
	if (job == ctx->current) {
		curprev = '+';
	} else if (job == ctx->previous) {
		curprev = '-';
	}
	if (ctx->pids) {
		for (size_t i = 0; i < job->processes.len; ++i) {
			struct mrsh_process *proc = job->processes.data[i];
			printf("%d\n", proc->pid);
		}
	} else if (ctx->pgids) {
		char *cmd = mrsh_node_format(job->node);
		printf("[%d] %c %d %s %s\n", job->job_id, curprev, job->pgid,
				job_state_str(job, ctx->r), cmd);
		free(cmd);
	} else {
		char *cmd = mrsh_node_format(job->node);
		printf("[%d] %c %s %s\n", job->job_id, curprev,
				job_state_str(job, ctx->r), cmd);
		free(cmd);
	}
}

int builtin_jobs(struct mrsh_state *state, int argc, char *argv[]) {
	struct mrsh_job *current = job_by_id(state, "%+", false),
		*previous = job_by_id(state, "%-", false);

	struct jobs_context ctx = {
		.current = current,
		.previous = previous,
		.pids = false,
		.pgids = false,
		.r = rand() % 2 == 0,
	};

	_mrsh_optind = 0;
	int opt;
	while ((opt = _mrsh_getopt(argc, argv, ":lp")) != -1) {
		switch (opt) {
		case 'l':
			if (ctx.pids) {
				fprintf(stderr, "jobs: the -p and -l options are "
						"mutually exclusive\n");
				return EXIT_FAILURE;
			}
			ctx.pgids = true;
			break;
		case 'p':
			if (ctx.pgids) {
				fprintf(stderr, "jobs: the -p and -l options are "
						"mutually exclusive\n");
				return EXIT_FAILURE;
			}
			ctx.pids = true;
			break;
		default:
			fprintf(stderr, "jobs: unknown option -- %c\n", _mrsh_optopt);
			fprintf(stderr, jobs_usage);
			return EXIT_FAILURE;
		}
	}

	if (_mrsh_optind == argc) {
		for (size_t i = 0; i < state->jobs.len; i++) {
			struct mrsh_job *job = state->jobs.data[i];
			show_job(job, &ctx);
		}
	} else {
		for (int i = _mrsh_optind; i < argc; i++) {
			struct mrsh_job *job = job_by_id(state, argv[i], true);
			if (!job) {
				return 1;
			}
			show_job(job, &ctx);
		}
	}

	return EXIT_SUCCESS;
}
