# VROverlaySample
openvr samples

## vrorigin
* overlay で原点に十字を表示する
* vr::TextureType_DirectX で ID3D11Texture2D を OpenVR に渡す
* 表示条件が不明で、毎フレームSetOverlayTextureをコールしている

### TODO:

* [ ] 一枚の画像で良さそう

## desktopdup
* Desktopを表示する
* vr::TextureType_DXGISharedHandle で ID3D11Texture2D の共有ハンドルを OpenVR に渡す
* 表示条件が、毎フレームソースになるテクスチャの更新ぽい？

### TODO:

* [ ] カーソル合成のアルファブレンド
