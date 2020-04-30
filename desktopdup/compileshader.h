#pragma once
#include <d3dcompiler.h>
#include <wrl/client.h>

Microsoft::WRL::ComPtr<ID3D10Blob> CompileShaderFromSource(const char *szFileName, const char *source, int sourceSize,
                                                           const char *szEntryPoint, const char *szShaderModel);
