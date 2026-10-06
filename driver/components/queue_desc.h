#ifndef __QUEUE_DESC_H_
#define __QUEUE_DESC_H_
#define QUEUE_SIZE 64
#include <stdint.h>
struct m_queue
{
		volatile uint8_t rear;
		volatile uint8_t front;
		volatile char buf[QUEUE_SIZE];
};
#endif
