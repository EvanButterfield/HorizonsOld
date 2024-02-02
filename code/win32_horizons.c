// Thanks tsherif for XAudio2 code!!!
// https://github.com/tsherif/xaudio2-c-demo/blob/master/xaudio2-c-demo.c

int _fltused;

#include "horizons_types.h"
#include "horizons_platform.h"

#define COBJMACROS
#include <windows.h>
#include <windowsx.h>
#include <xinput.h>
#include <Objbase.h>
#include <xaudio2.h>

#undef PlaySound
#undef CopyMemory
#undef ZeroMemory

internal inline window_dimension
Win32GetWindowDimension(HWND Window)
{
    window_dimension Dimension = {0};
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Dimension.Width = ClientRect.right - ClientRect.left;
    Dimension.Height = ClientRect.bottom - ClientRect.top;
    return(Dimension);
}

#define AssertHR(HR) Assert(SUCCEEDED(HR))
#include "win32_d3d11.c"

#include "win32_horizons.h"

global win32_state *GlobalState;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal PLATFORM_OUTPUT_STRING(Win32OutputString)
{
    OutputDebugStringA(Output);
}

internal void
Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryA("Xinput9_1_0.dll");
    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("Xinput1_4.dll");
        if(!XInputLibrary)
        {
            XInputLibrary = LoadLibraryA("Xinput1_3.dll");
        }
    }
    
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

internal void
Win32InitCOM(void)
{
    AssertHR(CoInitializeEx(0, COINIT_MULTITHREADED));
}

internal void
Win32InitXAudio2(void)
{
    IXAudio2 *XAudio;
    AssertHR(XAudio2Create(&XAudio, 0, XAUDIO2_DEFAULT_PROCESSOR));
    GlobalState->XAudio = XAudio;
    
    IXAudio2MasteringVoice *MasterVoice;
    AssertHR(IXAudio2_CreateMasteringVoice(XAudio, &MasterVoice,
                                           XAUDIO2_DEFAULT_CHANNELS,
                                           XAUDIO2_DEFAULT_SAMPLERATE,
                                           0, 0, 0, AudioCategory_GameEffects));
}

internal inline FILETIME
Win32GetLastWriteTime(string16 *FileName)
{
    FILETIME LastWriteTime = {0};
    
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesExW(FileName, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }
    
    return(LastWriteTime);
}

// NOTE(evan): THIS DOESNT WORK WITH D3D11 (it doesn't trigger a resize)
internal void
ToggleFullscreen(HWND Window)
{
    // NOTE(evan): This follows Raymond Chen's example for switching
    // to fullscreen and back. Check out the article for more information
    // https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
    DWORD WindowStyle = GetWindowLong(Window, GWL_STYLE);
    if(WindowStyle & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
        // if(GetWindowPlacement(Window, &GlobalWindowPosition) &&
        // GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, WindowStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER|SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE,
                      WindowStyle|WS_OVERLAPPEDWINDOW);
        // SetWindowPlacement(Window, &GlobalWindowPosition);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_FRAMECHANGED);
    }
}

internal win32_game_code
Win32LoadGameCode(string16 *SourceDllName, string16 *TempDllName, string16 *LockFileName)
{
    win32_game_code Result = {0};
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesExW(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        Result.LastWriteTime = Win32GetLastWriteTime(SourceDllName);
        
        CopyFileW(SourceDllName, TempDllName, false);
        
        Result.GameDll = LoadLibraryW(TempDllName);
        if(Result.GameDll)
        {
            Result.GameUpdateAndRender =
            (game_update_and_render *)GetProcAddress(Result.GameDll, "GameUpdateAndRender");
            
            Result.IsValid = Result.GameUpdateAndRender != 0;
        }
    }
    
    if(!Result.IsValid)
    {
        Result.GameUpdateAndRender = GameUpdateAndRenderStub;
    }
    
    return(Result);
}

internal void
Win32UnloadGameCode(win32_game_code *GameCode)
{
    if(GameCode->GameDll)
    {
        FreeLibrary(GameCode->GameDll);
        GameCode->GameDll = 0;
    }
    
    GameCode->IsValid = false;
    GameCode->GameUpdateAndRender = GameUpdateAndRenderStub;
}

internal PLATFORM_OPEN_FILE(Win32OpenFile)
{
    DWORD AccessFlags = 0;
    DWORD ShareMode = 0;
    if(Flags & FILE_OPEN_READ)
    {
        AccessFlags |= GENERIC_READ;
        ShareMode |= FILE_SHARE_READ;
    }
    if(Flags & FILE_OPEN_WRITE)
    {
        AccessFlags |= GENERIC_WRITE;
        ShareMode |= FILE_SHARE_WRITE;
    }
    
    HANDLE File = CreateFileW(FileName, AccessFlags, ShareMode, 0,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    return(File);
}

internal PLATFORM_OPEN_FILE_8(Win32OpenFile8)
{
    DWORD AccessFlags = 0;
    DWORD ShareMode = 0;
    if(Flags & FILE_OPEN_READ)
    {
        AccessFlags |= GENERIC_READ;
        ShareMode |= FILE_SHARE_READ;
    }
    if(Flags & FILE_OPEN_WRITE)
    {
        AccessFlags |= GENERIC_WRITE;
        ShareMode |= FILE_SHARE_WRITE;
    }
    
    HANDLE File = CreateFileA(FileName, AccessFlags, ShareMode, 0,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    return(File);
}

internal PLATFORM_GET_FILE_SIZE(Win32GetFileSize)
{
    u32 Result = 0;
    
    if(Handle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(Handle, &FileSize))
        {
            Result = SafeTruncateUInt64(FileSize.QuadPart);
        }
        else
        {
            // TODO(evan): Logging
        }
    }
    else
    {
        // TODO(evan): Logging
    }
    
    return(Result);
}

internal PLATFORM_READ_ENTIRE_FILE(Win32ReadEntireFile)
{
    if(Handle != INVALID_HANDLE_VALUE)
    {
        if(Dest)
        {
            DWORD BytesRead;
            if(ReadFile(Handle, Dest, FileSize, &BytesRead, 0) &&
               BytesRead == FileSize)
            {
            }
            else
            {
                // TODO(evan): Logging
            }
        }
        else
        {
            // TODO(evan): Logging
        }
    }
    else
    {
        // TODO(evan): Logging
    }
}

internal PLATFORM_WRITE_ENTIRE_FILE(Win32WriteEntireFile)
{
    b32 Result = false;
    
    HANDLE File = *((HANDLE *)Handle);
    if(File != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(File, Data, DataSize, &BytesWritten, 0) &&
           BytesWritten == DataSize)
        {
            Result = true;
        }
        else
        {
            // TODO(evan): Logging
        }
    }
    else
    {
        // TODO(evan): Logging
    }
    
    return(Result);
}

internal PLATFORM_CLOSE_FILE(Win32CloseFile)
{
    CloseHandle(Handle);
}

internal inline LARGE_INTEGER
Win32GetWallClock(void)
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

internal inline f32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    f32 Result = ((f32)(End.QuadPart - Start.QuadPart) /
                  (f32)GlobalState->PerfCountFrequency);
    return(Result);
}

internal PLATFORM_CREATE_MESH(Win32CreateMesh)
{
    win32_d3d11_mesh *Mesh = PushStruct(&GlobalState->Arena, win32_d3d11_mesh);
    Win32D3D11CreateMesh(&GlobalState->D3D11State, Mesh,
                         VertexData, VertexCount,
                         IndexData, IndexCount);
    
    return(Mesh);
}

internal PLATFORM_POP_MESH(Win32PopMesh)
{
    // NOTE(evan): Grab the latest mesh in the buffer
    win32_d3d11_mesh *Mesh = PopStruct(&GlobalState->Arena, win32_d3d11_mesh);
    ReleaseMesh(Mesh);
}

internal PLATFORM_CREATE_TEXTURE(Win32CreateTexture)
{
    win32_d3d11_texture *Texture = PushStruct(&GlobalState->Arena, win32_d3d11_texture);
    Win32D3D11CreateTexture(&GlobalState->D3D11State, Texture,
                            Data, Width, Height);
    
    return(Texture);
}

internal PLATFORM_CREATE_SHADER(Win32CreateShader)
{
    win32_d3d11_shader *Shader = PushStruct(&GlobalState->Arena, win32_d3d11_shader);
    Win32D3D11CreateShader(&GlobalState->D3D11State, Shader, ShaderPath, Is2D);
    
    return(Shader);
}

internal PLATFORM_RENDER_MESH(Win32RenderMesh)
{
    Win32D3D11RenderMesh(&GlobalState->D3D11State, GlobalState->WindowDimension, Mesh, Shader, Texture, Constants, Is2D);
}

internal loaded_sound
Win32LoadSound(string16 *FileName, memory_arena *Scratch)
{
    loaded_sound Result = {0};
    
    HANDLE File = CreateFileW(FileName, GENERIC_READ, FILE_SHARE_READ,
                              0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(File != INVALID_HANDLE_VALUE)
    {
        if(SetFilePointer(File, 0, 0, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
        {
            DWORD ChunkType;
            DWORD ChunkDataSize;
            DWORD FileFormat;
            DWORD BytesRead = 0;
            
            WAVEFORMATEXTENSIBLE Format = {0};
            
            ReadFile(File, &ChunkType, sizeof(DWORD), &BytesRead, 0);
            if(BytesRead != sizeof(DWORD))
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            if(ChunkType != 'FFIR')
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            ReadFile(File, &ChunkDataSize, sizeof(DWORD), &BytesRead, 0);
            if(BytesRead != sizeof(DWORD))
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            ReadFile(File, &FileFormat, sizeof(DWORD), &BytesRead, 0);
            if(BytesRead != sizeof(DWORD))
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            if(FileFormat != 'EVAW')
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            ReadFile(File, &ChunkType, sizeof(DWORD), &BytesRead, 0);
            if(BytesRead != sizeof(DWORD))
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            if(ChunkType != 'tmf')
            {
                // TODO(evan): Logging
                // CloseHandle(File);
                // return(Result);
            }
            
            ReadFile(File, &ChunkDataSize, sizeof(DWORD), &BytesRead, 0);
            if(BytesRead != sizeof(DWORD))
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            ReadFile(File, &Format, ChunkDataSize, &BytesRead, 0);
            if(BytesRead != ChunkDataSize)
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            ReadFile(File, &ChunkType, sizeof(DWORD), &BytesRead, 0);
            if(BytesRead != sizeof(DWORD))
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            if(ChunkType != 'atad')
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            ReadFile(File, &ChunkDataSize, sizeof(DWORD), &BytesRead, 0);
            if(BytesRead != sizeof(DWORD))
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            u8 *AudioData = PushSize(Scratch, ChunkDataSize);
            if(!AudioData)
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            ReadFile(File, AudioData, ChunkDataSize, &BytesRead, 0);
            if(BytesRead != ChunkDataSize)
            {
                // TODO(evan): Logging
                CloseHandle(File);
                return(Result);
            }
            
            Result.Data = AudioData;
            Result.Size = ChunkDataSize;
            CloseHandle(File);
            IXAudio2SourceVoice *SourceVoice;
            AssertHR(IXAudio2_CreateSourceVoice(GlobalState->XAudio, &SourceVoice,
                                                &Format.Format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, 0, 0));
            Result.Platform = PushSize(Scratch, sizeof(IXAudio2SourceVoice *));
            *Result.Platform = SourceVoice;
        }
        else
        {
            // TODO(evan): Logging
            CloseHandle(File);
        }
    }
    else
    {
        // TODO(evan): Logging
    }
    
    return(Result);
}

internal PLATFORM_CREATE_SOUND(Win32CreateSound)
{
    loaded_sound Result = Win32LoadSound(FileName, Scratch);
    
    IXAudio2SourceVoice *SourceVoice = *Result.Platform;
    IXAudio2Voice *Voice = *Result.Platform;
    AssertHR(IXAudio2Voice_SetVolume(Voice, Volume, XAUDIO2_COMMIT_NOW));
    AssertHR(IXAudio2SourceVoice_Start(SourceVoice, 0, XAUDIO2_COMMIT_NOW));
    
    return(Result);
}

internal PLATFORM_PLAY_SOUND(Win32PlaySound)
{
    XAUDIO2_BUFFER Buffer = {0};
    Buffer.pAudioData = Sound->Data;
    Buffer.AudioBytes = Sound->Size;
    
    IXAudio2SourceVoice *SourceVoice = *Sound->Platform;
    AssertHR(IXAudio2SourceVoice_Stop(SourceVoice, 0, XAUDIO2_COMMIT_NOW));
    AssertHR(IXAudio2SourceVoice_FlushSourceBuffers(SourceVoice));
    AssertHR(IXAudio2SourceVoice_SubmitSourceBuffer(SourceVoice, &Buffer, 0));
    AssertHR(IXAudio2SourceVoice_Start(SourceVoice, 0, XAUDIO2_COMMIT_NOW))
}

internal PLATFORM_SET_SOUND_VOLUME(Win32SetSoundVolume)
{
    IXAudio2Voice *Voice = *Sound->Platform;
    
    AssertHR(IXAudio2Voice_SetVolume(Voice, Volume, XAUDIO2_COMMIT_NOW));
}

internal PLATFORM_STOP_SOUND(Win32StopSound)
{
    IXAudio2SourceVoice *SourceVoice = *Sound->Platform;
    AssertHR(IXAudio2SourceVoice_Stop(SourceVoice, 0, XAUDIO2_COMMIT_NOW));
}

internal PLATFORM_SOUND_IS_PLAYING(Win32SoundIsPlaying)
{
    IXAudio2SourceVoice *SourceVoice = *Sound->Platform;
    XAUDIO2_VOICE_STATE State;
    IXAudio2SourceVoice_GetState(SourceVoice, &State, XAUDIO2_VOICE_NOSAMPLESPLAYED);
    
    return(State.BuffersQueued != 0);
}

internal PLATFORM_DELETE_SOUND(Win32DeleteSound)
{
    IXAudio2SourceVoice *SourceVoice = *Sound->Platform;
    if(SourceVoice)
    {
        IXAudio2Voice *Voice = (IXAudio2Voice *)SourceVoice;
        IXAudio2Voice_DestroyVoice(Voice);
    }
}

internal PLATFORM_COPY_MEMORY(Win32CopyMemory)
{
    RtlCopyMemory(Dest, Source, Length);
}

internal PLATFORM_ZERO_MEMORY(Win32ZeroMemory)
{
    RtlZeroMemory(Dest, Length);
}

internal LRESULT CALLBACK
Win32WindowProc(HWND Window,
                UINT Message,
                WPARAM WParam,
                LPARAM LParam);

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CmdLine,
        int CmdShow)
{
    // Assert((uint64)(*((int64*)__readgsqword(0x60) + 0x23)) >= 10);
    
    game_memory GameMemory = {0};
    {
#if HORIZONS_INTERNAL
        LPVOID BaseAddress = (LPVOID)Tebibytes(2);
#else
        LPVOID BaseAddress = 0;
#endif
        
        platform_api Platform;
        Platform.OpenFile = Win32OpenFile;
        Platform.OpenFile8 = Win32OpenFile8;
        Platform.GetFileSize = Win32GetFileSize;
        Platform.ReadEntireFile = Win32ReadEntireFile;
        Platform.WriteEntireFile = Win32WriteEntireFile;
        Platform.CloseFile = Win32CloseFile;
        Platform.CreateMesh = Win32CreateMesh;
        Platform.PopMesh = Win32PopMesh;
        Platform.CreateTexture = Win32CreateTexture;
        Platform.CreateShader = Win32CreateShader;
        Platform.RenderMesh = Win32RenderMesh;
        Platform.CreateSound = Win32CreateSound;
        Platform.PlaySound = Win32PlaySound;
        Platform.SetSoundVolume = Win32SetSoundVolume;
        Platform.StopSound = Win32StopSound;
        Platform.SoundIsPlaying = Win32SoundIsPlaying;
        Platform.DeleteSound = Win32DeleteSound;
        Platform.CopyMemory = Win32CopyMemory;
        Platform.ZeroMemory = Win32ZeroMemory;
        Platform.OutputString = Win32OutputString;
        
        GameMemory.PermanentStorageSize = Mebibytes(64);
        GameMemory.TransientStorageSize = Gibibytes(1);
        GameMemory.Platform = Platform;
        
        memory_index PlatformMemorySize = Mebibytes(64);
        
        memory_index TotalGameMemorySize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize + PlatformMemorySize;
        GlobalState = VirtualAlloc(BaseAddress, sizeof(win32_state) + TotalGameMemorySize,
                                   MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        
        InitializeArena(&GlobalState->Arena, (u8 *)GlobalState + sizeof(win32_state), PlatformMemorySize);
        GameMemory.PermanentStorage = GlobalState->Arena.Memory + PlatformMemorySize;
        GameMemory.TransientStorage = (u8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;
    }
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalState->PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    // NOTE(evan): Set the Windows schedular granularity to 1ms
    // so that Sleep() can be more granular
    UINT DesiredSchedularMS = 1;
    b32 SleepIsGranular = (timeBeginPeriod(DesiredSchedularMS) == TIMERR_NOERROR);
    
    Win32InitCOM();
    Win32LoadXInput();
    Win32InitXAudio2();
    
    WNDCLASSW WindowClass = {0};
    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = Win32WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursorW(0, IDC_ARROW);
    WindowClass.lpszClassName = L"ChariotWindowClass";
    
    string16 *GameCodeDllName = L"horizons.dll";
    string16 *GameCodeTempDllName = L"horizons_temp.dll";
    string16 *GameCodeLockName = L"lock.tmp";
    
    if(RegisterClassW(&WindowClass))
    {
        GlobalState->Window = CreateWindowW(WindowClass.lpszClassName, L"New Horizons",
                                            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                            CW_USEDEFAULT, CW_USEDEFAULT, 1400, 700,
                                            0, 0, Instance, 0);
        
        GlobalState->ShowCursor = true;
        
        if(GlobalState->Window)
        {
            HDC DeviceContext = GetDC(GlobalState->Window);
            
            s32 MonitorRefreshHz = 60;
            s32 Win32RefreshRate = GetDeviceCaps(DeviceContext, VREFRESH);
            if(Win32RefreshRate > 1)
            {
                MonitorRefreshHz = Win32RefreshRate;
            }
            f32 GameUpdateHz = ((f32)MonitorRefreshHz);
            f32 TargetSecondsPerFrame = 1.0f / (f32)GameUpdateHz;
            
            win32_game_code Game = Win32LoadGameCode(GameCodeDllName, GameCodeTempDllName, GameCodeLockName);
            
            if(GameMemory.PermanentStorage)
            {
                GlobalState->WindowDimension = Win32GetWindowDimension(GlobalState->Window);
                Win32InitD3D11(&GlobalState->D3D11State, GlobalState->Window);
                
                LARGE_INTEGER LastCounter = Win32GetWallClock();
                b32 ShouldClose = false;
                while(!ShouldClose)
                {
                    f32 DeltaTime;
                    {
                        LARGE_INTEGER EndCounter = Win32GetWallClock();
                        DeltaTime = Win32GetSecondsElapsed(LastCounter, EndCounter);
                        LastCounter = EndCounter;
                    }
                    
                    {
                        FILETIME NewDllWriteTime = Win32GetLastWriteTime(GameCodeDllName);
                        if(CompareFileTime(&Game.LastWriteTime, &NewDllWriteTime))
                        {
                            Win32UnloadGameCode(&Game);
                            Game = Win32LoadGameCode(GameCodeDllName, GameCodeTempDllName, GameCodeLockName);
                        }
                    }
                    
                    GlobalState->GameInput.Keyboard.Backspace = false;
                    GlobalState->GameInput.Keyboard.HasChar = false;
                    {
                        MSG Message;
                        while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
                        {
                            TranslateMessage(&Message);
                            DispatchMessageA(&Message);
                        }
                    }
                    
                    {
                        game_controller_input *Input = &GlobalState->GameInput.Controller;
                        
                        XINPUT_STATE ControllerState = {0};
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
                        if(XInputGetState(0, &ControllerState) == ERROR_SUCCESS)
                        {
                            Input->Connected = true;
                            
                            Input->Up = Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
                            Input->Down = Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                            Input->Left= Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                            Input->Right= Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
                            
                            Input->Start = Pad->wButtons & XINPUT_GAMEPAD_START;
                            
                            Input->LeftShoulder= Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
                            Input->RightShoulder = Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
                            
                            Input->A = Pad->wButtons & XINPUT_GAMEPAD_A;
                            Input->B = Pad->wButtons & XINPUT_GAMEPAD_B;
                            Input->X = Pad->wButtons & XINPUT_GAMEPAD_X;
                            Input->Y = Pad->wButtons & XINPUT_GAMEPAD_Y;
                            
                            Input->LeftTrigger = Pad->bLeftTrigger;
                            Input->RightTrigger = Pad->bLeftTrigger;
                            
                            Input->LeftThumbY = (Pad->sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ? Pad->sThumbLX : 0;
                            Input->LeftThumbX = (Pad->sThumbLY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) ? Pad->sThumbLY : 0;
                            
                            XINPUT_VIBRATION Vibration;
                            Vibration.wLeftMotorSpeed = Input->LeftVibration;
                            Vibration.wRightMotorSpeed = Input->RightVibration;
                            XInputSetState(0, &Vibration);
                        }
                        else
                        {
                            Input->Connected = false;
                            // NOTE(evan): Controller not connected, this is NOT an error
                        }
                    }
                    
                    {
                        window_dimension NewWindowDimension = Win32GetWindowDimension(GlobalState->Window);
                        b32 WindowResized = false;
                        if(NewWindowDimension.Width != GlobalState->WindowDimension.Width ||
                           NewWindowDimension.Height != GlobalState->WindowDimension.Height)
                        {
                            WindowResized = true;
                            GlobalState->WindowDimension = NewWindowDimension;
                        }
                        Win32D3D11BeginFrame(&GlobalState->D3D11State, GlobalState->WindowDimension, WindowResized);
                    }
                    
                    if(GlobalState->ShowCursor)
                    {
                        RECT WindowRect;
                        GetClientRect(GlobalState->Window, &WindowRect);
                        
                        POINT Center = { WindowRect.right/2, WindowRect.bottom/2 };
                        ClientToScreen(GlobalState->Window, &Center);
                        // SetCursorPos(Center.x, Center.y);
                    }
                    
                    ShouldClose = Game.GameUpdateAndRender(GlobalState->WindowFlags,
                                                           GlobalState->WindowDimension,
                                                           &GameMemory, &GlobalState->GameInput, DeltaTime);
                    
                    Win32D3D11EndFrame(GlobalState->D3D11State.SwapChain, true);
                    
                    GlobalState->WindowFlags = WINDOW_FLAGS_NONE;
                }
            }
        }
        else
        {
            // TODO(evan): Logging
            return(1);
        }
    }
    else
    {
        // TODO(evan): Logging
        return(1);
    }
    
    return(0);
}

void __stdcall
WinMainCRTStartup(void)
{
    int Result = WinMain(GetModuleHandle(0), 0, 0, 0);
    ExitProcess(Result);
}

internal LRESULT CALLBACK
Win32WindowProc(HWND Window,
                UINT Message,
                WPARAM WParam,
                LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_SETCURSOR:
        {
            if(GlobalState->ShowCursor)
            {
                Result = DefWindowProc(Window, Message, WParam, LParam);
            }
            else
            {
                ShowCursor(0);
            }
        } break;
        
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            u32 VKCode = (u32)WParam;
            b32 WasDown = LParam & (1 << 30);
            b32 IsDown = (LParam & (1 << 31)) == 0;
            
            if(WasDown != IsDown)
            {
                switch(VKCode)
                {
                    case 'W':
                    {
                        GlobalState->GameInput.Keyboard.W = IsDown;
                    } break;
                    
                    case 'A':
                    {
                        GlobalState->GameInput.Keyboard.A = IsDown;
                    } break;
                    
                    case 'S':
                    {
                        GlobalState->GameInput.Keyboard.S = IsDown;
                    } break;
                    
                    case 'D':
                    {
                        GlobalState->GameInput.Keyboard.D = IsDown;
                    } break;
                    
                    case 'Q':
                    {
                        GlobalState->GameInput.Keyboard.Q = IsDown;
                    } break;
                    
                    case 'E':
                    {
                        GlobalState->GameInput.Keyboard.E = IsDown;
                    } break;
                    
                    case 'M':
                    {
                        GlobalState->GameInput.Keyboard.M = IsDown;
                    } break;
                    
                    case VK_UP:
                    {
                        GlobalState->GameInput.Keyboard.Up = IsDown;
                    } break;
                    
                    case VK_LEFT:
                    {
                        GlobalState->GameInput.Keyboard.Left = IsDown;
                    } break;
                    
                    case VK_DOWN:
                    {
                        GlobalState->GameInput.Keyboard.Down = IsDown;
                    } break;
                    
                    case VK_RIGHT:
                    {
                        GlobalState->GameInput.Keyboard.Right = IsDown;
                    } break;
                    
                    case VK_BACK:
                    {
                        if(IsDown)
                        {
                            GlobalState->GameInput.Keyboard.Backspace = true;
                        }
                    } break;
                    
                    case VK_ESCAPE:
                    {
                        GlobalState->GameInput.Keyboard.Escape = IsDown;
                    } break;
                    
                    case VK_DELETE:
                    {
                        GlobalState->GameInput.Keyboard.Delete = IsDown;
                    } break;
                    
                    case VK_SPACE:
                    {
                        GlobalState->GameInput.Keyboard.Space = IsDown;
                    } break;
                    
                    case VK_RETURN:
                    {
                        GlobalState->GameInput.Keyboard.Enter = IsDown;
                    } break;
                    
                    case 0x31:
                    {
                        GlobalState->GameInput.Keyboard.One = IsDown;
                    } break;
                    
                    case 0x32:
                    {
                        GlobalState->GameInput.Keyboard.Two = IsDown;
                    } break;
                    
                    case 0x33:
                    {
                        GlobalState->GameInput.Keyboard.Three = IsDown;
                    } break;
                }
            }
            
            if(IsDown)
            {
                b32 AltKeyIsDown = (LParam & (1 << 29));
                GlobalState->GameInput.Keyboard.AltF4 = false;
                if(AltKeyIsDown && VKCode == VK_F4)
                {
                    GlobalState->GameInput.Keyboard.AltF4 = true;
                }
            }
        } break;
        
        case WM_CHAR:
        {
            wchar_t Char16 = (wchar_t)WParam;
            
            if(' ' <= Char16 && Char16 <= '~')
            {
                wchar_t String16[2] = { Char16, L'\0' };
                
                char String[2] = {0};
                WideCharToMultiByte(CP_UTF8, 0, String16, -1, String, ArrayCount(String), 0, 0);
                GlobalState->GameInput.Keyboard.InputChar = String[0];
                GlobalState->GameInput.Keyboard.HasChar = true;
            }
        } break;
        
        case WM_LBUTTONDOWN:
        {
            GlobalState->GameInput.Mouse.LButton = true;
        } break;
        
        case WM_LBUTTONUP:
        {
            GlobalState->GameInput.Mouse.LButton = false;
        } break;
        
        case WM_RBUTTONDOWN:
        {
            GlobalState->GameInput.Mouse.RButton = true;
        } break;
        
        case WM_RBUTTONUP:
        {
            GlobalState->GameInput.Mouse.RButton = false;
        } break;
        
        case WM_MOUSEMOVE:
        {
            GlobalState->GameInput.Mouse.X = GET_X_LPARAM(LParam);
            GlobalState->GameInput.Mouse.Y = GET_Y_LPARAM(LParam);
        } break;
        
        case WM_CLOSE:
        {
            GlobalState->WindowFlags |= WINDOW_FLAGS_CLOSED;
        } break;
        
        case WM_DESTROY:
        {
            GlobalState->WindowFlags |= WINDOW_FLAGS_CLOSED;
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}
