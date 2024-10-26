#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
using namespace std;

// Class that represents a simple thread pool
class ThreadPool {

private:
    // Vector to store worker threads
    vector<thread> threads;

    // Queue of tasks
    queue<function<void()> > tasks;

    // Mutex to synchronize access to shared data
    mutex queue_mutex;

    // Condition variable to signal changes in the state of
    // the tasks queue
    condition_variable condition;

    // Flag to indicate whether the thread pool should stop
    // or not
    bool stop = false;


public:
    // // Constructor to creates a thread pool with given
    // number of threads
    ThreadPool(size_t num_threads = thread::hardware_concurrency())
    {

        // Creating worker threads
        for (size_t i = 0; i < num_threads; ++i) {
            auto queue_unlock_lambda = [this]() {
                while (true) {
                    function<void()> task; //create a task which is a void function, cant use non void function
                    // unlock the queue before executing the task so that other threads can perform enqueue tasks
                    {
                        // Locking the queue, the lock will prevent thread race
                        unique_lock<mutex> lock(queue_mutex);

                        //lambda to check if the queue is empty or the pool is stopped
                        auto isEmptyOrStoppedLambda = [this]() {
                            return !tasks.empty() || stop;
                        };
                        // Waiting until there is a task to execute or the pool is stopped. need that the lambda be true for the lock to be unlocked
                        condition.wait(lock, isEmptyOrStoppedLambda);

                        // exit the thread in case the pool
                        // is stopped and there are no tasks
                        if (stop && tasks.empty()) {
                            return;
                        }

                        // Get the next task from the queue
                        task = move(tasks.front());
                        tasks.pop();
                    }

                    task();
                }
            };
            threads.emplace_back(queue_unlock_lambda);
        }
    }

    // Destructor to stop the thread pool
    ~ThreadPool()
    {
        {
            // Lock the queue to update the stop flag safely
            unique_lock<mutex> lock(queue_mutex);
            stop = true;
        }

        // Notify all threads
        condition.notify_all();

        // Joining all worker threads to ensure they have
        // completed their tasks
        for (auto& thread : threads) {
            thread.join();
        }
    }

    // Enqueue task for execution by the thread pool
    void enqueue(function<void()> task)
    {
        {
            unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(move(task));
        }
        condition.notify_one();
    }
};

int main()
{
    // Create a thread pool with 4 threads
    ThreadPool pool(4);

    // Enqueue tasks for execution
    for (int i = 0; i < 5; ++i) {
        pool.enqueue([i] {
            cout << "Task " << i << " is running on thread "
                 << this_thread::get_id() << endl;
            // Simulate some work
            this_thread::sleep_for(
                chrono::milliseconds(100));
        });
    }

    return 0;
}
