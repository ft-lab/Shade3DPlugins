/**
 * RIB形式のファイルの保存.
 */

#ifndef _SAVE_RIB_H
#define _SAVE_RIB_H

#include "GlobalHeader.h"
#include "TextureCtrl.h"
#include "MaterialCtrl.h"
#include "PolygonMeshCtrl.h"
#include "LightCtrl.h"

//-----------------------------------------------------------.
// 保存するRIBファイルの情報.
//-----------------------------------------------------------.
class CRIBInfo
{
public:
	std::string ribFileName;				// RIBファイルの名前.
	std::string filePath;					// 保存先のファイルパス.

	std::string renderingFileName;			// レンダリング画像名.
	sx::vec<int,2> renderingImageSize;		// レンダリング画像サイズ.

	float fov;								// カメラのFOV値.
	bool perspective;						// 透視投影の場合はtrue.
	sxsdk::mat4 worldToViewMatrix;			// ビュー変換行列.

	RIBParam::INTEGRATORS_TYPE type;		// レンダリングの種類.
	float pixelVariance;
	float minSamples, maxSamples;			// レンダリング時のピクセルごとのサンプリング数の最小と最大.
	bool incremental;
	float bias;								// レイを浮かせるバイアス.
	int maxDiffuseDepth;					// 拡散反射の反射回数最大.
	int maxSpecularDepth;					// 鏡面反射（映り込み含む）の反射回数最大.

	RIBParam::RENDERING_FORMAT_TYPE renderingFormatType;	// レンダリングファイルの種類.
	RIBParam::RENDERING_IMAGE_TYPE renderingImageType;		// レンダリング画像の種類.

	std::vector<CTextureInfo> textureList;	// テクスチャ情報.

	bool outputBackgroundImage;									// 背景画像を出力.
	RIBParam::BACKGROUND_IMAGE_SIZE backgroundImageSize;		// 背景画像のサイズ.
	float backgroundImageIntensity;								// 背景画像の明るさ.
	bool backgroundDraw;										// 背景の描画.

	bool colorColorToLinear;									// 色情報をリニアに計算して渡す.
	bool colorTextureToLinear;									// テクスチャ情報をリニアに計算して渡す.

	bool lightDayLight;											// 太陽光の有効化.

	bool statisticsEndOfFrame;									// レンダリング時間を表示.
	bool statisticsXMLFile;										// XMLファイルを出力.

public:
	CRIBInfo ();

	/**
	 * 情報をクリア.
	 */
	void Clear ();
};

//-----------------------------------------------------------.
// RIB出力クラス.
//-----------------------------------------------------------.
class CSaveRIB
{
private:
	sxsdk::shade_interface& shade;
	sxsdk::text_stream_interface* m_text_stream;
	sxsdk::stream_interface* m_stream;

	CPolygonMeshCtrl m_polygonMeshCtrl;			// ポリゴンメッシュ情報を一時格納用.
	CLightCtrl m_lightCtrl;						// 光源の一時格納クラス.

	CRIBInfo m_RIBInfo;							// RIB情報.
	int m_indent;								// インデントの深さ.

	RIBExportData m_dlgData;					// RIB Exportダイアログでのパラメータ.

	std::vector<CMaterialInfo> m_MaterialList;	// マテリアルリスト.

	sxsdk::scene_interface* m_pScene;
	sxsdk::shape_class* m_pCurrentShape;

	std::string m_backgroundTextureName;		// 背景テクスチャの名前.

	int m_currentSubdivisionType;				// ポリゴンメッシュのSubdivisionの種類 (sxsdk::polygon_mesh_classs::get_roundness_type() の値).

	/**
	 * ヘッダの出力.
	 */
	void m_WriteHeader ();

	/**
	 * カメラの出力.
	 */
	void m_WriteCamera ();

	/**
	 * テクスチャの出力.
	 */
	void m_WriteTextures ();

	/**
	 * インデントからスペースを取得.
	 */
	std::string m_IndentToText(const int indent);

	/**
	 * 1行分の出力.
	 */
	void m_WriteLine(const std::string& str);

	/**
	 * 位置情報を出力.
	 */
	void m_WriteMatrix(const sxsdk::mat4& m, const bool outScale = true);

	/**
	 * 光源の出力.
	 */
	void m_WriteLights (sxsdk::scene_interface* scene);

	/**
	 * 画像ファイルを保存.
	 */
	void m_OutputTextureFiles (sxsdk::scene_interface* scene);

	/**
	 * 背景をパノラマの画像ファイルとして保存.
	 */
	void m_OutputBackgroundTextureFile (sxsdk::scene_interface* scene);

	/**
	 * 画像ファイルをtexファイルとして保存.
	 */
	std::string m_OutputTexture (sxsdk::scene_interface* scene, sxsdk::shape_class* pShape, const int index);

	/**
	 * マテリアルの出力開始.
	 * 形状の情報を出力する前に呼ぶこと.
	 */
	void m_BeginWriteMaterial (sxsdk::scene_interface* scene, sxsdk::shape_class& shape);
	void m_BeginWriteMaterial (sxsdk::scene_interface* scene, sxsdk::master_surface_class& masterSurface);
	void m_BeginWriteMaterial (sxsdk::scene_interface* scene, sxsdk::shape_class* shape, sxsdk::master_surface_class* masterSurface);

	/**
	 * マテリアルの出力終了.
	 */
	void m_EndWriteMaterial ();

	/**
	 * テクスチャ番号に対応するテクスチャ名を取得.
	 */
	std::string m_GetTextureName (const int textureIndex, const bool fileF = true);

	/**
	 * マスターサーフェスとしてのマテリアル情報を出力.
	 */
	void m_WriteMasterSurfaceMaterials (sxsdk::scene_interface* scene);

	/**
	 * 逆ガンマ補正を行ったリニアな色を返す.
	 */
	sxsdk::rgb_class m_CalcLinearColor (const sxsdk::rgb_class& col);

	/**
	 * マテリアル出力時に、テクスチャの繰り返しや色反転などが存在する場合のPxrManifold2D出力.
	 * @param[in]  materialName  マスターサーフェス名.
	 * @param[in]  typeName      マテリアルの種類（diffuse/bump/normal/trim）.
	 * @param[in]  mappingLayer  マテリアルのマッピングレイヤリスト.
	 * @param[in]  layerIndex    マテリアルのマッピングレイヤ番号.
     * @return RGBを持つパターン名.
	 */
	std::string m_WriteMaterialTexture (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer, const int layerIndex, const bool isBumpNormal = false);

	/**
	 * マテリアル出力時に、フラクタルノイズの情報を出力.
	 * @param[in]  materialName  マスターサーフェス名.
	 * @param[in]  typeName      マテリアルの種類（diffuse/bump/normal/trim）.
	 * @param[in]  mappingLayer  マテリアルのマッピングレイヤリスト.
	 * @param[in]  layerIndex    マテリアルのマッピングレイヤ番号.
	 * @param[in]  baseColor     表面材質としての基本色 (baseTexNameの指定がない場合).
	 * @param[in]  baseTexName   ベースのテクスチャ名.
     * @return RGBを持つパターン名.
	 */
	std::string m_WriteMaterialFractal (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer, const int layerIndex, const sxsdk::rgb_class baseColor, const std::string baseTexName, const bool isBumpNormal = false);

	/**
	 * マルチレイヤに対応したマテリアルの出力（Diffuse/Trim）.
	 */
	std::string m_WriteMaterialRGB (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer, const sxsdk::rgb_class baseColor, std::string& retDiffuseFirst);

	/**
	 * マルチレイヤに対応したマテリアルの出力（Trim）.
	 */
	std::string m_WriteMaterialTrim (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer, const std::string diffuseTextureName);

	/**
	 * マルチレイヤに対応したマテリアルの出力（Bump or Normal）.
	 */
	std::string m_WriteMaterialNormal (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer);

	/**
	 * マルチレイヤに対応したマテリアルの出力（VolumeDistance）.
	 */
	std::string m_WriteMaterialVolumeDistance (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer);

	/**
	 * マルチレイヤに対応したマテリアルの出力（Reflection）.
	 */
	std::string m_WriteMaterialReflection (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer);

	/**
	 * マルチレイヤに対応したマテリアルの出力（Roughness）.
	 */
	std::string m_WriteMaterialRoughness (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer);

public:
	CSaveRIB (sxsdk::shade_interface& shade, sxsdk::stream_interface* stream, sxsdk::text_stream_interface* text_stream, const RIBExportData& dlgData);

	/**
	 * 出力したribファイル名を取得.
	 */
	std::string GetRIBFileName () { return m_RIBInfo.ribFileName; }

	/**
	 * カメラ情報など、シーンに関する情報を指定.
	 */
	void SetSceneInfo (sxsdk::scene_interface* scene);

	/**
	 * エクスポート開始.
	 */
	void BeginExport (sxsdk::scene_interface* scene);

	/**
	 * エクスポート終了.
	 */
	void EndExport ();

	/**
	 * ポリゴンメッシュ情報の格納開始.
	 */
	void BeginPolygonMesh (sxsdk::shape_class* shape);

	/**
	 * ポリゴンメッシュ情報の格納終了.
	 */
	void EndPolygonMesh ();

	/**
	 * 頂点座標追加.
	 */
	void AppendPolygonMeshVertex (const sxsdk::vec3& v);

	/**
	 * 面情報追加.
	 */
	void AppendPolygonMeshPolygon (const std::vector<int>& indices, const std::vector<sxsdk::vec3>& normals, const std::vector<sxsdk::vec2>& uvs, const int faceGroupIndex = -1);
};

#endif
