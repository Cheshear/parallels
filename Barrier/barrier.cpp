#include<thread>
#include<condition_variable>
#include <random>
#include<mutex>
#include<vector>
#include<iostream>

class Barrier {
public:
	Barrier(std::size_t num_threads_);
	//Barrier(const Barrier &bar);
	void enter();
	~Barrier();
private:
	int era;
	std::size_t num_threads_near;
	std::size_t num_threads;
	std::mutex mut;
	std::condition_variable cv;
};

Barrier::Barrier(std::size_t num_threads_): 
	num_threads(num_threads_), 
	num_threads_near(0),
	era(0){}

//Barrier::Barrier(const Barrier &bar) = default;
void Barrier::enter() {
	std::unique_lock<std::mutex> lock(mut);
	num_threads_near++;
	if ((num_threads_near == num_threads)) {
		era++;
		num_threads_near = 0;
		cv.notify_all();
	}
	else {
		std::size_t current_era = era;
		cv.wait(lock, [this, current_era]() {return current_era == era; });
	}
}

Barrier::~Barrier() {};

int main() {
	int N;
	std::cin >> N;
	std::vector<std::thread> workers;
	//Barrier bar(N);
	std::shared_ptr<Barrier> bar1= std::make_shared<Barrier>(N);
	auto call = [bar1](int i, std::mt19937 &generator) {
		//std::mutex mut1;
		std::mutex mut2;
		std::condition_variable cv;
	/*	std::lock_guard<std::mutex> lock(mut1);
		std::cout << "my number is " + i << std::endl;*/
		bar1->enter();
		{
			std::unique_lock<std::mutex> lock(mut2); 
			//cv.wait(lock);
			std::cout << i << std::endl; 
			//cv.notify_one();
		}
		std::this_thread::sleep_for(std::chrono::seconds(1 + generator() % 5));
	};
	std::mt19937 generator((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
	for (int i = 0; i < N; ++i) {
		workers.push_back(std::thread(std::bind(call, i, std::ref(generator))));
	}
	for (int i = 0; i < N; ++i) {
		workers[i].join();
	}
	system("pause");
	return 0;
}