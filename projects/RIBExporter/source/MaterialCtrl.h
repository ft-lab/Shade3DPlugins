﻿/**
 * マテリアル管理用.
 */
#ifndef _MATERIALCTRL_H
#define _MATERIALCTRL_H

#include "GlobalHeader.h"

//-------------------------------------------------------------------.
// RIS用のマテリアル情報.
//-------------------------------------------------------------------.
/**
 * RenderManでのマテリアル情報 (base).
 */
class CPxrMaterialBase
{
public:
	std::string name;			// マテリアル名.

public:
	CPxrMaterialBase ();
	virtual ~CPxrMaterialBase ();

	CPxrMaterialBase (const CPxrMaterialBase& v);
    CPxrMaterialBase& operator = (const CPxrMaterialBase &v) {
        this->name = v.name;
        return (*this);
    }

	/**
	 * 情報のクリア.
	 */
	virtual void Clear () = 0;
};

/**
 * RenderManでのマテリアル情報 (PxrDiffuse).
 */
class CPxrMaterialDiffuse : public CPxrMaterialBase
{
public:
	sxsdk::rgb_class diffuseColor;
	sxsdk::rgb_class transmissionColor;
	float presence;			// 未使用（これはテクスチャのアルファ透過やトリムと同様でテクスチャでのマスキング）.

public:
	CPxrMaterialDiffuse ();
	virtual ~CPxrMaterialDiffuse ();
	CPxrMaterialDiffuse (const CPxrMaterialDiffuse& v);
    CPxrMaterialDiffuse& operator = (const CPxrMaterialDiffuse &v) {
        this->diffuseColor = v.diffuseColor;
        this->transmissionColor = v.transmissionColor;
        this->presence = v.presence;
        return (*this);
    }

	/**
	 * 情報のクリア.
	 */
	virtual void Clear ();
};

/**
 * RenderManでのマテリアル情報 (PxrDisney).
 */
class CPxrMaterialDisney : public CPxrMaterialBase
{
public:
	sxsdk::rgb_class baseColor;
	sxsdk::rgb_class emitColor;
	float subsurface;
	sxsdk::rgb_class subsurfaceColor;
	float metallic;
	float specular;
	float specularTint;
	float roughness;
	float anisotropic;
	float sheen;
	float sheenTint;

public:
	CPxrMaterialDisney ();
	virtual ~CPxrMaterialDisney ();
	CPxrMaterialDisney (const CPxrMaterialDisney& v);
    CPxrMaterialDisney& operator = (const CPxrMaterialDisney &v) {
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
        return (*this);
    }
	
	/**
	 * 情報のクリア.
	 */
	virtual void Clear ();
};

/**
 * RenderManでのマテリアル情報 (PxrGlass).
 */
class CPxrMaterialGlass : public CPxrMaterialBase
{
public:
	float ior;
	float roughness;
	sxsdk::rgb_class reflectionColor;
	float reflectionGain;
	sxsdk::rgb_class transmissionColor;
	float transmissionGain;
	float absorptionGain;
	sxsdk::rgb_class absorptionColor;

public:
	CPxrMaterialGlass ();
	virtual ~CPxrMaterialGlass ();
	CPxrMaterialGlass (const CPxrMaterialGlass& v);
    CPxrMaterialGlass& operator = (const CPxrMaterialGlass &v) {
        this->ior = v.ior;
        this->roughness = v.roughness;
        this->reflectionColor = v.reflectionColor;
        this->reflectionGain = v.reflectionGain;
        this->transmissionColor = v.transmissionColor;
        this->transmissionGain = v.transmissionGain;
        this->absorptionGain = v.absorptionGain;
        this->absorptionColor = v.absorptionColor;
        return (*this);
    }

	/**
	 * 情報のクリア.
	 */
	virtual void Clear ();
};

/**
 * RenderManでのマテリアル情報 (PxrConstant).
 */
class CPxrMaterialConstant : public CPxrMaterialBase
{
public:
	sxsdk::rgb_class emitColor;

public:
	CPxrMaterialConstant ();
	virtual ~CPxrMaterialConstant ();
	CPxrMaterialConstant (const CPxrMaterialConstant& v);
    CPxrMaterialConstant& operator = (const CPxrMaterialConstant &v) {
        this->emitColor = v.emitColor;
        return (*this);
    }

	/**
	 * 情報のクリア.
	 */
	virtual void Clear ();
};

/**
 * RenderManでのマテリアル情報 (PxrVolume).
 */
class CPxrMaterialVolume : public CPxrMaterialBase
{
public:
	sxsdk::rgb_class diffuseColor;
	sxsdk::rgb_class emitColor;
	sxsdk::rgb_class densityColor;
	float densityFloat;
	float densityScale;
	float anisotropy;
	float maxDensity;
	bool multiScatter;

public:
	CPxrMaterialVolume ();
	virtual ~CPxrMaterialVolume ();
	CPxrMaterialVolume (const CPxrMaterialVolume& v);
    CPxrMaterialVolume& operator = (const CPxrMaterialVolume &v) {
        this->diffuseColor = v.diffuseColor;
        this->emitColor    = v.emitColor;
        this->densityColor = v.densityColor;
        this->densityFloat = v.densityFloat;
        this->densityScale = v.densityScale;
        this->anisotropy   = v.anisotropy;
        this->maxDensity   = v.maxDensity;
        this->multiScatter = v.multiScatter;
        return (*this);
    }

	/**
	 * 情報のクリア.
	 */
	virtual void Clear ();
};

/**
 * フローティングウィンドウとして保持するRIS用のマテリアル情報.
 */
class CRISMaterialInfo
{
public:
	bool useCustom;						// カスタムな情報を使用.
	RIBParam::MATERIAL_TYPE type;		// マテリアルの種類.

	bool useDepth;						// depthの使用.
	bool visibilityDiffuse;				// diffuseの有効化.
	bool visibilitySpecular;			// specularの有効化.
	bool visibilityIndirect;			// 間接照明の有効化.
	bool visibilityCamera;				// 可視かどうか.

	int maxdiffusedepth;				// diffuseの反射回数.
	int maxspeculardepth;				// specularの反射回数.

	CPxrMaterialDiffuse pxrDiffuse;
	CPxrMaterialDisney pxrDisney;
	CPxrMaterialGlass pxrGlass;
	CPxrMaterialConstant pxrConstant;
	CPxrMaterialVolume pxrVolume;

public:
	CRISMaterialInfo ();

	/**
	 * すべての情報をクリアする（初期値を入れる）.
	 */
	void Clear ();
};

//-------------------------------------------------------------------.
/**
 * Shade 3Dでのマテリアルのマッピングレイヤごとの情報.
 */
class CMaterialMappingLayerInfo
{
public:
	int textureIndex;				// テクスチャ番号.
	int repeatX, repeatY;			// 繰り返し数.
	bool flipColor;					// 反転.

	sxsdk::enums::pattern_type patternType;		// イメージ/スポットなどの種類 (procedural textureのものはRenderManのパターンとして渡す).
	sxsdk::rgb_class color;						// レイヤの色.
	float weight;								// ウエイト値.
	float softness;								// ソフトネス.
	float size;									// サイズ.

	bool normalMap;								// 法線マップの場合true/falseの場合はバンプマップ.

public:
	CMaterialMappingLayerInfo ();

	void Clear ();
};

/**
 * Shade 3Dでのマテリアル情報.
 */
class CMaterialInfo
{
public:
	std::string name;								// マテリアル名（master_surface 名）.

	sxsdk::master_surface_class* masterSurface;		// master surfaceの参照.
	sxsdk::surface_class* pSurface;					// surfaceの参照.

	sxsdk::rgb_class diffuseColor;		// Diffuse色.
	float diffuseValue;					// Diffuse値.

	sxsdk::rgb_class specularColor;		// Specular色.
	float specularValue;				// Specular値.
	float specularSize;					// Specularの強さ.

	float transparent;					// 透明度.
	float reflection;					// 反射.
	float refraction;					// 屈折.
	float roughness;					// 荒さ.
	float anisotropic;					// 異方性反射.

	sxsdk::rgb_class glowColor;			// 発光色.
	float glow;							// 発光.

	int mappingLayerCount;				// マッピングレイヤ数.

	bool noShading;						// シェーディングしない場合.
	bool noShadow;						// 影付けしない場合.

	bool transparentAlpha;				// アルファ透明.

	bool volumeRendering;				// ボリュームレンダリング.

	std::vector<CMaterialMappingLayerInfo> diffuseLayer;		// Diffuse Mapのレイヤ情報.
	std::vector<CMaterialMappingLayerInfo> normalLayer;			// Normal Mapのレイヤ情報.
	std::vector<CMaterialMappingLayerInfo> trimLayer;			// Trim Mapのレイヤ情報.
	std::vector<CMaterialMappingLayerInfo> volumeDistanceLayer;	// ボリュームの減衰距離レイヤ情報.
	std::vector<CMaterialMappingLayerInfo> reflectionLayer;		// Reflection Mapのレイヤ情報.
	std::vector<CMaterialMappingLayerInfo> roughnessLayer;		// Roughness Mapのレイヤ情報.

	std::string ribDiffusePatternName;			// 最終的なRIB出力時のDiffuseパターン名.
	std::string ribNormalPatternName;			// 最終的なRIB出力時のNormalパターン名.
	std::string ribTrimPatternName;				// 最終的なRIB出力時のTrimパターン名.
	std::string ribVolumeDistancePatternName;	// 最終的なRIB出力時のボリュームの減衰距離パターン名.
	std::string ribReflectionPatternName;		// 最終的なRIB出力時のReflectionのパターン名.
	std::string ribRoughnessPatternName;		// 最終的なRIB出力時のRoughnessのパターン名.

	void m_SetMaterial (sxsdk::scene_interface* scene, sxsdk::surface_class* surface);

public:
	CMaterialInfo ();

	void Clear ();

	/**
	 * 指定の形状でのマテリアルを格納.
	 */
	void SetMaterial (sxsdk::scene_interface* scene, sxsdk::shape_class& shape);
	void SetMaterial (sxsdk::scene_interface* scene, sxsdk::master_surface_class& masterSurface);
};

//-------------------------------------------------------------------.
// マテリアル情報を、 Shade 3D <==> RenderMan(RIS)でコンバート.
//-------------------------------------------------------------------.
namespace MaterialCtrl
{
	/**
	 * Shade 3Dでのマテリアル情報より、RIS向けにコンバート.
	 */
	void ConvShade3DToRIS (CMaterialInfo& materialInfo, CRISMaterialInfo& risMaterialInfo);
}

#endif
