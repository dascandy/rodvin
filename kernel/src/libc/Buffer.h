#ifndef BUFFER_H
#define BUFFER_H

#include <queue>
#include <functional>

template <typename T>
class Buffer {
public:
  Buffer() {}
  T pop() {
    T rv = storage.front();
    storage.pop();
    return rv;
  }
  bool empty() const {
    return storage.empty();
  }
  void push(T&& value) {
    storage.push(std::forward(value));
    onPush();
  }
  void push(const T& value) {
    storage.push(value);
    onPush();
  }
  std::function<void()> onPush = []{};
private:
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;
  std::queue<T> storage;
};

#endif


