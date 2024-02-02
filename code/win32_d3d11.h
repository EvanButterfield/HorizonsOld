#ifndef WIN32_D3D11_H
#define WIN32_D3D11_H

typedef struct win32_d3d11_state
{
    ID3D11Device *Device;
    ID3D11DeviceContext *Context;
    IDXGISwapChain1 *SwapChain;
    ID3D11RasterizerState *RasterizerState;
    ID3D11RasterizerState *NoCullRasterizerState;
    ID3D11DepthStencilState *DepthState;
    ID3D11BlendState *Blender;
    ID3D11SamplerState *Sampler;
    ID3D11RenderTargetView *RTView;
    ID3D11DepthStencilView *DSView;
    mat4 Projection, View;
} win32_d3d11_state;

typedef struct win32_d3d11_texture
{
    ID3D11ShaderResourceView *View;
} win32_d3d11_texture;

typedef struct win32_d3d11_mesh
{
    ID3D11Buffer *VBuffer;
    ID3D11Buffer *IBuffer;
    u32 IndexCount;
} win32_d3d11_mesh;

typedef struct win32_d3d11_shader
{
    ID3D11Buffer *UBuffer;
    ID3D11VertexShader *VShader;
    ID3D11PixelShader *PShader;
    ID3D11InputLayout *Layout;
} win32_d3d11_shader;

#endif //WIN32_D3D11_H
