#ifndef NEW_HORIZONS_H
#define NEW_HORIZONS_H

#include "horizons_types.h"
#include "horizons_platform.h"
#include "horizons_intrinsics.h"
#include "horizons_math.h"
#include "horizons_camera.h"

#define COLOR_WHITE v4{ 1.0f, 1.0f, 1.0f, 1.0f }
#define COLOR_RED   v4{ 1.0f, 0.0f, 0.0f, 1.0f }
#define COLOR_GREEN v4{ 0.0f, 1.0f, 0.0f, 1.0f }
#define COLOR_BLUE  v4{ 0.0f, 0.0f, 1.0f, 1.0f }
#define COLOR_BLACK v4{ 0.0f, 0.0f, 0.0f, 1.0f }
#define COLOR_GRAY  v4{ 0.5f, 0.5f, 0.5f, 1.0f }

struct high_level_vertex
{
    struct high_level_vertex *Next;
    vertex Data;
};

struct high_level_index
{
    struct high_level_index *Next;
    u32 Data;
};

struct mesh
{
    void *Platform;
    struct mesh *Next;
    
    v3 RelPosition;
    v3 RelScale;
    v3 RelRotation;
    
    vertex *Vertices;
    ui_vertex *UIVertices;
    u32 VertexCount;
    
    u32 *Indices;
    u32 IndexCount;
    
    material Material;
};

struct loaded_model
{
    mesh *Meshes;
    u32 MeshCount;
};

struct loaded_image
{
    void *Platform;
    v2 Size;
};

enum sound_asset_id
{
    SID_NONE,
    
    SID_EXPLOSION,
    
    SID_COUNT
};

enum image_asset_id
{
    IID_NONE,
    
    IID_SMILE,
    
    IID_COUNT
};

enum model_asset_id
{
    MID_NONE,
    
    // Primitives
    MID_BEAN,
    MID_SPHERE,
    
    // Game
    MID_SCENE,
    
    MID_COUNT
};

struct assets
{
    loaded_sound Sounds[SID_COUNT];
    loaded_image Images[IID_COUNT];
    loaded_model Models[MID_COUNT];
};

enum editor_mode
{
    EDITOR_MODE_NONE,
    EDITOR_MODE_UI_MOVE,
    EDITOR_MODE_UI_EDIT
};

enum ui_type
{
    UI_NONE = 0b0,
    UI_STRING = 0b1,
    UI_TOGGLE = 0b10
};
internal inline ui_type
operator|(ui_type A, ui_type B)
{
    ui_type Result = (ui_type)((int)A | (int)B);
    return(Result);
}

#define MAX_UI_STRING_LENGTH 128
struct ui_element
{
    b32 IsActive;
    
    loaded_image *Image;
    
    char String[MAX_UI_STRING_LENGTH];
    
    v2 Position;
    v2 Scale;
    f32 Thickness;
    
    b32 Toggled;
    
    ui_type Type;
};

#include "stb_truetype.h"

struct font
{
    stbtt_bakedchar *CData;
    s32 Width;
    s32 Height;
    void *Image;
    void *Meshes[95]; // ' ' through '~' is 95 glyphs
    f32 CharacterHeight;
};

#define RUN_ENTITY(name) void name(struct entity *Entity, f32 DeltaTime)
typedef RUN_ENTITY(run_entity);

#define ON_COLLIDE_ENTITY(name) void name(struct entity *Entity, struct entity *OtherEntity)
typedef ON_COLLIDE_ENTITY(on_collide_entity);

// TODO(evan): Quaternions
struct entity
{
    s32 Index;
    
    run_entity *RunEntity;
    on_collide_entity *OnCollideEntity;
    
    v3 Position;
    v3 Scale;
    v3 Rotation;
    
    entity *Parent;
    
    b32 HasPointLight;
    point_light Light;
    
    loaded_model *Model;
    
    camera Camera;
};

//
//
//

struct game_state
{
    game_input LastInput;
    
    window_dimension WindowDimension;
    
    dir_light DirLight;
    entity *PointLight;
    
    f32 Time;
    
    // NOTE(evan): We do this because multipe entities can "own" a camera
    //             The game needs to decide which one this points to
    //             (i.e. through which scene we are in)
    camera *Camera;
    
    void *Quad;
    void *BasicTexture;
    void *BasicShader;
    void *UIShader;
    
    memory_arena WorldArena;
    
    b32 DevMode;
    editor_mode EditorMode;
    
    font Font;
    
    memory_arena EntityArena;
    
    ui_element UIElements[MAX_UI_COUNT];
    s32 SelectedUI;
    v2 OriginalUIPosition; // NOTE(evan): This is so that we can reset UI when in move mode
};

struct transient_state
{
    b32 IsInitialized;
    memory_arena TranArena;
    memory_arena FrameArena;
    
    assets Assets;
    
    entity Entities[MAX_ENTITY_COUNT];
    s32 EntityCount;
};

#endif //NEW_HORIZONS_H