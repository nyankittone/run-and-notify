#include <stddef.h>

typedef struct {
    size_t length, capacity;
    char *data;
} CharVector;

// We need things for:
// creating strings
// appending to a string (with or without newline)
// deleting a string
// copying a string

typedef struct {
    _Bool should_free;
    char *string;
} MaybeFreeable;

typedef struct {
    const char *string;
    size_t length;
} StringAndLength;

