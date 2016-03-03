#include<atomic>
#include<vector>
#include<thread>
#include<math.h>
#include<array>
#include<cmath>
#include<iostream>

class peterson_mutex {
public:
	peterson_mutex() {
		want[0].store(false);
		want[1].store(false);
		victim.store(0);
	}

	peterson_mutex(const peterson_mutex &other)
	{
		want[0].store(other.get_want_0());
		want[1].store(other.get_want_1());
		victim.store(other.get_victim());
	}

	bool get_want_0() const
	{		
		return want[0].load();
	}

	bool get_want_1() const
	{
		return want[1].load();
	}

	bool get_victim() const
	{
		return victim.load();
	}

	peterson_mutex &operator= (const peterson_mutex &other) {
		want[0].store(other.get_want_0());
		want[1].store(other.get_want_1());
		victim.store(other.get_victim());
	}

	void lock(int t) {
		want[t].store(true);
		victim.store(t);
		while (want[1 - t].load() && victim.load() == t) {
			std::this_thread::yield();
			// wait
		}
	}

	void unlock(int t) {
		want[t].store(false);
	}

	~peterson_mutex() = default;

private:

	std::array<std::atomic<bool>, 2> want;
	std::atomic<int> victim;
	//bool power_of_2;
};

class TournamentTree
{
private:
	std::vector<peterson_mutex> lock;
	std::size_t num_threads;
	std::size_t num_levels;
	bool power_of_2;
	std::vector<std::size_t> p_id;
public:
	TournamentTree(std::size_t num_threads);
	void Acquire_mutex(std::size_t thread_index);
	void Release_mutex(std::size_t thread_index);
	~TournamentTree() {};
};

TournamentTree::TournamentTree(std::size_t num_threads_) 
{
	num_threads = num_threads_;
	double n_th = static_cast<double>(num_threads);
	double n_l = log2(n_th);
	num_levels = n_l;
	if (n_l > num_levels) {
		power_of_2 = false;
		 ++num_levels;
	}
	else {
		power_of_2 = true;
	}
	lock.resize(num_threads);
	const peterson_mutex a_i();
	p_id.resize(num_threads);
}

void TournamentTree::Acquire_mutex(std::size_t thread_index) {
	std::size_t node_id = thread_index + num_threads;
	//std::cout << "I start working!!!" << std::endl;
	if (power_of_2) {
		//std::cout << "Yahoo!!!" << std::endl;
		for (std::size_t level = 0; level < num_levels; ++level) {
			p_id[level] = node_id % 2;
			node_id = node_id / 2; 
			lock[node_id].lock(p_id[level]);
		}
	}
	else {
		//if (num_threads%2==0) {
		//std::cout << "Oh, no!!!" << std::endl;
		//std::cout << node_id / (pow(2.0, num_levels - 1)) << std::endl;
		double x, y;
		y = std::modf(node_id / (pow(2.0, num_levels - 1)) , &x);
			if (x == 1) {
				std::cout << node_id << std::endl;
				for (std::size_t level = 1; level < num_levels; ++level) {
					p_id[level] = node_id % 2;
					node_id = node_id / 2;
					lock[node_id].lock(p_id[level]);
				}
			}
			else {
				std::cout << node_id << std::endl;
				for (std::size_t level = 0; level < num_levels; ++level) {
					p_id[level] = node_id % 2;
					node_id = node_id / 2;
					lock[node_id].lock(p_id[level]);
				}
			}
	}
	//std::cout << "I finish working!!!" << std::endl;
}

void TournamentTree::Release_mutex(std::size_t thread_index) {
	std::size_t node_id_up = 1;
	if (power_of_2) {
		for (size_t level = num_levels; level > 0; --level) {
			lock[node_id_up].unlock(p_id[level]);
			node_id_up = 2 * node_id_up + p_id[level];
		}
	}
	else {
		std::size_t node_id_down = thread_index + num_threads;
		double x, y;
		y = std::modf(node_id_down / (pow(2.0, num_levels - 1)), &x);
		if (x == 1) {
			for (size_t level = num_levels; level > 1; --level) {
				lock[node_id_up].unlock(p_id[level]);
				node_id_up = 2 * node_id_up + p_id[level];
			}
		}
		else
		{
			for (size_t level = num_levels; level > 0; --level) {
				lock[node_id_up].unlock(p_id[level]);
				node_id_up = 2 * node_id_up + p_id[level];
			}
		}
	}
}
//std::size_t num_of_threads ;
//TournamentTree mutex(num_of_threads);

void thread_function(std::size_t i, std::size_t num_of_threads) {
	TournamentTree mutex(num_of_threads);
	mutex.Acquire_mutex(i);
	std::cout << "Hello parrallel world!" << std::endl;
	mutex.Release_mutex(i);
}

int main() {
	std::size_t num_of_threads;
	std::cin >> num_of_threads;
	std::vector<std::thread> threads;
	for (std::size_t i = 0; i < num_of_threads; ++i) {
		threads.push_back(std::thread(thread_function, i, num_of_threads));
	}
	for (int i = 0; i < num_of_threads; ++i) {
		threads[i].join();
	}
	system("pause");
	return 0;
}