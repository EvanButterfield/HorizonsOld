#ifndef HORIZONS_PLATFORM_H
#define HORIZONS_PLATFORM_H

#ifdef __cplusplus
extern "C"
{
#endif
    
#if COMPILER_MSVC
#include <intrin.h>
#endif
    
    typedef struct material
    {
        v4 BaseColor;
    } material;
    
    typedef struct dir_light
    {
        v3 Direction;
        f32 Padding1;
        
        v3 Ambient;
        f32 Padding2;
        v3 Diffuse;
        f32 Padding3;
    } dir_light;
    
    typedef struct point_light
    {
        v3 Position;
        f32 Padding1;
        
        f32 Constant;
        f32 Linear;
        f32 Quadratic;
        f32 Padding2;
        
        v3 Ambient;
        f32 Padding3;
        v3 Diffuse;
        f32 Padding4;
    } point_light;
    
    typedef struct shader_constants
    {
        material Material;
        dir_light DirLight;
        point_light PointLight;
        
        v3 CameraPosition;
        
        f32 Time;
        
        mat4 M;
        mat4 TransformMat;
        mat3 NormalMat;
        v3 Padding2;
    } shader_constants;
    
    typedef struct ui_shader_constants
    {
        v4 Color;
        mat4 M;
        f32 Z;
    } ui_shader_constants;
    
    typedef struct vertex
    {
        v3 Position;
        v2 UV;
        v3 Normal;
        v4 Color;
    } vertex;
    
    typedef struct ui_vertex
    {
        v2 Position;
        v2 UV;
    } ui_vertex;
    
    typedef struct game_keyboard_input
    {
        b32 HasChar;
        char InputChar;
        
        b32 W;
        b32 A;
        b32 S;
        b32 D;
        b32 Q;
        b32 E;
        b32 M;
        
        b32 Up;
        b32 Left;
        b32 Down;
        b32 Right;
        
        b32 Backspace;
        b32 Escape;
        b32 Delete;
        b32 Space;
        b32 Enter;
        
        b32 One;
        b32 Two;
        b32 Three;
        
        b32 AltF4;
    } game_keyboard_input;
    
    typedef struct game_mouse_input
    {
        b32 LButton;
        b32 RButton;
        
        s32 TargetX;
        s32 X;
        s32 TargetY;
        s32 Y;
    } game_mouse_input;
    
    typedef struct game_controller_input
    {
        b32 Connected;
        b32 Up;
        b32 Left;
        b32 Down;
        b32 Right;
        b32 Start;
        b32 LeftShoulder;
        b32 RightShoulder;
        b32 A;
        b32 B;
        b32 X;
        b32 Y;
        s16 LeftTrigger;
        s16 RightTrigger;
        s16 LeftThumbX;
        s16 LeftThumbY;
        s16 LeftVibration;
        s16 RightVibration;
    } game_controller_input;
    
    typedef struct game_input
    {
        game_keyboard_input Keyboard;
        game_mouse_input Mouse;
        game_controller_input Controller;
    } game_input;
    
    typedef void *platform_file_handle;
    
    typedef struct file_contents
    {
        platform_file_handle Handle;
        void *Data;
        u32 Size;
    } file_contents;
    
    typedef struct window_dimension
    {
        u32 Width;
        u32 Height;
    } window_dimension;
    
    typedef enum window_flags
    {
        WINDOW_FLAGS_NONE = 0,
        WINDOW_FLAGS_CLOSED = 0b1
    } window_flags;
    
    typedef enum file_open_flags
    {
        FILE_OPEN_NONE = 0,
        FILE_OPEN_READ = 0b1,
        FILE_OPEN_WRITE = 0b10
    } file_open_flags;
    
    typedef struct loaded_sound
    {
        u8 *Data;
        u32 Size;
        void **Platform;
    } loaded_sound;
    
    typedef struct memory_arena
    {
        u8 *Memory;
        memory_index Size;
        memory_index Used;
    } memory_arena;
    
    internal void
        InitializeArena(memory_arena *Arena, u8 *BackingBuffer, memory_index Size)
    {
        Arena->Memory = BackingBuffer;
        Arena->Size = Size;
        Arena->Used = 0;
    }
    
#define PushStruct(Arena, type) (type *)PushSize(Arena, sizeof(type))
#define PushArray(Arena, type, Count) (type *)PushSize(Arena, sizeof(type)*(Count))
    internal void *
        PushSize(memory_arena *Arena, memory_index Size)
    {
        void *Result = 0;
        if(Arena->Size >= Arena->Used + Size)
        {
            Result = Arena->Memory + Arena->Used;
            Arena->Used += Size;
        }
        
        return(Result);
    }
    
#define PopStruct(Arena, type) (type *)PopSize(Arena, sizeof(type))
    internal void *
        PopSize(memory_arena *Arena, memory_index Size)
    {
        if(Arena->Used - Size >= 0)
        {
            Arena->Used -= Size;
        }
        
        void *Result = Arena->Memory + Arena->Used;
        return(Result);
    }
    
    // TODO(evan): Get rid of thread
#define PLATFORM_OPEN_FILE(name) platform_file_handle name(wchar_t *FileName, file_open_flags Flags)
    typedef PLATFORM_OPEN_FILE(platform_open_file);
    
#define PLATFORM_OPEN_FILE_8(name) platform_file_handle name(char *FileName, file_open_flags Flags)
    typedef PLATFORM_OPEN_FILE_8(platform_open_file_8);
    
#define PLATFORM_GET_FILE_SIZE(name) u32 name(platform_file_handle Handle)
    typedef PLATFORM_GET_FILE_SIZE(platform_get_file_size);
    
#define PLATFORM_READ_ENTIRE_FILE(name) void name(platform_file_handle Handle, u32 FileSize, void *Dest)
    typedef PLATFORM_READ_ENTIRE_FILE(platform_read_entire_file);
    
#define PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(platform_file_handle Handle, u32 DataSize, void *Data)
    typedef PLATFORM_WRITE_ENTIRE_FILE(platform_write_entire_file);
    
#define PLATFORM_CLOSE_FILE(name) void name(platform_file_handle Handle)
    typedef PLATFORM_CLOSE_FILE(platform_close_file);
    
#define PLATFORM_CREATE_MESH(name) void *name(void *VertexData, u32 VertexCount, u32 *IndexData, u32 IndexCount, b32 Is2D)
    typedef PLATFORM_CREATE_MESH(platform_create_mesh);
    
#define PLATFORM_POP_MESH(name) void name(void)
    typedef PLATFORM_POP_MESH(platform_pop_mesh);
    
#define PLATFORM_CREATE_TEXTURE(name) void *name(u32 *Data, u32 Width, u32 Height)
    typedef PLATFORM_CREATE_TEXTURE(platform_create_texture);
    
#define PLATFORM_CREATE_SHADER(name) void *name(wchar_t *ShaderPath, b32 Is2D)
    typedef PLATFORM_CREATE_SHADER(platform_create_shader);
    
#define PLATFORM_RENDER_MESH(name) void name(void *Mesh, void *Texture, void *Shader, void *Constants, b32 Is2D)
    typedef PLATFORM_RENDER_MESH(platform_render_mesh);
    
#define PLATFORM_CREATE_SOUND(name) loaded_sound name(wchar_t *FileName, f32 Volume, memory_arena *Scratch)
    typedef PLATFORM_CREATE_SOUND(platform_create_sound);
    
#define PLATFORM_PLAY_SOUND(name) void name(loaded_sound *Sound)
    typedef PLATFORM_PLAY_SOUND(platform_play_sound);
    
#define PLATFORM_SET_SOUND_VOLUME(name) void name(loaded_sound *Sound, f32 Volume)
    typedef PLATFORM_SET_SOUND_VOLUME(platform_set_sound_volume);
    
#define PLATFORM_STOP_SOUND(name) void name(loaded_sound *Sound)
    typedef PLATFORM_STOP_SOUND(platform_stop_sound);
    
#define PLATFORM_SOUND_IS_PLAYING(name) b32 name(loaded_sound *Sound)
    typedef PLATFORM_SOUND_IS_PLAYING(platform_sound_is_playing);
    
#define PLATFORM_DELETE_SOUND(name) void name(loaded_sound *Sound)
    typedef PLATFORM_DELETE_SOUND(platform_delete_sound);
    
#define PLATFORM_COPY_MEMORY(name) void name(void *Dest, void *Source, memory_index Length)
    typedef PLATFORM_COPY_MEMORY(platform_copy_memory);
    
#define PLATFORM_ZERO_MEMORY(name) void name(void *Dest, memory_index Length)
    typedef PLATFORM_ZERO_MEMORY(platform_zero_memory);
    
#define PLATFORM_OUTPUT_STRING(name) void name(char *Output)
    typedef PLATFORM_OUTPUT_STRING(platform_output_string);
    
    typedef struct platform_api
    {
        platform_open_file *OpenFile;
        platform_open_file_8 *OpenFile8;
        platform_get_file_size *GetFileSize;
        platform_read_entire_file *ReadEntireFile;
        platform_write_entire_file *WriteEntireFile;
        platform_close_file *CloseFile;
        
        platform_create_mesh *CreateMesh;
        platform_pop_mesh *PopMesh;
        platform_create_texture *CreateTexture;
        platform_create_shader *CreateShader;
        platform_render_mesh *RenderMesh;
        
        platform_create_sound *CreateSound;
        platform_play_sound *PlaySound;
        platform_set_sound_volume *SetSoundVolume;
        platform_stop_sound *StopSound;
        platform_delete_sound *DeleteSound;
        platform_sound_is_playing *SoundIsPlaying;
        
        platform_copy_memory *CopyMemory;
        platform_zero_memory *ZeroMemory;
        
        platform_output_string *OutputString;
    } platform_api;
    
    typedef struct game_memory
    {
        memory_index PermanentStorageSize;
        void *PermanentStorage;
        
        memory_index TransientStorageSize;
        void *TransientStorage;
        
        platform_api Platform;
    } game_memory;
    
    internal memory_index
        AlignTo(memory_index Value, memory_index Alignment)
    {
        memory_index R = Value % Alignment;
        memory_index Result = R ? (Value + (Alignment - R)) : Value;
        
        return(Result);
    }
    
#define GAME_UPDATE_AND_RENDER(name) b32 name(window_flags WindowFlags, window_dimension WindowDimension, game_memory *Memory, game_input *GameInput, f32 DeltaTime)
    typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
    GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub)
    {
        return(false);
    }
    
#ifdef __cplusplus
};
#endif

#endif //HORIZONS_PLATFORM_H
