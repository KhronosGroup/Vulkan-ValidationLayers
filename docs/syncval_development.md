# Synchronization Validation Development

This document contains miscellaneous information related to development of synchronization validation.

## SyncVal error messages (how to diff them)

The syncval error reporting code uses helper routines to generate the parts of the error messages. One potential issues with this shared code is when modifying a specific error message it's possible to accidentally break another one. The easiest way to validate the scope of the changes is to compare the output of all negative tests before and after the change.

The following components of the error messages vary between the runs:
* gtest timing
* dispatchable handles

`--gtest_print_time=0` removes the timing information

Use [this patch](https://github.com/user-attachments/files/18318045/remove-dispatchable-handles.patch) to replace all dispatchable handles with zero values. It should be applied only locally to `layers/error_messages/logging.cpp`.

Run the negative tests before and after the change:
```
--gtest_filter=NegativeSyncVal*.* --print-vu --gtest_print_time=0 >syncval_{before/after}.txt
```

Diff-compare output files.

## SyncVal stats

Build the project with `VVL_ENABLE_SYNCVAL_STATS=1` preprocessor definition to enable the collection of syncval statistics. This can be set either as a -D option in CMake or modified manually in `layers/sync/sync_stats.h`

If `VVL_ENABLE_SYNCVAL_STATS=1` environment variable is also set, statistics will be printed to console when the application exits. During development, statistics can be printed at any time by calling `Stats::CreateReport()`. The statistics tracking object is a member of the syncval validator (`SyncValidator::stats`) and can be inspected directly during development.

If the *mimalloc* allocator is used, syncval statistics can also collect allocation information using the mimalloc stats system. The mimalloc dependency must be build with `MI_STAT=1` preprocessor definition. The total amount of allocated memory is tracked in `Stats::total_allocated_memory`, and all mimalloc stats are stored in `Stats::mi_stats`.

The mimalloc statistics are updated at fixed points: `vkQueueSubmit`, `vkQueuePresent`, and when generating a report via `Stats::CreateReport()`. To update mimalloc stats manually at arbitrary point, call `Stats::UpdateMemoryStats`.

## Queue progress tracking in SyncVal and Core Checks

Core Checks and SyncVal use different models of queue progress.

Core Checks simulates queue forward progress to track whether resources are still in use. A submission can take an arbitrarily long time to complete, so Core Checks cannot assume progress just because enough time has passed or many more submissions were made. Only a host synchronization operation, such as `vkWaitForFences`, moves the simulated queue forward.

SyncVal simulates queue forward progress to track resource accesses in submitted GPU commands. Once a submission's semaphore dependencies are resolved, SyncVal can process it without waiting for the device. No host synchronization operation is needed. A submission is usually processed during its own `QueueSubmit` call. For timeline semaphores, a wait-before-signal submission remains pending until the matching signal is submitted in a later `QueueSubmit` call.

Core Checks and SyncVal share the same `vvl::Semaphore` state. Because this state also supports the Core Checks progress model, its `timeline_` map keeps timepoints until a host synchronization operation moves the queue forward. SyncVal's own queue progress tracking does not need this full history. If an application continues submitting timeline semaphore operations without host synchronization, timepoints accumulate in the `timeline_` map. This is expected behavior of the shared progress model, not a practical memory issue.
