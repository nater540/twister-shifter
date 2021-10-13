#pragma once

template<typename T>
class Singleton {
public:
  static T& instance();

  Singleton(const Singleton&) = delete;
  Singleton& operator= (const Singleton) = delete;

protected:
  Singleton() {}
};

template<typename T>
T& Singleton<T>::instance() {
  static const std::unique_ptr<T> instance{new T};
  return *instance;
}
