internal void
RenderLine(v4 Color, v2 Position, v2 Scale, mat4 *Projection, f32 Z)
{
    Scale /= 2.0f;
    Position += Scale;
    
    mat4 Transform = Mat4Identity();
    TranslateMat4(&Transform, V3(Position, 0.0f));
    v3 RotationRadians = { 0.0f, 0.0f, 0.0f };
    if(RotationRadians.x || RotationRadians.y || RotationRadians.z)
    {
        RotateMat4(&Transform,
                   V3Length(RotationRadians),
                   V3Normalize(RotationRadians));
    }
    ScaleMat4(&Transform, V3(Scale, 1.0f));
    
    ui_shader_constants Constants;
    Constants.M = Transform * *Projection;
    Constants.Color = Color;
    Constants.Z = Z;
    Platform->RenderMesh(State->Quad, State->BasicTexture, State->UIShader, &Constants, true);
}

internal void
RenderBox(v2 BottomLeft, v2 TopRight, v4 Color, f32 Thickness, mat4 *Projection, f32 Z)
{
    f32 Width = TopRight.x - BottomLeft.x;
    f32 Height = BottomLeft.y - TopRight.y;
    
    v2 TopSize = { Width + Thickness, Thickness };
    v2 TopPos = { BottomLeft.x - Thickness/2.0f, TopRight.y - Thickness/2.0f };
    RenderLine(Color, TopPos, TopSize, Projection, Z);
    
    v2 BottomPos = { BottomLeft.x - Thickness/2.0f, BottomLeft.y - Thickness/2.0f };
    RenderLine(Color, BottomPos, TopSize, Projection, Z);
    
    v2 SideSize = { Thickness, Height };
    v2 LeftPos = { BottomLeft.x - Thickness/2.0f, TopRight.y };
    RenderLine(Color, LeftPos, SideSize, Projection, Z);
    
    v2 RightPos = { TopRight.x - Thickness/2.0f, TopRight.y };
    RenderLine(Color, RightPos, SideSize, Projection, Z);
}

internal b32
UIImage(loaded_image Image, v2 Position, v2 Scale, v4 Color, mat4 *Projection,
        f32 Z, b32 IsHoverable, b32 Selected, b32 IsButton, void *Mesh = State->Quad)
{
    v2 OriginalSize = Image.Size*Scale;
    v2 OriginalPosition = Position;
    Image.Size = OriginalSize/2.0f;
    Position = OriginalPosition + Image.Size;
    
    mat4 Transform = Mat4Identity();
    TranslateMat4(&Transform, V3(Position.x, Position.y, 0.0f));
    v3 RotationRadians = { 0.0f, 0.0f, 0.0f };
    if(RotationRadians.x || RotationRadians.y || RotationRadians.z)
    {
        RotateMat4(&Transform,
                   V3Length(RotationRadians),
                   V3Normalize(RotationRadians));
    }
    ScaleMat4(&Transform, V3(Image.Size.Width, Image.Size.Height, 1.0f));
    
    b32 Hovered = false;
    v2 TopLeft = OriginalPosition;
    v2 BottomRight = TopLeft + OriginalSize;
    if(IsHoverable)
    {
        if(Input->Mouse.X > TopLeft.x && Input->Mouse.X < BottomRight.x &&
           Input->Mouse.Y > TopLeft.y && Input->Mouse.Y < BottomRight.y)
        {
            Hovered = true;
            
            if(!State->DevMode && !Input->Mouse.LButton)
            {
                Color *= v4{ 0.8f, 0.8f, 0.8f, 1.0f };
            }
        }
    }
    
    if(!State->DevMode && Hovered && Input->Mouse.LButton)
    {
        Color *= v4{ 0.5f, 0.5f, 0.5f, 1.0f };
    }
    
    ui_shader_constants Constants;
    Constants.M = Transform * *Projection;
    Constants.Color = Color;
    Constants.Z = Z;
    
    Platform->RenderMesh(Mesh, Image.Platform, State->UIShader, &Constants, true);
    
    if(State->DevMode && (Hovered || Selected))
    {
        v4 OutlineColor = {0};
        if(Hovered && !Selected)
        {
            OutlineColor = V4(0.8f, 0.8f, 0.8f, 1.0f);
        }
        else if(Selected)
        {
            OutlineColor = V4(0.5f, 0.5f, 0.5f, 1.0f);
        }
        
        v2 OutlineSize = { 5.0f, 5.0f };
        
        Transform = Mat4Identity();
        TranslateMat4(&Transform, V3(Position.x, Position.y, 0.0f));
        RotationRadians = { 0.0f, 0.0f, 0.0f };
        if(RotationRadians.x || RotationRadians.y || RotationRadians.z)
        {
            RotateMat4(&Transform,
                       V3Length(RotationRadians),
                       V3Normalize(RotationRadians));
        }
        ScaleMat4(&Transform, V3(Image.Size + OutlineSize, 1.0f));
        
        Constants.M = Transform * *Projection;
        Constants.Color = OutlineColor;
        Constants.Z = Z + 0.0001f;
        Platform->RenderMesh(State->Quad, State->BasicTexture, State->UIShader, &Constants, true);
    }
    
    return(Hovered);
}

internal b32
UIString(v2 Position, char *Text, font *Font, v4 Color, mat4 *Projection,
         f32 Z, b32 IsHoverable, b32 IsSelected, f32 *Width)
{
    loaded_image Image;
    Image.Platform = Font->Image;
    
    v2 OriginalPosition = Position;
    
    while(*Text)
    {
        if(*Text >= 32 && *Text < 128)
        {
            stbtt_aligned_quad Quad;
            stbtt_GetBakedQuad(Font->CData, Font->Width, Font->Height, *Text - ' ',
                               &Position.x, &Position.y, &Quad, 0);
            
            v2 QuadPos;
            QuadPos.x = Quad.x0;
            QuadPos.y = Quad.y0;
            
            Image.Size = V2(Quad.x1 - Quad.x0, Quad.y1 - Quad.y0);
            
            ui_vertex Vertices[4] =
            {
                { { +1.0f, +1.0f }, { Quad.s1, Quad.t1 } }, // Top right
                { { +1.0f, -1.0f }, { Quad.s1, Quad.t0 } }, // Bottom right
                { { -1.0f, -1.0f }, { Quad.s0, Quad.t0 } }, // Bottom left
                { { -1.0f, +1.0f }, { Quad.s0, Quad.t1 } }  // Top left
            };
            
            u32 Indices[6] =
            {
                0, 1, 3,
                1, 2, 3
            };
            
            void *Mesh = Platform->CreateMesh(Vertices, ArrayCount(Vertices),
                                              Indices, ArrayCount(Indices), true);
            
            UIImage(Image, QuadPos, V2(1.0f, 1.0f), Color, Projection, Z, false, false, false, Mesh);
            
            Platform->PopMesh();
        }
        ++Text;
    }
    
    b32 Hovered = false;
    v2 BottomLeft = OriginalPosition;
    v2 TopRight = { Position.x, Position.y - Font->CharacterHeight/2.0f };
    if(Width) *Width = TopRight.x - OriginalPosition.x;
    if(IsHoverable)
    {
        if(Input->Mouse.X > BottomLeft.x && Input->Mouse.X < TopRight.x &&
           Input->Mouse.Y < BottomLeft.y && Input->Mouse.Y > TopRight.y)
        {
            Hovered = true;
        }
    }
    
    if(State->DevMode && (Hovered || IsSelected))
    {
        v4 OutlineColor = {0};
        if(Hovered && !IsSelected)
        {
            OutlineColor = V4(0.8f, 0.8f, 0.8f, 1.0f);
        }
        if(IsSelected)
        {
            OutlineColor = V4(0.5f, 0.5f, 0.5f, 1.0f);
        }
        
        f32 OutlineWidth = 3.0f;
        f32 OutlineZ = Z - 0.0001f;
        
        RenderBox(BottomLeft, TopRight, OutlineColor, OutlineWidth, Projection, OutlineZ);
    }
    
    return(Hovered);
}

internal b32
UIToggle(v2 Position, v2 BoxDimension, f32 Thickness, char *Text, font *Font,
         v4 Color, v4 StringColor, mat4 *Projection,
         f32 Z, b32 IsHoverable, b32 IsSelected, b32 IsToggled)
{
    v2 BoxBottomLeft = { Position.x, Position.y + BoxDimension.y };
    v2 BoxTopRight = { Position.x + BoxDimension.x, Position.y };
    if(IsToggled && !State->DevMode)
    {
        v2 TogglePosition = { BoxBottomLeft.x + BoxDimension.x/2.0f, BoxTopRight.y + BoxDimension.y/2.0f };
        
        mat4 Transform = Mat4Identity();
        TranslateMat4(&Transform, V3(TogglePosition, 0.0f));
        ScaleMat4(&Transform, V3(BoxDimension / 2.0f, 1.0f));
        
        ui_shader_constants Constants;
        Constants.M = Transform * *Projection;
        Constants.Color = Color;
        Constants.Z = Z;
        Platform->RenderMesh(State->Quad, State->BasicTexture, State->UIShader, &Constants, true);
    }
    else
    {
        RenderBox(BoxBottomLeft, BoxTopRight, Color, Thickness, Projection, Z);
    }
    
    f32 StringSpacing = Thickness/2.0f;
    v2 StringPos;
    StringPos.x = BoxTopRight.x + StringSpacing;
    StringPos.y = BoxBottomLeft.y - Font->CharacterHeight/4.0f;
    
    f32 StringWidth;
    UIString(StringPos, Text, Font, StringColor, Projection, Z, false, false, &StringWidth);
    
    b32 Hovered = false;
    v2 BottomLeft = {0};
    v2 TopRight = {0};
    if(IsHoverable)
    {
        BottomLeft = BoxBottomLeft;
        TopRight.x = StringPos.x + StringWidth;
        TopRight.y = BoxTopRight.y;
        
        if(Input->Mouse.X > BottomLeft.x && Input->Mouse.X < TopRight.x &&
           Input->Mouse.Y < BottomLeft.y && Input->Mouse.Y > TopRight.y)
        {
            Hovered = true;
        }
    }
    
    if(State->DevMode && (Hovered || IsSelected))
    {
        v4 OutlineColor = {0};
        if(Hovered && !IsSelected)
        {
            OutlineColor = V4(0.8f, 0.8f, 0.8f, 1.0f);
        }
        if(IsSelected)
        {
            OutlineColor = V4(0.5f, 0.5f, 0.5f, 1.0f);
        }
        
        f32 OutlineWidth = 3.0f;
        f32 OutlineZ = Z + 0.0001f;
        
        RenderBox(BottomLeft, TopRight, OutlineColor, OutlineWidth, Projection, OutlineZ);
    }
    
    return(Hovered);
}