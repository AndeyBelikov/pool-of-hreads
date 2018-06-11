#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

class threadPool {
private:

	std::mutex pushMutex;
	std::condition_variable condWar;
	//std::vector<std::exception_ptr> ex;

	bool done = false;

public:

	void complete() 
	{
		done = true;
	}

	void producer(std::queue<int> &intQueue, int id) {

		std::this_thread::sleep_for(std::chrono::seconds(1));

		{
			std::unique_lock<std::mutex> lock(pushMutex);
			for (int i = 0; i < 10; ++i) {
				intQueue.push(id);
			}
		}

		condWar.notify_one();
	}

	void consumer(std::queue<int> &intQueue) {

		while (!done) {

			std::unique_lock<std::mutex> lock(pushMutex);
			condWar.wait(lock, [&]() { return !intQueue.empty(); });

			while (!intQueue.empty())
			{
				std::cout << intQueue.front() << " ";
				intQueue.pop();
			}

			std::cout << std::endl;
		}
	}
};

int main() {

	std::vector<std::thread> threads;
	std::queue<int> intQueue;
	threadPool pool;

	std::thread consumerThread( [&pool, &intQueue]() {pool.consumer(intQueue); } );

	for (int i = 0; i < 10; i++) {
		threads.emplace_back( [&pool, &intQueue, i]() {pool.producer(intQueue, i); } );
	}

	std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

	pool.complete();

	consumerThread.join();

	std::cout << "hello from main" << std::endl;

	getchar();
	return 0;
}