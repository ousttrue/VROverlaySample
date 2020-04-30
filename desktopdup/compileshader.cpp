#include "compileshader.h"

Microsoft::WRL::ComPtr<ID3D10Blob> CompileShaderFromSource(const CHAR *szFileName, const CHAR *source, int sourceSize,
                                                           LPCSTR szEntryPoint, LPCSTR szShaderModel)
{
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(DEBUG) || defined(_DEBUG)
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif //defiend(DEBUG) || defined(_DEBUG)

#if defined(NDEBUG) || defined(_NDEBUG)
    dwShaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif //defined(NDEBUG) || defined(_NDEBUG)

    Microsoft::WRL::ComPtr<ID3DBlob> ppBlobOut;
    Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompile(
        source,
        sourceSize,
        szFileName,
        NULL,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        szEntryPoint,
        szShaderModel,
        dwShaderFlags,
        0,
        &ppBlobOut,
        &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob != NULL)
        {
            OutputDebugStringA((char *)pErrorBlob->GetBufferPointer());
        }
        return nullptr;
    }

    return ppBlobOut;
}
