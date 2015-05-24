/**
 * マテリアル管理用.
 */

#include "MaterialCtrl.h"
#include "Util.h"

//-------------------------------------------------------------------.
// RIS用のマテリアル情報.
//-------------------------------------------------------------------.

/**
 * RenderManでのマテリアル情報 (base).
 */
CPxrMaterialBase::CPxrMaterialBase ()
{
	name = "";
}

/**
 * RenderManでのマテリアル情報 (PxrDiffuse).
 */
CPxrMaterialDiffuse::CPxrMaterialDiffuse ()
{
	name = "PxrDiffuse";
	Clear();
}

void CPxrMaterialDiffuse::Clear ()
{
	diffuseColor      = sxsdk::rgb_class(0.5f, 0.5f, 0.5f);
	transmissionColor = sxsdk::rgb_class(0.0f, 0.0f, 0.0f);
	presence          = 1.0f;
}

/**
 * RenderManでのマテリアル情報 (PxrDisney).
 */
CPxrMaterialDisney::CPxrMaterialDisney ()
{
	name = "PxrDisney";
	Clear();
}

void CPxrMaterialDisney::Clear ()
{
	baseColor       = sxsdk::rgb_class(0.2f, 0.5f, 0.8f);
	emitColor       = sxsdk::rgb_class(0.0f, 0.0f, 0.0f);
	subsurface      = 0.0f;
	subsurfaceColor = sxsdk::rgb_class(0.0f, 0.0f, 0.0f);
	metallic        = 0.0f;
	specular        = 0.5f;
	specularTint    = 0.0f;
	roughness       = 0.25f;
	anisotropic     = 0.0f;
	sheen           = 0.0f;
	sheenTint       = 0.5f;
}

/**
 * RenderManでのマテリアル情報 (PxrGlass).
 */
CPxrMaterialGlass::CPxrMaterialGlass ()
{
	name = "PxrGlass";
	Clear();
}

void CPxrMaterialGlass::Clear ()
{
	ior                 = 1.5f;
	roughness           = 0.0f;
	reflectionColor     = sxsdk::rgb_class(0.5f, 0.5f, 0.5f);
	reflectionGain      = 1.0f;
	transmissionColor   = sxsdk::rgb_class(0.8f, 0.8f, 0.8f);
	transmissionGain    = 1.0f;
	absorptionGain      = 0.0f;
	absorptionColor     = sxsdk::rgb_class(1, 1, 1);
}

/**
 * RenderManでのマテリアル情報 (PxrConstant).
 */
CPxrMaterialConstant::CPxrMaterialConstant ()
{
	name = "PxrConstant";
	Clear();
}

void CPxrMaterialConstant::Clear ()
{
	emitColor = sxsdk::rgb_class(1, 1, 1);
}

/**
 * RenderManでのマテリアル情報 (PxrVolume).
 */
CPxrMaterialVolume::CPxrMaterialVolume ()
{
	name = "pxrVolume";
	Clear();
}

void CPxrMaterialVolume::Clear ()
{
	diffuseColor = sxsdk::rgb_class(1, 1, 1);
	emitColor    = sxsdk::rgb_class(0, 0, 0);
	densityColor = sxsdk::rgb_class(1, 1, 1);
	densityFloat = 1.0f;
	densityScale = 1.0f;
	anisotropy   = 0.0f;
	maxDensity   = 1.0f;
	multiScatter = false;
}

//--------------------------------------------------------.
CRISMaterialInfo::CRISMaterialInfo ()
{
	Clear();
}


/**
 * すべての情報をクリアする（初期値を入れる）.
 */
void CRISMaterialInfo::Clear ()
{
	useCustom = false;

	type = RIBParam::pxrDiffuse;
	pxrDiffuse.Clear();
	pxrDisney.Clear();
	pxrGlass.Clear();
	pxrConstant.Clear();
	pxrVolume.Clear();

	visibilityDiffuse  = true;
	visibilitySpecular = true;
	visibilityIndirect = true;
	visibilityCamera   = true;
	useDepth = false;
	maxdiffusedepth  = 5;
	maxspeculardepth = 10;
}

//-------------------------------------------------------------------.
/**
 * Shade 3Dでのマテリアルのマッピングレイヤごとの情報.
 */
CMaterialMappingLayerInfo::CMaterialMappingLayerInfo ()
{
	Clear();
}

void CMaterialMappingLayerInfo::Clear ()
{
	textureIndex = -1;
	repeatX = repeatY = 1;
	flipColor = false;
}

/**
 * Shade 3Dでのマテリアル情報.
 */
CMaterialInfo::CMaterialInfo ()
{
	Clear();
}

void CMaterialInfo::Clear ()
{
	name = "";
	masterSurface = NULL;
	pSurface = NULL;

	diffuseColor = sxsdk::rgb_class(1, 1, 1);
	diffuseValue = 1.0f;

	specularColor = sxsdk::rgb_class(1, 1, 1);
	specularValue = 0.5f;
	specularSize  = 0.5f;

	glowColor = sxsdk::rgb_class(1, 1, 1);
	glow      = 0.0f;

	transparent = 0.0f;
	reflection  = 0.0f;
	refraction  = 1.0f;
	roughness   = 0.0f;
	anisotropic = 0.0f;

	noShading = false;
	noShadow  = false;

	mappingLayerCount = 0;

	diffuseLayer.Clear();
	normalLayer.Clear();
	trimLayer.Clear();
}

/**
 * 指定の形状でのマテリアルを格納.
 */
void CMaterialInfo::SetMaterial (sxsdk::scene_interface* scene, sxsdk::shape_class& shape)
{
	Clear();
	if (!shape.has_surface()) return;

	name = std::string(shape.get_name()) + std::string("_material");

	masterSurface = shape.get_master_surface();
	if (masterSurface) name = std::string(masterSurface->get_name()) + std::string("_material");

	sxsdk::surface_class* surface = shape.get_surface();
	if (!surface) return;

	m_SetMaterial(scene, surface);
}

void CMaterialInfo::SetMaterial (sxsdk::scene_interface* scene, sxsdk::master_surface_class& masterSurface)
{
	Clear();
	this->masterSurface = &masterSurface;

	name = std::string(masterSurface.get_name()) + std::string("_material");

	sxsdk::surface_class* surface = masterSurface.get_surface();
	if (!surface) return;

	m_SetMaterial(scene, surface);
}

void CMaterialInfo::m_SetMaterial (sxsdk::scene_interface* scene, sxsdk::surface_class* surface)
{
	pSurface = surface;

	noShading = surface->get_no_shading();
	noShadow  = surface->get_do_not_cast_shadow();

	diffuseValue = surface->get_diffuse();
	diffuseColor = surface->get_diffuse_color();

	specularValue = surface->get_highlight();
	specularColor = surface->get_highlight_color();
	specularSize  = surface->get_highlight_size();

	transparent   = surface->get_transparency();
	reflection    = surface->get_reflection();
	refraction    = surface->get_refraction();

	roughness     = surface->get_roughness();
	anisotropic   = surface->get_anisotropic();

	glow          = surface->get_glow();
	glowColor     = surface->get_glow_color();

	transparentAlpha = false;

	mappingLayerCount = surface->get_number_of_mapping_layers();

	diffuseLayer.Clear();
	normalLayer.Clear();
	trimLayer.Clear();

	if (mappingLayerCount > 0) {
		// マッピングレイヤのテクスチャに対応するインデックスを取得.
		for (int i = 0; i < mappingLayerCount; i++) {
			sxsdk::mapping_layer_class& mappingLayer = surface->mapping_layer(i);
			if (mappingLayer.get_pattern() != sxsdk::enums::image_pattern) continue;
				
			compointer<sxsdk::image_interface> image(mappingLayer.get_image_interface());
			if (!image || !(image->has_image())) continue;
				
			if (diffuseLayer.textureIndex < 0 && mappingLayer.get_type() == sxsdk::enums::diffuse_mapping) {
				transparentAlpha          = (mappingLayer.get_channel_mix() == sxsdk::enums::mapping_transparent_alpha_mode);
				diffuseLayer.textureIndex = Util::GetMasterImageIndex(scene, image);
				diffuseLayer.repeatX      = mappingLayer.get_repetition_X();
				diffuseLayer.repeatY      = mappingLayer.get_repetition_Y();
				diffuseLayer.flipColor    = mappingLayer.get_flip_color();
			}

			if (normalLayer.textureIndex < 0 && (mappingLayer.get_type() == sxsdk::enums::normal_mapping || mappingLayer.get_type() == sxsdk::enums::bump_mapping)) {
				normalLayer.textureIndex = Util::GetMasterImageIndex(scene, image);
				normalLayer.repeatX      = mappingLayer.get_repetition_X();
				normalLayer.repeatY      = mappingLayer.get_repetition_Y();
				normalLayer.flipColor    = mappingLayer.get_flip_color();
			}

			if (normalLayer.textureIndex < 0 && mappingLayer.get_type() == sxsdk::enums::trim_mapping) {
				trimLayer.textureIndex = Util::GetMasterImageIndex(scene, image);
				trimLayer.repeatX      = mappingLayer.get_repetition_X();
				trimLayer.repeatY      = mappingLayer.get_repetition_Y();
				trimLayer.flipColor    = mappingLayer.get_flip_color();
			}
		}
	}
}

//-------------------------------------------------------------------.
// マテリアル情報を、 Shade 3D <==> RenderMan(RIS)でコンバート.
//-------------------------------------------------------------------.

/**
 * Shade 3Dでのマテリアル情報より、RIS向けにコンバート.
 */
void MaterialCtrl::ConvShade3DToRIS (CMaterialInfo& materialInfo, CRISMaterialInfo& risMaterialInfo)
{
	risMaterialInfo.Clear();

	sxsdk::rgb_class diffuseCol         = materialInfo.diffuseColor * materialInfo.diffuseValue;
	const sxsdk::rgb_class specularCol  = materialInfo.specularColor * materialInfo.specularValue;
	const sxsdk::rgb_class emmitCol     = materialInfo.glowColor * materialInfo.glow;
	const float roughnessVal            = materialInfo.roughness;

	if (materialInfo.transparent > 0.2f || materialInfo.reflection > 0.2f) {
		float maxV = diffuseCol.red;
		maxV = std::max(maxV, diffuseCol.green);
		maxV = std::max(maxV, diffuseCol.blue);
		if (!sx::zero(maxV)) {
			diffuseCol.red   /= maxV;
			diffuseCol.green /= maxV;
			diffuseCol.blue  /= maxV;
		}
	}

	risMaterialInfo.pxrConstant.emitColor      = diffuseCol;
	risMaterialInfo.pxrGlass.reflectionColor   = diffuseCol;
	risMaterialInfo.pxrGlass.transmissionColor = diffuseCol;
	risMaterialInfo.pxrDisney.baseColor        = diffuseCol;
	risMaterialInfo.pxrDiffuse.diffuseColor    = diffuseCol;
	risMaterialInfo.pxrVolume.diffuseColor     = diffuseCol;

	risMaterialInfo.pxrDisney.anisotropic = materialInfo.anisotropic;
	risMaterialInfo.pxrVolume.anisotropy  = materialInfo.anisotropic;

	const float fMin = 0.01;
	if (materialInfo.transparent > fMin) {
		//----------------------------------------------------------.
		// 透過がある場合.
		//----------------------------------------------------------.
		risMaterialInfo.type = RIBParam::pxrGlass;

		risMaterialInfo.pxrGlass.reflectionColor   = diffuseCol;
		risMaterialInfo.pxrGlass.transmissionColor = diffuseCol;
		risMaterialInfo.pxrGlass.roughness         = roughnessVal;
		risMaterialInfo.pxrGlass.ior               = materialInfo.refraction;

	} else if (materialInfo.specularValue > fMin && materialInfo.transparent < fMin) {
		//----------------------------------------------------------.
		// Specularありで透過なしの場合.
		//----------------------------------------------------------.
		risMaterialInfo.type = RIBParam::pxrDisney;

		risMaterialInfo.pxrDisney.baseColor     = diffuseCol;
		risMaterialInfo.pxrDisney.emitColor     = emmitCol;
		risMaterialInfo.pxrDisney.metallic      = materialInfo.reflection;
		risMaterialInfo.pxrDisney.specular      = materialInfo.specularValue;
		risMaterialInfo.pxrDisney.roughness     = materialInfo.roughness;
		risMaterialInfo.pxrDisney.anisotropic   = materialInfo.anisotropic;

	} else {
		//----------------------------------------------------------.
		// Specularなしで透過なしの場合.
		//----------------------------------------------------------.
		risMaterialInfo.type = RIBParam::pxrDiffuse;

		risMaterialInfo.pxrDiffuse.diffuseColor = diffuseCol;
	}
}

