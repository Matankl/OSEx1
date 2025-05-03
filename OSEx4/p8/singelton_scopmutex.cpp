#include <mutex>

// Singleton class with thread safety
class Singleton {
protected:
    Singleton() = default;    // Protected constructor
    ~Singleton() = default;   // Protected destructor

public:
    Singleton(const Singleton &) = delete;            // Delete copy constructor
    Singleton &operator=(const Singleton &) = delete; // Delete copy assignment operator

    // Get the single instance of the class
    static Singleton &getInstance() {
        static Singleton instance; // Guaranteed to be thread-safe in C++11 and later
        return instance;
    }
};

// Scope Guard to manage mutex lock/unlock
class ScopeGuard {
private:
    std::mutex &m; // Reference to the mutex

public:
    explicit ScopeGuard(std::mutex &mutex) : m(mutex) {
        m.lock(); // Lock the mutex
    }
    ~ScopeGuard() {
        m.unlock(); // Unlock the mutex
    }
};
