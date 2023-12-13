//
// Created by August Pemberton on 13/12/2023.
//

#pragma once

class ConditionLock
{
public:
    void wait() {
        std::unique_lock<std::mutex> lock(_mutex);
        _cv.wait(lock, [&] { return flag; });
    }

    void notify() {
        std::unique_lock<std::mutex> lock(_mutex);
        flag = true;
        lock.unlock();
        _cv.notify_all();
    }

    void lock() {
        flag = false;
    }

    bool getFlag() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return flag;
    }
private:
    mutable std::mutex _mutex;
    std::condition_variable _cv;
    bool flag;
};