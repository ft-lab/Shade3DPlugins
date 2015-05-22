/**
 * テクスチャ情報格納用.
 */
#ifndef _TEXTURECTRL_H
#define _TEXTURECTRL_H

#include "GlobalHeader.h"

class CTextureInfo
{
public:
	int index;						// master imageの番号.
	std::string fileName;			// ファイル名.

	bool isBumpMap;					// bump mapの場合.
	bool isNormalMap;				// normal mapの場合.
	bool isRealColor;				// hdrやIBLの場合 (この場合は、RenderManでのテクスチャとしてのリニア変換は不要).

	bool useTexture;				// テクスチャを使用しているか.
	bool useTransparentAlpha;		// アルファ透明を使用するか.

	/**
	 * 再帰的にシーンをたどってimageの画像が存在するかチェックし、バンプか法線マップか調べる.
	 */
	bool m_SearchImageFromShape (sxsdk::shape_class& shape, sxsdk::image_interface* image, bool& isBumpMap, bool& isNormalMap, bool& useTransparentAlpha, sxsdk::shape_class* ignoreShape = NULL);

	/**
	 * 再帰的にシーンをたどってimageの画像が使用されているかチェック.
	 */
	bool m_SearchUseImage (sxsdk::shape_class& shape, sxsdk::image_interface* image);

public:
	CTextureInfo ();

	void Clear ();

	/**
	 * 指定のmaster imageがバンプか法線マップか調べる.
	 */
	void CheckTexturePattern (sxsdk::scene_interface* scene, sxsdk::master_image_class& masterImage, bool& isBumpMap, bool& isNormalMap, bool& useTransparentAlpha);

	/**
	 * 指定のmaster imageが形状/表面材質で使用されているか.
	 */
	bool IsUseTexture (sxsdk::scene_interface* scene, sxsdk::master_image_class& masterImage);
};

#endif
