#include "StanceQueue.h"

StanceNode::StanceNode(bleedstance_t* src)
{
	memcpy(&(this->stance), src, sizeof(bleedstance_t));
	this->nextNode = NULL;
}

StanceQueue::StanceQueue()
{
	this->count = 0;
	this->headNode = NULL;
	this->tailNode = NULL;
}

StanceQueue::~StanceQueue()
{
	this->clear();
}

void  StanceQueue::enqueue(bleedstance_t* x)
{
	if (x == NULL) {
		return;
	}
	StanceNode* n = new StanceNode(x);
	if (this->headNode == NULL || this->tailNode == NULL)
	{
		this->headNode = (void*)n;
		this->tailNode = (void*)n;
		this->count = 1;
	}
	else
	{
		StanceNode* t = (StanceNode*)(this->tailNode);
		t->nextNode = (void*)n;
		this->tailNode = (void*)n;
		this->count++;
	}
}

bool StanceQueue::dequeue(bleedstance_t* dest)
{
	bool ret = this->peek(dest);
	if (ret == false) {
		return false;
	}
	StanceNode* h = (StanceNode*)(this->headNode);
	StanceNode* hn = (StanceNode*)(h->nextNode);

	this->headNode = (void*)hn;
	if (this->tailNode == h) {
		this->tailNode = NULL;
	}

	delete h;

	this->count--;
	if (this->count < 0) {
		this->count = 0;
	}

	return true;
}

bool StanceQueue::peek(bleedstance_t* dest)
{
	if (this->headNode == NULL) {
		return false;
	}
	if (dest == NULL) {
		return true;
	}
	StanceNode* h = (StanceNode*)(this->headNode);
	memcpy(dest, &(h->stance), sizeof(bleedstance_t));
	return true;
}

void StanceQueue::clear()
{
	while (this->headNode != NULL)
	{
		this->dequeue(NULL);
	}

	this->count = 0;
	this->headNode = NULL;
	this->tailNode = NULL;
}

int StanceQueue::getCount()
{
	if (this->headNode == NULL || this->tailNode == NULL) {
		this->count = 0;
	}
	return this->count;
}