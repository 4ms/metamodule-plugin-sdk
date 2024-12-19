## Async Threads

If a certain task can't be performed in the audio task because it does not meet
real-time requirements (for example, allocating memory or reading files, or
performing complex analysis such as large FFTs) then it can be run in an
AsyncThread.

The API is defined in `CoreModules/async_thread.hh`


### Usage

Create an `AsyncThread` object in your module class. Provide a callable object (e.g. a lambda) in the constructor. When you want the thread to start running call `start(id)` where id is the CoreProcessor::id (which is defined and set automatically when the patch is loaded). Alternatively, you can provide the lambda and id in `start()`.

```c++

	AsyncThread async{[this]() {
        // do something in the background
        do_something(this->params);
	}};

	MyModule() {
		async.start(id);
	}

```


The lambda will be called at irregular intervals (depending on audio load), so if you want it to run more or less at a steady period, do something like this:

```c++

	long long last_tm = 0;

	AsyncThread async{[this]() {
		auto now = std::chrono::steady_clock::now().time_since_epoch().count() / 1'000'000LL;

		if (now - last_tm > 1000) {
			last_tm = now;
			printf("Out 1 is at %f\n", output1);
		}
	}};

```


### Concurrency

The engine has a simple scheduler that runs all AsyncThreads periodically at 
a lower priority than the audio thread. This means the audio thread cannot be 
interrupted by the AsyncThread, and the AsyncThread will often be interrupted by
the audio thread. 
At most two AsyncThreads may be running at once. If multiple copies of a module
are present in a patch, then multiple copies of an AsyncThread might be running
at the same time, on different cores.

Keep this in mind when sharing data between threads.

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
