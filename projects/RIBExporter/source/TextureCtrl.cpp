/**
 * �e�N�X�`�����i�[�p.
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
 * �w���master image���o���v���@���}�b�v�����ׂ�.
 */
void CTextureInfo::CheckTexturePattern (sxsdk::scene_interface* scene, sxsdk::master_image_class& masterImage, bool& isBumpMap, bool& isNormalMap, bool& useTransparentAlpha)
{
	sxsdk::shape_class& rootShape = scene->get_shape();

	isBumpMap   = false;
	isNormalMap = false;
	useTransparentAlpha = false;

	// master surface�p�[�g���擾.
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

		// �`��̕\�ʍގ������ǂ�A�o���v���@���}�b�v���`�F�b�N (master surface part��������).
		if (pMasterSurfacePart) {
			if (m_SearchImageFromShape(*pMasterSurfacePart, image, isBumpMap, isNormalMap, useTransparentAlpha)) return;
		}

		// master surface part�ȊO�̕���������.
		m_SearchImageFromShape(rootShape, image, isBumpMap, isNormalMap, useTransparentAlpha, pMasterSurfacePart);

	} catch (...) { }
}

/**
 * �ċA�I�ɃV�[�������ǂ���image�̉摜�����݂��邩�`�F�b�N���A�o���v���@���}�b�v�����ׂ�.
 */
bool CTextureInfo::m_SearchImageFromShape (sxsdk::shape_class& shape, sxsdk::image_interface* image, bool& isBumpMap, bool& isNormalMap, bool& useTransparentAlpha, sxsdk::shape_class* ignoreShape)
{
	if (ignoreShape) {
		if (ignoreShape->get_handle() == shape.get_handle()) return false;
	}

	// �\�ʍގ����A�o���v���@���}�b�v�œ������̂������`�F�b�N.
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
					if (targetImage->has_real_color()) continue;		// 64bit�ȏ�̐F�����ꍇ�́A�@���}�b�v��o���v�Ƃ͔F�����Ȃ�.

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
 * �w���master image���`��/�\�ʍގ��Ŏg�p����Ă��邩.
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
 * �ċA�I�ɃV�[�������ǂ���image�̉摜���g�p����Ă��邩�`�F�b�N.
 */
bool CTextureInfo::m_SearchUseImage (sxsdk::shape_class& shape, sxsdk::image_interface* image)
{
	// �\�ʍގ��̃}�b�s���O���C���̃C���[�W�ŎQ�Ƃ���Ă��邩�`�F�b�N.
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
