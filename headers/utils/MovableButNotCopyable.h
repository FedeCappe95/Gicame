#ifndef __GICAME__MOVABLE_BUT_NOT_COPYABLE_H__
#define __GICAME__MOVABLE_BUT_NOT_COPYABLE_H__


#include "../common.h"


// Classes and structs
namespace Gicame {

    class GICAME_API MovableButNonCopyable {

    public:
        MovableButNonCopyable() = default;
        MovableButNonCopyable(const MovableButNonCopyable&&) noexcept {};
        MovableButNonCopyable& operator=(const MovableButNonCopyable&&) noexcept { return *this; };

    private:
        MovableButNonCopyable(const MovableButNonCopyable&) = delete;
        MovableButNonCopyable& operator=(const MovableButNonCopyable&) = delete;

    };

};


#define MOVABLE_BUT_NOT_COPYABLE Gicame::MovableButNonCopyable movableButNonCopyable


#endif