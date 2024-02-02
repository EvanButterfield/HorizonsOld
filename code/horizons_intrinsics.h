#ifndef HORIZONS_INTRINSICS_H
#define HORIZONS_INTRINSICS_H

#include <math.h>

internal inline s32
RoundValue32ToInt32(f32 Value)
{
    s32 Result = (s32)roundf(Value);
    return(Result);
}

internal inline u8
RoundValue32ToUInt8(f32 Value)
{
    u8 Result = (u8)roundf(Value);
    return(Result);
}

internal inline f32
Sin(f32 Value)
{
    return(sinf(Value));
}

internal inline f32
Cos(f32 Value)
{
    return(cosf(Value));
}

internal inline f32
Tan(f32 Value)
{
    return(tanf(Value));
}

internal inline f32
Sqrt(f32 Value)
{
    return(sqrtf(Value));
}

internal inline f32
Abs(f32 Value)
{
    return((f32)(fabs(Value)));
}

typedef struct bit_scan_result
{
    b32 Found;
    u32 Index;
} bit_scan_result;

internal inline bit_scan_result
FindLeastSignificantSetBit(u32 Value)
{
    bit_scan_result Result = {0};
    
#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(u32 Test = 0;
        Test < 32;
        ++Test)
    {
        if(Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }
#endif
    
    return(Result);
}

#endif //HORIZONS_INTRINSICS_H
