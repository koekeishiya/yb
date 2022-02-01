#ifndef EVENT_LOOP_EVENT_H
#define EVENT_LOOP_EVENT_H

#define EVENT_CALLBACK(name) uint32_t name(void *context, int param1)
typedef EVENT_CALLBACK(event_callback);

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_CREATED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_DESTROYED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_FOCUSED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_MOVED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_RESIZED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_ORDER_CHANGED);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_IS_VISIBLE);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_IS_INVISIBLE);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_QUERY_YABAI);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_MISSION_CONTROL_ENTER);
static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_MISSION_CONTROL_EXIT);

#define EVENT_QUEUED    0x0
#define EVENT_PROCESSED 0x1

#define EVENT_SUCCESS 0x0
#define EVENT_FAILURE 0x1

#define event_status(e) ((e)  & 0x1)
#define event_result(e) ((e) >> 0x1)

enum event_type
{
    EVENT_TYPE_UNKNOWN,

    WINDOW_CREATED,
    WINDOW_DESTROYED,
    WINDOW_FOCUSED,
    WINDOW_MOVED,
    WINDOW_RESIZED,
    WINDOW_ORDER_CHANGED,
    WINDOW_IS_VISIBLE,
    WINDOW_IS_INVISIBLE,
    WINDOW_QUERY_YABAI,
    WINDOW_MISSION_CONTROL_ENTER,
    WINDOW_MISSION_CONTROL_EXIT,

    EVENT_TYPE_COUNT
};

static const char *event_type_str[] =
{
    [EVENT_TYPE_UNKNOWN]                 = "event_type_unknown",

    [WINDOW_CREATED]                     = "window_created",
    [WINDOW_DESTROYED]                   = "window_destroyed",
    [WINDOW_FOCUSED]                     = "window_focused",
    [WINDOW_MOVED]                       = "window_moved",
    [WINDOW_RESIZED]                     = "window_resized",
    [WINDOW_ORDER_CHANGED]               = "window_order_changed",
    [WINDOW_IS_VISIBLE]                  = "window_is_visible",
    [WINDOW_IS_INVISIBLE]                = "window_is_invisible",
    [WINDOW_QUERY_YABAI]                 = "window_query_yabai",
    [WINDOW_MISSION_CONTROL_ENTER]       = "window_mission_control_enter",
    [WINDOW_MISSION_CONTROL_EXIT]        = "window_mission_control_exit",

    [EVENT_TYPE_COUNT]                   = "event_type_count"
};

static event_callback *event_handler[] =
{
    [WINDOW_CREATED]                     = EVENT_HANDLER_WINDOW_CREATED,
    [WINDOW_DESTROYED]                   = EVENT_HANDLER_WINDOW_DESTROYED,
    [WINDOW_FOCUSED]                     = EVENT_HANDLER_WINDOW_FOCUSED,
    [WINDOW_MOVED]                       = EVENT_HANDLER_WINDOW_MOVED,
    [WINDOW_RESIZED]                     = EVENT_HANDLER_WINDOW_RESIZED,
    [WINDOW_ORDER_CHANGED]               = EVENT_HANDLER_WINDOW_ORDER_CHANGED,
    [WINDOW_IS_VISIBLE]                  = EVENT_HANDLER_WINDOW_IS_VISIBLE,
    [WINDOW_IS_INVISIBLE]                = EVENT_HANDLER_WINDOW_IS_INVISIBLE,
    [WINDOW_QUERY_YABAI]                 = EVENT_HANDLER_WINDOW_QUERY_YABAI,
    [WINDOW_MISSION_CONTROL_ENTER]       = EVENT_HANDLER_WINDOW_MISSION_CONTROL_ENTER,
    [WINDOW_MISSION_CONTROL_EXIT]        = EVENT_HANDLER_WINDOW_MISSION_CONTROL_EXIT
};

struct event
{
    void *context;
    volatile uint32_t *info;
    enum event_type type;
    int param1;
};

#endif
