#pragma once

#include <atomic>

namespace imagiro {

// Lock-free triple buffer for single-producer / single-consumer latest-value communication.
// Writer always has a free buffer. Reader always gets the latest complete snapshot.
// Neither side ever blocks.
template <typename T>
class TripleBuffer {
public:
    T& writeBuffer() { return buffers_[writeIdx_]; }

    void publish() {
        writeIdx_ = middle_.exchange(writeIdx_, std::memory_order_acq_rel);
        newData_.store(true, std::memory_order_release);
    }

    const T& read() {
        if (newData_.exchange(false, std::memory_order_acquire)) {
            readIdx_ = middle_.exchange(readIdx_, std::memory_order_acq_rel);
        }
        return buffers_[readIdx_];
    }

private:
    T buffers_[3] = {};
    std::atomic<int> middle_{1};
    int writeIdx_ = 2;
    int readIdx_ = 0;
    std::atomic<bool> newData_{false};
};

} // namespace imagiro
