#include<queue>
#include<future>
#include<iostream>
#include<condition_variable>
#include<thread>
#include<mutex>
#include<string>
#include<array>
#include<ctime>
#include<cstdlib>
#include<cmath>
#include"Queue.h"

const std::size_t queue_size = 15;
const unsigned int poison_pill = 0;
const unsigned int number_of_consumers = 5;
const std::size_t number_of_tasks = 100;

thread_safe_queue<int> my_queue(queue_size);

void producer_func() {
	int value = 0;
	for (int i = 0; i < number_of_tasks; ++i) {
		//srand(time(NULL));
		value = 1 + std::rand() % 10000;
		my_queue.enqueue(value);
	}
	for (int i = 0; i < number_of_consumers; ++i) {
		my_queue.enqueue(poison_pill);
	}
}

bool is_simple(int value) {
	int k = 2;
	if (value < 2) {
		return false;
	}
	else if (value == 2) {
		return true;
	}
	while (k != (int)pow(value, 1/2)) {
		if (value%k == 0) {
			return false;
		}
		++k;
	}
	return true;
}

void safe_print(const std::string& s) {
	std::mutex mut;
	std::unique_lock<std::mutex> lock(mut);
	std::cout << s << std::endl;
	lock.unlock();
}

void consumers_func() {
	int value = 0;
	while (true) {
		my_queue.pop(value);
		if (is_simple(value)) {
			safe_print(std::to_string(value) + " simple");
		}
		else {
			safe_print(std::to_string(value) + " not simple");
		}
		if (value == poison_pill) {
			break;
		}
	}
}

int main() {
	std::thread producer(producer_func);
	std::array<std::thread, number_of_consumers> consumers;
	for (int i = 0; i < number_of_consumers; ++i ) {
		consumers[i] = std::thread(consumers_func);
	}
	producer.join();
	for (int i = 0; i < number_of_consumers; ++i ) {
		consumers[i].join();
	}
	system("pause");
	return 0;
}