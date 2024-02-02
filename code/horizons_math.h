#ifndef HORIZONS_MATH_H
#define HORIZONS_MATH_H

//
// NOTE(evan): v2 operations
//

internal inline v2
V2(f32 X, f32 Y)
{
    v2 Result = {X, Y};
    
    return(Result);
}

internal inline v2
V2(s32 X, s32 Y)
{
    v2 Result = {(f32)X, (f32)Y};
    
    return(Result);
}

internal inline v2
operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return(Result);
}

internal inline v2 &
operator+=(v2 &A, v2 B)
{
    A.x += B.x;
    A.y += B.y;
    return(A);
}

internal inline v2
operator*(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x*B.x;
    Result.y = A.y*B.y;
    
    return(Result);
}

internal inline v2 &
operator*=(v2 &A, v2 B)
{
    A = A*B;
    return(A);
}

internal inline v2
operator*(v2 A, f32 B)
{
    v2 Result;
    
    Result.x = A.x*B;
    Result.y = A.y*B;
    
    return(Result);
}

internal inline v2
operator*(f32 B, v2 A)
{
    v2 Result;
    
    Result.x = A.x*B;
    Result.y = A.y*B;
    
    return(Result);
}

internal inline v2 &
operator*=(v2 &A, f32 B)
{
    A = A*B;
    return(A);
}

internal inline v2
operator/(v2 A, f32 B)
{
    v2 Result;
    
    Result.x = A.x/B;
    Result.y = A.y/B;
    
    return(Result);
}

internal inline v2
operator/(f32 B, v2 A)
{
    v2 Result;
    
    Result.x = A.x/B;
    Result.y = A.y/B;
    
    return(Result);
}

internal inline v2 &
operator/=(v2 &A, f32 B)
{
    A.x /= B;
    A.y /= B;
    return(A);
}

internal inline f32
V2MultiplyDot(v2 A, v2 B)
{
    f32 Result = A.x*B.x + A.y*B.y;
    
    return(Result);
}

internal inline f32
V2Length(v2 A)
{
    f32 Result = V2MultiplyDot(A, A);
    if(Result)
    {
        Result = Sqrt(Result);
    }
    
    return(Result);
}

internal inline v2
V2Normalize(v2 A)
{
    f32 Length = V2Length(A);
    
    f32 K = 0.0f;
    if(Length)
    {
        K = 1.0f / Length;
    }
    
    v2 Result = A*K;
    return(Result);
}

//
// NOTE(evan): v3 operations
//

internal inline v3
V3Cross(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.y*B.z - A.z*B.y;
    Result.y = A.z*B.x - A.x*B.z;
    Result.z = A.x*B.y + A.y*B.x;
    
    return(Result);
}

internal inline b32
operator==(v3 A, v3 B)
{
    b32 Result = (A.x == B.x &&
                  A.y == B.y &&
                  A.z == B.z);
    return(Result);
}

internal inline b32
operator!=(v3 A, v3 B)
{
    b32 Result = !(A == B);
    return(Result);
}

internal inline b32
operator<(v3 A, v3 B)
{
    b32 X = A.x < B.x;
    b32 Y = A.y < B.y;
    b32 Z = A.z < B.z;
    b32 Result = X && Y && Z;
    
    return(Result);
}

internal inline b32
operator>(v3 A, v3 B)
{
    b32 X = A.x > B.x;
    b32 Y = A.y > B.y;
    b32 Z = A.z > B.z;
    b32 Result = X && Y && Z;
    
    return(Result);
}

internal inline v3
V3Min(v3 A, v3 B)
{
    v3 Result = A < B ? A : B;
    return(Result);
}

internal inline v3
V3Max(v3 A, v3 B)
{
    v3 Result = A > B ? A : B;
    return(Result);
}

internal inline v3
V3(f32 X, f32 Y, f32 Z)
{
    v3 Result = { X, Y, Z };
    
    return(Result);
}

internal inline v3
V3(v2 A, f32 B)
{
    v3 Result = { A.x, A.y, B };
    return(Result);
}

internal inline v3
V3(v4 A)
{
    v3 Result = { A.x, A.y, A.z };
    return(Result);
}

internal inline v3
operator+(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return(Result);
}

internal inline v3 &
operator+=(v3 &A, v3 B)
{
    A = A + B;
    return(A);
}

internal inline v3
operator-(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    
    return(Result);
}

internal inline v3
operator*(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x*B.x;
    Result.y = A.y*B.y;
    Result.z = A.z*B.z;
    
    return(Result);
}

internal inline v3 &
operator*=(v3 &A, v3 B)
{
    A = A*B;
    return(A);
}

internal inline v3
operator*(v3 A, f32 B)
{
    v3 Result;
    
    Result.x = A.x*B;
    Result.y = A.y*B;
    Result.z = A.z*B;
    
    return(Result);
}

internal inline v3
operator*(f32 B, v3 A)
{
    v3 Result;
    
    Result.x = A.x*B;
    Result.y = A.y*B;
    Result.z = A.z*B;
    
    return(Result);
}

internal inline v3 &
operator*=(v3 &A, f32 B)
{
    A = A*B;
    return(A);
}

internal inline f32
V3MultiplyDot(v3 A, v3 B)
{
    f32 Result = A.x*B.x + A.y*B.y + A.z*B.z;
    
    return(Result);
}

internal inline f32
V3Length(v3 A)
{
    return(Sqrt(V3MultiplyDot(A, A)));
}

internal inline v3
V3Normalize(v3 A)
{
    f32 K = 1.0f / V3Length(A);
    
    v3 Result = A*K;
    return(Result);
}

internal inline v3
V3EulerToRotation(v3 Euler)
{
    v3 Direction;
    Direction.Pitch = (Cos(Euler.Yaw*DEG_TO_RAD) *
                       Cos(Euler.Pitch*DEG_TO_RAD));
    Direction.Yaw = Sin(Euler.Pitch*DEG_TO_RAD);
    Direction.Roll = (Sin(Euler.Yaw*DEG_TO_RAD) *
                      Cos(Euler.Pitch*DEG_TO_RAD));
    Direction = V3Normalize(Direction);
    
    return(Direction);
}

//
// NOTE(evan): v4 operations
//

internal inline v4
V4(f32 X, f32 Y, f32 Z, f32 W)
{
    v4 Result = { X, Y, Z, W };
    
    return(Result);
}

internal inline v4
V4(v3 A, f32 B)
{
    v4 Result = { A.x, A.y, A.z, B };
    
    return(Result);
}

internal inline v4
operator+(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;
    
    return(Result);
}

internal inline v4
operator*(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x*B.x;
    Result.y = A.y*B.y;
    Result.z = A.z*B.z;
    Result.w = A.w*B.w;
    
    return(Result);
}

internal inline v4
operator*(v4 A, f32 B)
{
    v4 Result;
    Result.x = A.x*B;
    Result.y = A.y*B;
    Result.z = A.z*B;
    Result.w = A.w*B;
    
    return(Result);
}

internal inline v4
operator*(f32 B, v4 A)
{
    v4 Result;
    Result.x = A.x*B;
    Result.y = A.y*B;
    Result.z = A.z*B;
    Result.w = A.w*B;
    
    return(Result);
}

internal inline v4 &
operator*=(v4 &A, v4 B)
{
    A = A*B;
    return(A);
}

internal inline b32
operator==(v4 A, v4 B)
{
    b32 Result = (A.r == B.r &&
                  A.g == B.g &&
                  A.b == B.b &&
                  A.a == B.a);
    return(Result);
}

internal inline b32
operator!=(v4 A, v4 B)
{
    b32 Result = (A.r != B.r &&
                  A.g != B.g &&
                  A.b != B.b &&
                  A.a != B.a);
    return(Result);
}

//
// NOTE(evan): mat3 operations
//

internal mat3
Mat3FromMat4(mat4 *A)
{
    mat3 Result =
    {
        A->e[0].x, A->e[0].y, A->e[0].z,
        A->e[1].x, A->e[1].y, A->e[1].z,
        A->e[2].x, A->e[2].y, A->e[2].z,
    };
    
    return(Result);
}

//
// NOTE(evan): mat4 operations
//

internal inline mat4
Mat4Identity(void)
{
    mat4 Result =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    return(Result);
}

internal void
Mat4Transpose(mat4 *Result, mat4 *A)
{
    mat4 Mat =
    {
        A->e[0].x, A->e[1].x, A->e[2].x, A->e[3].x,
        A->e[0].y, A->e[1].y, A->e[2].y, A->e[3].y,
        A->e[0].z, A->e[1].z, A->e[2].z, A->e[3].z,
        A->e[0].w, A->e[1].w, A->e[2].w, A->e[3].w
    };
    
    *Result = Mat;
}

internal void
Mat4Inverse(mat4 *Result, mat4 *A)
{
    f32 S[6];
    S[0] = A->m[0][0]*A->m[1][1] - A->m[1][0]*A->m[0][1];
    S[1] = A->m[0][0]*A->m[1][2] - A->m[1][0]*A->m[0][2];
    S[2] = A->m[0][0]*A->m[1][3] - A->m[1][0]*A->m[0][3];
    S[3] = A->m[0][1]*A->m[1][2] - A->m[1][1]*A->m[0][2];
    S[4] = A->m[0][1]*A->m[1][3] - A->m[1][1]*A->m[0][3];
    S[5] = A->m[0][2]*A->m[1][3] - A->m[1][2]*A->m[0][3];
    
    f32 C[6];
    C[0] = A->m[2][0]*A->m[3][1] - A->m[3][0]*A->m[2][1];
    C[1] = A->m[2][0]*A->m[3][2] - A->m[3][0]*A->m[2][2];
    C[2] = A->m[2][0]*A->m[3][3] - A->m[3][0]*A->m[2][3];
    C[3] = A->m[2][1]*A->m[3][2] - A->m[3][1]*A->m[2][2];
    C[4] = A->m[2][1]*A->m[3][3] - A->m[3][1]*A->m[2][3];
    C[5] = A->m[2][2]*A->m[3][3] - A->m[3][2]*A->m[2][3];
    
    f32 Idet = 1.0f/(S[0]*C[5] - S[1]*C[4] + S[2]*C[3] + S[3]*C[2] - S[4]*C[1] + S[5]*C[0]);
    
    Result->m[0][0] = ( A->m[1][1]*C[5] - A->m[1][2]*C[4] + A->m[1][3]*C[3])*Idet;
    Result->m[0][1] = (-A->m[0][1]*C[5] + A->m[0][2]*C[4] - A->m[0][3]*C[3])*Idet;
    Result->m[0][2] = ( A->m[3][1]*S[5] - A->m[3][2]*S[4] + A->m[3][3]*S[3])*Idet;
    Result->m[0][3] = (-A->m[2][1]*S[5] + A->m[2][2]*S[4] - A->m[2][3]*S[3])*Idet;
    
    Result->m[1][0] = (-A->m[1][0]*C[5] + A->m[1][2]*C[2] - A->m[1][3]*C[1])*Idet;
    Result->m[1][1] = ( A->m[0][0]*C[5] - A->m[0][2]*C[2] + A->m[0][3]*C[1])*Idet;
    Result->m[1][2] = (-A->m[3][0]*S[5] + A->m[3][2]*S[2] - A->m[3][3]*S[1])*Idet;
    Result->m[1][3] = ( A->m[2][0]*S[5] - A->m[2][2]*S[2] + A->m[2][3]*S[1])*Idet;
    
    Result->m[2][0] = ( A->m[1][0]*C[4] - A->m[1][1]*C[2] + A->m[1][3]*C[0])*Idet;
    Result->m[2][1] = (-A->m[0][0]*C[4] + A->m[0][1]*C[2] - A->m[0][3]*C[0])*Idet;
    Result->m[2][2] = ( A->m[3][0]*S[4] - A->m[3][1]*S[2] + A->m[3][3]*S[0])*Idet;
    Result->m[2][3] = (-A->m[2][0]*S[4] + A->m[2][1]*S[2] - A->m[2][3]*S[0])*Idet;
    
    Result->m[3][0] = (-A->m[1][0]*C[3] + A->m[1][1]*C[1] - A->m[1][2]*C[0])*Idet;
    Result->m[3][1] = ( A->m[0][0]*C[3] - A->m[0][1]*C[1] + A->m[0][2]*C[0])*Idet;
    Result->m[3][2] = (-A->m[3][0]*S[3] + A->m[3][1]*S[1] - A->m[3][2]*S[0])*Idet;
    Result->m[3][3] = ( A->m[2][0]*S[3] - A->m[2][1]*S[1] + A->m[2][2]*S[0])*Idet;
}

internal mat4
operator*(mat4 A, mat4 B)
{
    mat4 Result =
    {
        A.m[0][0] * B.m[0][0] + A.m[0][1] * B.m[1][0] + A.m[0][2] * B.m[2][0] + A.m[0][3] * B.m[3][0],
        A.m[0][0] * B.m[0][1] + A.m[0][1] * B.m[1][1] + A.m[0][2] * B.m[2][1] + A.m[0][3] * B.m[3][1],
        A.m[0][0] * B.m[0][2] + A.m[0][1] * B.m[1][2] + A.m[0][2] * B.m[2][2] + A.m[0][3] * B.m[3][2],
        A.m[0][0] * B.m[0][3] + A.m[0][1] * B.m[1][3] + A.m[0][2] * B.m[2][3] + A.m[0][3] * B.m[3][3],
        A.m[1][0] * B.m[0][0] + A.m[1][1] * B.m[1][0] + A.m[1][2] * B.m[2][0] + A.m[1][3] * B.m[3][0],
        A.m[1][0] * B.m[0][1] + A.m[1][1] * B.m[1][1] + A.m[1][2] * B.m[2][1] + A.m[1][3] * B.m[3][1],
        A.m[1][0] * B.m[0][2] + A.m[1][1] * B.m[1][2] + A.m[1][2] * B.m[2][2] + A.m[1][3] * B.m[3][2],
        A.m[1][0] * B.m[0][3] + A.m[1][1] * B.m[1][3] + A.m[1][2] * B.m[2][3] + A.m[1][3] * B.m[3][3],
        A.m[2][0] * B.m[0][0] + A.m[2][1] * B.m[1][0] + A.m[2][2] * B.m[2][0] + A.m[2][3] * B.m[3][0],
        A.m[2][0] * B.m[0][1] + A.m[2][1] * B.m[1][1] + A.m[2][2] * B.m[2][1] + A.m[2][3] * B.m[3][1],
        A.m[2][0] * B.m[0][2] + A.m[2][1] * B.m[1][2] + A.m[2][2] * B.m[2][2] + A.m[2][3] * B.m[3][2],
        A.m[2][0] * B.m[0][3] + A.m[2][1] * B.m[1][3] + A.m[2][2] * B.m[2][3] + A.m[2][3] * B.m[3][3],
        A.m[3][0] * B.m[0][0] + A.m[3][1] * B.m[1][0] + A.m[3][2] * B.m[2][0] + A.m[3][3] * B.m[3][0],
        A.m[3][0] * B.m[0][1] + A.m[3][1] * B.m[1][1] + A.m[3][2] * B.m[2][1] + A.m[3][3] * B.m[3][1],
        A.m[3][0] * B.m[0][2] + A.m[3][1] * B.m[1][2] + A.m[3][2] * B.m[2][2] + A.m[3][3] * B.m[3][2],
        A.m[3][0] * B.m[0][3] + A.m[3][1] * B.m[1][3] + A.m[3][2] * B.m[2][3] + A.m[3][3] * B.m[3][3],
    };
    
    return(Result);
}

internal v4
operator*(mat4 A, v4 B)
{
    v4 Result =
    {
        B.x*A.m[0][0] + B.y*A.m[0][1] + B.z*A.m[0][2] + B.w*A.m[0][3],
        B.x*A.m[1][0] + B.y*A.m[1][1] + B.z*A.m[1][2] + B.w*A.m[1][3],
        B.x*A.m[2][0] + B.y*A.m[2][1] + B.z*A.m[2][2] + B.w*A.m[2][3],
        B.x*A.m[3][0] + B.y*A.m[3][1] + B.z*A.m[3][2] + B.w*A.m[3][3]
    };
    
    return(Result);
}

internal void
TranslateMat4(mat4 *Result, v3 Translation)
{
    v4 First = Result->e[0] * Translation.x;
    v4 Second = Result->e[1] * Translation.y;
    v4 Third = Result->e[2] * Translation.z;
    
    v4 Final = First +Second;
    Final = Final + Third;
    Final = Final + Result->e[3];
    
    Result->e[3] = Final;
}

internal void
RotateMat4(mat4 *OutResult, f32 Angle, v3 RawAxis)
{
    f32 C = Cos(Angle);
    f32 S = Sin(Angle);
    
    v3 Axis = V3Normalize(RawAxis);
    v3 Temp = Axis * (1.0f - C);
    
    mat4 Rotate = {0};
    Rotate.m[0][0] = C + Temp.e[0]*Axis.e[0];
    Rotate.m[0][1] = Temp.e[0]*Axis.e[1] + S*Axis.e[2];
    Rotate.m[0][2] = Temp.e[0]*Axis.e[2] - S*Axis.e[1];
    
    Rotate.m[1][0] = Temp.e[1]*Axis.e[0] - S*Axis.e[2];
    Rotate.m[1][1] = C + Temp.e[1]*Axis.e[1];
    Rotate.m[1][2] = Temp.e[1]*Axis.e[2] + S*Axis.e[0];
    
    Rotate.m[2][0] = Temp.e[2]*Axis.e[0] + S*Axis.e[1];
    Rotate.m[2][1] = Temp.e[2]*Axis.e[1] - S*Axis.e[0];
    Rotate.m[2][2] = C + Temp.e[2]*Axis.e[2];
    
    v4 First, Second, Third;
    
    mat4 Result = {0};
    
    First = OutResult->e[0] * Rotate.m[0][0];
    Second = OutResult->e[1] * Rotate.m[0][1];
    Third = OutResult->e[2] * Rotate.m[0][2];
    Result.e[0] = First + Second + Third;
    
    First = OutResult->e[0] * Rotate.m[1][0];
    Second = OutResult->e[1] * Rotate.m[1][1];
    Third = OutResult->e[2] * Rotate.m[1][2];
    Result.e[1] = First + Second + Third;
    
    First = OutResult->e[0] * Rotate.m[2][0];
    Second = OutResult->e[1] * Rotate.m[2][1];
    Third = OutResult->e[2] * Rotate.m[2][2];
    Result.e[2] = First + Second + Third;
    
    Result.e[3] = OutResult->e[3];
    
    *OutResult = Result;
}

internal void
ScaleMat4(mat4 *Result, v3 Scale)
{
    Result->e[0] = Result->e[0] * Scale.x;
    Result->e[1] = Result->e[1] * Scale.y;
    Result->e[2] = Result->e[2] * Scale.z;
}

internal void
CreatePerspective(mat4 *Mat4, f32 Aspect, f32 FOV, f32 Near, f32 Far)
{
    Assert(Abs(Aspect - EPSILON) > 0.0f);
    
    f32 TanHalfFOV = Tan(FOV / 2.0f);
    
    mat4 Result = {0};
    Result.m[0][0] = 1.0f / (Aspect * TanHalfFOV);
    Result.m[1][1] = 1.0f / (TanHalfFOV);
    Result.m[2][2] = Far / (Far - Near);
    Result.m[2][3] = 1.0f;
    Result.m[3][2] = -(Far*Near) / (Far - Near);
    
    *Mat4 = Result;
}

internal void
CreateOrthographic(mat4 *Mat4,
                   f32 Left, f32 Right,
                   f32 Top, f32 Bottom,
                   f32 Near, f32 Far)
{
    mat4 Result = Mat4Identity();
    Result.m[0][0] = 2.0f / (Right - Left);
    Result.m[1][1] = 2.0f / (Top - Bottom);
    Result.m[2][2] = -2.0f / (Far - Near);
    Result.m[3][0] = -(Right + Left) / (Right - Left);
    Result.m[3][1] = -(Top + Bottom) / (Top - Bottom);
    Result.m[3][2] = -(Far + Near) / (Far - Near);
    
    *Mat4 = Result;
}

internal void
Mat4LookAt(mat4 *Result, v3 Eye, v3 Center, v3 Up)
{
    v3 F = V3Normalize(Center - Eye);
    v3 S = V3Normalize(V3Cross(F, Up));
    v3 U = V3Cross(S, F);
    
    *Result = Mat4Identity();
    Result->m[0][0] = S.x;
    Result->m[1][0] = S.y;
    Result->m[2][0] = S.z;
    Result->m[0][1] = U.x;
    Result->m[1][1] = U.y;
    Result->m[2][1] = U.z;
    Result->m[0][2] = -F.x;
    Result->m[1][2] = -F.y;
    Result->m[2][2] = -F.z;
    Result->m[3][0] = -V3MultiplyDot(S, Eye);
    Result->m[3][1] = -V3MultiplyDot(U, Eye);
    Result->m[3][2] = V3MultiplyDot(F, Eye);
}

#endif //HORIZONS_MATH_H
