global f32 GlobalSensitivity = 50.0f;
global f32 GlobalMaxPitch = 89.0f;
global f32 GlobalMinPitch = -89.0f;

global f32 GlobalSpeed = 12.0f;

global f32 Gravity = -19.62f*3.0f;
global f32 JumpHeight = 3.5f;
global f32 YVelocity;

global f32 XRotation = 0.0f;
global f32 YRotation = 0.0f;

RUN_ENTITY(RunPlayer)
{
    s32 MouseXAxis = Input->Mouse.X - State->LastInput.Mouse.X;
    s32 MouseYAxis = Input->Mouse.Y - State->LastInput.Mouse.Y;
    
    f32 MouseX = MouseXAxis * GlobalSensitivity * DeltaTime;
    f32 MouseY = MouseYAxis * GlobalSensitivity * DeltaTime;
    
    XRotation += MouseY;
    if(XRotation > GlobalMaxPitch)
        XRotation = GlobalMaxPitch;
    if(XRotation < GlobalMinPitch)
        XRotation = GlobalMinPitch;
    
    YRotation -= MouseX;
    
    Entity->Camera.Rotation = V3EulerToRotation(V3(XRotation, YRotation, 0.0f));
    Entity->Rotation = V3(0.0f, -YRotation, 0.0f);
    
    v2 Movement = {};
    if(Input->Keyboard.W)
    {
        Movement.z = -1.0f;
    }
    if(Input->Keyboard.S)
    {
        Movement.z = 1.0f;
    }
    if(Input->Keyboard.D)
    {
        Movement.x = 1.0f;
    }
    if(Input->Keyboard.A)
    {
        Movement.x = -1.0f;
    }
    Movement = V2Normalize(Movement);
    Movement *= GlobalSpeed*DeltaTime;
    
    v3 GroundedRotation = V3Normalize(V3(Entity->Camera.Rotation.x, 0.0f, Entity->Camera.Rotation.z));
    Entity->Camera.Position = Entity->Camera.Position + GroundedRotation*Movement.z;
    Entity->Camera.Position = (Entity->Camera.Position +
                               V3Normalize(V3Cross(GroundedRotation, Entity->Camera.Up)) *
                               Movement.x);
    if(Input->Keyboard.Enter && !State->LastInput.Keyboard.Enter)
    {
        Entity->Position.y = 10.0f;
    }
    
    if(Input->Keyboard.Space && Entity->Position.y <= 3.0f)
    {
        YVelocity = Sqrt(JumpHeight * -2.0f * Gravity);
    }
    YVelocity += Gravity*DeltaTime;
    Entity->Position.y += YVelocity*DeltaTime;
    
    if(Entity->Position.y < 3.0f)
    {
        YVelocity = -0.2f;
        Entity->Position.y = 3.0f;
    }
    
    Entity->Camera.Position.y = Entity->Position.y;
    
    Entity->Position.x = Entity->Camera.Position.x;
    Entity->Position.z = Entity->Camera.Position.z;
}

ON_COLLIDE_ENTITY(OnCollidePlayer)
{
    int Y = 0;
}