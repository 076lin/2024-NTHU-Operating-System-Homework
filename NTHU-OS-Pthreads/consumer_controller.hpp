#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include "consumer.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"

#ifndef CONSUMER_CONTROLLER
#define CONSUMER_CONTROLLER

class ConsumerController : public Thread {
public:
	// constructor
	ConsumerController(
		TSQueue<Item*>* worker_queue,
		TSQueue<Item*>* writer_queue,
		Transformer* transformer,
		int check_period,
		int low_threshold,
		int high_threshold
	);

	// destructor
	~ConsumerController();
	virtual void start();
private:
	std::vector<Consumer*> consumers;

	TSQueue<Item*>* worker_queue;
	TSQueue<Item*>* writer_queue;

	Transformer* transformer;

	// Check to scale down or scale up every check period in microseconds.
	int check_period;
	// When the number of items in the worker queue is lower than low_threshold,
	// the number of consumers scaled down by 1.
	int low_threshold;
	// When the number of items in the worker queue is higher than high_threshold,
	// the number of consumers scaled up by 1.
	int high_threshold;

	static void* process(void* arg);
};

// Implementation start

ConsumerController::ConsumerController(
	TSQueue<Item*>* worker_queue,
	TSQueue<Item*>* writer_queue,
	Transformer* transformer,
	int check_period,
	int low_threshold,
	int high_threshold
) : worker_queue(worker_queue),
	writer_queue(writer_queue),
	transformer(transformer),
	check_period(check_period),
	low_threshold(low_threshold),
	high_threshold(high_threshold){
}

ConsumerController::~ConsumerController() {}

void ConsumerController::start() {
	// TODO: starts a ConsumerController thread
	pthread_create(&t, nullptr, ConsumerController::process, this);
}

void* ConsumerController::process(void* arg) {
	// TODO: implements the ConsumerController's work
	ConsumerController* controller = (ConsumerController*)arg;  // Cast the argument to ConsumerController*
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);

    while (true) {
        usleep(controller->check_period);  // Sleep for the check period
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

        // Check the size of the worker queue
        int queue_size = controller->worker_queue->get_size();
		
        // Scale up (add consumers) if needed
        if (queue_size > controller->high_threshold ) {
            Consumer* new_consumer = new Consumer(controller->worker_queue, controller->writer_queue, controller->transformer);
            new_consumer->start();
            controller->consumers.push_back(new_consumer);
            std::cout<< "Scaling up consumers from "<<controller->consumers.size()-1<<" to "<<controller->consumers.size()<<'\n';
        }

        if (queue_size < controller->low_threshold && controller->consumers.size() > 1) {
            Consumer* consumer_to_remove = controller->consumers.back();
            controller->consumers.pop_back();
            consumer_to_remove->cancel();
			consumer_to_remove->join();
			delete consumer_to_remove;
            std::cout<<"Scaling down consumers from "<<controller->consumers.size()+1<<" to "<<controller->consumers.size()<<'\n';
        }
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    }
    return nullptr;  // Exit the process thread
}
#endif // CONSUMER_CONTROLLER_HPP
