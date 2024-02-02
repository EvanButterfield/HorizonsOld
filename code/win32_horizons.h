#ifndef WIN32_HORIZONS_H
#define WIN32_HORIZONS_H

typedef struct win32_state
{
    game_input GameInput;
    s64 PerfCountFrequency;
    b32 ShowCursor;
    
    win32_d3d11_state D3D11State;
    HWND Window;
    
    window_flags WindowFlags;
    window_dimension WindowDimension;
    
    IXAudio2 *XAudio;
    
    memory_arena Arena;
} win32_state;

typedef struct win32_game_code
{
    b32 IsValid;
    HMODULE GameDll;
    FILETIME LastWriteTime;
    game_update_and_render *GameUpdateAndRender;
} win32_game_code;

#endif //WIN32_HORIZONS_H
