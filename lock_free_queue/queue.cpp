#include <condition_variable>
#include <mutex>
#include <atomic>
#include <thread>

thread_local auto is_slow_thread = bool{ false };

auto slow_thread_waiting = bool{ false };


auto node_freed = bool{ false };

std::condition_variable cond{};

std::mutex cond_mutex{};

// Implementation of the Michael-Scott algorithm
template <typename T> class queue_t {
	struct node_t;
	struct alignas(16) pointer_t {
		node_t* ptr;
		unsigned int count;
		pointer_t() noexcept : ptr{ nullptr }, count{ 0 } {}
		pointer_t(node_t* ptr) : ptr{ ptr }, count{ 0 } {}
		pointer_t(node_t* ptr, unsigned int count) : ptr{ ptr }, count{ count } {}
		bool operator ==(const pointer_t & other) const {
			return ptr == other.ptr && count == other.count;
		}
	};
	struct node_t {
		T value;
		std::atomic<pointer_t> next;
		// A dummy node, next is initialized with a zero-initialized ptr
		node_t() : next{ pointer_t{} } {}
		// A node filled with a given value, next is initialized with a zero-initialized ptr
		node_t(T value) : value(value), next{ pointer_t{} } {}
	};

	// We're going to do atomic ops on Head
	std::atomic<pointer_t> Head;
	// We're going to do atomic ops on Tail
	std::atomic<pointer_t> Tail;

public:
	queue_t() : Head{ new node_t{} }, Tail{ Head.load().ptr } {}

	void enqueue(T value) {
		// Node is initialized in ctor, so three lines in one
		auto node = new node_t{ value }; // E1, E2, E3
		decltype(Tail.load()) tail;
		while (true) { // E4
			tail = Tail.load(); // E5
								// If we're the slow thread, we wait until the node we just loaded is freed.
			if (is_slow_thread) {
				{
					std::lock_guard<std::mutex> lock{ cond_mutex };
					slow_thread_waiting = true;
				}
				// Let the main thread know we're waiting
				cond.notify_one();
				auto lock = std::unique_lock<std::mutex>{ cond_mutex };
				// Wait until the main thread tells us the node is freed.
				cond.wait(lock, [] { return node_freed; });
			}
			// Use-after-free here in slow thread!
			auto next = tail.ptr->next.load(); //  Read next ptr andcountﬁelds together 
			if (tail == Tail.load()) { //  Are tail andnext consistent? 
				if (!next.ptr) { //  Was Tail pointing to the last node? 
					if (tail.ptr->next.compare_exchange_weak(next, pointer_t{ node, next.count + 1 })) { // E9
						break; //  Enqueueis done. Exit loop 
					} 
				}
				else { // Tail wasnot pointing to the last node 
					Tail.compare_exchange_weak(tail, pointer_t{ next.ptr, tail.count + 1 }); //  Try to swingTail to the next node 
				} 
			} 
		} 

		Tail.compare_exchange_weak(tail, pointer_t{ node, tail.count + 1 }); // E17
	}

	bool dequeue(T* pvalue) {
		decltype(Head.load()) head;
		while (true) { // Keep trying until Dequeueis done 
			head = Head.load(); //  Read Head 
			auto tail = Tail.load(); // Read Tail 
			auto next = head.ptr->next.load(); //  Read Head.ptr–>next
			if (head == Head.load()) { // Are head,tail, and next consistent? 
				if (head.ptr == tail.ptr) { //  Is queue empty or Tail falling behind? 
					if (!next.ptr) { //  Is queue empty?
						return false; // Queueis empty,couldn’t dequeue 
					} 
					Tail.compare_exchange_weak(tail, pointer_t{ next.ptr, tail.count + 1 }); //  Tail is falling behind. Try to advanceit 
				}
				else { // Noneedto dealwith Tail 
					*pvalue = next.ptr->value; 
					if (Head.compare_exchange_weak(head, pointer_t{ next.ptr, head.count + 1 })) { //  Try to swingHead to the next node 
						break; //  Dequeueis done. Exit loop 
					} 
				} 
			} 
		} 
		delete head.ptr; //  It is safenow to free the old dummy node 
		return true; //  Queuewasnot empty, dequeuesucceeded
	}
};

// Empty struct to fill our queue with
struct empty {};

// Our queue
queue_t<empty> queue{};

// The slow thread
void slow_thread() {
	// Set that we're the slow thread
	is_slow_thread = true;
	// Enqueue something
	queue.enqueue(empty{});
};

// The main thread
int main() {
	// Launch the slow thread
	auto slow = std::thread{ slow_thread };
	{
		auto lock = std::unique_lock<std::mutex>{ cond_mutex };
		// Wait until the slow thread is waiting
		cond.wait(lock, [] { return slow_thread_waiting; });
	}
	// Enqueue something
	queue.enqueue(empty{});
	empty ref;
	// Dequeue something
	queue.dequeue(&ref);
	{
		std::lock_guard<std::mutex> lock{ cond_mutex };
		node_freed = true;
	}
	cond.notify_one();
	slow.join();
	return 0;
}
