#pragma once
/**
 * @brief header for simple thread-safe watchdog system
 * @author Adev
 * @date 20/09/11
 * 
 * */

#include <glib.h> 
 
typedef struct _GWatchdog{
	GTimer* time;
	int duration;
	gpointer data;
	GThreadFunc func;
	GMutex* mutex;
	gboolean started;
	gboolean cancel;
	GThread* thread;
 } GWatchdog;
 
 
 
GWatchdog* gwatchdog_new(gint64 duration, GThreadFunc func, gpointer data);


int gwatchdog_start(GWatchdog* dog);

int gwatchdog_extend(GWatchdog* dog, int duration);


void gwatchdog_delete(GWatchdog* dog); 
