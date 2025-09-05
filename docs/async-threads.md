## Async Threads

If a certain task can't be performed in the audio task because it does not meet
real-time requirements (for example, allocating memory or reading files, or
performing complex analysis such as large FFTs) then it can be run in an
AsyncThread.

The API is defined in [`threads/async_thread.hh`](../core-interface/threads/async_thread.hh)

Each AsyncThread is called periodically (about every 0.5ms) and will be run from start to
finish. The only thing that can interrupt an AsyncThread is the audio thread
itself.

You should design your AsyncThreads to be run repeatedly. Typically they will check 
for some flag or state, perform an action if needed, or immediately exit if no action is needed.

If you have a one-off action that 
needs to be performed, you can use an AsyncThread to do this, but it might be more simple
to perform that action in the constructor or destructor of your module.

## Background and Best Practices

Async Threads run in the background of the **audio** thread. That is, the audio
will continue playing without interrupts while the async thread is run intermittently
when it can without disturbing the audio processing.

Async Threads do **not** run in the background of the GUI thread. They are more or less
concurrent to the GUI thread (the scheduling is complicated since there are two Async Thread
runners -- one of which will interrupt the GUI thread and the other one will run simulataneously
with the GUI thread). The goal of Async Threads is not to be able to allow the user
to use the GUI while the thread is running, but rather the goal is to allow the audio
to play without interruption or overload.

If your AsyncThread takes too long to execute, the GUI thread might stall until it completes.
So try to break up your tasks into shorter sub-tasks, and perform the sub-tasks in order
each time the AsyncThread is run.

The thread runners start when a patch is loaded. The runners are stopped when the
audio is paused (muted) or the patch is unloaded. They are resumed again when
the patch is un-muted. Whenever a thread runner is paused, the currently
executing task will always run to completion. Then, no more threads will be run
until the thread runner is un-paused.


### Basic Usage

Create an `AsyncThread` object in your module class. Provide `this` (which is the pointer
to your module) and a callable object (e.g. a lambda) in the constructor. When
you want the thread to start running call `start()`.

```c++

    AsyncThread async{this, [this]() {
        do_something();
    }};

    MyModule() {
        async.start();
    }

    void do_something() {
        // something happens in the background here..
    }

```

Alternatively, you can provide the lambda in `start()` (you still have to pass
`this` to the constructor to AsyncThread)

If you want it to stop running, call `stop()`. This will not halt current
execution of the thread (if any), it merely prevents it from starting again.

## More realistic usage

The above example is not very useful because `do_something()` will be called every time the thread is run,
and the audio thread and AsyncThreads do not share any information.
A more useful AsyncThread will communicate with the module to know when (and what) to do.

Use `std::atomic` to communicate between threads, i.e. to pass control of data
structures and to signal changes in state.

For example: 

```c++
    std::atomic<bool> need_more_data{false};
    std::atomic<bool> data_ready{false};

    AsyncThread async{this, [this]() {
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

    MyModule() {
        async.start();
    }

```

To be more robust in the above example, you should use the proper concurrency memory model when setting data_ready:
```c++
data_ready.store(true, std::memory_order_release)
```

Doing this prevents the compiler from setting `data_ready` to true before `read_file()` completes

## Timed execution

If you want it to run at a slow, but more-or-less steady
period, do something like this:

```c++

    long long last_tm = 0;

    AsyncThread async{this, [this]() {
        auto now_ms = std::chrono::steady_clock::now().time_since_epoch().count() / 1'000'000LL;

        if (now_ms - last_tm > 1000) {
            last_tm = now_ms;
            printf("Output 1 reads %f\n", output1);
        }
    }};

```


## One-shot

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


### Concurrency

The engine has a simple scheduler that runs all AsyncThreads periodically at 
a lower priority than the audio thread. This means the audio thread cannot be 
interrupted by the AsyncThread, and the AsyncThread will often be interrupted by
the audio thread. 

At most two AsyncThreads may be running at once (one on each core). If multiple
copies of a module are present in a patch, then multiple copies of an
AsyncThread might be running at the same time, on different cores.

AsyncThreads running on Core 0 tend to have less time to run because more
audio-related tasks are run on that core. These AsyncThreads will not slow
down the GUI when run. On the other hand, AsyncThreads running on Core 1 tend
to have more execution time, but they will stall the GUI thread whenever
running. Since you cannot control which core your AsyncThreads are running on,
always design your system such that each time the AsyncThread is called, it
does the minimum amount of work necessary and then returns. This will help keep
the GUI responsive, and share time with other modules' AsyncThreads.

