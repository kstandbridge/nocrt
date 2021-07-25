#ifndef NOCRT_PLATFORM_H
#define NOCRT_PLATFORM_H

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

enum control_type
{
    ControlType_Button
};
typedef void create_control(control_type ControlType, char *Text, s64 Id);
typedef void display_message(char *Title, char *Message);
struct platform_api
{
    create_control *CreateControl;
    display_message *DisplayMessage;
};

typedef void create_controls(platform_api *PlatformAPI);
typedef void handle_command(s64 Id);
#endif //NOCRT_PLATFORM_H
