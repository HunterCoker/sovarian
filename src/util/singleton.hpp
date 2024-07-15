#pragma once

#include <cstdint>

typedef uint32_t    flags_t;

namespace util {    

class singleton {
protected:
    singleton()
        : _M_flags(0) {}
public: 
    singleton(const singleton&) = delete;
    void operator=(const singleton&) = delete;
    ~singleton() = default;

    virtual bool initialize() = 0;

    // flags allow global objects to have mutable settings
    void apply_flags(flags_t flags) { _M_flags |= flags; }
    void remove_flags(flags_t flags) { _M_flags ^= _M_flags & flags; }
    const flags_t& get_flags() const { return _M_flags; }
protected:
    flags_t _M_flags;
};

}  // util