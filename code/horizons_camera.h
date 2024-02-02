#ifndef HORIZONS_CAMERA_H
#define HORIZONS_CAMERA_H

typedef struct camera
{
    v3 Position;
    v3 Rotation;
    v3 Up;
    v3 Euler;
    f32 FOV;
    f32 Near;
    f32 Far;
} camera;

#endif //HORIZONS_CAMERA_H
