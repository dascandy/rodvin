#ifndef FUTURE_H
#define FUTURE_H

#include <functional>
#include <memory>
#include <vector>
#include <utility>
#include <stdio.h>

template <typename T>
class shared_state;
template <typename T>
class promise;
template <typename T>
class future;

class executor {
public:
  virtual void queue(const std::function<void()>& f) = 0;
  template <typename T>
  auto async(T func) -> future<decltype(func())>;
};

void run_queued_tasks();
executor& default_executor();

template <typename T>
auto async(T task, executor& ex = default_executor()) -> future<decltype(task())> {
  return ex.async(task);
}

template <typename T>
class future {
public:
  T get() const;
  bool is_ready() const;
  template <typename U>
  auto then(U func, executor &exec = default_executor()) -> future<decltype(func(*(future<T>*)0))>;
//private:
  future(const std::shared_ptr<shared_state<T>> &state);
  future(const future& f)
  : state(f.state)
  {
    printf("%s:%d\n", __FILE__, __LINE__);
  }
  ~future() {
    printf("%s:%d\n", __FILE__, __LINE__);
  }
  friend class promise<T>;
  std::shared_ptr<shared_state<T>> state;
};

template <typename T>
class shared_state {
public:
  shared_state()
  : storage(NULL)
  {
  }
  ~shared_state()
  {
    delete storage;
  }
  void set(T &&value) {
    storage = new T(std::move(value));
    value_set = true;
    for (const auto &p : cbs)
      p.first->async(p.second);
    cbs.clear();
  }
  void set(const T &value) {
    storage = new T(value);
    value_set = true;
    for (const auto &p : cbs)
      p.first->async(p.second);
    cbs.clear();
  }
  T get() {
    return *storage;
  }
  void then(const std::function<void()> &cb, executor &exec) {
    if (value_set)
      exec.async(cb);
    else
      cbs.push_back(std::make_pair(&exec, cb));
  }
  std::vector<std::pair<executor*, std::function<void()>>> cbs;
  T *storage;
  bool value_set = false;
  static constexpr const char* name = "shared_state<T>";
};

template <>
struct shared_state<void> {
public:
  void set() {
    value_set = true;
    for (const auto &p : cbs)
      p.first->async(p.second);
    cbs.clear();
  }
  void get() const {
  }
  void then(const std::function<void()> &cb, executor &exec) {
    if (value_set)
      exec.async(cb);
    else
      cbs.push_back(std::make_pair(&exec, cb));
  }
  std::vector<std::pair<executor*, std::function<void()>>> cbs;
  bool value_set = false;
  static constexpr const char* name = "shared_state<void>";
};

template <typename T>
future<T> make_ready_future(T value) {
  std::shared_ptr<shared_state<T>> state = std::make_shared<shared_state<T>>();
  future<T> f(state);
  state->set(std::move(value));
  return f;
}

template <typename T>
class promise;

template <typename T>
T future<T>::get() const {
  return state->get();
}

template <typename T>
bool future<T>::is_ready() const {
  return state->value_set;
}

template <typename T>
future<T>::future(const std::shared_ptr<shared_state<T>> &state)
: state(state)
{
    printf("%s:%d\n", __FILE__, __LINE__);
}

template <typename T>
class promise {
public:
  future<T> get_future() { 
    return future<T>(state); 
  }
  void set_value(T &&value) {
    state->set(std::forward<T>(value));
  }
  void set_value(const T &value) {
    state->set(value);
  }
  std::shared_ptr<shared_state<T>> state = std::make_shared<shared_state<T>>();
  static constexpr const char* name = "promise<T>";
};

template <>
class promise<void> {
public:
  future<void> get_future() { 
    return future<void>(state); 
  }
  void set_value() {
    state->set();
  }
  std::shared_ptr<shared_state<void>> state = std::make_shared<shared_state<void>>();
  static constexpr const char* name = "promise<void>";
};

template <typename T>
struct all {
  bool operator()(const std::vector<future<T>> &futures) {
    for (const auto &f : futures) {
      if (!f.is_ready())
        return false;
    }
    return true;
  }
};

template <typename T>
struct any {
  bool operator()(const std::vector<future<T>> &futures) {
    for (const auto &f : futures) {
      if (f.is_ready())
        return true;
    }
    return false;
  }
};

template <template <typename> class P, typename T>
future<std::vector<future<T>>> when(const std::vector<future<T>> &futures) {
  struct when_holder {
    std::vector<future<T>> futures;
    P<T> pred;
    promise<std::vector<future<T>>> prom;
    bool value_set;
    when_holder(std::vector<future<T>> v) 
    : futures(v)
    {
      value_set = false;
    }
    void updatePromise() 
    {
      if (!value_set && pred(futures))
      {
        prom.set_value(std::move(futures));
        value_set = true;
      }
    }
  };
  std::shared_ptr<when_holder> inst = std::make_shared<when_holder>(futures);
  for (auto &f : inst->futures) {
    f.then([inst](const future<T> &) {
      inst->updatePromise();
    });
  }
  future<std::vector<future<T>>> rv = inst->prom.get_future();
  inst->updatePromise();
  return rv;
}

template <typename T, typename U>
struct then_impl {
  static future<U> impl(future<T> t, const std::function<U(const future<T>&)> &func, executor &exec)
  {
    std::shared_ptr<promise<U>> value = std::make_shared<promise<U>>();
    future<U> rv = value->get_future();
    t.state->then([t, value, func]{
      value->set_value(func(t));
    }, exec);
    return rv;
  }
};

template <typename T>
struct then_impl<T, void> {
  static future<void> impl(future<T> t, const std::function<void(const future<T>&)> &func, executor &exec)
  {
    std::shared_ptr<promise<void>> value = std::make_shared<promise<void>>();
    future<void> rv = value->get_future();
    t.state->then([t, value, func]{ 
      func(t);
      value->set_value(); 
    }, exec);
    return rv;
  }
};

template <typename T>
template <typename U>
auto future<T>::then(U func, executor &exec) -> future<decltype(func(*(future<T>*)0))> {
  std::function<decltype(func(*(future<T>*)0))(const future<T> &)> f = func;
  return then_impl<T, decltype(func(*(future<T>*)0))>::impl(*this, f, exec);
}

template <typename U>
struct async_impl {
  static future<U> impl(executor *exec, const std::function<U()> &func) {
    std::shared_ptr<promise<U>> value = std::make_shared<promise<U>>();
    future<U> rv = value->get_future();
    exec->queue([value, func]{
      value->set_value(func());
    });
    return rv;
  }
};

template <>
struct async_impl<void> {
  static future<void> impl(executor *exec, const std::function<void()> &func) {
    std::shared_ptr<promise<void>> value = std::make_shared<promise<void>>();
    future<void> rv = value->get_future();
    exec->queue([value, func]{
      func();
      value->set_value();
    });
    return rv;
  }
};

template <typename T>
auto executor::async(T func) -> future<decltype(func())> {
  std::function<decltype(func())()> f = func;
  return async_impl<decltype(func())>::impl(this, f);
}

#endif


