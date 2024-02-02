#include <stdarg.h>
#include <stdio.h>

internal s32
StringLength(char *Str)
{
    s32 Counter = 0;
    char *Ptr = Str;
    while(*Ptr++)
    {
        ++Counter;
    }
    
    return(Counter);
}

internal string8
CreateString(char *String)
{
    string8 Result;
    Result.String = String;
    Result.Length = StringLength(String);
    return(Result);
}

internal void
StringSubstring(string8 Destination, string8 Source, s32 Position, s32 Length)
{
    s32 Counter = 0;
    
    while(Counter < Length)
    {
        Destination.String[Counter] = Source.String[Position + Counter - 1];
        ++Counter;
    }
    Destination.String[Counter] = 0;
}

internal void
StringConcat(string8 *Destination, string8 Source)
{
    char *Ptr = Destination->String + Destination->Length;
    for(s32 Index = 0;
        Index < Source.Length;
        ++Index)
    {
        *Ptr++ = *Source.String++;
    }
    *Ptr = '\0';
    
    Destination->Length += Source.Length;
}

internal b32
CompareStringBinary(string8 A, string8 B)
{
    if(A.Length != B.Length)
    {
        return(false);
    }
    
    for(s32 Index = 0;
        Index < A.Length;
        ++Index)
    {
        if(A.String[Index] != B.String[Index])
        {
            return(false);
        }
    }
    
    return(true);
}

internal s32
VariableToString(char *Dest, char *Format, ...)
{
    if(Dest)
    {
        va_list ArgList;
        va_start(ArgList, Format);
        
        char Buffer[320000];
        s32 Written = vsnprintf(Buffer, ArrayCount(Buffer), Format, ArgList);
        Buffer[Written] = 0;
        Platform->CopyMemory(Dest, Buffer, (Written + 1) * sizeof(char));
        
        va_end(ArgList);
        
        return(Written);
    }
    
    return(-1);
}

internal void
GetPathNoFileName(string8 *Destination, string8 Path)
{
    for(s32 CharacterIndex = Path.Length - 1;
        CharacterIndex > 0;
        --CharacterIndex)
    {
        --Path.Length;
        if(Path.String[CharacterIndex] == '/')
        {
            break;
        }
    }
    
    Platform->CopyMemory(Destination->String, Path.String, Path.Length);
    Destination->Length = Path.Length;
    Destination->String[Path.Length] = 0;
}