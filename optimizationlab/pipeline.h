/*******************************************************************************/
/*===----===----************ DO NOT TOUCH THIS BLOCK ************----===----===*/
#ifndef __CSYS_PIPELINE_H__
#define __CSYS_PIPELINE_H__

#define LOG_SIZE (1024UL * 1024 * 1024) // 1 GiB
#define BUF_SIZE LOG_SIZE // For simplicity, we read the whole log file into memory

#define MAX_LINES 16384
#define MAX_LINE_LENGTH 1024
#define MAX_NR_SUBSTR 16
#define NR_LOG_LEVELS 4
#define LOG_FILE "server.log"

typedef void (*Stage)();
typedef struct Pipeline {
    int nr_stage;
    Stage *stages;
} Pipeline;

typedef struct {
    char **lines;
    int count;
    int capacity;
} LogList;

static inline void run_pipeline(struct Pipeline *pipeline)
{
    for (int i = 0; i < pipeline->nr_stage; i++)
        pipeline->stages[i]();
}
/*===----===----************ DO NOT TOUCH THIS BLOCK ************----===----===*/
/*******************************************************************************/

#define NR_THREADS 4

#endif
