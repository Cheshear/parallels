#include<stdio.h>
#include<iostream>
#include<vector>
#include<thread>
#include <chrono>
#include<future>
#include<string>
#include<queue>
#include <stdlib.h>
#include<condition_variable>
#include<mutex>
#include<functional>

class Semaphore {
public:
	Semaphore(int count_ = 0):
		count(count_) {}

	inline void notify()
	{
		std::unique_lock<std::mutex> lock(mtx);
		count++;
		cv.notify_one();
	}

	inline void wait()
	{
		std::unique_lock<std::mutex> lock(mtx);
	
		while (count == 0) {
			cv.wait(lock);
		}
		count--;
	}

private:
	std::mutex mtx;
	std::condition_variable cv;
	int count;
};

void step(std::string str) {
	std::cout << str << std::endl;
}

class Robot
{
public:
	Robot();
	~Robot();
	void walk(std::string str) {
		std::unique_lock<std::mutex> lock(mut);
		step(str);
		cv.notify_all();
	}
	void sem_walk(std::string str) {
		sem1.notify();
		std::unique_lock<std::mutex> lock(mut);
		std::cout << str << std::endl;
		lock.unlock();
		sem1.wait();
	}
private:
	std::mutex mut;
	std::condition_variable cv;
	Semaphore sem1;
};

Robot::Robot()
{
	std::cout << "I am Robot, I am going for a walk" << std::endl;
}

Robot::~Robot()
{
	std::cout << "I am stoped" << std::endl;
}

Robot robot;

void func(std::string str) {
	while (true) {
		robot.walk(str);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void func1(std::string str) {
	int i = 0;
	while (i < 20)
	{
		robot.sem_walk(str);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		i++;
	}
}

int main() {
	std::thread thread1(func1, "left");
	//abort();
	std::thread thread2(func1, "right");
	//_set_abort_behavior(0, _WRITE_ABORT_MSG);
}