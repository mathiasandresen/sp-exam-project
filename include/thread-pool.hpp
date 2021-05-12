/**
 * Solution to exercise about thread pool implementation
 * Author Marius Mikucionis <marius@cs.aau.dk>
 * Assumes C++17 (due to std::apply and constexpr).
 * Can be improved in C++20 with perfect capture without tuple.
 */

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <queue>

class thread_pool
{
	std::vector<std::thread> threads;
	std::mutex mutex; // guards tasks and termination condition
	std::condition_variable cond; // signals between threads
	std::queue<std::function<void()>> tasks;
	bool terminate{false};

public:
	thread_pool(size_t number_of_threads) {
		while (number_of_threads-->0)
			threads.emplace_back(
				[this] {
					auto task = std::function<void()>{};
					while (true) {
						{
							auto lock = std::unique_lock(mutex);
							while (tasks.empty() && !terminate)
								cond.wait(lock); // release,wait,acquire
							if (!tasks.empty()) {
								task = tasks.front();
								tasks.pop();
							} else return;
						}
						task(); // execute the task
					}
				});
	}
	thread_pool(): thread_pool(std::thread::hardware_concurrency()) {}

	template <typename Fn, typename...Args,
			  typename Ret=std::result_of_t<std::decay_t<Fn>(std::decay_t<Args>...)>>
	std::future<Ret> async(Fn&& fn, Args&&... args) {
		 // promise is not copyable, thus wrap into smart pointer
		auto ret = std::make_shared<std::promise<Ret>>();
		auto fut = ret->get_future();
		auto l = [ret, fn=std::forward<Fn>(fn),
				  args=std::make_tuple(std::forward<Args>(args)...)]()
				  	mutable
				 { // C++20 allows perfect capture without tuple
					 try {
						 if constexpr(std::is_same_v<Ret,void>) {
                            std::apply(fn, std::move(args)); // requires C++17
						    ret->set_value();
                         } else {
						    ret->set_value(std::apply(fn, std::move(args)));
						 }
					 } catch (...) {
						 ret->set_exception(std::current_exception());
					 }
				 };
		{
			auto lock = std::unique_lock(mutex);
			tasks.push(std::move(l));
			cond.notify_all();
		}
		return fut;
	}

	template <typename Fn, typename In, typename Out=std::invoke_result_t<Fn,In>>
	std::vector<std::future<Out>> dispatch(Fn&& fn, const std::vector<In>& inputs)
	{
		auto res = std::vector<std::future<Out>>{};
		res.reserve(inputs.size());
		for (auto& input: inputs)
			res.push_back(async(std::forward<Fn>(fn), input));
		return res;
	}

	~thread_pool(){
		{
			auto lock = std::unique_lock(mutex);
			terminate = true;
			cond.notify_all();
		}
		for (auto& thread: threads)
			thread.join();
	}
};


template <typename T>
std::vector<T> collect(std::vector<std::future<T>>&& futs) {
	auto res = std::vector<T>{};
	res.reserve(futs.size());
	for (auto& f: futs)
		res.push_back(f.get());
	return res;
}

#endif /* THREAD_POOL_HPP */
