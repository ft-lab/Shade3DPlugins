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

CPxrMaterialBase::~CPxrMaterialBase ()
{
}

CPxrMaterialBase::CPxrMaterialBase (const CPxrMaterialBase& v)
{
	this->name = v.name;
}

/**
 * RenderManでのマテリアル情報 (PxrDiffuse).
 */
CPxrMaterialDiffuse::CPxrMaterialDiffuse ()
{
	name = "PxrDiffuse";
	Clear();
}

CPxrMaterialDiffuse::~CPxrMaterialDiffuse ()
{
}

CPxrMaterialDiffuse::CPxrMaterialDiffuse (const CPxrMaterialDiffuse& v)
{
	this->name = v.name;
	this->diffuseColor = v.diffuseColor;
	this->transmissionColor = v.transmissionColor;
	this->presence = v.presence;
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
CPxrMaterialDisney::~CPxrMaterialDisney ()
{
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

CPxrMaterialDisney::CPxrMaterialDisney (const CPxrMaterialDisney& v)
{
	this->name = v.name;
	this->baseColor = v.baseColor;
	this->emitColor = v.emitColor;
	this->subsurface = v.subsurface;
	this->subsurfaceColor = v.subsurfaceColor;
	this->metallic = v.metallic;
	this->specular = v.specular;
	this->specularTint = v.specularTint;
	this->roughness = v.roughness;
	this->anisotropic = v.anisotropic;
	this->sheen = v.sheen;
	this->sheenTint = v.sheenTint;
}

/**
 * RenderManでのマテリアル情報 (PxrGlass).
 */
CPxrMaterialGlass::CPxrMaterialGlass ()
{
	name = "PxrGlass";
	Clear();
}
CPxrMaterialGlass::~CPxrMaterialGlass ()
{
}

CPxrMaterialGlass::CPxrMaterialGlass (const CPxrMaterialGlass& v)
{
	this->name = v.name;
	this->ior = v.ior;
	this->roughness = v.roughness;
	this->reflectionColor = v.reflectionColor;
	this->reflectionGain = v.reflectionGain;
	this->transmissionColor = v.transmissionColor;
	this->transmissionGain = v.transmissionGain;
	this->absorptionGain = v.absorptionGain;
	this->absorptionColor = v.absorptionColor;
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
CPxrMaterialConstant::~CPxrMaterialConstant ()
{
}
CPxrMaterialConstant::CPxrMaterialConstant (const CPxrMaterialConstant& v)
{
	this->emitColor = v.emitColor;
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
	densityFloat = 0.05f;
	densityScale = 0.05f;
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
	flipColor   = false;
	patternType = sxsdk::enums::no_pattern;

	color    = sxsdk::rgb_class(0, 0, 0);
	weight   = 1.0f;
	softness = 1.0f;
	size     = 1.0f;
	normalMap = false;
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
	volumeRendering = false;

	mappingLayerCount = 0;

	diffuseLayer.clear();
	normalLayer.clear();
	trimLayer.clear();
	volumeDistanceLayer.clear();

	ribDiffusePatternName = "";
	ribNormalPatternName  = "";
	ribTrimPatternName    = "";
	ribVolumeDistancePatternName = "";
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
	if (masterSurface) name = std::string(masterSurface->get_name());

	sxsdk::surface_class* surface = shape.get_surface();
	if (!surface) return;

	m_SetMaterial(scene, surface);
}

void CMaterialInfo::SetMaterial (sxsdk::scene_interface* scene, sxsdk::master_surface_class& masterSurface)
{
	Clear();
	this->masterSurface = &masterSurface;

	name = std::string(masterSurface.get_name());

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
	volumeRendering = (surface->get_volume_type() == sxsdk::enums::volume_method);

	mappingLayerCount = surface->get_number_of_mapping_layers();

	diffuseLayer.clear();
	normalLayer.clear();
	trimLayer.clear();

	if (mappingLayerCount > 0) {
		bool chkTransparentAlpha = false;
		// マッピングレイヤのテクスチャに対応するインデックスを取得.
		for (int i = 0; i < mappingLayerCount; i++) {
			sxsdk::mapping_layer_class& mappingLayer = surface->mapping_layer(i);
			const sxsdk::enums::pattern_type pattern = (sxsdk::enums::pattern_type)mappingLayer.get_pattern();
			const sxsdk::enums::mapping_type type    = (sxsdk::enums::mapping_type)mappingLayer.get_type();

			// procedural textureの場合.
			// サポートはスポット/雲.
			if (pattern == sxsdk::enums::spotted_pattern || pattern == sxsdk::enums::cloud_pattern) {
				CMaterialMappingLayerInfo* layerInfo = NULL;
				if (type == sxsdk::enums::diffuse_mapping) {
					diffuseLayer.push_back(CMaterialMappingLayerInfo());
					layerInfo = &(diffuseLayer.back());
				} else if (type == sxsdk::enums::normal_mapping || type == sxsdk::enums::bump_mapping) {
					normalLayer.push_back(CMaterialMappingLayerInfo());
					layerInfo = &(normalLayer.back());
				} else if (type == sxsdk::enums::trim_mapping) {
					trimLayer.push_back(CMaterialMappingLayerInfo());
					layerInfo = &(trimLayer.back());
				} else if (type == sxsdk::enums::volume_distance_mapping) {
					volumeDistanceLayer.push_back(CMaterialMappingLayerInfo());
					layerInfo = &(volumeDistanceLayer.back());
				}
				if (layerInfo) {
					layerInfo->patternType  = pattern;
					layerInfo->color        = mappingLayer.get_mapping_color();
					layerInfo->weight       = mappingLayer.get_weight();
					layerInfo->softness     = mappingLayer.get_softness();
					layerInfo->size         = mappingLayer.get_mapping_size();
					layerInfo->flipColor    = mappingLayer.get_flip_color();
					layerInfo->normalMap    = (type == sxsdk::enums::normal_mapping);
				}
				continue;
			}

			if (pattern != sxsdk::enums::image_pattern) continue;
			compointer<sxsdk::image_interface> image(mappingLayer.get_image_interface());
			if (!image || !(image->has_image())) continue;
			
			if (type == sxsdk::enums::diffuse_mapping) {
				diffuseLayer.push_back(CMaterialMappingLayerInfo());
				CMaterialMappingLayerInfo& layerInfo = diffuseLayer.back();
				if (!chkTransparentAlpha) {
					chkTransparentAlpha = true;
					transparentAlpha    = (mappingLayer.get_channel_mix() == sxsdk::enums::mapping_transparent_alpha_mode);
				}
				layerInfo.patternType  = sxsdk::enums::image_pattern;
				layerInfo.textureIndex = Util::GetMasterImageIndex(scene, image);
				layerInfo.repeatX      = mappingLayer.get_repetition_X();
				layerInfo.repeatY      = mappingLayer.get_repetition_Y();
				layerInfo.flipColor    = mappingLayer.get_flip_color();
				layerInfo.weight       = mappingLayer.get_weight();
			}

			if (type == sxsdk::enums::normal_mapping || type == sxsdk::enums::bump_mapping) {
				normalLayer.push_back(CMaterialMappingLayerInfo());
				CMaterialMappingLayerInfo& layerInfo = normalLayer.back();
				layerInfo.patternType  = sxsdk::enums::image_pattern;
				layerInfo.textureIndex = Util::GetMasterImageIndex(scene, image);
				layerInfo.repeatX      = mappingLayer.get_repetition_X();
				layerInfo.repeatY      = mappingLayer.get_repetition_Y();
				layerInfo.flipColor    = mappingLayer.get_flip_color();
				layerInfo.weight       = mappingLayer.get_weight();
				layerInfo.normalMap    = (type == sxsdk::enums::normal_mapping);
			}

			if (type == sxsdk::enums::trim_mapping) {
				trimLayer.push_back(CMaterialMappingLayerInfo());
				CMaterialMappingLayerInfo& layerInfo = trimLayer.back();
				layerInfo.patternType  = sxsdk::enums::image_pattern;
				layerInfo.textureIndex = Util::GetMasterImageIndex(scene, image);
				layerInfo.repeatX      = mappingLayer.get_repetition_X();
				layerInfo.repeatY      = mappingLayer.get_repetition_Y();
				layerInfo.flipColor    = mappingLayer.get_flip_color();
				layerInfo.weight       = mappingLayer.get_weight();
			}

			if (type == sxsdk::enums::volume_distance_mapping) {
				volumeDistanceLayer.push_back(CMaterialMappingLayerInfo());
				CMaterialMappingLayerInfo& layerInfo = volumeDistanceLayer.back();
				layerInfo.patternType  = sxsdk::enums::image_pattern;
				layerInfo.textureIndex = Util::GetMasterImageIndex(scene, image);
				layerInfo.repeatX      = mappingLayer.get_repetition_X();
				layerInfo.repeatY      = mappingLayer.get_repetition_Y();
				layerInfo.flipColor    = mappingLayer.get_flip_color();
				layerInfo.weight       = mappingLayer.get_weight();
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
	if (materialInfo.volumeRendering) {
		//----------------------------------------------------------.
		// ボリューム表現の場合.
		//----------------------------------------------------------.
		risMaterialInfo.type = RIBParam::pxrVolume;

	} else if (materialInfo.transparent > fMin) {
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

