/**
 * @brief simple thread-safe watchdog system
 * @author Adev
 * @date 20/09/11
 * 
 * */

#include "gwatchdog.h" 
#include <unistd.h>

static gpointer internal_runtime(gpointer data){
	GWatchdog* dog = (GWatchdog*) data;
	while( !cancel){
		g_mutex_lock(dog->mutex);	
		if(g_timer_elapsed(time,NULL) >= g_atomic_int_get(&dog->duration)){
			dog->started = FALSE;
			g_mutex_unlock(dog->mutex);
			dog->func(dog->data);	 // execute the watchdog functions
			g_thread_exit(0);
		}
		g_mutex_unlock(dog->mutex);
		usleep(50000);
	}
	g_thread_exit(1);
}
 
/**
 * create a new watchdog
 **/
GWatchdog* gwatchdog_new(int duration, GThreadFunc func, gpointer data){
	GWatchdog* dog = g_new(struct _GWatchdog, 1);
	dog->time =  g_timer_new();
	dog->duration = duration;
	dog->func = func;
	dog->data = data;
	dog->started = FALSE;
	dog->mutex = g_mutex_new();
	dog->cancel = FALSE;
	return dog;
} 


/***
 * start a new watchdog, return -1 if already started
 */
int gwatchdog_start(GWatchdog* dog){
	g_return_val_if_fail(timer != NULL, -2);
	int ret = -1;
	g_mutex_lock(dog->mutex);
	if(dog->started == FALSE){
		g_timer_start(dog->time);
		dog->thread =  g_thread_create(dog->func, dog->data, TRUE, NULL);
		ret = 0;	
	}
	g_mutex_unlock(dog->mutex);
	return ret;
}


/***
 * extend the timer of a duration x
 *  return 0 if success else -1 if already stopped
 */
int gwatchdog_extend(GWatchdog* dog, int duration){
	g_return_val_if_fail(timer != NULL, -2);
	int ret = -1;
	if(dog->started == TRUE && dog->cancel == FALSE){
		g_atomic_int_add(&dog->duration,duration);
		ret = 0;
	}
	return ret;
}  




void gwatchdog_delete(GWatchdog* dog){
	if(dog){
		dog->cancel = TRUE;
		g_thread_join(dog->thread);
		g_timer_free(dog->time);
		g_free(dog);
	}
} 
