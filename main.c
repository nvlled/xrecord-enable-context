#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <xcb/xkb.h>
#include <X11/XKBlib.h>

#include <X11/keysym.h>
#include <X11/Xlibint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/record.h>

void hook_event_proc(XPointer closure, XRecordInterceptData *recorded_data);

typedef struct _hook_info
{
	struct _data
	{
		Display *display;
		XRecordRange *range;
	} data;
	struct _ctrl
	{
		Display *display;
		XRecordContext context;
	} ctrl;
} hook_info;
static hook_info *hook;

int main()
{
	hook = malloc(sizeof(hook_info));
	if (hook == NULL)
	{
		printf("[%u]: Failed to allocate memory for hook structure!\n", __LINE__);
		return -1;
	}

	hook->ctrl.display = XOpenDisplay(NULL);

	hook->data.display = XOpenDisplay(NULL);
	if (!(hook->ctrl.display != NULL && hook->data.display != NULL))
	{
		printf("[%u]: XOpenDisplay failure!\n", __LINE__);
		return -1;
	}

	printf("[%u]: XOpenDisplay successful.\n", __LINE__);

	// Check to make sure XRecord is installed and enabled.
	int major, minor;
	if (XRecordQueryVersion(hook->ctrl.display, &major, &minor) == 0)
	{
		printf("[%u]: XRecord is not currently available!\n", __LINE__);
		return -1;
	}

	printf("[%u]: XRecord version: %i.%i.\n", __LINE__, major, minor);

	// Make sure the data display is synchronized to prevent late event delivery!
	// See Bug 42356 for more information.
	// https://bugs.freedesktop.org/show_bug.cgi?id=42356#c4
	XSynchronize(hook->data.display, True);

	// Setup XRecord range.
	XRecordClientSpec clients = XRecordAllClients;

	hook->data.range = XRecordAllocRange();
	if (hook->data.range == NULL)
	{
		printf("[%u]: XRecordAllocRange failure!\n", __LINE__);
		return -1;
	}

	printf("[%u]: XRecordAllocRange successful.\n", __LINE__);

	hook->data.range->device_events.first = KeyPress;
	hook->data.range->device_events.last = MotionNotify;

	// Note that the documentation for this function is incorrect,
	// hook->data.display should be used!
	// See: http://www.x.org/releases/X11R7.6/doc/libXtst/recordlib.txt
	hook->ctrl.context = XRecordCreateContext(hook->data.display, XRecordFromServerTime, &clients, 1, &hook->data.range, 1);
	if (hook->ctrl.context == 0)
	{
		printf("[%u]: XRecordCreateContext failure!\n", __LINE__);
		return -1;
	}

	printf("[%u]: XRecordCreateContext successful.\n", __LINE__);

	XPointer closure = NULL;

	// Sync blocks until XRecordDisableContext() is called.
	XRecordEnableContext(hook->data.display, hook->ctrl.context, hook_event_proc, closure);

	return 0;
}

void hook_event_proc(XPointer closure, XRecordInterceptData *recorded_data)
{
	printf("-> %lu %lu %d\n", recorded_data->id_base, recorded_data->server_time, recorded_data->category);
}