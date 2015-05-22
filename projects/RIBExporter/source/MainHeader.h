/**
 *  @file   MainHeader.h
 *  @brief  メインヘッダ情報.
 *  @date   2011.06.22 - 2011.06.22  
 *  @author Yutaka Yoshisaka
 */

#ifndef MAINHEADER_H
#define MAINHEADER_H

#include "sxsdk.cxx"

// FBXインポータプラグインのプラグインID.
static const sx::uuid_class FBX_IMPORTER_ID("6805E7CC-4A9C-46a4-86F2-3E73AB936ECD");
// FBXエクスポータプラグインのプラグインID.
static const sx::uuid_class FBX_EXPORTER_ID("A9A9AE0A-A259-4f73-80BC-80A51AE45337");
// アニメーションテイクを選択するダイアログを出すID.
static const sx::uuid_class FBX_SELECT_ANIMATION_TAKE_DLG_ID("A8E0A50C-7B3D-4421-952E-6EAD5E884F4C");
// テクスチャファイルを選択するダイアログを出すID.
static const sx::uuid_class FBX_LOCATE_FILE_DLG_ID("61AFD6ED-60D6-414A-88BD-D3FBDED16E8D");

// FBXエクスポータでのpreferenceに保持する情報のバージョン.
#define FBX_EXPORTER_PREFERNECE_VERSION		0x100
// FBXインポータでのpreferenceに保持する情報のバージョン.
#define FBX_IMPORTER_PREFERNECE_VERSION		0x100

#define OBJECT_TYPE_PART			1			///< パート用オブジェクト.
#define OBJECT_TYPE_LINE			2			///< ライン用オブジェクト.
#define OBJECT_TYPE_POLYGONMESH		3			///< ポリゴンメッシュ用オブジェクト.

/**
 * mqoとしての面情報.
 */
typedef struct {
	int vCou;					///< 頂点数（3～4）.
	int index[4];				///< 頂点インデックス.
	sxsdk::vec3 normal[4];		///< 頂点法線.
	sxsdk::vec2 uv[4];			///< UV.
	unsigned int vColor[4];		///< 頂点カラー.
	int materialIndex;			///< メタセコでは面ごとにマテリアルを持つ、-1の場合はマテリアルなし。.
	int materialFaceGroupIndex;	///< Shade12のフェイスグループ番号.
} OBJECT_FACE_INFO;

/**
 * mqoとしてのオブジェクト情報（パート、ライン、ポリゴンメッシュに対応）.
 */
typedef struct {
	int type;									///< 管理用の種類（OBJECT_TYPE_xxx）.
	sxsdk::shape_class *pShape;					///< Shadeの形状クラスのポインタ.

	int depth;									///< 階層の表現用（0～）.
	char name[128];								///< オブジェクト名.
	int verticesCount;							///< 頂点数.
	sxsdk::vec3 *pVertices;						///< 頂点座標.

	sxsdk::vec3 scale;							///< 拡大.
	sxsdk::vec3 rotation;						///< 回転.
	sxsdk::vec3 translation;					///< 移動.
	bool visible;								///< 表示・非表示.
	bool locking;								///< ロック.
	float smootingAngle;						///< スムージング角度（0.0 - 180.0）.
	bool shading;								///< falseの場合はフラット.

	int mirror;									///< 鏡面のタイプ（0 : なし、1:左右を分離、2:左右を接続）.
	int mirror_axis;							///< 鏡面の適用軸（1 : X軸、 2 : Y軸、 4: Z軸）.

	int patch;									///< 0 : 曲面なし、 1 : 曲面タイプ1、 2 : 曲面タイプ2、3 : カトマルクラーク.
	int segment;								///< 曲面時の分割数（デフォルト4）.

	int facesCount, facesMaxCount;				///< 面数.
	OBJECT_FACE_INFO *pFaces;					///< 面情報.

	int materialCount;							///< 使用しているマテリアルの数.
	int *pMaterialIndex;						///< 使用しているマテリアルのリスト.
} OBJECT_INFO;

/**
 * mqoとしてのマテリアル情報.
 */
typedef struct {
	char name[128];						///< マテリアル名.
	sxsdk::rgba_class diffuseCol;		///< 拡散反射色.
	float diffuseVal;					///< 拡散反射値.
	float specularVal;					///< 鏡面反射値.
	float specularSize;					///< 鏡面反射の強さ.
	float ambientVal;					///< 環境値.
	float emissionVal;					///< 発光値.
	char texName[128];					///< テクスチャ名.
	char alphaTexName[128];				///< アルファ用のテクスチャ名.
	char bumpTexName[128];				///< バンプのテクスチャ名.

	sxsdk::surface_class *pSurface;		///< 対応するShadeでの表面材質管理クラス.
	int uvLayerIndex;					///< 距離補正の場合は0、パラメータUVの場合は1、最大7まで。.
} MATERIAL_INFO;

namespace ShadeLight {
//	const float	very_far			= 100000.0; // /*transmat::translation(-dir*std::numeric_limits<float>::max())*/
	const float	bit_far				= 5000.0;	// /*transmat::translation(-dir*std::numeric_limits<float>::max())*/
	const float	cIntensity_Default	= 100.0;	// see kfbxlight.h
	const char	shade_distant_light[] = "shade_distantlight";
}
namespace {
	const int PROFILE_SUPPORT_LIST_ITEM_PROPERTY_XVALUE = 470121;
}

#endif
