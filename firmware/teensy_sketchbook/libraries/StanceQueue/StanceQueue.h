#ifndef _STANCEQUEUE_H_
#define _STANCEQUEUE_H_

#include <stdint.h>
#include <stdbool.h>
#include <BleedStance.h>

class StanceNode
{
	public:
		StanceNode(bleedstance_t*);
		void* nextNode;
		bleedstance_t stance;
}

class StanceQueue
{
	public:
		StanceQueue();
		~StanceQueue();
		void enqueue(bleedstance_t*);
		bool dequeue(bleedstance_t*);
		bool peek(bleedstance_t*);
		void clear();
		int getCount();
	private:
		void* headNode;
		void* tailNode;
		int count;
}

#endif
