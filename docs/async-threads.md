## Async Threads

If a certain task can't be performed in the audio task because it does not meet
real-time requirements (for example, allocating memory or reading files, or
performing complex analysis such as large FFTs) then it can be run in an
AsyncThread.

The API is defined in [`CoreModules/async_thread.hh`](https://github.com/4ms/metamodule-core-interface/blob/v2.0-dev/CoreModules/async_thread.hh)


### Usage

Create an `AsyncThread` object in your module class. Provide `this` (which is the pointer
to your module) and a callable object (e.g. a lambda) in the constructor. When
you want the thread to start running call `start()`.

```c++

    AsyncThread async{this, []() {
        // do something in the background
        do_something();
    }};

    MyModule() {
        async.start();
    }

```

Alternatively, you can provide the lambda in `start()` (you still have to pass
`this` to the constructor to AsyncThread)

The thread will be called at irregular intervals (depending on audio load) and
run until completion. If you want it to run at a slow, but more-or-less steady
period, do something like this:

```c++

    long long last_tm = 0;

    AsyncThread async{[this]() {
        auto now_ms = std::chrono::steady_clock::now().time_since_epoch().count() / 1'000'000LL;

        if (now_ms - last_tm > 1000) {
            last_tm = now_ms;
            printf("Out 1 is at %f\n", output1);
        }
    }};

```

If you want it to stop running, call `stop()`. This will not halt current
execution of the thread (if any), it merely prevents it from starting again.

You can also run a thread once by calling `run_once()` instead of `start()`:

```c++
    std::atomic<bool> file_loaded{false};

    AsyncThread file_loader_async{this, [this]() {
        bool success = load_big_file(&filedata);
        if (success)
            file_loaded = true;
    }};

    void process() {
        if (user_pressed_load_button)
            file_loader_async.run_once();

        if (file_loaded)
            do_something(filedata);
    }

```

Threads are stopped when the audio is paused (muted), and resumed when the
patch is played. Like calling `stop()`, they will not have their execution
halted, instead they simply will not get run again until unpaused.


### Concurrency

The engine has a simple scheduler that runs all AsyncThreads periodically at 
a lower priority than the audio thread. This means the audio thread cannot be 
interrupted by the AsyncThread, and the AsyncThread will often be interrupted by
the audio thread. 

At most two AsyncThreads may be running at once (one on each core). If multiple
copies of a module are present in a patch, then multiple copies of an
AsyncThread might be running at the same time, on different cores.

Using `std::atomic` is recommended to pass control of data structures between threads.

For example: 

```c++
    std::atomic<bool> need_more_data{false};
    std::atomic<bool> data_ready{false};

    AsyncThread async{[this]() {
        if (need_more_data) {
            need_more_data = false;
            read_file(buffer); // slow operation
            data_ready = true;
        }
    }};

    void process() {
        //...
        if (buffer.empty()) {
            need_more_data = true;
        }

        if (data_ready) {
            data_ready = false;
            output_data(buffer);
        }
        //...
    }


```
