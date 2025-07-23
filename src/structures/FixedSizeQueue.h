#pragma once
#include <array>

template<typename T, size_t N>
class FixedSizeQueue {
    std::array<T, N> data;
    size_t head = 0;
    size_t tail = 0;
    size_t count = 0;

public:
    void push(const T& item) {
        data[tail] = item;
        tail = (tail + 1) % N;

        if (count == N) {
            // Queue is full, advance head to overwrite oldest
            head = tail;
        } else {
            ++count;
        }
    }

    bool pop(T& item) {
        if (count == 0) return false;
        item = data[head];
        head = (head + 1) % N;
        --count;
        return true;
    }

    bool empty() const { return count == 0; }
    size_t size() const { return count; }
};