// -c: command to run
// -C: command to run, with string interpolation
// -st: Success title
// -sb: Success body
// -si: Success icon
// -ft: Fail title
// -fb: Fail body
// -fi: Fail icon
// -t: generic title
// -b: generic body
// -i: generic icon
// -trans: use command's exit code as this program's
// --: treat any args to the right as positional args
//
// {brace}: literal opening brace
// {0}, {1}, ...: positional arguments
// {cmd}: full command name
// {out}: stdout
// {err}: stderr
// {code}: exit code
// {name}: program name

//#include "glib-object.h"
//#include <libnotify/notification.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <libnotify/notify.h>

#include <parser.h>

#define FATAL_ERROR_TEXT "\33[1;91mfatal:\33[m "
enum ExitCode {
    EXIT_PARSING_FAILURE,
};

// Btw, positionals is just a pointer to argv from main. Be warned.
typedef struct {
    char *command,
        *title_success, *body_success, *icon_success,
        *title_failure, *body_failure, *icon_failure;
    bool transfer_exit_code;
    char **positionals;
    int positional_count;
} Args;

// hahahahaa louis remember the time I was writing an argument parser in C
static ParserError parseArg(const char *const arg, void *object, char ***argv_traveller) {
    #define args ((Args *const) object)

    // NOTE: the semicolons missinbg here are not only possible here, but in-fact required, due to
    // mSetNext being a macro. I hate this.
    if(!strcmp(arg, "c")) mSetNext(args->command, argv_traveller)
    else if(!strcmp(arg, "C")) mSetNext(args->command, argv_traveller)
    else if(!strcmp(arg, "st")) mSetNext(args->title_success, argv_traveller)
    else if(!strcmp(arg, "sb")) mSetNext(args->body_success, argv_traveller)
    else if(!strcmp(arg, "si")) mSetNext(args->icon_success, argv_traveller)
    else if(!strcmp(arg, "ft")) mSetNext(args->title_failure, argv_traveller)
    else if(!strcmp(arg, "fb")) mSetNext(args->body_failure, argv_traveller)
    else if(!strcmp(arg, "fi")) mSetNext(args->icon_failure, argv_traveller)

    return PARSER_SUCCEEDED;
    #undef args
}

int main(int argc, char *argv[]) {
    Args args = {0};
    if(printParserError(parseArgs(&args, ++argv, &parseArg), FATAL_ERROR_TEXT)) {
        return EXIT_PARSING_FAILURE;
    }

    puts(args.command);
    puts(args.body_success);

    notify_init("Hello, libnotify!");
    NotifyNotification *hi = notify_notification_new (
        "Hello, world!~ :3",
        "&lt;i&gt;This is my first time using libnotify from C!!! :333&lt;/i&gt;",
        "dialog-information"
    );

    // notify_notification_set_urgency(hi, NOTIFY_URGENCY_CRITICAL);
    notify_notification_show(hi, NULL);
    g_object_unref(hi);
    notify_uninit();

    return 0;
}

