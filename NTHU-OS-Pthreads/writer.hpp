#include <fstream>
#include "thread.hpp"
#include "ts_queue.hpp"
#include "item.hpp"

#ifndef WRITER_HPP
#define WRITER_HPP

class Writer : public Thread {
public:
	// constructor
	Writer(int expected_lines, std::string output_file, TSQueue<Item*>* output_queue);

	// destructor
	~Writer();

	virtual void start() override;
private:
	// the expected lines to write,
	// the writer thread finished after output expected lines of item
	int expected_lines;

	std::ofstream ofs;
	TSQueue<Item*> *output_queue;

	// the method for pthread to create a writer thread
	static void* process(void* arg);
};

// Implementation start

Writer::Writer(int expected_lines, std::string output_file, TSQueue<Item*>* output_queue)
	: expected_lines(expected_lines), output_queue(output_queue) {
	ofs = std::ofstream(output_file);
}

Writer::~Writer() {
	ofs.close();
}

void Writer::start() {
	// TODO: starts a Writer thread
    if (pthread_create(&t, nullptr, Writer::process, this) != 0) {
        std::cerr << "Failed to create writer thread!" << '\n';
        exit(1);  // Exit if thread creation fails
    }
}

void* Writer::process(void* arg) {
	// TODO: implements the Writer's work
	Writer* writer = static_cast<Writer*>(arg);

    int lines_written = 0;

    // Process items until the expected number of lines is written
    while (lines_written < writer->expected_lines) {
        Item* item = writer->output_queue->dequeue();
		//std::cout << item->key << " " << item->val << " " << item->opcode << '\n';
        writer->ofs << item->key << " " << item->val << " " << item->opcode << '\n';
        ++lines_written;
        delete item;
    }
    return nullptr;
}

#endif // WRITER_HPP
