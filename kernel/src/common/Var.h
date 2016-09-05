#ifndef VAR_H
#define VAR_H

template <typename T>
class VarT {
public:
  void addref() {
    refcount++;
  }
  void decref() {
    refcount--;
    if (!refcount)
      delete this;
  }
  virtual T get() = 0;
protected:
  virtual ~VarT() {}
private:
  size_t refcount;
};

template <typename T>
class VarNum : public VarT<T> {
public:
  VarNum(T value)
  : value(value)
  {
  }
  T get() override {
    return value;
  }
private:
  T value;
};

// Handle class
template <typename T>
class Var {
public:
  Var(T value = T())
  : inst(new VarNum<T>(value))
  {
    inst->addref();
  }
  Var(const Var& v) 
  : inst(v.inst)
  {
    inst->addref();
  }
  Var& operator=(const Var<T>& vr) {
    vr.inst->addref();
    inst->decref();
    inst = vr.inst;
    return *this;
  }
  ~Var() {
    inst->decref();
  }
  T operator*() {
    return inst->get();
  }
private:
  VarT<T> *inst;
};

template <typename T>
class VarRef : public VarT<T> {
public:
  VarRef(Var<T> &v)
  : v(v)
  {
  }
  T get() override {
    return *v;
  }
private:
  Var<T> &v;
};

template <typename T>
class VarFunc : public VarT<T> {
public:
  class VarFuncImpl {
  public:
    virtual ~VarFuncImpl() {}
    virtual T get() = 0;
  };
  template <typename U>
  class VarFuncImpl0 : public VarFuncImpl {
  public:
    U u;
    T get() override { return u(); }
  };
  template <typename U>
  VarFunc(U u) {
    vfi = new VarFuncImpl0<U>(u);
  }
  T get() override {
    return vfi->get();
  }
private:
  VarFuncImpl* vfi;
};

#endif


