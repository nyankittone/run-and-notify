#include <stddef.h>

#define FATAL_ERROR_TEXT "\33[1;91mfatal:\33[m "

enum ExitCode {
    EXIT_PARSING_FAILURE = 1,
    EXIT_OUT_OF_MEMORY,
    EXIT_PRE_LAUNCH_ERROR,
};

void panic(int exit_code, const char *const message_format, ...);
void OOMPanic(void);

void *reallocOrDie(void *old_block, size_t new_allocated_size);
void *mallocOrDie(size_t new_allocated_size);
