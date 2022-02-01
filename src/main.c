int g_connection;
struct table g_windows;
struct event_loop g_event_loop;
char g_socket_file[255];

static TABLE_HASH_FUNC(hash_windows)
{
    return *(uint32_t *) key;
}

static TABLE_COMPARE_FUNC(cmp_windows)
{
    return *(uint32_t *) key_a == *(uint32_t *) key_b;
}

static GLOBAL_CALLBACK(global_handler)
{
    if (type == 806) {
        event_loop_post(&g_event_loop, WINDOW_MOVED, (void *) (intptr_t) (*(uint32_t *) data), 0, NULL);
    } else if (type == 807) {
        event_loop_post(&g_event_loop, WINDOW_RESIZED, (void *) (intptr_t) (* (uint32_t *) data), 0, NULL);
    } else if (type == 808) {
        event_loop_post(&g_event_loop, WINDOW_ORDER_CHANGED, (void *) (intptr_t) (* (uint32_t *) data), 0, NULL);
    } else if (type == 815) {
        event_loop_post(&g_event_loop, WINDOW_IS_VISIBLE, (void *) (intptr_t) (* (uint32_t *) data), 0, NULL);
    } else if (type == 816) {
        event_loop_post(&g_event_loop, WINDOW_IS_INVISIBLE, (void *) (intptr_t) (* (uint32_t *) data), 0, NULL);
    } else {
        printf("global type = %d, data_length = %zu\n", type, data_length);
    }
}

int main(int argc, char **argv)
{
    NSApplicationLoad();
    g_connection = SLSMainConnectionID();

    table_init(&g_windows, 1024, hash_windows, cmp_windows);

    event_loop_init(&g_event_loop);
    event_loop_begin(&g_event_loop);

    char *user = getenv("USER");
    if (!user) {
        return 1;
    }

    snprintf(g_socket_file, sizeof(g_socket_file), "/tmp/yb_%s.socket", user);

    if (!message_loop_begin(g_socket_file)) {
        return 1;
    }

    // SLSRegisterNotifyProc(global_handler, 800, NULL); // Obscured
    // SLSRegisterNotifyProc(global_handler, 801, NULL); // Unobscured
    // SLSRegisterNotifyProc(global_handler, 802, NULL); // Ordered In
    // SLSRegisterNotifyProc(global_handler, 803, NULL); // Ordered Out
    // SLSRegisterNotifyProc(global_handler, 804, NULL); // Terminated
    SLSRegisterNotifyProc(global_handler, 806, NULL); // Move
    SLSRegisterNotifyProc(global_handler, 807, NULL); // Resize
    SLSRegisterNotifyProc(global_handler, 808, NULL); // Changed Order
    SLSRegisterNotifyProc(global_handler, 815, NULL); // Is Visible
    SLSRegisterNotifyProc(global_handler, 816, NULL); // Is Invisible

    //
    // query yabai for all open windows and create borders
    //
    event_loop_post(&g_event_loop, WINDOW_QUERY_YABAI, NULL, 0, NULL);

    CFRunLoopRun();
    return 0;
}
