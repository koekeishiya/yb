extern int g_connection;
extern struct table g_windows;

static bool is_in_mission_control;

struct border
{
    uint32_t id;
    CFArrayRef id_ref;
    CGContextRef context;
    CFTypeRef region;
    CGRect frame;
    CGMutablePathRef path;
};

static void update_window_notifications(uint32_t wid)
{
    int window_count = 0;
    uint32_t window_list[1024] = {};

    if (wid) window_list[window_count++] = wid;

    for (int window_index = 0; window_index < g_windows.capacity; ++window_index) {
        struct bucket *bucket = g_windows.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                uint32_t wid = *(uint32_t *) bucket->key;
                window_list[window_count++] = wid;
            }

            bucket = bucket->next;
        }
    }

    SLSRequestNotificationsForWindows(g_connection, window_list, window_count);
}

static CFArrayRef cfarray_of_cfnumbers(void *values, size_t size, int count, CFNumberType type)
{
    CFNumberRef temp[count];

    for (int i = 0; i < count; ++i) {
        temp[i] = CFNumberCreate(NULL, type, ((char *)values) + (size * i));
    }

    CFArrayRef result = CFArrayCreate(NULL, (const void **)temp, count, &kCFTypeArrayCallBacks);

    for (int i = 0; i < count; ++i) {
        CFRelease(temp[i]);
    }

    return result;
}

static uint64_t *window_space_list(uint32_t wid, int *count)
{
    uint64_t *space_list = NULL;
    CFArrayRef window_list_ref = cfarray_of_cfnumbers(&wid, sizeof(uint32_t), 1, kCFNumberSInt32Type);
    CFArrayRef space_list_ref = SLSCopySpacesForWindows(g_connection, 0x7, window_list_ref);
    if (!space_list_ref) goto err;

    *count = CFArrayGetCount(space_list_ref);
    if (!*count) goto out;

    space_list = malloc(*count * sizeof(uint64_t));
    for (int i = 0; i < *count; ++i) {
        CFNumberRef id_ref = CFArrayGetValueAtIndex(space_list_ref, i);
        CFNumberGetValue(id_ref, CFNumberGetType(id_ref), space_list + i);
    }

out:
    CFRelease(space_list_ref);
err:
    CFRelease(window_list_ref);
    return space_list;
}

static void ensure_same_space(struct border *border, uint32_t wid)
{
    int space_count;
    uint64_t *space_list = window_space_list(wid, &space_count);
    if (!space_list) return;

    if (space_count > 1) {
        uint64_t tag = 1ULL << 11;
        SLSSetWindowTags(g_connection, border->id, &tag, 64);
    } else {
        uint64_t tag = 1ULL << 11;
        SLSClearWindowTags(g_connection, border->id, &tag, 64);
        SLSMoveWindowsToManagedSpace(g_connection, border->id_ref, space_list[0]);
    }

    free(space_list);
}

static struct border *create_border(uint32_t wid)
{
    struct border *border = malloc(sizeof(struct border));
    memset(border, 0, sizeof(struct border));

    CGRect frame = {};
    SLSGetWindowBounds(g_connection, wid, &frame);
    CGSNewRegionWithRect(&frame, &border->region);
    border->frame.size = frame.size;

    // printf("frame: %.2f, %.2f, %.2f, %.2f\n", frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);

    uint64_t tag1 = 1ULL << 3;
    uint64_t tag2 = 1ULL << 46;

    int window_level = 0;
    SLSGetWindowLevel(g_connection, wid, &window_level);

    SLSNewWindow(g_connection, 2, 0, 0, border->region, &border->id);
    SLSSetWindowTags(g_connection, border->id, &tag1, 64);
    SLSSetWindowTags(g_connection, border->id, &tag2, 64);
    SLSSetWindowResolution(g_connection, border->id, 1.0f);
    SLSSetWindowOpacity(g_connection, border->id, 0);
    SLSSetWindowLevel(g_connection, border->id, window_level);

    border->id_ref = cfarray_of_cfnumbers(&border->id, sizeof(uint32_t), 1, kCFNumberSInt32Type);
    border->context = SLWindowContextCreate(g_connection, border->id, 0);
    CGContextSetLineWidth(border->context, 4);
    CGContextSetRGBStrokeColor(border->context, 1.0f, 0.5f, 0.5f, 1.0f);

    border->path = CGPathCreateMutable();
    CGPathAddRoundedRect(border->path, NULL, border->frame, 0, 0);

    SLSDisableUpdate(g_connection);
    SLSSetWindowShape(g_connection, border->id, 0.0f, 0.0f, border->region);
    CGContextClearRect(border->context, border->frame);
    CGContextAddPath(border->context, border->path);
    CGContextStrokePath(border->context);
    CGContextFlush(border->context);
    SLSReenableUpdate(g_connection);

    SLSOrderWindow(g_connection, border->id, 1, wid);

    return border;
}

static void destroy_border(struct border *border)
{
    if (border->id_ref) CFRelease(border->id_ref);
    if (border->region) CFRelease(border->region);
    if (border->path)   CGPathRelease(border->path);

    CGContextRelease(border->context);
    SLSReleaseWindow(g_connection, border->id);
    free(border);
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_CREATED)
{
    uint32_t wid = (uint32_t) (intptr_t) context;
    // printf("window created: %d\n", wid);

    update_window_notifications(wid);
    struct border *border = create_border(wid);
    table_add(&g_windows, &wid, border);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_DESTROYED)
{
    uint32_t wid = (uint32_t) (intptr_t) context;
    // printf("window destroyed: %d\n", wid);

    struct border *border = table_find(&g_windows, &wid);
    if (border) {
        table_remove(&g_windows, &wid);
        update_window_notifications(0);
        destroy_border(border);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_FOCUSED)
{
    uint32_t wid = (uint32_t) (intptr_t) context;
    // printf("window focused: %d\n", wid);

    struct border *border = table_find(&g_windows, &wid);
    if (border) {
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_MOVED)
{
    uint32_t wid = (uint32_t) (intptr_t) context;
    // printf("window moved: %d\n", wid);

    struct border *border = table_find(&g_windows, &wid);
    if (border) {
        CGRect frame = {};
        SLSGetWindowBounds(g_connection, wid, &frame);
        SLSMoveWindow(g_connection, border->id, &frame.origin);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_RESIZED)
{
    uint32_t wid = (uint32_t) (intptr_t) context;
    // printf("window resized: %d\n", wid);

    struct border *border = table_find(&g_windows, &wid);
    if (border) {
        if (border->region) CFRelease(border->region);
        if (border->path)   CGPathRelease(border->path);

        CGRect frame = {};
        SLSGetWindowBounds(g_connection, wid, &frame);
        CGSNewRegionWithRect(&frame, &border->region);
        border->frame.size = frame.size;

        border->path = CGPathCreateMutable();
        CGPathAddRoundedRect(border->path, NULL, border->frame, 0, 0);

        SLSDisableUpdate(g_connection);
        SLSSetWindowShape(g_connection, border->id, 0.0f, 0.0f, border->region);
        CGContextClearRect(border->context, border->frame);
        CGContextAddPath(border->context, border->path);
        CGContextStrokePath(border->context);
        CGContextFlush(border->context);
        SLSReenableUpdate(g_connection);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_ORDER_CHANGED)
{
    if (is_in_mission_control) goto out;

    uint32_t wid = (uint32_t) (intptr_t) context;
    // printf("window order changed: %d\n", wid);

    struct border *border = table_find(&g_windows, &wid);
    if (border) {
        int window_level = 0;
        SLSGetWindowLevel(g_connection, wid, &window_level);
        SLSSetWindowLevel(g_connection, border->id, window_level);
        SLSOrderWindow(g_connection, border->id, 1, wid);
    }

out:
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_IS_VISIBLE)
{
    if (is_in_mission_control) goto out;

    uint32_t wid = (uint32_t) (intptr_t) context;
    // printf("window is visible: %d\n", wid);

    struct border *border = table_find(&g_windows, &wid);
    if (border) {
        ensure_same_space(border, wid);
        SLSOrderWindow(g_connection, border->id, 1, wid);
    }

out:
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_IS_INVISIBLE)
{
    uint32_t wid = (uint32_t) (intptr_t) context;
    // printf("window is invisible: %d\n", wid);

    struct border *border = table_find(&g_windows, &wid);
    if (border) {
        SLSOrderWindow(g_connection, border->id, 0, wid);
        ensure_same_space(border, wid);
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_MISSION_CONTROL_ENTER)
{
    is_in_mission_control = true;

    for (int window_index = 0; window_index < g_windows.capacity; ++window_index) {
        struct bucket *bucket = g_windows.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                struct border *border = bucket->value;
                SLSOrderWindow(g_connection, border->id, 0, 0);
            }

            bucket = bucket->next;
        }
    }

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_MISSION_CONTROL_EXIT)
{
    is_in_mission_control = false;

    for (int window_index = 0; window_index < g_windows.capacity; ++window_index) {
        struct bucket *bucket = g_windows.buckets[window_index];
        while (bucket) {
            if (bucket->value) {
                uint32_t wid = *(uint32_t *) bucket->key;
                struct border *border = bucket->value;
                SLSOrderWindow(g_connection, border->id, 1, wid);
            }

            bucket = bucket->next;
        }
    }

    return EVENT_SUCCESS;
}

static char *query_yabai_window_for_space(void)
{
    int cursor = 0;
    int bytes_read = 0;
    char *result = NULL;
    char buffer[BUFSIZ];

    FILE *handle = popen("$(which yabai) -m query --windows --space | $(which jq) 'map(.id)'", "r");
    if (!handle) return NULL;

    while ((bytes_read = read(fileno(handle), buffer, sizeof(buffer)-1)) > 0) {
        char *temp = realloc(result, cursor+bytes_read+1);
        if (!temp) goto err;

        result = temp;
        memcpy(result+cursor, buffer, bytes_read);
        cursor += bytes_read;
    }

    if (result && bytes_read != -1) {
        result[cursor] = '\0';
        // printf("%s\n", result);
    } else {
err:
        if (result) {
            free(result);
            result = NULL;
        }
    }

    pclose(handle);
    return result;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_QUERY_YABAI)
{
    char *result = query_yabai_window_for_space();
    if (!result) goto out;

    bool updated = false;
    char *at = result;

    while (*at) {
        if (at[0] >= '0' && at[0] <= '9') {
            char *start = at++;
            while (at[0] >= '0' && at[0] <= '9') {
                ++at;
            }
            int length = at - start;

            char buffer[length + 1];
            memcpy(buffer, start, length);
            buffer[length] = '\0';

            uint32_t wid = 0;
            sscanf(buffer, "%d", &wid);
            // printf("parsed window id: %d\n", wid);

            if (!table_find(&g_windows, &wid)) {
                // printf("new window with id: %d\n", wid);
                struct border *border = create_border(wid);
                table_add(&g_windows, &wid, border);
                updated = true;
            }
        } else {
            ++at;
        }
    }

    if (updated) {
        update_window_notifications(0);
    }

    free(result);

out:
    return EVENT_SUCCESS;
}
