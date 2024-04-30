#include "notifications.h"
#include <stdio.h>
#include <stdlib.h>
// #include <libnotify/notify.h>

void send_notification(const char* title, const char* message) {
    // if (!notify_init("Monitor")) {
    //     fprintf(stderr, "Failed to initialize libnotify\n");
    //     exit(EXIT_FAILURE);
    // }

    // NotifyNotification* notification = notify_notification_new(title, message, NULL);
    // if (!notification) {
    //     fprintf(stderr, "Failed to create notification\n");
    //     exit(EXIT_FAILURE);
    // }

    // if (!notify_notification_show(notification, NULL)) {
    //     fprintf(stderr, "Failed to show notification\n");
    //     exit(EXIT_FAILURE);
    // }

    // g_object_unref(G_OBJECT(notification));
    // notify_uninit();
}
