macOS Monterey window borders proof of concept

Requires yabai version 4.0.0 and jq (JSON processing tool)

Build by running `make` or `make install` for an optimized build.

Add the resulting binary somewhere in your path.

Add the following to your yabai config file:

```
pkill -x yb
yb &

yabai -m signal --add event=window_created action="echo \"create \$YABAI_WINDOW_ID\" | nc -U /tmp/yb_$USER.socket"
yabai -m signal --add event=window_destroyed action="echo \"destroy \$YABAI_WINDOW_ID\" | nc -U /tmp/yb_$USER.socket"
yabai -m signal --add event=space_changed action="echo \"query\" | nc -U /tmp/yb_$USER.socket"
yabai -m signal --add event=display_changed action="echo \"query\" | nc -U /tmp/yb_$USER.socket"
yabai -m signal --add event=mission_control_enter action="echo \"mission_control_enter\" | nc -U /tmp/yb_$USER.socket"
yabai -m signal --add event=mission_control_exit action="echo \"mission_control_exit\" | nc -U /tmp/yb_$USER.socket"
```
