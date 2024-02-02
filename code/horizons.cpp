#include "horizons.h"

global game_state *State;
global transient_state *TranState;
global game_input *Input;
global platform_api *Platform;
global b32 ShouldClose;

#include "horizons_string.cpp"
#include "horizons_camera.cpp"
#include "horizons_ui.cpp"

#define STBI_ASSERT(x) Assert(x)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBTT_ASSERT(x) Assert(x)
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define PlatformOutputString(Format, ...) \
{ \
char Buffer[32000];\
VariableToString(Buffer, Format, __VA_ARGS__); \
Platform->OutputString(Buffer); \
}

//
// NOTE(evan): Entities
//
#include "horizons_player.cpp"
//
//
//

internal file_contents
ReadFile8(char *FileName, memory_arena *Arena)
{
    file_contents Result = {0};
    
    Result.Handle = Platform->OpenFile8(FileName, FILE_OPEN_READ);
    Result.Size = Platform->GetFileSize(Result.Handle);
    Result.Data = PushSize(Arena, Result.Size);
    Platform->ReadEntireFile(Result.Handle, Result.Size, Result.Data);
    
    return(Result);
}

#include "horizons_gltf.cpp"

internal void
CalculateTransform(mat4 *Result, v3 Position, v3 Scale, v3 Rotation)
{
    TranslateMat4(Result, Position);
    v3 RotationRadians = Rotation*DEG_TO_RAD;
    if(RotationRadians.x || RotationRadians.y || RotationRadians.z)
    {
        RotateMat4(Result,
                   V3Length(RotationRadians),
                   V3Normalize(RotationRadians));
    }
    ScaleMat4(Result, Scale);
}

internal void
RenderEntity(entity *Entity, entity *Parent,
             void *Texture, void *Shader, point_light PointLight,
             mat4 *Projection, mat4 *View)
{
    loaded_model Model = *Entity->Model;
    
    v3 TempPosition = Entity->Position;
    v3 TempScale = Entity->Scale;
    v3 TempRotation = Entity->Rotation;
    
    mat4 PrevM = *View * *Projection;
    
    for(u32 MeshIndex = 0;
        MeshIndex < Model.MeshCount;
        ++MeshIndex)
    {
        mesh *Mesh = &Model.Meshes[MeshIndex];
        
        mat4 EntityTransformMat = Mat4Identity();
        TempPosition += Mesh->RelPosition;
        TempScale *= Mesh->RelScale;
        CalculateTransform(&EntityTransformMat, TempPosition, TempScale, TempRotation);
        
        mat4 ParentTransformMat = Mat4Identity();
        if(Parent)
        {
            CalculateTransform(&ParentTransformMat, Parent->Position, Parent->Scale, Parent->Rotation);
        }
        
        mat4 Transform = EntityTransformMat;
        if(Parent)
        {
            Transform = Transform*ParentTransformMat;
        }
        
        mat4 M4Normal;
        Mat4Inverse(&M4Normal, &Transform);
        Mat4Transpose(&M4Normal, &M4Normal);
        
        mat3 NormalMat = Mat3FromMat4(&M4Normal);
        
        mat4 M = Transform*PrevM;
        shader_constants Constants;
        Constants.Material = Mesh->Material;
        Constants.DirLight = State->DirLight;
        Constants.PointLight = PointLight;
        Constants.CameraPosition = V3(PrevM*V4(State->Camera->Position, 1.0f));
        Constants.Time = State->Time;
        Constants.M = M;
        Constants.TransformMat = Transform;
        Constants.NormalMat = NormalMat;
        
        Platform->RenderMesh(Mesh->Platform, Texture, Shader, &Constants, false);
    }
}

internal inline f32
Lerp(f32 Start, f32 Target, f32 Current)
{
    f32 Result = Start + Current*(Target - Start);
    return(Result);
}

internal file_contents
ReadFile16(string16 *FileName, memory_arena *FileArena)
{
    file_contents Result = {0};
    
    Result.Handle = Platform->OpenFile(FileName, FILE_OPEN_READ);
    Result.Size = Platform->GetFileSize(Result.Handle);
    Result.Data = PushSize(FileArena, Result.Size);
    Platform->ReadEntireFile(Result.Handle, Result.Size, Result.Data);
    
    return(Result);
}

internal loaded_image
CreateImage(char *ImagePath, memory_arena *Arena)
{
    s32 Width, Height, Channels;
    u32 *TextureData = (u32 *)stbi_load(ImagePath, &Width, &Height, &Channels, 4);
    
    loaded_image Result;
    Result.Size.Width = (f32)Width;
    Result.Size.Height = (f32)Height;
    Result.Platform = Platform->CreateTexture(TextureData, Width, Height);
    
    stbi_image_free(TextureData);
    
    return(Result);
}

internal font
LoadFont(string16 *Path, memory_arena *Arena, f32 FontHeight)
{
    file_contents File = ReadFile16(Path, Arena);
    
#define TEMP_WIDTH 512
#define TEMP_HEIGHT 512
    u8 TempBitmap[TEMP_WIDTH*TEMP_HEIGHT];
    stbtt_bakedchar CData[96]; // ' ' through '~' is 95 glyphs
    
    stbtt_BakeFontBitmap((u8 *)File.Data, 0, FontHeight, TempBitmap, TEMP_WIDTH, TEMP_HEIGHT, ' ', 96, CData);
    
    u32 *Texture = PushArray(Arena, u32, TEMP_WIDTH*TEMP_HEIGHT);
    
    u8 *Source = TempBitmap;
    u8 *DestRow = (u8 *)Texture;
    for(s32 Y = 0;
        Y < TEMP_HEIGHT;
        ++Y)
    {
        u32 *Dest = (u32 *)DestRow;
        for(s32 X = 0;
            X < TEMP_WIDTH;
            ++X)
        {
            u8 Alpha = *Source++;
            *Dest++ = ((Alpha << 24) |
                       (255 << 16)   |
                       (255 << 8)    |
                       (255 << 0));
        }
        
        DestRow += TEMP_WIDTH*sizeof(u32);
    }
    
    font Result;
    Result.CData = PushArray(Arena, stbtt_bakedchar, TEMP_WIDTH*TEMP_HEIGHT);
    Platform->CopyMemory(Result.CData, CData, TEMP_WIDTH*TEMP_HEIGHT);
    Result.Width = TEMP_WIDTH;
    Result.Height = TEMP_HEIGHT;
    Result.Image = Platform->CreateTexture(Texture, TEMP_WIDTH, TEMP_HEIGHT);
    Result.CharacterHeight = FontHeight;
    
#undef TEMP_WIDTH
#undef TEMP_HEIGHT
    
    return(Result);
}

internal loaded_sound *
GetSoundAsset(sound_asset_id ID)
{
    loaded_sound *Result = 0;
    Result = &TranState->Assets.Sounds[ID];
    
    return(Result);
}

internal loaded_image *
GetImageAsset(image_asset_id ID)
{
    loaded_image*Result = 0;
    Result = &TranState->Assets.Images[ID];
    
    return(Result);
}

internal loaded_model *
GetModelAsset(model_asset_id ID)
{
    loaded_model *Result = 0;
    Result = &TranState->Assets.Models[ID];
    
    return(Result);
}

internal entity *
CreateEntity(entity *Parent,
             b32 HasPointLight, point_light Light,
             model_asset_id MID, v3 Position, v3 Scale, v3 Rotation,
             run_entity *RunEntity = 0, on_collide_entity *OnCollideEntity = 0)
{
    entity Entity;
    
    Entity.RunEntity = RunEntity;
    Entity.OnCollideEntity = OnCollideEntity;
    
    Entity.Position = Position;
    Entity.Scale = Scale;
    Entity.Rotation = Rotation;
    
    Entity.HasPointLight = HasPointLight;
    Entity.Light = Light;
    
    if(MID)
    {
        Entity.Model = GetModelAsset(MID);
    }
    else
    {
        Entity.Model = 0;
    }
    
    Entity.Camera = {};
    
    Entity.Parent = Parent;
    
    Entity.Index = TranState->EntityCount;
    TranState->Entities[TranState->EntityCount++] = Entity;
    
    return(&TranState->Entities[TranState->EntityCount - 1]);
}

GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Input = GameInput;
    
    State = (game_state *)Memory->PermanentStorage;
    TranState = (transient_state *)Memory->TransientStorage;
    
    State->Time += DeltaTime;
    
    Platform = &Memory->Platform;
    
    //
    // NOTE(evan): Initialization
    //
    if(!TranState->IsInitialized)
    {
        memory_index FrameArenaSize = Memory->PermanentStorageSize/2;
        
        InitializeArena(&State->WorldArena, (u8 *)Memory->PermanentStorage + sizeof(game_state),
                        Memory->PermanentStorageSize - sizeof(game_state));
        InitializeArena(&TranState->FrameArena, (u8 *)Memory->TransientStorage + sizeof(transient_state), FrameArenaSize);
        InitializeArena(&TranState->TranArena, (u8 *)Memory->TransientStorage + sizeof(transient_state) + FrameArenaSize,
                        Memory->TransientStorageSize - sizeof(transient_state) - FrameArenaSize);
        InitializeArena(&State->EntityArena,
                        (u8 *)PushArray(&State->WorldArena, entity, MAX_ENTITY_COUNT),
                        sizeof(entity)*MAX_ENTITY_COUNT);
        
        Platform->ZeroMemory(TranState->Entities, sizeof(entity *)*MAX_ENTITY_COUNT);
        
        {
            u32 Data[1] =
            {
                0xFFFFFFFF
            };
            u32 Width = 1;
            u32 Height = 1;
            State->BasicTexture = Platform->CreateTexture(Data, Width, Height);
            
            ui_vertex Vertices[4] =
            {
                { { +1.0f, +1.0f }, { 1.0f, 1.0f } }, // Top right
                { { +1.0f, -1.0f }, { 1.0f, 0.0f } }, // Bottom right
                { { -1.0f, -1.0f }, { 0.0f, 0.0f } }, // Bottom left
                { { -1.0f, +1.0f }, { 0.0f, 1.0f } }  // Top left
            };
            
            u32 Indices[6] =
            {
                0, 1, 3,
                1, 2, 3
            };
            State->Quad = Platform->CreateMesh(Vertices, ArrayCount(Vertices),
                                               Indices, ArrayCount(Indices), true);
        }
        
        State->Font = LoadFont(L"../assets/RandyGG.ttf", &TranState->TranArena, 32.0f);
        
        // NOTE(evan): Sounds
        *GetSoundAsset(SID_EXPLOSION) = Platform->CreateSound(L"../assets/sound.wav", 0.5f, &State->WorldArena);
        
        // NOTE(evan): Images
        *GetImageAsset(IID_SMILE) = CreateImage("../assets/test.png", &State->WorldArena);
        
        // NOTE(evan): Models
        *GetModelAsset(MID_SCENE) = LoadGLTF("../assets/models/scene.gltf", &State->WorldArena);
        *GetModelAsset(MID_SPHERE) = LoadGLTF("../assets/models/primitives/sphere.gltf", &State->WorldArena);
        *GetModelAsset(MID_BEAN) = LoadGLTF("../assets/models/primitives/bean.gltf", &State->WorldArena);
        
        CreateEntity(0, false, {}, MID_SCENE, {}, V3(1.0f, 1.0f, 1.0f), {});
        entity *Player = CreateEntity(0, false, {}, MID_NONE, V3(0.0f, 10.0f, 0.0f), V3(1.0f, 1.0f, 1.0f), {}, RunPlayer);
        CreateEntity(Player, false, {}, MID_BEAN, V3(0.0f, 0.0f, 0.0f), V3(1.0f, 1.0f, 1.0f), {}, 0, OnCollidePlayer);
        
        InitializeCamera(&Player->Camera, Player->Position, Player->Position, Player->Position, 60, 0.01f, 100.0f);
        State->Camera = &Player->Camera;
        
        State->DirLight.Direction = V3(-0.2f, -1.0f, -0.3f);
        State->DirLight.Ambient = V3(0.1f, 0.1f, 0.1f);
        State->DirLight.Diffuse = V3(0.3f, 0.3f, 0.3f);
        
        {
            point_light Light;
            Light.Position = V3(0.0f, 3.0f, 0.0f);
            Light.Constant = 0.1f;
            Light.Linear = 0.9f;
            Light.Quadratic = 0.08f;
            Light.Ambient = V3(0.1f, 0.1f, 0.1f);
            Light.Diffuse = V3(0.8f, 0.8f, 0.8f);
            State->PointLight = CreateEntity(0, true, Light, MID_SPHERE,
                                             V3(0.0f, 3.0f, 0.0f), V3(1.0f, 1.0f, 1.0f), V3(0.0f, 0.0f, 0.0f));
        }
        
        State->BasicShader = Platform->CreateShader(L"../code/shaders.hlsl", false);
        State->UIShader = Platform->CreateShader(L"../code/ui_shaders.hlsl", true);
        
        State->DevMode = false;
        State->SelectedUI = -1;
        State->EditorMode = EDITOR_MODE_NONE;
        
        TranState->IsInitialized = true;
    }
    //
    //
    //
    State->WindowDimension = WindowDimension;
    
    for(s32 EntityIndex = 0;
        EntityIndex < TranState->EntityCount;
        ++EntityIndex)
    {
        entity *Entity = &TranState->Entities[EntityIndex];
        if(Entity && Entity->RunEntity)
        {
            Entity->RunEntity(Entity, DeltaTime);
        }
    }
    
    if(Input->Keyboard.Space && !State->LastInput.Keyboard.Space &&
       State->EditorMode == EDITOR_MODE_NONE)
    {
        State->DevMode = !State->DevMode;
    }
    
    if(Input->Keyboard.AltF4 || (WindowFlags & WINDOW_FLAGS_CLOSED))
    {
        ShouldClose = true;
    }
    
    if(State->DevMode && State->EditorMode == EDITOR_MODE_NONE &&
       Input->Keyboard.One && !State->LastInput.Keyboard.One)
    {
        for(s32 UIIndex = 0;
            UIIndex < MAX_UI_COUNT;
            ++UIIndex)
        {
            ui_element *UI = &State->UIElements[UIIndex];
            
            if(UI->IsActive)
            {
                continue;
            }
            
            UI->IsActive = true;
            UI->Image = GetImageAsset(IID_SMILE);
            UI->Position.x = (f32)Input->Mouse.X;
            UI->Position.y = (f32)Input->Mouse.Y;
            UI->Scale = V2(0.25f, 0.25f);
            UI->Type = UI_NONE;
            
            break;
        }
    }
    
    if(State->DevMode && State->EditorMode == EDITOR_MODE_NONE &&
       Input->Keyboard.Two && !State->LastInput.Keyboard.Two)
    {
        for(s32 UIIndex = 0;
            UIIndex < MAX_UI_COUNT;
            ++UIIndex)
        {
            ui_element *UI = &State->UIElements[UIIndex];
            
            if(UI->IsActive)
            {
                continue;
            }
            
            UI->IsActive = true;
            Platform->CopyMemory(UI->String, "EDIT TEXT", StringLength("HELLO, WORLD!"));
            UI->Position.x = (f32)Input->Mouse.X;
            UI->Position.y = (f32)Input->Mouse.Y;
            UI->Type = UI_STRING;
            
            break;
        }
    }
    
    if(State->DevMode && State->EditorMode == EDITOR_MODE_NONE &&
       Input->Keyboard.Three && !State->LastInput.Keyboard.Three)
    {
        for(s32 UIIndex = 0;
            UIIndex < MAX_UI_COUNT;
            ++UIIndex)
        {
            ui_element *UI = &State->UIElements[UIIndex];
            
            if(UI->IsActive)
            {
                continue;
            }
            
            UI->IsActive = true;
            Platform->CopyMemory(UI->String, "EDIT TEXT", StringLength("HELLO, WORLD!"));
            UI->Position.x = (f32)Input->Mouse.X;
            UI->Position.y = (f32)Input->Mouse.Y;
            UI->Scale = v2{ 30.0f, 30.0f };
            UI->Thickness = 2.0f;
            UI->Toggled = false;
            UI->Type = UI_STRING|UI_TOGGLE;
            
            break;
        }
    }
    
    if(State->DevMode && State->SelectedUI >= 0)
    {
        ui_element *UI = &State->UIElements[State->SelectedUI];
        
        v2 Position = UI->Position;
        v2 Scale = UI->Scale;
        ui_type Type = UI->Type;
        
        if(Input->Keyboard.Escape)
        {
            if(State->EditorMode == EDITOR_MODE_UI_MOVE)
            {
                UI->Position = State->OriginalUIPosition;
            }
            
            if(Type & UI_STRING && UI->String[0] == 0)
            {
                // NOTE(evan): Can't have an empty string taking up precious UI space
            }
            else
            {
                State->EditorMode = EDITOR_MODE_NONE;
            }
        }
        
        if(Input->Keyboard.M && State->EditorMode == EDITOR_MODE_NONE)
        {
            State->OriginalUIPosition = Position;
            State->EditorMode = EDITOR_MODE_UI_MOVE;
        }
        
        if(Input->Keyboard.E &&
           State->EditorMode == EDITOR_MODE_NONE &&
           (Type & UI_STRING))
        {
            Input->Keyboard.HasChar = false;
            State->EditorMode = EDITOR_MODE_UI_EDIT;
        }
        
        if(Input->Keyboard.Delete)
        {
            UI->IsActive = false;
            State->SelectedUI = -1;
        }
    }
    else if(State->SelectedUI <= 0)
    {
        State->EditorMode = EDITOR_MODE_NONE;
    }
    
    if(State->DevMode && State->EditorMode == EDITOR_MODE_UI_MOVE)
    {
        State->UIElements[State->SelectedUI].Position = V2(Input->Mouse.X, Input->Mouse.Y);
    }
    
    if(State->DevMode && State->EditorMode == EDITOR_MODE_UI_EDIT)
    {
        ui_element *UI = &State->UIElements[State->SelectedUI];
        
        if(Input->Keyboard.HasChar)
        {
            s32 Length = StringLength(UI->String);
            UI->String[Length] = Input->Keyboard.InputChar;
            UI->String[Length + 1] = 0;
            
            int Y = 0;
        }
        
        if(Input->Keyboard.Backspace && !State->LastInput.Keyboard.Backspace)
        {
            s32 Length = StringLength(UI->String);
            UI->String[Length - 1] = 0;
        }
    }
    
    //
    // NOTE(evan): Rendering
    //
    {
        f32 Aspect = (f32)WindowDimension.Width / (f32)WindowDimension.Height;
        mat4 Projection;
        CreatePerspective(&Projection, Aspect, State->Camera->FOV*DEG_TO_RAD,
                          State->Camera->Near, State->Camera->Far);
        mat4 View;
        GetCameraView(State->Camera, &View);
        
        for(s32 EntityIndex = 0;
            EntityIndex < TranState->EntityCount;
            ++EntityIndex)
        {
            entity *Entity = &TranState->Entities[EntityIndex];
            if(Entity)
            {
                if(Entity->Model)
                {
                    RenderEntity(Entity, Entity->Parent, State->BasicTexture, State->BasicShader,
                                 State->PointLight->Light, &Projection, &View);
                }
            }
        }
        
        CreateOrthographic(&Projection,
                           0.0f, (f32)WindowDimension.Width,
                           0.0f, (f32)WindowDimension.Height,
                           State->Camera->Near, State->Camera->Far);
        
        b32 AlreadyHovered = false;
        f32 ZIndex = 0.0f;
        for(s32 UIIndex = MAX_UI_COUNT - 1;
            UIIndex >= 0;
            --UIIndex)
        {
            ui_element *UI = &State->UIElements[UIIndex];
            
            if(!UI->IsActive)
            {
                continue;
            }
            
            loaded_image Image = *UI->Image;
            v2 Position = UI->Position;
            v2 Scale = UI->Scale;
            f32 Thickness = UI->Thickness;
            ui_type Type = UI->Type;
            
            b32 Selected = UIIndex == State->SelectedUI;
            
            b32 Hovered = false;
            
            // TODO(evan): Make this not stupud
            if(Type & UI_STRING)
            {
                if(Type & UI_TOGGLE)
                {
                    Hovered = UIToggle(Position, Scale, Thickness, UI->String, &State->Font,
                                       COLOR_WHITE, COLOR_WHITE, &Projection, ZIndex,
                                       !AlreadyHovered, Selected, UI->Toggled);
                }
                else
                {
                    Hovered = UIString(Position, UI->String, &State->Font, COLOR_WHITE, &Projection, ZIndex,
                                       !AlreadyHovered, Selected, 0);
                }
            }
            else
            {
                Hovered = UIImage(Image, Position, Scale, COLOR_WHITE, &Projection, ZIndex,
                                  !AlreadyHovered, Selected, true);
            }
            
            if(Hovered)
            {
                AlreadyHovered = true;
                
                if(Input->Mouse.LButton && !State->LastInput.Mouse.LButton && !State->DevMode)
                {
                    UI->Toggled = !UI->Toggled;
                }
                
                if(Input->Mouse.LButton && State->DevMode)
                {
                    Selected = true;
                    State->SelectedUI = UIIndex;
                }
                
                if(Input->Mouse.RButton)
                {
                }
            }
            
            ZIndex += 0.001f;
        }
        
        if(Input->Mouse.LButton && !State->LastInput.Mouse.LButton)
        {
            ui_element UI = State->UIElements[State->SelectedUI];
            if((UI.Type & UI_STRING) && UI.String[0] == 0)
            {
                // NOTE(evan): Can't have an empty string taking up precious UI space
            }
            else
            {
                State->SelectedUI = -1;
            }
        }
    }
    //
    //
    //
    
    State->LastInput = *Input;
    TranState->FrameArena.Used = 0;
    
    return(ShouldClose);
}
