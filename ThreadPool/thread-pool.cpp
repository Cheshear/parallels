#include<stdio.h>
#include<iostream>
#include<vector>
#include<thread>
#include<future>
#include<queue>
#include<condition_variable>
#include<mutex>
#include<functional>

int *poison_pill = NULL;

template<typename T>
class thread_safe_queue {
public:
	thread_safe_queue():queue_capacity(0) {}
	thread_safe_queue(std::size_t capacity);
	void enqueue(T item);
	std::size_t size();
	void pop(T* item);
	~thread_safe_queue() = default;
private:
	std::queue<T> internalqueue;
	std::mutex mut;
	std::size_t queue_capacity;
	std::condition_variable condvar_empty;
	thread_safe_queue(const thread_safe_queue&) = delete;
	void operator= (const thread_safe_queue&) = delete;
};

template<typename T>
thread_safe_queue<T>::thread_safe_queue(std::size_t capacity) :queue_capacity(capacity) {}

template<typename T>
void thread_safe_queue<T>::enqueue(T item) {
	std::unique_lock<std::mutex> lock(mut);
	internalqueue.push(item);
	lock.unlock();
	condvar_empty.notify_one();
}

template<typename T>
std::size_t thread_safe_queue<T>::size() {
	std::unique_lock<std::mutex> lock(mut);
	return internalqueue.size();
}

template<typename T>
void thread_safe_queue<T>::pop(T* item) {
	std::unique_lock<std::mutex> lock(mut);
	condvar_empty.wait(lock, [this]() {return internalqueue.size() > 0; });
	*item = internalqueue.front();
	internalqueue.pop();
	lock.unlock();
}

//template <typename T>

template<typename T>
class thread_pool {
public:
	struct tasks_and_results {
		std::shared_ptr<std::promise<T>> promise;
		std::function<T()> func;

		tasks_and_results(std::function<T()> _func, std::shared_ptr<std::promise<T>> prom_ptr)
			: func(_func),
			promise(prom_ptr)
		{}
		tasks_and_results()
			: func(nullptr), 
			promise(std::make_shared<std::promise<T>>())
		{}
		tasks_and_results& operator=(tasks_and_results tar) {
			this->func = tar.func;
			this->promise = tar.promise;
			return *this;
		}
	};
	thread_pool();
	thread_pool(std::size_t num_threads);
	std::future<T> submit(std::function<T()> func);
	void shutdown();
	~thread_pool();
private:
	std::size_t num_threads;
	std::vector<std::thread> workers;
	thread_safe_queue<tasks_and_results> tasks;
};

template<typename T>
thread_pool<T>::thread_pool(std::size_t _num_threads) {
	tasks();
	num_threads = _num_threads;
	auto call = [this]()->void {
		tasks_and_results current;
		tasks.pop(&current);
		while (current.func != nullptr) {
			//std::cout << "while!" << std::endl;
			current.promise->set_value(current.func());
			tasks.pop(&current);

		}
	};
	for (int i = 0; i < num_threads; ++i) {
		workers.push_back(std::thread(call));
	}
}

template<typename T>
thread_pool<T>::thread_pool() {
	num_threads = 1;
	auto call = [this]()->void{
			tasks_and_results current;
			tasks.pop(&current);
			while (current.func != nullptr) {
				//std::cout << "while!" << std::endl;
				current.promise->set_value(current.func());
				tasks.pop(&current);
			
		}
	};
	workers.push_back(std::thread(call));
}

template<typename T>
std::future<T> thread_pool<T>::submit(std::function<T()> func) {
	std::shared_ptr<std::promise<T>> prom = std::make_shared<std::promise<T>>(std::promise<T>());
	tasks.enqueue(tasks_and_results(func, prom ));
	return prom->get_future();
}
template<typename T>
void thread_pool<T>::shutdown() {
	for (int i = 0; i < num_threads; ++i) {
		tasks.enqueue(tasks_and_results());
	}
}


template<typename T>
thread_pool<T>::~thread_pool() {
	shutdown();
	for (int i = 0; i < num_threads; ++i) {
		workers[i].join();
	}
}

int sum(int a, int b) {
	return a + b;
}

int main() {
	thread_pool<int> my_pool;
	std::vector<std::future<int>> results;
	std::vector<int> a;
	std::vector<int> b;
	for (int i = 0; i < 10; ++i) {
		a.push_back(std::rand());
		b.push_back(std::rand());
		results.push_back(my_pool.submit(std::bind(sum, a[i], b[i])));
	}
	for (int i = 0; i < 10; ++i) {
		std::cout << a[i] << " " << b[i] << " " << results[i].get() << std::endl;
	}
	system("pause");
	return 0;
}