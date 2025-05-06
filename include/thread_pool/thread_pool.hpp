#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

namespace tp 
{
	struct TaskQueue
	{
		std::queue<std::function<void()>> task_;
		std::mutex mutex_;
		std::atomic<uint32_t> remaining_task_ = 0;

		template<typename TCallback>
		void addTask(TCallback& callback)
		{
			std::lock_guard<std::mutex> lock_guard{ mutex_ };
			task_.push(std::forward<TCallback>(callback));
			remaining_task_++;
		}

		void getTask(std::function<void()>& target_callback)
		{
			{
				std::lock_guard<std::mutex> lock_guard{ mutex_ };
				if (task_.empty())
				{
					return;
				}
				target_callback = std::move(task_.front());
				task_.pop();
			}
		}

		static void wait()
		{
			std::this_thread::yield();
		}

		void waitForCompletion() const
		{
			while (remaining_task_ > 0)
			{
				wait();
			}
		}

		void workDone()
		{
			remaining_task_--;
		}
	};

	struct Worker 
	{
		uint32_t id_ = 0;
		std::thread thread_;
		std::function<void()> task_ = nullptr;
		bool running_ = true;
		TaskQueue* queue_ = nullptr;

		Worker() = default;

		Worker(TaskQueue& queue, uint32_t id) 
			: id_{ id }, queue_{ &queue }
		{
			thread_ = std::thread([this]() {
				run();
			});
		}

		void run()
		{
			while (running_) 
			{
				queue_->getTask(task_);
				if (task_ == nullptr)
				{
					TaskQueue::wait();
				}
				else
				{
					task_();
					queue_->workDone();
					task_ = nullptr;
				}
			}
		}

		void stop()
		{
			running_ = false;
			thread_.join();
		}
	};

	struct ThreadPool
	{
		uint32_t thread_count_ = 0;
		TaskQueue queue_;
		std::vector<Worker> workers_;

		explicit 
			ThreadPool(uint32_t thread_count) 
			: thread_count_(thread_count)
		{
			workers_.reserve(thread_count);
			for (uint32_t i{thread_count}; i--;)
			{
				workers_.emplace_back(queue_, static_cast<uint32_t>(workers_.size()));
			}
		}

		virtual ~ThreadPool()
		{
			for (Worker& worker : workers_)
			{
				worker.stop();
			}
		}

		template<typename TCallback>
		void addTask(TCallback&& callback)
		{
			queue_.addTask(std::forward<TCallback>(callback));
		}

		void waitForCompletion() const
		{
			queue_.waitForCompletion();
		}

		template<typename TCallback>
		void dispatch(uint32_t element_count, TCallback&& callback)
		{
			const uint32_t batch_size = element_count / thread_count_;
			for (uint32_t i{0}; i < thread_count_; ++i)
			{
				const uint32_t start = batch_size * i;
				const uint32_t end = start + batch_size;
				addTask([start, end, &callback]() { callback(start, end); });
			}

			if (batch_size * thread_count_ < element_count)
			{
				const uint32_t start = batch_size * thread_count_;
				callback(start, element_count);
			}

			waitForCompletion();
		}
	};
}
#endif // !THREADPOOL_H
