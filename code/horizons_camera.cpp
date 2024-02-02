internal void
InitializeCamera(camera *Camera, v3 Position, v3 Rotation, v3 Up,
                 f32 FOV, f32 Near, f32 Far)
{
    Camera->Position = Position;
    Camera->Rotation = Rotation;
    Camera->Up = Up;
    Camera->Euler.Yaw = 90.0f;
    Camera->FOV = FOV;
    Camera->Near = Near;
    Camera->Far = Far;
}

internal void
GetCameraView(camera *Camera, mat4 *Result)
{
    Mat4LookAt(Result, Camera->Position, Camera->Position + Camera->Rotation, Camera->Up);
}