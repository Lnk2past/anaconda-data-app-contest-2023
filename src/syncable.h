#pragma once

#include <atomic>
#include <barrier>
#include <functional>
#include <future>
#include <thread>
#include <vector>

/**
 * A thread synchronization wrapper that keeps any number of callables in lockstep with eachother.
 * A pair of std::barrier's are used to keep threads synchronized with a "driver" thread calling
 * Syncable::trigger
 */
struct Syncable
{
    /**
     * Initializes mechanisms with the number of threads specified.
     * 
     * Arguments:
     *     nthreads: number of threads to use
     */
    Syncable(const std::size_t nthreads):
        num_threads(nthreads),
        sync_point_1(nthreads+1),
        sync_point_2(nthreads+1)
    {
        threads.reserve(num_threads);
    }

    /**
     * Initializes mechanisms with the number of threads specified. Also creates the threads from
     * the callables
     * 
     * Arguments:
     *     callables: set of functions to assign to each thread
     */
    Syncable(std::vector<std::function<void(void)>> callables):
        num_threads(callables.size()),
        sync_point_1(callables.size()+1),
        sync_point_2(callables.size()+1)
    {
        threads.reserve(num_threads);
        for (std::size_t i = 0; i < num_threads; ++i)
        {
            threads.emplace_back(
                &Syncable::worker,
                std::ref(*this),
                callables[i]
            );
        }
    }

    /**
     * Destructor to disable the lock and unblock the threads to synchronize.
     */
    ~Syncable()
    {
        lock = false;
        sync_point_2.arrive_and_drop();
        sync_point_1.arrive_and_wait();
    }

    /**
     * Create the threads from the given callables.
     *
     * Arguments:
     *     callables: set of functions to assign to each thread
     */
    void initialize(std::vector<std::function<void(void)>> callables)
    {
        for (std::size_t i = 0; i < num_threads; ++i)
        {
            threads.push_back(
                std::jthread(
                    std::bind(
                        &Syncable::worker,
                        std::ref(*this),
                        callables[i]
                    )
                )
            );
        }
    }

    /**
     * Releases the barrier to execute all pending threads and wait for each thread to complete.
     */
    void trigger()
    {
        if (lock.load())
        {
            sync_point_1.arrive_and_wait();
            sync_point_2.arrive_and_wait();
        }
    }

    /**
     * Wrapper method for the callables to synchronize and execute them repeatedly on a thread.
     * Two sync-points are used to establish when to start all threads and when the trigger
     * thread can proceed.
     *
     * Arguments:
     *     callable: function to execute and synchronize
     */
    void worker(std::function<void(void)> callable)
    {
        while (lock.load())
        {
            sync_point_1.arrive_and_wait();
            if (!lock.load())
            {
                break;
            }
            callable();
            sync_point_2.arrive_and_wait();
        }
    }

    std::atomic<bool> lock = {true};    // flag to signal ending a thread
    std::size_t num_threads {1};        // number of threads
    std::barrier<> sync_point_1 {1};    // synchronization mechanism to start all work on threads for a single step
    std::barrier<> sync_point_2 {1};    // synchronization mechanism to wait for all work to complete for a single step
    std::vector<std::jthread> threads;  // thread pool
};
