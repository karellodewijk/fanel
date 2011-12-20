#ifndef COPYABLE_UNIQUE_PTR_H
#define COPYABLE_UNIQUE_PTR_H

#include <memory>

/** Usage of this class is only safe if you can guarantee that only the most recent copy
 *  of the smart pointer will be used. It uses move on copy semantics like the deprecated
 *  auto_ptr.
 */

template <class T, class Del = std::default_delete<T> >
class copyable_unique_ptr : public std::unique_ptr<T, Del> {
  public:
    //A fully C++0x compliant c++ compiler should accept the following line to inherit constructors from unique_ptr.
    //gcc as of 4.5.2 with -std=c++0x, however does not. So we manually write wrappers for the constructors we need
    //using std::unique_ptr<T, Del>::std::unique_ptr<T, Del>;
  
    copyable_unique_ptr(T* pointer, Del deleter = Del()) : std::unique_ptr<T, Del>(pointer, deleter) {}
    copyable_unique_ptr(std::unique_ptr<T,Del>& pointer) : std::unique_ptr<T, Del>(std::move(pointer)) {}
    
    //copy constructors which moves the pointer from the old unique_ptr to the new one when moved.
    copyable_unique_ptr(const copyable_unique_ptr<T,Del>& other) : std::unique_ptr<T, Del>(std::move(const_cast<copyable_unique_ptr<T,Del>& >(other))) {}
};

#endif // COPYABLE_UNIQUE_PTR_H
