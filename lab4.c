#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#define N 1024

char xor( char text, char key)
{
	return text^key;
}

char *key_generate(char *key, int len)
{
	int i, out_key;
	for(i = 0; i < len; i++) {
		key[i] = rand();
	}

	out_key = open("key.out", O_WRONLY|O_CREAT, 0600);
	if (out_key == -1) {
		fprintf(stderr, "cannot open file: %s\n", "key.out");
		exit(EXIT_FAILURE);
	}
	write(out_key, key, len);
	close(out_key);

	return key;
}

int main(int argc, char const *argv[])
{
	char file_in[N] = {0};
	char file_out[N] = {0};
	char file_key[N] = {0};

	int data_processed;
	int file_pipes[2];

	char buffer[BUFSIZ + 1] = {0};
	char buffer_key[BUFSIZ + 1] = {0};
	pid_t fork_result;
	char buf[1] = {0};
	int i, k, f = 1;
	int out;

	strcat(file_out, argv[1]);
	strcat(file_in, argv[2]);

	for (k = 0; k<f; k++) {
		out = open(file_out, O_WRONLY|O_CREAT, 0600);
		if (out == -1) {
			fprintf(stderr, "cannot open file: %s\n", "file.out");
			return -1;
		}

		if (pipe(file_pipes) == 0) {
			fork_result = fork();
			if (fork_result == -1) {
				fprintf(stderr, "Fork failure");
				exit(EXIT_FAILURE);
			}

			if (fork_result == 0) {
				close(1);
				dup(file_pipes[1]);
				close(file_pipes[0]);
				if (k == 0)
					execlp("cat", "cat", file_in, (char *)0);
				if (k == 1)
					execlp("cat", "cat", file_key, (char *)0);
				exit(EXIT_FAILURE);
			}

			else {
				int stat_val;
				close(file_pipes[1]);
				if (k == 0) {
					data_processed = read(file_pipes[0], buffer,
								BUFSIZ);
				}
				if (k == 1) {
					data_processed = read(file_pipes[0], buffer_key,
								BUFSIZ);
				}

				if (argv[3] == NULL)
					key_generate(buffer_key, data_processed);
				else {
					f = 2;
					strcat(file_key, argv[3]);
				}
				fork_result = wait(&stat_val);
				write(1, buffer, BUFSIZ);
				for(i = 0; i<data_processed; i++) {
					buf[0] = xor(buffer[i],buffer_key[i]);
					write(out, buf, 1);
				}
				close(out);
			}
		}
	}
	exit(EXIT_SUCCESS);
}