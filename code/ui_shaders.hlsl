struct vs_input
{
    float2 Position : POSITION;
    float2 UV       : UV;
};

struct ps_input
{
    float4 Position : SV_POSITION;
    float2 UV       : UV;
    float4 Color    : COLOR;
};

cbuffer CBuffer0 : register(b0)
{
    float4 Color;
    row_major float4x4 M;
    float Z;
}

sampler Sampler0 : register(s0);
Texture2D<float4> Texture0 : register(t0);

ps_input Vertex(vs_input Input)
{
    ps_input Output;
    
    Output.Position = mul(float4(Input.Position, 0.0, 1.0), M);
    Output.Position.z = Z;
    Output.UV = Input.UV;
    Output.Color = Color;
    
    return(Output);
}

float4 Pixel(ps_input Input) : SV_TARGET
{
    float4 Texture = Texture0.Sample(Sampler0, Input.UV);
    
    return(Input.Color*Texture);
}