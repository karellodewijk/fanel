#ifndef COPYABLE_UNIQUE_PTR_H
#define COPYABLE_UNIQUE_PTR_H

#include <memory>

/** Usage of this class is only safe if you can guarantee that only the most recent copy
 *  of this pointer will be used.
 */

template <class T, class Del = std::default_delete<T> >
class copyable_unique_ptr {
  public:
    copyable_unique_ptr(T* pointer, Del deleter = Del()) : unique_pointer(pointer, deleter) {}
    copyable_unique_ptr(std::unique_ptr<T,Del> pointer, Del deleter = Del()) : unique_pointer(std::move(pointer), deleter) {}
    copyable_unique_ptr(const copyable_unique_ptr& wrapper) : unique_pointer(std::move(wrapper.unique_pointer)) {}
    
    inline T operator*() const {return unique_pointer.operator*();}
    inline T* operator->() const {return unique_pointer.operator->();}
    inline T* release() {unique_pointer.release();}
    inline void reset(T* pointer) {unique_pointer.reset(pointer);}
    inline T* get() {unique_pointer.get();}
    
    mutable std::unique_ptr<T, Del> unique_pointer;
};

#endif // COPYABLE_UNIQUE_PTR_H
