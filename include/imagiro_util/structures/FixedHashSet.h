#pragma once
#include <array>
#include <functional>

template<typename T, size_t N>
class FixedHashSet {
    struct Slot { T value; bool occupied = false; bool deleted = false; };
    std::array<Slot, N> table;

    size_t hash(const T& key) const { return std::hash<T>{}(key) % N; }

public:
    // Default constructor
    FixedHashSet() = default;

    // Copy constructor
    FixedHashSet(const FixedHashSet& other)
        : table(other.table), active_count(other.active_count) {}

    // Copy assignment operator
    FixedHashSet& operator=(const FixedHashSet& other) {
        if (this != &other) {
            table = other.table;
            active_count = other.active_count;
        }
        return *this;
    }

    // Move constructor
    FixedHashSet(FixedHashSet&& other) noexcept
        : table(std::move(other.table)), active_count(other.active_count) {
        other.active_count = 0;
    }

    // Move assignment operator
    FixedHashSet& operator=(FixedHashSet&& other) noexcept {
        if (this != &other) {
            table = std::move(other.table);
            active_count = other.active_count;
            other.active_count = 0;
        }
        return *this;
    }

    class iterator {
        Slot* slots;
        size_t pos;
        size_t size;

        void advance_to_valid() {
            while (pos < size && (!slots[pos].occupied || slots[pos].deleted)) {
                ++pos;
            }
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator(Slot* slots, size_t pos, size_t size)
            : slots(slots), pos(pos), size(size) {
            advance_to_valid();
        }

        reference operator*() { return slots[pos].value; }
        pointer operator->() { return &slots[pos].value; }

        iterator& operator++() {
            ++pos;
            advance_to_valid();
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return pos == other.pos;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };

    class const_iterator {
        const Slot* slots;
        size_t pos;
        size_t size;

        void advance_to_valid() {
            while (pos < size && (!slots[pos].occupied || slots[pos].deleted)) {
                ++pos;
            }
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator(const Slot* slots, size_t pos, size_t size)
            : slots(slots), pos(pos), size(size) {
            advance_to_valid();
        }

        // Allow conversion from iterator to const_iterator
        const_iterator(const iterator& it)
            : slots(it.slots), pos(it.pos), size(it.size) {}

        reference operator*() const { return slots[pos].value; }
        pointer operator->() const { return &slots[pos].value; }

        const_iterator& operator++() {
            ++pos;
            advance_to_valid();
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const {
            return pos == other.pos;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }
    };

    iterator begin() { return iterator(table.data(), 0, N); }
    iterator end() { return iterator(table.data(), N, N); }

    const_iterator begin() const { return const_iterator(table.data(), 0, N); }
    const_iterator end() const { return const_iterator(table.data(), N, N); }

    const_iterator cbegin() const { return const_iterator(table.data(), 0, N); }
    const_iterator cend() const { return const_iterator(table.data(), N, N); }

    bool insert(const T& key) {
        size_t pos = hash(key);
        while (table[pos].occupied && !table[pos].deleted) {
            if (table[pos].value == key) return false;
            pos = (pos + 1) % N;
        }
        table[pos] = {key, true, false};
        active_count++;
        return true;
    }

    bool erase(const T& key) {
        size_t pos = hash(key);
        while (table[pos].occupied) {
            if (table[pos].value == key && !table[pos].deleted) {
                table[pos].deleted = true;
                active_count--;
                return true;
            }
            if (table[pos].value == key && table[pos].deleted) {
                return false; // Already deleted
            }
            pos = (pos + 1) % N;
        }
        return false;
    }

    // For std::erase_if compatibility
    template<typename Predicate>
    size_t erase_if(Predicate pred) {
        size_t count = 0;
        for (size_t i = 0; i < N; ++i) {
            if (table[i].occupied && !table[i].deleted && pred(table[i].value)) {
                table[i].deleted = true;
                active_count--;
                ++count;
            }
        }
        return count;
    }

    bool contains(const T& key) const {
        size_t pos = hash(key);
        while (table[pos].occupied) {
            if (table[pos].value == key && !table[pos].deleted) {
                return true;
            }
            pos = (pos + 1) % N;
        }
        return false;
    }

    void clear() {
        for (size_t i = 0; i < N; ++i) {
            table[i].occupied = false;
            table[i].deleted = false;
        }
        active_count = 0;
    }

    bool empty() const {
        return active_count == 0;
    }

    size_t size() const {
        return active_count;
    }

private:
    size_t active_count = 0;
};