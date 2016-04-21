#include<vector>
#include<forward_list>
#include<list>
#include<algorithm>
#include<iostream>
#include <functional> 
#include<atomic>
#include<thread>
#include <ctime>
#include <chrono>
#include <random>
#include<shared_mutex>
#include<array>

template<typename ElementType, class HashFunction = std::hash<ElementType>,
class Comparator = std::equal_to<ElementType >>
class CMyHashTable {
public:
	CMyHashTable(int num_stripes, double policy_, double expend_val_):
		policy(policy_),
		expend_val(expend_val_),
		locks(num_stripes)
	{
		std::forward_list<ElementType> a;
		data.resize(num_stripes, a);
		size.store(0);
	}
	void add(const ElementType& t) {
		size_t hash = HashFunction()(t);
		size_t lockNumber = hash % locks.size();
		std::unique_lock<std::shared_timed_mutex> sharedlocker(locks[lockNumber]);
		size_t bucketNum = hash % data.size();
		for (auto iter = data[bucketNum].cbegin(); iter != data[bucketNum].cend(); ++iter) {
			if (Comparator()(*iter, t)) {
				return;
			}
		}
		data[bucketNum].push_front(t);
		std::cout << t << std::endl;
		int newSize = size++;
		//std::cout <<"realsize" << size.load() << std::endl;
		if (double(newSize / data.size()) > policy) {
			std::cout << "if" << newSize / data.size() << std::endl;
			//std::cout << "Oh now!" << std::endl;
			int oldDataSize = data.size();
			sharedlocker.unlock();
			resize(oldDataSize);
		}
	}
	void remove(const ElementType& t) {
		size_t hash = HashFunction()(t);
		size_t lockNumber = hash % locks.size();
		std::unique_lock<std::shared_timed_mutex> sharedlocker(locks[lockNumber]);
		size_t bucketNum = hash % data.size();
		auto it = std::find(data[bucketNum].begin(), data[bucketNum].end(), t);
		if (it != data[bucketNum].end()) {
			data[bucketNum].remove(t);
			sharedlocker.unlock();
			size--;
		}
		else {
			return;
		}
	}
	bool contains(const ElementType& t) {
		size_t hash = HashFunction()(t);
		size_t lockNumber = hash % locks.size();
		std::unique_lock<std::shared_timed_mutex> sharedlocker(locks[lockNumber]);
		size_t bucketNum = hash % data.size();
		auto it = std::find(data[bucketNum].begin(), data[bucketNum].end(), t);
		bool res = true;
		if (it == data[bucketNum].end()) {
			res = false;
		}
		sharedlocker.unlock();
		return res;
	}
	


	void add(const ElementType& t, const HashFunction& hasher) {
		int hash = hasher(t);
	}
	~CMyHashTable() = default;
private:
	std::vector<std::shared_timed_mutex> locks;
	std::vector<std::forward_list<ElementType>> data;
	std::atomic<int> size;
	double expend_val;
	double policy; // > =0  == max(size/ data.size())
	void resize(int OldSize) {
		//std::cout << " OldSize: " << OldSize << std::endl;
		std::vector<std::unique_lock<std::shared_timed_mutex>> my_locks;
		for (size_t i = 0; i < locks.size(); ++i) {
			my_locks.emplace_back(std::unique_lock<std::shared_timed_mutex>(locks[i]));
		}
		std::vector<std::forward_list<ElementType>> newdata;
		newdata.resize(OldSize*expend_val);
		//std::cout << "newdata_size: " << newdata.size() << std::endl;
		size.store(OldSize*expend_val);
		//std::cout << size.load() << std::endl;
		for (int i = 0; i < OldSize; ++i) {
			for (auto iter = data[i].begin(); iter != data[i].end(); ++iter) {
				size_t hash = HashFunction()(*iter);
				size_t bucketNum = hash % newdata.size();
				newdata[bucketNum].push_front(*iter);
			}
		}
		data.resize(OldSize*expend_val);
		//std::cout << data.size() << std::endl;
		data = std::move(newdata);
		//std::cout << data.size() << std::endl;
		for (size_t i = 0; i < locks.size(); ++i) {
			my_locks[i].unlock();
		}
		std::cout << "data:  " << std::endl;
		for (int i = 0; i < data.size(); ++i) {
			for (auto iter = data[i].begin(); iter != data[i].end(); ++iter) {
				std::cout << *iter << " ";
			}
		}
	}
};

struct CMyStruct {
	int a;
	int b;

	//int GetHash() const { return a + (b << 16); }
};
namespace std {
	template<>
	struct hash<CMyStruct> {
		size_t operator()(const CMyStruct& key)const
		{
			return key.a + (key.b << 16);
		}
	};
};

const int N = 10;
const double policy_ = 1.5;
const double expend_val_ = 2;
CMyHashTable<int> table(N, policy_, expend_val_);

void test(){
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::minstd_rand0 generator(seed);
	std::vector<int> gen;
	gen.resize(8);
	for (int i = 0; i < 8; ++i) {
		gen[i] = generator();
		table.add(gen[i]);
	}
	for (int i = 0; i < 5; ++i) {
		int a = gen[i];
		if (table.contains(a)) {
			std::cout << "contains: " << a << std::endl;
		}
	}
	std::cout << "remove: " << std::endl;
	bool flag = false;
	for (int i = 0; i < 5; ++i) {
		int b = gen[std::rand()%7];
		if (table.contains(b)) {
			std::cout << "contains: " << b << std::endl;
			flag = true;
		}
		table.remove(b);
		if (flag) {
			if (!table.contains(b)) {
				std::cout << "Ok!" << std::endl;
			}
		}
		flag = false;
	}
	std::cout << "test end" << std::endl;
}


int main() {
	int thread_number;
	std::cin >> thread_number;
	CMyHashTable<int> table(N, policy_, expend_val_);
	std::vector<std::thread> threads;
	for (int i = 0; i < thread_number; ++i ) 
	{
		threads.push_back(std::thread(test));
	}
	system("pause");
}