#include<queue>
#include<future>
#include<iostream>
#include<condition_variable>
#include<thread>
#include<mutex>

template<typename T>
class thread_safe_queue {
public:
	thread_safe_queue(std::size_t capacity);
	void enqueue(T item);
	void pop(T& item);
	~thread_safe_queue()=default;
private:
	std::queue<T> internalqueue;
	std::mutex mut;
	std::size_t queue_capacity;
	std::condition_variable condvar_full;
	std::condition_variable condvar_empty;
	thread_safe_queue(const thread_safe_queue&) = delete;
	void operator= (const thread_safe_queue&) = delete;
};

template<typename T>
thread_safe_queue<T>::thread_safe_queue(std::size_t capacity):queue_capacity(capacity) {}

template<typename T>
void thread_safe_queue<T>::enqueue(T item) {
	std::unique_lock<std::mutex> lock(mut);
	condvar_full.wait(lock, [this]() {return internalqueue.size() < queue_capacity; });
	internalqueue.push(item);
	lock.unlock();
	condvar_empty.notify_one();
}
template<typename T>
void thread_safe_queue<T>::pop(T& item) {
	std::unique_lock<std::mutex> lock(mut);
	condvar_empty.wait(lock, [this]() {return internalqueue.size() > 0; });
	item = internalqueue.front();
	internalqueue.pop();
	lock.unlock();
	condvar_full.notify_one();
}

