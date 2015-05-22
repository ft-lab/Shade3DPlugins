/**
 * テクスチャ情報格納用.
 */

#include "TextureCtrl.h"

CTextureInfo::CTextureInfo ()
{
	Clear();
}

void CTextureInfo::Clear ()
{
	index       = -1;
	fileName    = "";
	isBumpMap   = false;
	isNormalMap = false;
	isRealColor = false;
	useTexture  = false;
	useTransparentAlpha = false;
}

/**
 * 指定のmaster imageがバンプか法線マップか調べる.
 */
void CTextureInfo::CheckTexturePattern (sxsdk::scene_interface* scene, sxsdk::master_image_class& masterImage, bool& isBumpMap, bool& isNormalMap, bool& useTransparentAlpha)
{
	sxsdk::shape_class& rootShape = scene->get_shape();

	isBumpMap   = false;
	isNormalMap = false;
	useTransparentAlpha = false;

	// master surfaceパートを取得.
	sxsdk::shape_class* pMasterSurfacePart = NULL;
	if (rootShape.has_son()) {
		sxsdk::shape_class* pS = rootShape.get_son();
		while (pS) {
			if (!pS->has_bro()) break;
			pS = pS->get_bro();
			if (!pS) break;
			if (pS->get_type() == sxsdk::enums::part && pS->get_part().get_part_type() == sxsdk::enums::master_surface_part) {
				pMasterSurfacePart = pS;
				break;
			}
		}
	}


	try {
		compointer<sxsdk::image_interface> image(masterImage.get_image());
		if (!image || !(image->has_image())) return;

		// 形状の表面材質をたどり、バンプか法線マップかチェック (master surface part内を検索).
		if (pMasterSurfacePart) {
			if (m_SearchImageFromShape(*pMasterSurfacePart, image, isBumpMap, isNormalMap, useTransparentAlpha)) return;
		}

		// master surface part以外の部分を検索.
		m_SearchImageFromShape(rootShape, image, isBumpMap, isNormalMap, useTransparentAlpha, pMasterSurfacePart);

	} catch (...) { }
}

/**
 * 再帰的にシーンをたどってimageの画像が存在するかチェックし、バンプか法線マップか調べる.
 */
bool CTextureInfo::m_SearchImageFromShape (sxsdk::shape_class& shape, sxsdk::image_interface* image, bool& isBumpMap, bool& isNormalMap, bool& useTransparentAlpha, sxsdk::shape_class* ignoreShape)
{
	if (ignoreShape) {
		if (ignoreShape->get_handle() == shape.get_handle()) return false;
	}

	// 表面材質より、バンプか法線マップで同じものを持つかチェック.
	if (shape.get_has_surface_attributes()) {
		sxsdk::surface_class* surface = shape.get_surface();
		if (surface) {
			const int LCou = surface->get_number_of_mapping_layers();
			for (int i = 0; i < LCou; i++) {
				sxsdk::mapping_layer_class& mappingLayer = surface->mapping_layer(i);
				if (mappingLayer.get_pattern() != sxsdk::enums::image_pattern) continue;

				try {
					compointer<sxsdk::image_interface> targetImage(mappingLayer.get_image_interface());
					if (!targetImage || !(targetImage->has_image())) continue;
					if (targetImage->has_real_color()) continue;		// 64bit以上の色を持つ場合は、法線マップやバンプとは認識しない.

					if (targetImage->is_same_as(image)) {
						if (mappingLayer.get_type() == sxsdk::enums::normal_mapping) isNormalMap = true;
						else if (mappingLayer.get_type() == sxsdk::enums::bump_mapping) isBumpMap = true;

						useTransparentAlpha = (mappingLayer.get_channel_mix() == sxsdk::enums::mapping_transparent_alpha_mode);
						return true;
					}
				} catch (...) { }
			}
		}
	}

	if (shape.has_son()) {
		sxsdk::shape_class* pS = shape.get_son();
		while (pS) {
			if (!pS->has_bro()) break;
			pS = pS->get_bro();
			if (!pS) break;

			if (m_SearchImageFromShape(*pS, image, isBumpMap, isNormalMap, useTransparentAlpha, ignoreShape)) return true;
		}
	}

	return false;
}

/**
 * 指定のmaster imageが形状/表面材質で使用されているか.
 */
bool CTextureInfo::IsUseTexture (sxsdk::scene_interface* scene, sxsdk::master_image_class& masterImage)
{
	sxsdk::shape_class& rootShape = scene->get_shape();

	try {
		compointer<sxsdk::image_interface> image(masterImage.get_image());
		if (!image || !(image->has_image())) return false;

		return m_SearchUseImage(rootShape, image);

	} catch (...) { }

	return false;
}

/**
 * 再帰的にシーンをたどってimageの画像が使用されているかチェック.
 */
bool CTextureInfo::m_SearchUseImage (sxsdk::shape_class& shape, sxsdk::image_interface* image)
{
	// 表面材質のマッピングレイヤのイメージで参照されているかチェック.
	if (shape.get_has_surface_attributes()) {
		sxsdk::surface_class* surface = shape.get_surface();
		if (surface) {
			const int LCou = surface->get_number_of_mapping_layers();
			for (int i = 0; i < LCou; i++) {
				sxsdk::mapping_layer_class& mappingLayer = surface->mapping_layer(i);
				if (mappingLayer.get_pattern() != sxsdk::enums::image_pattern) continue;

				try {
					compointer<sxsdk::image_interface> targetImage(mappingLayer.get_image_interface());
					if (!targetImage || !(targetImage->has_image())) continue;
					if (targetImage->is_same_as(image)) {
						return true;
					}
				} catch (...) { }
			}
		}
	}

	if (shape.has_son()) {
		sxsdk::shape_class* pS = shape.get_son();
		while (pS) {
			if (!pS->has_bro()) break;
			pS = pS->get_bro();
			if (!pS) break;

			if (m_SearchUseImage(*pS, image)) return true;
		}
	}
	return false;
}
