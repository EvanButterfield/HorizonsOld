#ifndef HORIZONS_TYPES_H
#define HORIZONS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif
    
    //
    // NOTE(evan): Compilers
    //
#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif
    
#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif
    
#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
    // TODO(evan): More compilers!
#undef COMPILER_LLVM
#define COMPILER_LLVM
#endif
#endif
    
#define internal static
#define global static
#define local_persist static
    
#define Kibibytes(Value) (Value*1024)
#define Mebibytes(Value) (Kibibytes(Value)*1024LL)
#define Gibibytes(Value) (Mebibytes(Value)*1024LL)
#define Tebibytes(Value) (Gibibytes(Value)*1024LL)
    
#define Pi32 3.14159265359f
#define DEG_TO_RAD (Pi32 / 180)
#define EPSILON 1.19209290E-07F
    
#if HORIZONS_SLOW
#  define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#  define Assert(Expression)
#endif
    
#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))
    
#define MAX_ENTITY_COUNT 128
#define MAX_POINT_LIGHT_COUNT 8
#define MAX_UI_COUNT 32
    
#include "stdint.h"
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;
    
    typedef int8_t s8;
    typedef int16_t s16;
    typedef int32_t s32;
    typedef int64_t  s64;
    
    typedef s32 b32;
#undef true
#undef false
#define true 1
#define false 0
    
    typedef size_t memory_index;
    
    // TODO(evan): Make these structs containing the pointer and the size
    
    typedef struct string8
    {
        char *String;
        s32 Length;
    } string8;
    
    typedef wchar_t string16;
    
    typedef float f32;
    typedef double f64;
    
    typedef union v2
    {
        struct
        {
            f32 x, y;
        };
        struct
        {
            f32 x_, z;
        };
        struct
        {
            f32 Width, Height;
        };
        struct
        {
            f32 Forward, Backward;
        };
        
        f32 e[2];
    } v2;
    
    typedef union v3
    {
        struct
        {
            f32 x, y, z;
        };
        struct
        {
            f32 r, g, b;
        };
        struct
        {
            f32 Pitch, Yaw, Roll;
        };
        
        f32 e[3];
    } v3;
    
    typedef union uv3
    {
        struct
        {
            u32 x, y, z;
        };
        
        u32 e[3];
    } uv3;
    
    typedef union v4
    {
        struct
        {
            f32 x, y, z, w;
        };
        struct
        {
            f32 r, g, b, a;
        };
        
        f32 e[4];
    } v4;
    
    typedef union mat3
    {
        v3 e[3];
        f32 m[3][3];
    } mat3;
    
    typedef union mat4
    {
        v4 e[4];
        f32 m[4][4];
    } mat4;
    
    internal inline u32
        SafeTruncateUInt64(u64 Value)
    {
        Assert(Value <= 0xFFFFFFFF);
        u32 Result = (u32)Value;
        return(Result);
    }
    
#ifdef __cplusplus
};
#endif

#endif //HORIZONS_TYPES_H
