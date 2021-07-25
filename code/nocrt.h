#ifndef NOCRT_H
#define NOCRT_H

#include "nocrt_platform.h"


struct memory_arena
{
    memory_index Size;
    u8 *Base;
    memory_index Used;
};

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (u8 *)Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
inline void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return(Result);
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
inline void
ZeroSize(memory_index Size, void *Ptr)
{
    // TODO(kstandbridge): Check this guy for performance
    u8 *Byte = (u8 *)Ptr;
    while(Size--)
    {
        *Byte++ = 0;
    }
}

// NOTE(kstandbridge): Headers here

// NOTE(kstandbridge): dll only types

#endif //NOCRT_H
