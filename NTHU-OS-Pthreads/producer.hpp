#include <pthread.h>
#include "thread.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"

#ifndef PRODUCER_HPP
#define PRODUCER_HPP

class Producer : public Thread {
public:
	// constructor
	Producer(TSQueue<Item*>* input_queue, TSQueue<Item*>* worker_queue, Transformer* transfomrer);

	// destructor
	~Producer();

	virtual void start();
private:
	TSQueue<Item*>* input_queue;
	TSQueue<Item*>* worker_queue;

	Transformer* transformer;

	// the method for pthread to create a producer thread
	static void* process(void* arg);
};

Producer::Producer(TSQueue<Item*>* input_queue, TSQueue<Item*>* worker_queue, Transformer* transformer)
	: input_queue(input_queue), worker_queue(worker_queue), transformer(transformer) {
}

Producer::~Producer() {}

void Producer::start() {
	// TODO: starts a Producer thread
	pthread_create(&t, nullptr, Producer::process, this);
}

void* Producer::process(void* arg) {
	// TODO: implements the Producer's work
	Producer* producer = static_cast<Producer*>(arg); // Cast to Producer to access members

    while (true) {
        // Dequeue item from the input queue
        Item* item = producer->input_queue->dequeue();

        // Apply the transformation to the item
        item->val=item->val=producer->transformer->producer_transform(item->opcode,item->val);

        // Enqueue the transformed item to the worker queue
        producer->worker_queue->enqueue(item);
    }

    return nullptr; // Return null since it's a void* return type for the thread
}

#endif // PRODUCER_HPP
