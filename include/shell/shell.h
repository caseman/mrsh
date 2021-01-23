#ifndef SHELL_SHELL_H
#define SHELL_SHELL_H

#include <mrsh/shell.h>
#include <termios.h>
#include "job.h"
#include "process.h"

struct mrsh_variable {
	char *value;
	uint32_t attribs; // enum mrsh_variable_attrib
};

struct mrsh_function {
	struct mrsh_command *body;
};

enum mrsh_branch_control {
	MRSH_BRANCH_BREAK,
	MRSH_BRANCH_CONTINUE,
	MRSH_BRANCH_RETURN,
	MRSH_BRANCH_EXIT,
};

struct mrsh_call_frame_priv {
	struct mrsh_call_frame pub;

	enum mrsh_branch_control branch_control;
	int nloops;
};

/**
 * A context holds state information and per-job information. A context is
 * guaranteed to be shared between all members of a job.
 */
struct mrsh_context {
	struct mrsh_state *state;
	// When executing a pipeline, this is set to the job created for the
	// pipeline
	struct mrsh_job *job;
	// When executing an asynchronous list, this is set to true
	bool background;
};

void function_destroy(struct mrsh_function *fn);

struct mrsh_call_frame_priv *call_frame_get_priv(struct mrsh_call_frame *frame);

void push_frame(struct mrsh_state *state, int argc, const char *argv[]);
void pop_frame(struct mrsh_state *state);

#endif
