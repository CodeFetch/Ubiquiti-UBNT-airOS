#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#include <debug/log.h>
#include "airos-stats.h"

static ssize_t read_output(int fd, char* buf, size_t buf_len) {
	memset(buf, 0, buf_len);
	ssize_t len, pos = 0;
	while ((len = read(fd, buf + pos, buf_len - pos)) > 0)
		pos += len;

	return pos;
}

static ssize_t get_process_output(const char *path, const char *arg1, const char *arg2, char* out, size_t len) {
	int pipefd[2];
	if (pipe(pipefd) == -1) {
		log_printf(LOG_VERBOSE, "pipe() error: %s (%d)\n", strerror(errno), errno);
		return -1;
	}

	pid_t pid = fork();
	if (pid == -1) {
		log_printf(LOG_VERBOSE, "fork() error: %s (%d)\n", strerror(errno), errno);
		return -1;
	}

	if (pid == 0) {
		prctl(PR_SET_PDEATHSIG, SIGHUP); /* SIGHUP when parent terminates */
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[0]);
		int rc = execl(path, arg1, arg2, (char*) NULL);
		if (rc == -1)
			close(STDOUT_FILENO);
		_exit(EXIT_FAILURE);
	}
	else {
		close(pipefd[1]);
		ssize_t got = read_output(pipefd[0], out, len);
		close(pipefd[0]);

		int status;
		waitpid(pid, &status, WUNTRACED | WCONTINUED);
		if (WIFEXITED(status) && WEXITSTATUS(status)) {
			log_printf(LOG_VERBOSE, "child exited with status %d\n", WEXITSTATUS(status));
		}

		return got;
	}

	return -1;
}

const airos_stats_t *airos_get_stats(void) {
	static char buf[32768];

	size_t got = get_process_output("/usr/bin/ubntbox", "ubntbox", "snmp", buf, sizeof(buf));
	if (got < sizeof(airos_stats_t))
		return NULL;

	airos_stats_t* stats = (airos_stats_t*) buf;

	if (stats->sta_list.count > 0) {
		size_t stalist_offset = sizeof(*stats);
		size_t stalist_size = sizeof(*stats->sta_list.sta) * stats->sta_list.count;
		if (got == stalist_offset + stalist_size) {
			stats->sta_list.sta = buf + stalist_offset;
		}
		else {
			stats->sta_list.count = 0;
			stats->sta_list.sta = NULL;
		}
	}

	return stats;
}

