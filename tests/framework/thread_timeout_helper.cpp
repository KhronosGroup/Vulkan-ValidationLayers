/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "thread_timeout_helper.h"

bool ThreadTimeoutHelper::WaitForThreads(int timeout_in_seconds) {
    std::unique_lock lock(mutex_);
    return cv_.wait_for(lock, std::chrono::seconds{timeout_in_seconds}, [this] {
        std::lock_guard lock_guard(active_thread_mutex_);
        return active_threads_ == 0;
    });
}

void ThreadTimeoutHelper::OnThreadDone() {
    bool last_worker = false;
    {
        std::lock_guard lock(active_thread_mutex_);
        active_threads_--;
        assert(active_threads_ >= 0);
        if (!active_threads_) {
            last_worker = true;
        }
    }
    if (last_worker) {
        cv_.notify_one();
    }
}
