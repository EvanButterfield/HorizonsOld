struct material
{
    float4 BaseColor;
};

struct dir_light
{
    float3 Direction;
    float Padding1;
    
    float3 Ambient;
    float Padding2;
    float3 Diffuse;
    float Padding3;
};

float3 CalcDirLight(dir_light Light, float3 Normal, float3 ViewDirection)
{
    float3 LightDirection = normalize(-Light.Direction);
    
    float Diff = max(dot(Normal, LightDirection), 0.0);
    
    float3 Ambient = Light.Ambient;
    float3 Diffuse = Light.Diffuse*Diff;
    return(Ambient + Diffuse);
}

struct point_light
{
    float3 Position;
    float Padding1;
    
    float Constant;
    float Linear;
    float Quadratic;
    float Padding2;
    
    float3 Ambient;
    float Padding3;
    float3 Diffuse;
    float Padding4;
};

float3 CalcPointLight(point_light Light, float3 Normal, float3 FragPosition, float3 ViewDirection)
{
    float3 LightDirection = normalize(Light.Position - FragPosition);
    
    float Diff = max(dot(Normal, LightDirection), 0.0);
    
    float Distance = length(Light.Position - FragPosition);
    float Attenuation = 1.0 / (Light.Constant + Light.Linear*Distance + Light.Quadratic*Distance*Distance);
    
    float3 Ambient = Light.Ambient*Attenuation;
    float3 Diffuse = Light.Diffuse*Attenuation;
    return(Ambient + Diffuse);
}

///
///
///

struct VS_INPUT
{
    float3 Position : POSITION;
    float2 UV       : TEXCOORD;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD;
    float3 FragPosition : FRAG_POS;
    float3 CameraPosition : CAM_POS;
    float3 Normal : NORMAL;
    
    material Material : MATERIAL;
    
    dir_light DirLight : DIR_LIGHT;
    point_light PointLight : POINT_LIGHT;
};

cbuffer CBuffer0 : register(b0)
{
    material Material;
    dir_light DirLight;
    point_light PointLight;
    
    float3 CameraPosition;
    
    float Time;
    
    row_major float4x4 M;
    row_major float4x4 TransformMat;
    row_major float3x3 NormalMat;
    float3 Padding2;
}

sampler Sampler0 : register(s0);
Texture2D<float4> Texture0 : register(t0);

PS_INPUT Vertex(VS_INPUT Input)
{
    PS_INPUT Output;
    
    Output.Pos = mul(float4(Input.Position, 1.0), M);
    float4 FragPosition4 = mul(float4(Input.Position, 1.0), TransformMat);
    Output.FragPosition = FragPosition4.xyz;
    Output.UV = Input.UV;
    Output.CameraPosition = CameraPosition;
    Output.Normal = normalize(mul(Input.Normal, NormalMat));
    Output.Color = Input.Color;
    Output.Material = Material;
    Output.DirLight = DirLight;
    Output.PointLight = PointLight;
    
    return(Output);
}

float4 Pixel(PS_INPUT Input) : SV_TARGET
{
    float4 Tex = Texture0.Sample(Sampler0, Input.UV);
    
    float3 ViewDirection = normalize(Input.CameraPosition - Input.FragPosition);
    
    float4 Color;
    Color = float4(CalcDirLight(Input.DirLight, Input.Normal, ViewDirection), 1.0);
    Color += float4(CalcPointLight(Input.PointLight, Input.Normal, Input.FragPosition, ViewDirection), 0.0);
    
    Color *= Input.Material.BaseColor;
    Color *= Input.Color;
    Color *= Tex;
    
    return(Color);
}