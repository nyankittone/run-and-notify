//#include "glib-object.h"
//#include <libnotify/notification.h>
#include <stdio.h>
#include <libnotify/notify.h>

int main(void) {
    puts("Heya! >:3");

    notify_init("Hello, libnotify!");
    NotifyNotification *hi = notify_notification_new (
        "Hello, world!~ :3",
        "This is my first time using libnotify from C!!! :333",
        "dialog-information"
    );

    notify_notification_show(hi, NULL);
    g_object_unref(hi);
    notify_uninit();

    return 0;
}

