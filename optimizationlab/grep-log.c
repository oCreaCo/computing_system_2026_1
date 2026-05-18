/**
 * Fall, 2025, Computing System for Data Science
 * Assignment 03: Pipeline Processing
 * grep-log.c - A multi-threaded grep-like utility using pipeline log processing
 */

/*******************************************************************************/
/*===----===----************ DO NOT TOUCH THIS BLOCK ************----===----===*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <pthread.h>

#include "pipeline.h"
#include "utility.h"

const char *target_level;
const char **substrs;
int num_substrs;

char buffer[BUF_SIZE];
pthread_t threads[NR_THREADS];
/*===----===----************ DO NOT TOUCH THIS BLOCK ************----===----===*/
/*******************************************************************************/

/**
 * - Usage:
 * 
 *  void *thread_func(void *arg)
 *  {
 *      ThreadArgs thr_arg = *((ThreadArgs*)arg);
 *      int a = thr_arg.a;
 *      int b = thr_arg.b;
 * 
 *      ...
 *  }
 *
 *  void function()
 *  {
 *      ThreadArgs args[NR_THREADS];
 * 
 *      for (int i = 0; i < NR_THREADS; i++) {
 *          args[i].a = ##;
 *          args[i].b = ##;
 *
 *          ...
 *
 *          pthread_create(&threads[i], NULL, thread_##, (void *)(&(args[i])));
 *      }
 * 
 *      for (int i = 0; i < NR_THREADS; i++) {
 *          pthread_join(threads[i], NULL);
 *      }
 *  }
 */
typedef struct {
/**
 * TODO: Implement your own arguments for thread if needed.
 * 
 * e.g.,
 *  int thread_id;
 */
} ThreadArgs;

/*******************************************************************************/
/* [Workhorse Functions] *******************************************************/
/*******************************************************************************/
LogList log_list[NR_LOG_LEVELS][NR_THREADS]; // ERROR, WARNING, DEBUG, INFO
int substring_count[MAX_NR_SUBSTR] = { 0, };
pthread_mutex_t substring_count_mutex = PTHREAD_MUTEX_INITIALIZER;

static void read_log(size_t size, off_t off)
{
    int fd = open(LOG_FILE, O_RDONLY);
    if (fd < 0) {
        perror("file open failed");
        return;
    }

    if (pread(fd, buffer + off, size, off) != size) {
        perror("file read failed");
        return;
    }

    close(fd);
}

static void classify_logs(int thread_id, size_t size, off_t off) {
    char *severity;
    int severity_index;

    for (int i = 0; i < NR_LOG_LEVELS; i++) {
        log_list[i][thread_id].capacity = MAX_LINES;
        log_list[i][thread_id].count = 0;
        log_list[i][thread_id].lines = (char**)malloc(sizeof(char*) * MAX_LINES);
    }

    char *next;
    char *line = strtok_r((buffer + off), "\n", &next);
    char *end = buffer + off + size;
    while (line != NULL && line < end) {
        /* Extract severity level from "[level]" */
        if (line[0] == '[') {
            char *end_bracket = strchr(line, ']');
            if (end_bracket != NULL) {
                int level_len = end_bracket - line - 1;
                severity = line + 1;

                if (strncmp(severity, "ERROR", level_len) == 0) {
                    severity_index = 0;
                } else if (strncmp(severity, "WARNING", level_len) == 0) {
                    severity_index = 1;
                } else if (strncmp(severity, "DEBUG", level_len) == 0) {
                    severity_index = 2;
                } else if (strncmp(severity, "INFO", level_len) == 0) {
                    severity_index = 3;
                }

                // Cache the whole line
                log_list[severity_index][thread_id].lines[log_list[severity_index][thread_id].count] = strdup(line);
                log_list[severity_index][thread_id].count++;
            } else {
                fprintf(stderr, "strchr() failed\n");
            }
        } else {
            fprintf(stderr, "unexpected log line format\n");
        }

        if (log_list[severity_index][thread_id].count 
            >= log_list[severity_index][thread_id].capacity) {
            log_list[severity_index][thread_id].capacity *= 2;
            log_list[severity_index][thread_id].lines =
                (char**)realloc(log_list[severity_index][thread_id].lines, 
                                sizeof(char*) * log_list[severity_index][thread_id].capacity);
        }

        line = strtok_r(NULL, "\n", &next);
    }

    int count = 0;
    for (int i = 0; i < NR_LOG_LEVELS; i++)
        count += log_list[i][thread_id].count;

    fprintf(stderr, "thread_id: %d, line count: %d\n", thread_id, count);
}

static void match_and_print(const char *target_level, const char **substrings,
                            int num_substrs) {
    int severity_index;

    if (strcmp(target_level, "ERROR") == 0) {
        severity_index = 0;
    } else if (strcmp(target_level, "WARNING") == 0) {
        severity_index = 1;
    } else if (strcmp(target_level, "DEBUG") == 0) {
        severity_index = 2;
    } else if (strcmp(target_level, "INFO") == 0) {
        severity_index = 3;
    }

/**
 * TODO: This code block contains an inefficient access pattern.
 *       Optimize the code to improve the performance.
 * NOTE: You MUST use `perf` to measure the related metrics before and after
 *       your optimization. (Which metrics to measure is on the pdf file.)
 */
    for (int i = 0; i < NR_THREADS; i++) {
        for (int j = 0; j < num_substrs; j++) {
            for (int k = 0; k < log_list[severity_index][i].count; k++) {
                if (strstr(log_list[severity_index][i].lines[k],
                           substrings[j]) != NULL)
                    substring_count[j]++;
            }
        }
    }

    for (int k = 0; k < num_substrs; k++) {
        fprintf(stdout, "Substring '%s': %d matches\n",
                substrings[k], substring_count[k]);
    }
}

/*******************************************************************************/
/* [Thread Functions] **********************************************************/
/*******************************************************************************/
void *thread_read_log(void *arg) {
    /**
     * TODO: Implement this function to parallelize the log reading stage.
     */
    return NULL;
}

void *thread_classify_logs(void *arg) {
    /**
     * TODO: Implement this function to parallelize the log classification stage.
     */
    return NULL;
}

void *thread_match_and_print(void *arg) {
    /**
     * TODO: Implement this function to parallelize the log matching and printing stage.
     * HINT: You may need to use mutexes to protect shared data structures.
     *       Maintain critical sections as small as possible to minimize contention.
     */
    return NULL;
}

/*******************************************************************************/
/* [Memory Free Functions] *****************************************************/
/*******************************************************************************/
static void free_log_list() {
    for (int i = 0; i < NR_LOG_LEVELS; i++) {
        for (int j = 0; j < NR_THREADS; j++) {
            for (int k = 0; k < log_list[i][j].count; k++) {
                free(log_list[i][j].lines[k]);
            }
            free(log_list[i][j].lines);
        }
    }
}

/*******************************************************************************/
/* [Pipeline Stages] ***********************************************************/
/*******************************************************************************/

/**
 * Hints about the assignment:
 * - If you want to measure the time of each stage, you can use
 *   CLOCK_START() and CLOCK_END() macros defined in utility.h.
 *   For example:
 *
 *   void f() {
 *      ...
 *      CLOCK_START(any_tag_you_want);
 *      // Your code that you want to measure the latency.
 *      CLOCK_END(any_tag_you_want);
 *      ...
 *   }
 *
 *   This will print the elapsed time with the given tag name.
 *   (e.g., [TIMER] any_tag_you_want: 0.123456 seconds)
 *   The tag name should be unique for each measurement.
 *
 * - You can create additional helper functions if needed.
 * - Make sure to handle memory allocation and deallocation properly to avoid
 *   memory leaks.
 * - You can modify the arguments of each stage function if necessary.
 * - Ensure thread safety when accessing shared resources.
 */

/**
 * Stage 1: Read log file into the memory buffer.
 */
static void read_log_stage()
{
    read_log(LOG_SIZE, 0);
}

/**
 * Stage 2: Classify logs by their severity levels using multi-threading.
 */
static void classify_logs_stage()
{
    classify_logs(0, LOG_SIZE, 0);
}

/**
 * Stage 3: Match and count logs that contain the specified substrings.
 */
static void match_and_print_stage()
{
    match_and_print(target_level, substrs, num_substrs);
}

/*******************************************************************************/
/*===----===----************ DO NOT TOUCH THIS BLOCK ************----===----===*/
/**
 * Stage 4: Free allocated memory for log lists.
 */
static void free_log_list_stage()
{
    free_log_list();
}
/*===----===----************ DO NOT TOUCH THIS BLOCK ************----===----===*/
/*******************************************************************************/

/*******************************************************************************/
/*===----===----************ DO NOT TOUCH THIS BLOCK ************----===----===*/
int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: ./grep-log <debug_level> <substring1> [substring2] ...\n");
        fprintf(stderr, "example: ./grep-log INFO \"Cache\" \"warmed\"\n");
        return 1;
    }

    target_level = argv[1];
    num_substrs = argc-2;
    substrs = (const char **)&argv[2];

    if (strcmp(target_level, "DEBUG") != 0 &&
        strcmp(target_level, "INFO") != 0 &&
        strcmp(target_level, "WARNING") != 0 &&
        strcmp(target_level, "ERROR") != 0) {
        fprintf(stderr, "invalid target level: %s\n", target_level);
        fprintf(stderr, "log levels: DEBUG, INFO, WARNING, ERROR\n");
        return 1;
    }
/*===----===----************ DO NOT TOUCH THIS BLOCK ************----===----===*/
/*******************************************************************************/
    /*
     * Run the pipeline with the defined stages.
     * You can add/remove stages or modify the order of stages here.
     * [ Read Log -> Classify Logs -> Match and Print -> Free Log List ]
     */
    Stage stages[] = {
        read_log_stage,
        classify_logs_stage,
        match_and_print_stage,
        free_log_list_stage,
    };

    Pipeline pipeline = {
        .nr_stage = 4,
        .stages = stages,
    };

    run_pipeline(&pipeline);
    
    return 0;
}