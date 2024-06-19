// This header file defines functions and whatnot for a heap-allocated, compound array of errors.

#include <stddef.h>

// This struct houses a simple pointer to a string, and then whether or not the pointer should be
// free()'d when we clean up the memory of the compound error.
typedef struct {
    _Bool should_free;
    char *error_string;
} SingleError;

// This struct is essentially just a vector for a bunch of SingleErrors.
typedef struct {
    size_t length, capacity;
    // char *error_header, *single_error_header;
    SingleError *errors;
} CompoundError;

typedef enum {
    ERROR_ADD_SUCCESS,
    ERROR_ADD_FAIL_BAD_CALLER,
    ERROR_ADD_FAIL_OUT_OF_MEMORY,
} AddErrorReturn;

// Creates a new CompoundError. NOTE: the array for errors is only created when we the first error
// is added by the `addError` function; that way, no memory allocation is performed if no errors
// occur.
CompoundError newCompoundError(void);

// Adds an error to a specified CompoundError that is marked as not to be `free()`'d
AddErrorReturn addStaticError(CompoundError *errors, char *message);

// Adds an error to a specified CompoundError that is marked to be `free()`'d
AddErrorReturn addFreedError(CompoundError *errors, char *message);

// Displays the list of errors in `errors`, is there are any, to stderr. `main_header` and
// `sub_header` are used for formatting the errors shown.
// The array in `errors` is `free()`'d automatically, so make sure you don't be silly and use
// `errors` after running this.
// `true` is returned when there were errors present.
_Bool useCompoundError (
    CompoundError *const errors, const char *const main_header, const char *const sub_header,
    void (*headerMaker)(char *const error_summary, unsigned long error_count)
);

