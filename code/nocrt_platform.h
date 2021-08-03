#ifndef NOCRT_PLATFORM_H

#include <stdint.h>
#include <stddef.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef s32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t memory_index;

typedef float r32;
typedef double r64;

#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359f

#if NOCRT_SLOW
// TODO(kstandbridge): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define InvalidCodePath Assert(!"InvalidCodePath");

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

struct app_memory
{
    b32 IsInitialized;
    
    u64 PermanentStorageSize;
    void *PermanentStorage; // NOTE(kstandbridge): REQUIRED to be cleared to zero at startup
    
    u64 TransientStorageSize;
    void *TransientStorage; // NOTE(kstandbridge): REQUIRED to be cleared to zero at startup
};

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
enum control_layout
{
    ControlLayout_Horizontal,
    ControlLayout_Verticle,
};
typedef void add_button(s64 ParentId, s64 ControlId, char *Text, r32 Size);
typedef void add_edit(s64 ParentId, s64 ControlId, char *Text, r32 Size);
typedef void add_group_box(s64 ParentId, s64 ControlId, char *Text, control_layout Layout);
typedef void add_panel(s64 ParentId, s64 ControlId, control_layout Layout);
typedef void add_spacer(s64 ParentId, r32 Size);
typedef void add_static(s64 ParentId, s64 ControlId, char *Text, r32 Size);



// NOTE(kstandbridge): Platform API
typedef void display_message(char *Title, char *Message);
typedef void get_control_text(s64 ControlId, char *Buffer, s32 BufferSize);
typedef void set_control_text(s64 ControlId, char *Buffer);

struct platform_api
{
    add_button *AddButton;
    add_edit *AddEdit;
    add_group_box *AddGroupBox;
    add_panel *AddPanel;
    add_spacer *AddSpacer;
    add_static *AddStatic;
    
    display_message *DisplayMessage;
    get_control_text *GetControlText;
    set_control_text *SetControlText;
};

// NOTE(kstandbridge): App API
typedef void create_controls(platform_api *PlatformAPI);
typedef void handle_command(s64 Id);

#define ID_WINDOW 0
#define SIZE_FILL 10000.0f

#define NOCRT_PLATFORM_H
#endif //NOCRT_PLATFORM_H6