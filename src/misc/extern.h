extern void NSApplicationLoad(void);

extern int SLSMainConnectionID(void);

#define GLOBAL_CALLBACK(name) void name(uint32_t type, void *data, size_t data_length, void *context)
typedef GLOBAL_CALLBACK(global_callback);
extern CGError SLSRegisterNotifyProc(global_callback *handler, uint32_t event, void *context);

extern CGError SLSRequestNotificationsForWindows(int cid, uint32_t *window_list, int window_count);

extern CGError SLSGetWindowBounds(int cid, uint32_t wid, CGRect *frame);

extern CGError SLSMoveWindow(int cid, uint32_t wid, CGPoint *point);

extern CGError SLSGetWindowLevel(int cid, uint32_t wid, int *level);

extern CGError SLSSetWindowLevel(int cid, uint32_t wid, int level);

extern CGError CGSNewRegionWithRect(CGRect *rect, CFTypeRef *region);

extern CGContextRef SLWindowContextCreate(int cid, uint32_t wid, CFDictionaryRef options);

extern CGError SLSNewWindow(int cid, int type, float x, float y, CFTypeRef region, uint32_t *wid);

extern CGError SLSSetWindowTags(int cid, uint32_t wid, uint64_t *tags, int tag_size);

extern CGError SLSClearWindowTags(int cid, uint32_t wid, uint64_t *tags, int tag_size);

extern CGError SLSSetWindowShape(int cid, uint32_t wid, float x_offset, float y_offset, CFTypeRef shape);

extern CGError SLSSetWindowOpacity(int cid, uint32_t wid, bool opaque);

extern CGError SLSSetWindowResolution(int cid, uint32_t wid, double resolution);

extern CGError SLSDisableUpdate(int cid);

extern CGError SLSReenableUpdate(int cid);

extern CGError SLSOrderWindow(int cid, uint32_t wid, int mode, uint32_t rel_wid);

extern CGError SLSSetWindowShape(int cid, uint32_t wid, float x_offset, float y_offset, CFTypeRef shape);

extern CGError SLSReleaseWindow(int cid, uint32_t wid);

extern CFArrayRef SLSCopySpacesForWindows(int cid, int selector, CFArrayRef window_list);

extern void SLSMoveWindowsToManagedSpace(int cid, CFArrayRef window_list, uint64_t sid);
