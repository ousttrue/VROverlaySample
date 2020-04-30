#include "quad.h"
#include "compileshader.h"
#include <string>
#include <vector>
#include <DirectXMath.h>

const std::string g_hlsl =
#include "quad.hlsl"
    ;

static DXGI_FORMAT GetDxgiFormat(D3D10_REGISTER_COMPONENT_TYPE type, BYTE mask)
{
    if ((mask & 0x0F) == 0x0F)
    {
        // xyzw
        switch (type)
        {
        case D3D10_REGISTER_COMPONENT_FLOAT32:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
    }

    if ((mask & 0x07) == 0x07)
    {
        // xyz
        switch (type)
        {
        case D3D10_REGISTER_COMPONENT_FLOAT32:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        }
    }

    if ((mask & 0x03) == 0x03)
    {
        // xy
        switch (type)
        {
        case D3D10_REGISTER_COMPONENT_FLOAT32:
            return DXGI_FORMAT_R32G32_FLOAT;
        }
    }

    if ((mask & 0x1) == 0x1)
    {
        // x
        switch (type)
        {
        case D3D10_REGISTER_COMPONENT_FLOAT32:
            return DXGI_FORMAT_R32_FLOAT;
        }
    }

    return DXGI_FORMAT_UNKNOWN;
}

// input-assembler
struct Vertex
{
    DirectX::XMFLOAT2 pos;
    DirectX::XMFLOAT2 tex;
};

bool QuadRenderer::Initialize(const Microsoft::WRL::ComPtr<ID3D11Device> &device,
                              const Microsoft::WRL::ComPtr<ID3D11Texture2D> &dst,
                              float scalingFactor)
{
    if (FAILED(device->CreateRenderTargetView(dst.Get(), nullptr, &m_rtv)))
    {
        return false;
    }

    m_scalingFactor = scalingFactor;

    {
        D3D11_TEXTURE2D_DESC desc;
        dst->GetDesc(&desc);
        m_vp.Width = static_cast<float>(desc.Width) * m_scalingFactor;
        m_vp.Height = static_cast<float>(desc.Height) * m_scalingFactor;
        m_vp.MinDepth = 0.0f;
        m_vp.MaxDepth = 1.0f;
        m_vp.TopLeftX = 0;
        m_vp.TopLeftY = 0;
    }

    // vertex shader
    {
        auto vs = CompileShaderFromSource("SOURCE", g_hlsl.c_str(), static_cast<UINT>(g_hlsl.size()), "vsMain", "vs_5_0");
        if (!vs)
        {
            return false;
        }
        if (FAILED(device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &m_vs)))
        {
            return false;
        }

        // vertex shader reflection
        Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
        if (FAILED(D3DReflect(vs->GetBufferPointer(), vs->GetBufferSize(), IID_PPV_ARGS(&pReflector))))
        {
            return false;
        }

        D3D11_SHADER_DESC shaderdesc;
        pReflector->GetDesc(&shaderdesc);

        // Create InputLayout
        std::vector<D3D11_INPUT_ELEMENT_DESC> vbElement;
        for (UINT i = 0; i < shaderdesc.InputParameters; ++i)
        {
            D3D11_SIGNATURE_PARAMETER_DESC sigdesc;
            pReflector->GetInputParameterDesc(i, &sigdesc);

            auto format = GetDxgiFormat(sigdesc.ComponentType, sigdesc.Mask);

            D3D11_INPUT_ELEMENT_DESC eledesc = {
                sigdesc.SemanticName, sigdesc.SemanticIndex, format, 0 // hardcoding
                ,
                D3D11_APPEND_ALIGNED_ELEMENT // hardcoding
                ,
                D3D11_INPUT_PER_VERTEX_DATA // hardcoding
                ,
                0 // hardcoding
            };
            vbElement.push_back(eledesc);
        }

        if (!vbElement.empty())
        {
            if (FAILED(device->CreateInputLayout(&vbElement[0], static_cast<UINT>(vbElement.size()),
                                                 vs->GetBufferPointer(), vs->GetBufferSize(), &m_inputLayout)))
            {
                return false;
            }
        }
    }

    // pixel shader
    {
        auto ps = CompileShaderFromSource("SOURCE", g_hlsl.c_str(), static_cast<UINT>(g_hlsl.size()), "psMain", "ps_5_0");
        if (!ps)
        {
            return false;
        }
        if (FAILED(device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &m_ps)))
        {
            return false;
        }
    }

    // constants
    {
        D3D11_BUFFER_DESC desc = {0};
        desc.ByteWidth = sizeof(DirectX::XMFLOAT4);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = 0;
        if (FAILED(device->CreateBuffer(&desc, nullptr, &m_constant)))
        {
            return false;
        }
    }

    // vertices
    {
        auto size = 1.0f;
        Vertex pVertices[] =
            {
                {DirectX::XMFLOAT2(0, 0), DirectX::XMFLOAT2(0, 0)},
                {DirectX::XMFLOAT2(1, 0), DirectX::XMFLOAT2(1, 0)},
                {DirectX::XMFLOAT2(1, -1), DirectX::XMFLOAT2(1, 1)},
                {DirectX::XMFLOAT2(0, -1), DirectX::XMFLOAT2(0, 1)},
            };
        D3D11_BUFFER_DESC vdesc = {0};
        vdesc.ByteWidth = sizeof(pVertices);
        vdesc.Usage = D3D11_USAGE_DEFAULT;
        vdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vdesc.CPUAccessFlags = 0;
        D3D11_SUBRESOURCE_DATA vertexData = {0};
        vertexData.pSysMem = pVertices;
        if (FAILED(device->CreateBuffer(&vdesc, &vertexData, &m_vertices)))
        {
            return false;
        }
    }

    // indices
    {
        unsigned int pIndices[] =
            {
                0,
                1,
                2,
                2,
                3,
                0,
            };
        m_indexCount = _countof(pIndices);
        D3D11_BUFFER_DESC idesc = {0};
        idesc.ByteWidth = sizeof(pIndices);
        idesc.Usage = D3D11_USAGE_DEFAULT;
        idesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        idesc.CPUAccessFlags = 0;
        D3D11_SUBRESOURCE_DATA indexData;
        ZeroMemory(&indexData, sizeof(indexData));
        indexData.pSysMem = pIndices;
        if (FAILED(device->CreateBuffer(&idesc, &indexData, &m_indices)))
        {
            return false;
        }
    }

    // blend state
    {
        D3D11_BLEND_DESC desc = {0};
        desc.RenderTarget[0].BlendEnable = TRUE;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        if (FAILED(device->CreateBlendState(&desc, &m_blendState)))
        {
            return false;
        }
    }

    // sampler state
    {
        D3D11_SAMPLER_DESC desc = {0};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        if (FAILED(device->CreateSamplerState(&desc, &m_sampler)))
        {
            return false;
        }
    }

    return true;
}

void QuadRenderer::Render(const Microsoft::WRL::ComPtr<ID3D11DeviceContext> &context,
                          const Microsoft::WRL::ComPtr<ID3D11Texture2D> &texture, int x, int y)
{
#if 0
        // clear
        float clearColor[] = {0.2f, 0.2f, 0.4f, 1.0f};
        context->ClearRenderTargetView(m_rtv.Get(), clearColor);

#else
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    context->GetDevice(&device);

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    if (FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, &srv)))
    {
        return;
    }

    // Output-Merger stage
    context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), nullptr);

    // Rasterizer stage
    context->RSSetViewports(1, &m_vp);

    // shader
    context->VSSetShader(m_vs.Get(), NULL, 0);
    context->PSSetShader(m_ps.Get(), NULL, 0);

    // texture
    context->PSSetShaderResources(0, 1, srv.GetAddressOf());
    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

    // blend state
    float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    UINT sampleMask = 0xffffffff;
    context->OMSetBlendState(m_blendState.Get(), blendFactor, sampleMask);

    // update constant buffer
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);
    auto factor = m_scalingFactor * 2;
    DirectX::XMFLOAT4 offsetScale = {
        x / m_vp.Width * factor - 1,
        -y / m_vp.Height * factor + 1,
        desc.Width / m_vp.Width * factor,
        desc.Height / m_vp.Height * factor};
    context->UpdateSubresource(m_constant.Get(), 0, nullptr, &offsetScale, 0, 0);
    context->VSSetConstantBuffers(0, 1, m_constant.GetAddressOf());

    // mesh
    context->IASetInputLayout(m_inputLayout.Get());

    // set vertexbuffer
    ID3D11Buffer *pBufferTbl[] = {m_vertices.Get()};
    UINT SizeTbl[] = {sizeof(Vertex)};
    UINT OffsetTbl[] = {0};
    context->IASetVertexBuffers(0, 1, pBufferTbl, SizeTbl, OffsetTbl);
    // set indexbuffer
    context->IASetIndexBuffer(m_indices.Get(), DXGI_FORMAT_R32_UINT, 0);
    // draw
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(m_indexCount, 0, 0);
#endif

    context->Flush();
}
