#ifndef __WORK_QUEUE_DESC_H
#define __WORK_QUEUE_DESC_H
typedef void(*work_queue_callback_t)(void*args);
struct work_queue_item{
	work_queue_callback_t work;
	void*args;
};
#endif
