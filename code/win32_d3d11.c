#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>

#include "win32_d3d11.h"

internal void
Win32InitD3D11(win32_d3d11_state *State, HWND Window)
{
    window_dimension WindowDimension = Win32GetWindowDimension(Window);
    
    State->RTView = 0;
    State->DSView = 0;
    
    D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    
    {
        UINT Flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(HORIZONS_INTERNAL)
        Flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        
        AssertHR(D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, Flags,
                                   FeatureLevels, ArrayCount(FeatureLevels), D3D11_SDK_VERSION,
                                   &State->Device, 0, &State->Context));
    }
    
#if defined(HORIZONS_INTERNAL)
    {
        ID3D11InfoQueue *Info;
        ID3D11Device_QueryInterface(State->Device, &IID_ID3D11InfoQueue, &Info);
        ID3D11InfoQueue_SetBreakOnSeverity(Info, D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
        ID3D11InfoQueue_SetBreakOnSeverity(Info, D3D11_MESSAGE_SEVERITY_ERROR, true);
        ID3D11InfoQueue_Release(Info);
    }
#endif
    
    {
        IDXGIFactory2 *Factory;
        AssertHR(CreateDXGIFactory(&IID_IDXGIFactory2, &Factory));
        
        DXGI_SWAP_CHAIN_DESC1 Desc = {0};
        Desc.Width = WindowDimension.Width;
        Desc.Height = WindowDimension.Height;
        Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        Desc.SampleDesc.Count = 2;
        Desc.SampleDesc.Quality = 0;
        Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        Desc.BufferCount = 2;
        Desc.Scaling = DXGI_SCALING_STRETCH;
        Desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        
        AssertHR(IDXGIFactory2_CreateSwapChainForHwnd(Factory, (IUnknown *)State->Device, Window, &Desc, 0, 0, &State->SwapChain));
        
        IDXGIFactory_MakeWindowAssociation(Factory, Window, DXGI_MWA_NO_ALT_ENTER);
        IDXGIFactory_Release(Factory);
    }
    
    {
        D3D11_SAMPLER_DESC Desc =
        {
            .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
            .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
            .ComparisonFunc = D3D11_COMPARISON_NEVER
        };
        
        ID3D11Device_CreateSamplerState(State->Device, &Desc, &State->Sampler);
    }
    
    {
        D3D11_BLEND_DESC Desc =
        {
            .RenderTarget[0] =
            {
                .BlendEnable = true,
                .SrcBlend = D3D11_BLEND_SRC_ALPHA,
                .DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
                .BlendOp = D3D11_BLEND_OP_ADD,
                .SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA,
                .DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA,
                .BlendOpAlpha = D3D11_BLEND_OP_ADD,
                .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL
            }
        };
        
        ID3D11Device_CreateBlendState(State->Device, &Desc, &State->Blender);
    }
    
    {
        D3D11_RASTERIZER_DESC Desc =
        {
            .FillMode = D3D11_FILL_SOLID,
            .CullMode = D3D11_CULL_BACK,
            .MultisampleEnable = true
        };
        
        ID3D11Device_CreateRasterizerState(State->Device, &Desc, &State->RasterizerState);
    }
    
    {
        D3D11_RASTERIZER_DESC Desc =
        {
            .FillMode = D3D11_FILL_SOLID,
            .CullMode = D3D11_CULL_NONE
        };
        
        ID3D11Device_CreateRasterizerState(State->Device, &Desc, &State->NoCullRasterizerState);
    }
    
    {
        D3D11_DEPTH_STENCIL_DESC Desc =
        {
            .DepthEnable = true,
            .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
            .DepthFunc = D3D11_COMPARISON_LESS
        };
        
        ID3D11Device_CreateDepthStencilState(State->Device, &Desc, &State->DepthState);
    }
}

internal void
Win32D3D11BeginFrame(win32_d3d11_state *State,
                     window_dimension WindowDimension, b32 WindowResized)
{
    if(!State->RTView || WindowResized)
    {
        if(State->RTView)
        {
            ID3D11DeviceContext_ClearState(State->Context);
            ID3D11RenderTargetView_Release(State->RTView);
            ID3D11DepthStencilView_Release(State->DSView);
            State->RTView = 0;
        }
        
        if(WindowDimension.Width != 0 && WindowDimension.Height != 0)
        {
            AssertHR(IDXGISwapChain1_ResizeBuffers(State->SwapChain, 0, WindowDimension.Width, WindowDimension.Height, DXGI_FORMAT_UNKNOWN, 0));
            
            ID3D11Texture2D *Backbuffer;
            IDXGISwapChain1_GetBuffer(State->SwapChain, 0, &IID_ID3D11Texture2D, &Backbuffer);
            ID3D11Device_CreateRenderTargetView(State->Device, (ID3D11Resource *)Backbuffer, 0, &State->RTView);
            
            D3D11_TEXTURE2D_DESC DepthDesc;
            ID3D11Texture2D_GetDesc(Backbuffer, &DepthDesc);
            DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            DepthDesc.SampleDesc.Count = 2;
            
            ID3D11Texture2D *Depth;
            ID3D11Device_CreateTexture2D(State->Device, &DepthDesc, 0, &Depth);
            ID3D11Device_CreateDepthStencilView(State->Device, (ID3D11Resource *)Depth, 0, &State->DSView);
            ID3D11Texture2D_Release(Depth);
            ID3D11Texture2D_Release(Backbuffer);
        }
    }
    
    if(State->RTView)
    {
        D3D11_VIEWPORT Viewport =
        {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width = (FLOAT)WindowDimension.Width,
            .Height = (FLOAT)WindowDimension.Height,
            .MinDepth = 0,
            .MaxDepth = 1
        };
        
        // TODO(evan): Move this to the game code
        FLOAT Color[] = { 0.392f, 0.584f, 0.929f, 1.0f };
        ID3D11DeviceContext_ClearRenderTargetView(State->Context, State->RTView, Color);
        ID3D11DeviceContext_ClearDepthStencilView(State->Context, State->DSView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        
        ID3D11DeviceContext_IASetPrimitiveTopology(State->Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ID3D11DeviceContext_RSSetViewports(State->Context, 1, &Viewport);
        ID3D11DeviceContext_PSSetSamplers(State->Context, 0, 1, &State->Sampler);
        ID3D11DeviceContext_OMSetBlendState(State->Context, State->Blender, 0, ~0U);
        ID3D11DeviceContext_OMSetRenderTargets(State->Context, 1, &State->RTView, State->DSView);
    }
}

internal void
Win32D3D11RenderMesh(win32_d3d11_state *State, window_dimension WindowDimension,
                     win32_d3d11_mesh *Mesh, win32_d3d11_shader *Shader, win32_d3d11_texture *Texture,
                     shader_constants *Constants, b32 Is2D)
{
    if(State->RTView)
    {
        memory_index MapSize;
        UINT Stride;
        if(Is2D)
        {
            ID3D11DeviceContext_RSSetState(State->Context, State->NoCullRasterizerState);
            ID3D11DeviceContext_OMSetDepthStencilState(State->Context, State->DepthState, 0);
            MapSize = sizeof(ui_shader_constants);
            Stride = sizeof(ui_vertex);
        }
        else
        {
            ID3D11DeviceContext_RSSetState(State->Context, State->RasterizerState);
            ID3D11DeviceContext_OMSetDepthStencilState(State->Context, State->DepthState, 0);
            MapSize = sizeof(shader_constants);
            Stride = sizeof(vertex);
        }
        
        D3D11_MAPPED_SUBRESOURCE Mapped;
        ID3D11DeviceContext_Map(State->Context, (ID3D11Resource *)Shader->UBuffer,
                                0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
        memcpy(Mapped.pData, Constants, MapSize);
        ID3D11DeviceContext_Unmap(State->Context, (ID3D11Resource *)Shader->UBuffer, 0);
        
        ID3D11DeviceContext_IASetInputLayout(State->Context, Shader->Layout);
        UINT Offset = 0;
        ID3D11DeviceContext_IASetVertexBuffers(State->Context, 0, 1, &Mesh->VBuffer, &Stride, &Offset);
        ID3D11DeviceContext_IASetIndexBuffer(State->Context, Mesh->IBuffer, DXGI_FORMAT_R32_UINT, 0);
        
        ID3D11DeviceContext_VSSetConstantBuffers(State->Context, 0, 1, &Shader->UBuffer);
        ID3D11DeviceContext_VSSetShader(State->Context, Shader->VShader, 0, 0);
        
        ID3D11DeviceContext_PSSetShaderResources(State->Context, 0, 1, &Texture->View);
        ID3D11DeviceContext_PSSetShader(State->Context, Shader->PShader, 0, 0);
        
        ID3D11DeviceContext_DrawIndexed(State->Context, Mesh->IndexCount, 0, 0);
    }
}

internal void
Win32D3D11EndFrame(IDXGISwapChain1 *SwapChain, b32 VSync)
{
    HRESULT Result = IDXGISwapChain1_Present(SwapChain, VSync, 0);
    if(Result == DXGI_STATUS_OCCLUDED)
    {
        if(VSync)
        {
            Sleep(10);
        }
    }
    else
    {
        AssertHR(Result);
    }
}

internal void
Win32D3D11CreateTexture(win32_d3d11_state *State, win32_d3d11_texture *Texture,
                        u32 *Data, u32 Width, u32 Height)
{
    D3D11_TEXTURE2D_DESC Desc =
    {
        .Width = Width,
        .Height = Height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc.Count = 1,
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE
    };
    
    D3D11_SUBRESOURCE_DATA ResourceData = 
    {
        .pSysMem = Data,
        .SysMemPitch = Width * sizeof(u32)
    };
    
    ID3D11Texture2D *Tex;
    ID3D11Device_CreateTexture2D(State->Device, &Desc, &ResourceData, &Tex);
    ID3D11Device_CreateShaderResourceView(State->Device, (ID3D11Resource *)Tex, 0, &Texture->View);
    ID3D11Texture2D_Release(Tex);
}

internal void
Win32D3D11CreateMesh(win32_d3d11_state *State, win32_d3d11_mesh *Mesh,
                     vertex *VertexData, s32 VertexCount,
                     u32 *IndexData, s32 IndexCount)
{
    Mesh->IndexCount = IndexCount;
    
    {
        D3D11_BUFFER_DESC Desc =
        {
            .ByteWidth = sizeof(vertex) * VertexCount,
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_VERTEX_BUFFER
        };
        
        D3D11_SUBRESOURCE_DATA Initial = { .pSysMem = VertexData };
        ID3D11Device_CreateBuffer(State->Device, &Desc, &Initial, &Mesh->VBuffer);
    }
    
    {
        D3D11_BUFFER_DESC Desc =
        {
            .ByteWidth = sizeof(u32) * IndexCount,
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_INDEX_BUFFER
        };
        
        D3D11_SUBRESOURCE_DATA Initial = { .pSysMem = IndexData };
        ID3D11Device_CreateBuffer(State->Device, &Desc, &Initial, &Mesh->IBuffer);
    }
}

internal void
Win32D3D11CreateShader(win32_d3d11_state *State, win32_d3d11_shader *Shader, string16 *ShaderPath, b32 Is2D)
{
    {
        UINT Flags = (D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR|
                      D3DCOMPILE_ENABLE_STRICTNESS|
                      D3DCOMPILE_WARNINGS_ARE_ERRORS);
#if defined(HORIZONS_INTERNAL)
        Flags |= (D3DCOMPILE_DEBUG|
                  D3DCOMPILE_SKIP_OPTIMIZATION);
#else
        Flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
        
        ID3DBlob *Error;
        
        ID3DBlob *VBlob;
        HRESULT Result = D3DCompileFromFile(ShaderPath, 0, 0,
                                            "Vertex", "vs_5_0", Flags,
                                            0, &VBlob, &Error);
        if(FAILED(Result))
        {
            const char *Message = ID3D10Blob_GetBufferPointer(Error);
            OutputDebugStringA(Message);
            Assert(0);
        }
        
        ID3DBlob *PBlob;
        Result = D3DCompileFromFile(ShaderPath, 0, 0,
                                    "Pixel", "ps_5_0", Flags,
                                    0, &PBlob, &Error);
        if(FAILED(Result))
        {
            const char *Message = ID3D10Blob_GetBufferPointer(Error);
            OutputDebugStringA(Message);
            Assert(0);
        }
        
        ID3D11Device_CreateVertexShader(State->Device, ID3D10Blob_GetBufferPointer(VBlob),
                                        ID3D10Blob_GetBufferSize(VBlob),
                                        0, &Shader->VShader);
        ID3D11Device_CreatePixelShader(State->Device, ID3D10Blob_GetBufferPointer(PBlob),
                                       ID3D10Blob_GetBufferSize(PBlob),
                                       0, &Shader->PShader);
        
        if(Is2D)
        {
            D3D11_INPUT_ELEMENT_DESC Desc[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(ui_vertex, Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "UV",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(ui_vertex, UV),       D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };
            
            ID3D11Device_CreateInputLayout(State->Device, Desc, ArrayCount(Desc),
                                           ID3D10Blob_GetBufferPointer(VBlob),
                                           ID3D10Blob_GetBufferSize(VBlob),
                                           &Shader->Layout);
        }
        else
        {
            D3D11_INPUT_ELEMENT_DESC Desc[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(vertex, Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(vertex, UV),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(vertex, Normal),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(vertex, Color),    D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };
            
            ID3D11Device_CreateInputLayout(State->Device, Desc, ArrayCount(Desc),
                                           ID3D10Blob_GetBufferPointer(VBlob),
                                           ID3D10Blob_GetBufferSize(VBlob),
                                           &Shader->Layout);
        }
        
        ID3D10Blob_Release(PBlob);
        ID3D10Blob_Release(VBlob);
    }
    
    {
        D3D11_BUFFER_DESC Desc =
        {
            .ByteWidth = (UINT)AlignTo(sizeof(shader_constants), 16),
            .Usage = D3D11_USAGE_DYNAMIC,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
        };
        ID3D11Device_CreateBuffer(State->Device, &Desc, 0, &Shader->UBuffer);
    }
}

internal void
ReleaseMesh(win32_d3d11_mesh *Mesh)
{
    ID3D11Buffer_Release(Mesh->VBuffer);
    ID3D11Buffer_Release(Mesh->IBuffer);
}