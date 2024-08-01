#include <stdio.h>
#include <stdlib.h>
#include <interpret.h>

void printInstruction(const AssembleInstruction *const inst) {
    fputs("assemble ", stdout);
    switch(inst->item_type) {
        case ASSEMBLE_STRING:
            fputs("string, '", stdout);
            fwrite(inst->item.as_string.string, 1, inst->item.as_string.length, stdout);
            putchar('\'');
            break;
        case ASSEMBLE_OPEN_BRACE:
            fputs("open brace", stdout);
            break;
        case ASSEMBLE_EXIT_CODE:
            fputs("exit code", stdout);
            break;
        case ASSEMBLE_STDOUT:
            fputs("stdout", stdout);
            break;
        case ASSEMBLE_STDERR:
            fputs("stderr", stdout);
            break;
        case ASSEMBLE_NEWLINE:
            fputs("newline", stdout);
            break;
        case ASSEMBLE_IDK:
            fputs("idk", stdout);
            break;
        default:
            fputs("unknown", stdout);
            break;
    }

    putchar('\n');
}

void printInstructions(const AssembleInstructions *const inst) {
    if(inst->just_one) {
        printInstruction(&inst->data.as_one);
        return;
    }

    // fprintf(stderr, "%lu\n", inst->data.as_many.amount);

    for(size_t i = 0; i < inst->data.as_many.amount; i++) {
        printInstruction(inst->data.as_many.ptr + i);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) return 1;

    AssembleInstructions inst;
    void *thing = preInterpolate(&inst, 1, argc - 2, argv + 2, NULL, argv[1]);
    printInstructions(&inst);
    if(thing) free(thing);

    return 0;
}

