/*******************************************************************************/
/*===----===----************ DO NOT TOUCH THIS BLOCK ************----===----===*/
#ifndef __CSYS_UTILITY_H__
#define __CSYS_UTILITY_H__

#define CLOCK_START(tag) \
    struct timespec __csys_start_##tag, __csys_end_##tag; \
    const char *__csys_tag_name_##tag = #tag; \
    clock_gettime(CLOCK_MONOTONIC, &__csys_start_##tag)

#define CLOCK_END(tag) \
    clock_gettime(CLOCK_MONOTONIC, &__csys_end_##tag); \
    double elapsed_##tag = (__csys_end_##tag.tv_sec - __csys_start_##tag.tv_sec) + \
                           (__csys_end_##tag.tv_nsec - __csys_start_##tag.tv_nsec) / 1e9; \
    printf("[TIMER] %s: %.6f seconds\n", __csys_tag_name_##tag, elapsed_##tag)

#endif
/*===----===----************ DO NOT TOUCH THIS BLOCK ************----===----===*/
/*******************************************************************************/