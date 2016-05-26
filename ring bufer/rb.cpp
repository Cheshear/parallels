#ifndef SPSC_RING_BUFFER
#define SPSC_RING_BUFFER

#include <iostream>
#include<thread>
#include <atomic>
#include <vector>

template <class T>
class spsc_ring_buffer {
public:
	//конструктор принимает размер буффера
	explicit spsc_ring_buffer(size_t buf_size);
	//довавить элемент в очередь, сообщить успешно ли прошло добавление
	bool enqueue(T e);
	//достать элемент из очереди, сообщить успешно ли удален элемент
	bool dequeue(T& e);
private:

	const std::size_t size;
	struct node_t {
		node_t(T el) :data(el) {};
		T data;
		char pad[128];
	};
	std::atomic<std::size_t> head;
	char pad[128];

	std::atomic<std::size_t> tail;

	std::vector<node_t> data;
};

template <class T>
spsc_ring_buffer<T>::spsc_ring_buffer(size_t buf_size) :
	size(buf_size + 1),
	head(0), 
	tail(0), 
	data(buf_size + 1, node_t(0)) {}

template <class T>
bool spsc_ring_buffer<T>::enqueue(T e) {
	size_t current_head = head.load(std::memory_order_acquire);
	size_t current_tail = tail.load(std::memory_order_relaxed);

	if (current_head == (current_tail + 1) % size)
	{
		return 0;
	}

	data[current_tail] = node_t(e);
	current_tail = (current_tail + 1) % size;

	tail.store(current_tail, std::memory_order_release);
	return 1;
}

template <class T>
bool spsc_ring_buffer<T>::dequeue(T& e) {
	size_t curr_head = head.load(std::memory_order_relaxed);
	size_t curr_tail = tail.load(std::memory_order_acquire);

	if (current_head == current_tail) 
	{
		return 0;
	}

	e = data[current_head].data;
	if (current_head > 0) 
	{
		current_head--;
	}
	else
	{
		current_head = size - 1;
	}

	head.store(current_head, std::memory_order_release);
	return 1;
}

spsc_ring_buffer<int> buf(10);

void call() {
	int a=0;
	buf.enqueue(a);
	//std::cout << a;
}

#endif
int main() {
	int k;
	std::cin >> k;
	int a;
	for (int i = 0; i < k; ++i) {
		std::cin >> a;
		buf.enqueue(a);
	}

	std::thread consumer(call);
	consumer.join();
	return 0;
}
