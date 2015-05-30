/**
 * RIB形式のファイルの保存.
 */

#include "SaveRIB.h"
#include "Util.h"
#include "MathUtil.h"
#include "SaveTiff.h"
#include "CameraCtrl.h"
#include "StreamCtrl.h"
#include "BackgroundTexture.h"

#include <stdio.h>
#if SXWINDOWS
#include <direct.h>
#endif
#include <sys/stat.h>

#define USE_PRMAN_RIS	1		// RenderManのGIレンダリングモードを使用する.

#define BACKGROUND_TEXTURE_NAME "background_panorama"		// 背景画像の名前.

namespace {
	// sxsdk::rgb_classの色が(0, 0, 0)であるか判定.
	bool CheckColorBlack (const sxsdk::rgb_class& col) {
		return (sx::zero(col.red) && sx::zero(col.green) && sx::zero(col.blue));
	}

	// sxsdk::rgb_classの色が(1, 1, 1)であるか判定.
	bool CheckColorWhite (const sxsdk::rgb_class& col) {
		return (sx::zero(col.red - 1.0f) && sx::zero(col.green - 1.0f) && sx::zero(col.blue - 1.0f));
	}
}

/*
	RenderManは、左手系のZ-up.
*/

//-----------------------------------------------------------.
// 保存するRIBファイルの情報.
//-----------------------------------------------------------.
CRIBInfo::CRIBInfo ()
{
	Clear();
}

/**
 * 情報をクリア.
 */
void CRIBInfo::Clear ()
{
	renderingImageSize = sx::vec<int,2>(640, 480);
	fov         = 45.0f;
	perspective = true;
	worldToViewMatrix = sxsdk::mat4::identity;
}

//-----------------------------------------------------------.
// RIB出力クラス.
//-----------------------------------------------------------.

CSaveRIB::CSaveRIB (sxsdk::shade_interface& shade, sxsdk::stream_interface* stream, sxsdk::text_stream_interface* text_stream, const RIBExportData& dlgData) :
	shade(shade), m_stream(stream), m_text_stream(text_stream), m_polygonMeshCtrl(shade), m_lightCtrl(shade), m_dlgData(dlgData)
{
	m_RIBInfo.ribFileName = Util::GetFileNameToStream(m_stream);
	m_RIBInfo.filePath    = Util::GetFilePath(m_stream);

	// RIB Exportダイアログのデータを保持.
	{
		m_RIBInfo.type                = m_dlgData.type;
		m_RIBInfo.pixelVariance       = m_dlgData.pixelVariance;
		m_RIBInfo.minSamples          = m_dlgData.minSamples;
		m_RIBInfo.maxSamples          = m_dlgData.maxSamples;
		m_RIBInfo.incremental         = m_dlgData.incremental;
		m_RIBInfo.bias                = m_dlgData.bias;
		m_RIBInfo.maxDiffuseDepth     = m_dlgData.maxDiffuseDepth;
		m_RIBInfo.maxSpecularDepth    = m_dlgData.maxSpecularDepth;
		m_RIBInfo.renderingFormatType = m_dlgData.renderingFormatType;
		m_RIBInfo.renderingImageType  = m_dlgData.renderingImageType;
		m_RIBInfo.outputBackgroundImage    = m_dlgData.outputBackgroundImage;
		m_RIBInfo.backgroundImageSize      = m_dlgData.backgroundImageSize;
		m_RIBInfo.backgroundImageIntensity = m_dlgData.backgroundImageIntensity;
		m_RIBInfo.backgroundDraw           = m_dlgData.backgroundDraw;
		m_RIBInfo.colorColorToLinear       = m_dlgData.colorColorToLinear;
		m_RIBInfo.colorTextureToLinear     = m_dlgData.colorTextureToLinear;
		m_RIBInfo.lightDayLight            = m_dlgData.lightDayLight;
		m_RIBInfo.statisticsEndOfFrame     = m_dlgData.statisticsEndOfFrame;
		m_RIBInfo.statisticsXMLFile        = m_dlgData.statisticsXMLFile;
	}

	// 出力画像名はribFileNameをtiffもしくはexrにしたものにする.
	{
		m_RIBInfo.renderingFileName = (m_RIBInfo.renderingFormatType == RIBParam::openexr) ? "output.exr" : "output.tiff";
		const int iPos = m_RIBInfo.ribFileName.find(".");
		if (iPos != std::string::npos) {
			m_RIBInfo.renderingFileName = m_RIBInfo.ribFileName.substr(0, iPos) + ((m_RIBInfo.renderingFormatType == RIBParam::openexr) ? std::string(".exr") : std::string(".tiff"));
		}
	}

	m_indent = 0;
}

/**
 * ヘッダの出力.
 */
void CSaveRIB::m_WriteHeader ()
{
	// Header comment.
	{
		std::stringstream s;
		s << "# " << m_RIBInfo.ribFileName;
		m_text_stream->write_line(s.str().c_str());
		m_text_stream->write_line("#");
		m_text_stream->write_line("");
	}

	// Display.
	// RISの場合はリニアな色になっている。そのままexrかtiffで指定.
	{
		std::stringstream s;
		if (m_RIBInfo.renderingFormatType == RIBParam::openexr) {
			s << "Display \"" << m_RIBInfo.renderingFileName << "\" \"openexr\"";
		} else {
			s << "Display \"" << m_RIBInfo.renderingFileName << "\" \"tiff\"";
		}
		if (m_RIBInfo.renderingImageType == RIBParam::image_rgb) {
			s << " \"rgb\"";
		} else {
			s << " \"rgba\"";
		}

		m_text_stream->write_line(s.str().c_str());
	}

#if !USE_PRMAN_RIS
	if (!m_outputExr) {
		std::stringstream s;
		s << "Exposure 1.0 2.2";		// Gain gammaの指定.
		m_text_stream->write_line(s.str().c_str());
	}
#endif

	// これはデフォルトのままのほうが速い.
#if 0
	{
		std::stringstream s;
		s << "Hider \"zbuffer\"";
		m_text_stream->write_line(s.str().c_str());
	}
#endif

	// Format.
	{
		std::stringstream s;
		s << "Format " << m_RIBInfo.renderingImageSize.x << " " << m_RIBInfo.renderingImageSize.y << " 1";
		m_text_stream->write_line(s.str().c_str());
	}

	// Projection.
	{
		std::stringstream s;
		s << "Projection \"perspective\" " << "\"fov\" [" << m_RIBInfo.fov << "]";
		m_text_stream->write_line(s.str().c_str());
	}

	//-----------------------------------------.
	// レンダリング情報.
	// OptionはWorldBeginの外に書く.
	// http://renderman.pixar.com/view/raytracing-fundamentals
	//-----------------------------------------.
	// prman実行後にレンダリング時間を表示.
	if (m_RIBInfo.statisticsEndOfFrame) {
		std::stringstream s;
		s << "Option \"statistics\" \"endofframe\" [1]";
		m_WriteLine(s.str());
	}

	// peman実行後に解析のためのXMLファイルを出力.
	if (m_RIBInfo.statisticsXMLFile) {
		std::string name = m_RIBInfo.ribFileName;
		const int iPos = name.find(".");
		if (iPos != std::string::npos) {
			name = name.substr(0, iPos);
		}

		std::stringstream s;
		s << "Option \"statistics\" \"string xmlfilename\" [\"" << name << ".xml\"]";
		m_WriteLine(s.str());
	}

	// レイの追跡レベル.
	{
		std::stringstream s;
		s << "Option \"trace\" \"int maxdepth\" [" << 10 << "]";
		m_WriteLine(s.str());
	}

#if 0
	// 面光源時のサンプリング数.
	{
		const int samples = 32;
		std::stringstream s;
		s << "Option \"shading\" \"int directlightingsamples\" [" << samples << "]";
		m_WriteLine(s.str());
	}
#endif

	// 値が小さいほど細かいレンダリング.
	{
		const int rateVal = 1;
		std::stringstream s;
		s << "ShadingRate " << rateVal;
		m_WriteLine(s.str());
	}
	m_WriteLine("");

	// RISの設定.
#if USE_PRMAN_RIS
	{
		// bucketは、レンダリング中の表示効果.
#if 0
		{
			std::stringstream s;
			s << "Option \"limits\" \"bucketsize\" [8 8]";
			m_WriteLine(s.str());
		}
		{
			std::stringstream s;
			s << "Option \"bucket\" \"string order\" [\"spiral\"]";
			m_WriteLine(s.str());
		}
#endif
		{
			std::stringstream s;
			s << "PixelVariance " << m_RIBInfo.pixelVariance;
			m_WriteLine(s.str());
		}

		// ピクセルサンプリング数など.
		{
			std::stringstream s;
			s << "Hider \"raytrace\" \"int minsamples\" [" << m_RIBInfo.minSamples << "] \"int maxsamples\" [" << m_RIBInfo.maxSamples << "] \"int incremental\" [" << (m_RIBInfo.incremental ? 1 : 0) << "]";
			m_WriteLine(s.str());
		}

		// レンダリングの種類.
		{
			std::string typeStr = "PxrPathTracer";
			switch (m_RIBInfo.type) {
				case RIBParam::pxrVCM:
					typeStr = "PxrVCM";
					break;

				case RIBParam::pxrDirectLighting:
					typeStr = "PxrDirectLighting";
					break;
			}

			std::stringstream s;
			s << "Integrator \"" << typeStr << "\" \"handle\"";
			m_WriteLine(s.str());
		}
		m_WriteLine("");
	}
#endif
}

/**
 * カメラの出力.
 */
void CSaveRIB::m_WriteCamera ()
{
	// Camera.
	m_WriteLine("# Camera --------");
	sxsdk::mat4 m = m_RIBInfo.worldToViewMatrix;
	m_WriteMatrix(m, false);
	m_WriteLine("");
}

/**
 * テクスチャの出力.
 */
void CSaveRIB::m_WriteTextures ()
{
	if (m_RIBInfo.textureList.size() == 0 && m_backgroundTextureName.size() == 0) return;

#if USE_PRMAN_RIS
	m_WriteLine("# Textures --------");

	for (int i = 0; i < m_RIBInfo.textureList.size(); i++) {
		CTextureInfo& textureInfo = m_RIBInfo.textureList[i];
		if (!textureInfo.useTexture) continue;

		const std::string texFileName = textureInfo.fileName;
		std::string texName = texFileName;
		const int pos = texName.find(".");
		if (pos != std::string::npos) {
			texName = texFileName.substr(0, pos);
		}

		// sRGBからのリニア変換を行う場合は1。hdrなテクスチャの場合は変換不要.
		const int linearize = (textureInfo.isRealColor || !m_RIBInfo.colorTextureToLinear) ? 0 : 1;

		// "int linearize" [1] で、画像のsRGBをgamma 2.2の逆数で補正してリニアにする.
		// "int invertT" [0] で、テクスチャの垂直方向の反転を行わない.
		if (!textureInfo.isBumpMap && !textureInfo.isNormalMap) {
			std::stringstream s;
			s << "Pattern \"PxrTexture\" \"" << texName << "\" \"string filename\" [\"" << texFileName << "\"] \"int linearize\" [" << linearize << "] \"int invertT\" [0]";
			m_WriteLine(s.str());

		} else {
			std::stringstream s;
			s << "Pattern \"PxrTexture\" \"" << texName << "\" \"string filename\" [\"" << texFileName << "\"] \"int linearize\" [0] \"int invertT\" [0]";
			m_WriteLine(s.str());
		}
	}

	// 背景のための画像.
	if (m_backgroundTextureName.size() > 0) {
		std::stringstream s;
		s << "Pattern \"PxrTexture\" \"" << m_backgroundTextureName << "\" \"string filename\" [\"" << m_backgroundTextureName << ".tiff\"] \"int linearize\" [0] \"int invertT\" [0]";
		m_WriteLine(s.str());
	}

	m_WriteLine("");

#endif
}

/**
 * カメラ情報など、シーンに関する情報を指定.
 */
void CSaveRIB::SetSceneInfo (sxsdk::scene_interface* scene)
{
	// カメラ情報の取得.
	m_RIBInfo.worldToViewMatrix = scene->get_world_to_view_matrix();

	// カメラ情報を計算.
	CCameraCtrl cameraCtrl(scene);
	m_RIBInfo.fov = cameraCtrl.GetFOVDegress();								// fovを取得.
	m_RIBInfo.renderingImageSize = cameraCtrl.GetRenderingImageSize();		// レンダリング画像サイズを取得.
}

/**
 * インデントからスペースを取得.
 */
std::string CSaveRIB::m_IndentToText (const int indent)
{
	std::string str = "";
	for (int i = 0; i < indent; ++i) str += "  ";

	return str;
}

/**
 * エクスポート開始.
 */
void CSaveRIB::BeginExport (sxsdk::scene_interface* scene)
{
	m_pScene = scene;

	// テクスチャを出力.
	m_OutputTextureFiles(scene);

	// 背景画像を出力.
	m_OutputBackgroundTextureFile(scene);

	{
		int textureCou = 0;
		for (int i = 0; i < m_RIBInfo.textureList.size(); i++) {
			if (m_RIBInfo.textureList[i].useTexture) textureCou++;
		}
		if (m_backgroundTextureName.size() > 0) textureCou++;

		if (textureCou > 0) {
			shade.message("[ textures ]");
			for (int i = 0; i < m_RIBInfo.textureList.size(); i++) {
				if (m_RIBInfo.textureList[i].useTexture) {
					{
						std::stringstream s;
						s << "  " << m_RIBInfo.textureList[i].fileName;
						shade.message(s.str().c_str());
					}
				}
			}
			if (m_backgroundTextureName.size() > 0) {
				std::stringstream s;
				s << "  " << m_backgroundTextureName << ".tiff";
				shade.message(s.str().c_str());
			}
			shade.message("");
		}
	}

	m_indent = 0;

	// ヘッダ情報を出力.
	m_WriteHeader();

	// テクスチャの参照を出力.
	m_WriteTextures();

	// マスターサーフェスとしてのマテリアル情報を出力.
	m_WriteMasterSurfaceMaterials(scene);

	// カメラの変換を出力.
	m_WriteCamera();

	m_WriteLine("WorldBegin");
	m_indent++;

	// バイアス.
	{
		std::stringstream s;
		s << "Attribute \"trace\" \"float bias\" [" << m_RIBInfo.bias << "]";
		m_WriteLine(s.str());
	}
	// 拡散反射と鏡面反射の最大.
	{
		std::stringstream s;
		s << "Attribute \"trace\" " << "\"int maxdiffusedepth\" [" << m_RIBInfo.maxDiffuseDepth << "] \"int maxspeculardepth\" [" << m_RIBInfo.maxSpecularDepth << "]";
		m_WriteLine(s.str());
	}

	// 光源の出力.
	m_WriteLights(scene);
}

/**
 * エクスポート終了.
 */
void CSaveRIB::EndExport ()
{
	m_indent--;
	m_WriteLine("WorldEnd");
}

/**
 * 逆ガンマ補正を行ったリニアな色を返す.
 */
sxsdk::rgb_class CSaveRIB::m_CalcLinearColor (const sxsdk::rgb_class& col)
{
	return m_RIBInfo.colorColorToLinear ? MathUtil::CalcGamma(col) : col;
}

/**
 * 1行分の出力.
 */
void CSaveRIB::m_WriteLine(const std::string& str)
{
	std::string str2 = m_IndentToText(m_indent) + str;
	m_text_stream->write_line(str2.c_str());
}

/**
 * 位置情報を出力.
 */
void CSaveRIB::m_WriteMatrix(const sxsdk::mat4& m, const bool outScale)
{
	sxsdk::vec3 scale, translate, rotate, shear;
	m.unmatrix(scale, shear, rotate, translate);

	float flipF = 1.0f;
	if (scale.x < 0.0f && scale.y < 0.0f && scale.z < 0.0f) {
		flipF = -1.0f;
		scale = -scale;
	}

	// 右手系から左手系の回転補正.
	if (flipF < 0.0f) {
		translate.z *= flipF;

		rotate.x *= flipF;
		rotate.y *= flipF;

		rotate.z -= sx::pi;
	}

	// 回転の順番に注意!!.
	{
		std::stringstream s;
		s << "Translate " << translate.x << " " << translate.y << " " << -translate.z;
		m_WriteLine(s.str());
	}

	if (!sx::zero(rotate.z)) {
		std::stringstream s;
		s << "Rotate " << (rotate.z * 180.0f / sx::pi) << " 0 0 1";
		m_WriteLine(s.str());
	}
	if (!sx::zero(rotate.y)) {
		std::stringstream s;
		s << "Rotate " << (rotate.y * 180.0f / sx::pi) << " 0 1 0";
		m_WriteLine(s.str());
	}
	if (!sx::zero(rotate.x)) {
		std::stringstream s;
		s << "Rotate " << (rotate.x * 180.0f / sx::pi) << " 1 0 0";
		m_WriteLine(s.str());
	}

	if (outScale) {
		if (!sx::zero(scale.x - 1.0f) || !sx::zero(scale.y - 1.0f) || !sx::zero(scale.z - 1.0f)) {
			std::stringstream s;
			s << "Scale " << scale.x << " " << scale.y << " " << scale.z;
			m_WriteLine(s.str());
		}
	}
}

/**
 * テクスチャ番号に対応するテクスチャ名を取得.
 */
std::string CSaveRIB::m_GetTextureName (const int textureIndex, const bool fileF)
{
	std::string textureName = "";

	for (int i = 0; i < m_RIBInfo.textureList.size(); i++) {
		CTextureInfo& textureInfo = m_RIBInfo.textureList[i];
		if (textureInfo.index == textureIndex) {
			textureName = textureInfo.fileName;
			break;
		}
	}

	// ファイルの拡張子をカット.
	if (!fileF && textureName.size() > 0) {
		const int pos = textureName.find_last_of(".");
		if (pos != std::string::npos) {
			textureName = textureName.substr(0, pos);
		}
	}

	return textureName;
}

/**
 * マスターサーフェスとしてのマテリアル情報を出力.
 */
void CSaveRIB::m_WriteMasterSurfaceMaterials (sxsdk::scene_interface* scene)
{
	m_MaterialList.clear();

	// master surfaceパートを取得.
	sxsdk::shape_class& rootShape = scene->get_shape();
	if (!rootShape.has_son()) return;

	sxsdk::shape_class* pMasterSurfacePart = NULL;
	sxsdk::shape_class* pS = rootShape.get_son();
	while (pS->has_bro()) {
		pS = pS->get_bro();
		if (!pS) break;
		if (pS->get_type() == sxsdk::enums::part) {
			if (pS->get_part().get_part_type() == sxsdk::enums::master_surface_part) {
				pMasterSurfacePart = pS;
				break;
			}
		}
	}
	if (!pMasterSurfacePart) return;
	if (!pMasterSurfacePart->has_son()) return;

	// master surfaceのマテリアル情報を取得.
	pS = pMasterSurfacePart->get_son();
	while (pS->has_bro()) {
		pS = pS->get_bro();
		if (!pS) break;
		if (pS->get_type() == sxsdk::enums::master_surface) {
			sxsdk::master_surface_class* pMasterSurface = pS->get_master_surface();
			m_MaterialList.push_back(CMaterialInfo());
			CMaterialInfo& material = m_MaterialList.back();
			material.SetMaterial(scene, *pMasterSurface);
		}
	}
	if (m_MaterialList.size() == 0) return;

	m_WriteLine("");
	m_WriteLine("# Material patterns --------");
	for (int i = 0; i < m_MaterialList.size(); i++) {
		CMaterialInfo& material = m_MaterialList[i];
		if (material.mappingLayerCount == 0) continue;

		// master surfaceにRenderMan用の属性が格納されている場合は格納.
		CRISMaterialInfo risMaterialInfo;
		if (StreamCtrl::HasRIBMaterial(*material.masterSurface)) {
			risMaterialInfo = StreamCtrl::LoadRIBMaterial(*material.masterSurface);
		}

		// カスタム情報を使用しない場合は、表面材質からのパラメータを格納.
		if (!risMaterialInfo.useCustom) {
			MaterialCtrl::ConvShade3DToRIS(material, risMaterialInfo);
		}

		{
			std::stringstream s;
			s << "# " << material.name;
			m_WriteLine(s.str());
		}

		// マテリアルのPattern情報を出力し、最終的なPatern名を保持.
		std::string diffuseFirst = "";
		material.ribDiffusePatternName = m_WriteMaterialRGB(material.name, "diffuse", material.diffuseLayer, material.diffuseColor, diffuseFirst);
		material.ribNormalPatternName  = m_WriteMaterialNormal(material.name, "normal", material.normalLayer);

		// 「アルファ透明」も加味した透明度データ.
		std::string alphaTransTexName = "";
		if (material.transparentAlpha) alphaTransTexName = diffuseFirst;
		material.ribTrimPatternName = m_WriteMaterialTrim(material.name, "trim", material.trimLayer, alphaTransTexName);

		m_WriteLine("");
	}

	m_WriteLine("");
}

/**
 * マテリアルの出力開始.
 * 形状の情報を出力する前に呼ぶこと.
 */
void CSaveRIB::m_BeginWriteMaterial (sxsdk::scene_interface* scene, sxsdk::shape_class& shape)
{
	sxsdk::master_surface_class* pMasterSurface = shape.get_master_surface();
	m_BeginWriteMaterial(scene, &shape, pMasterSurface);
}

void CSaveRIB::m_BeginWriteMaterial (sxsdk::scene_interface* scene, sxsdk::master_surface_class& masterSurface)
{
	m_BeginWriteMaterial(scene, NULL, &masterSurface);
}

void CSaveRIB::m_BeginWriteMaterial (sxsdk::scene_interface* scene, sxsdk::shape_class* shape, sxsdk::master_surface_class* masterSurface)
{
	// 表面材質が存在する親までたどる.
	sxsdk::shape_class* shape2 = shape;
	while (shape2) {
		if (shape2->has_surface()) break;
		if (!shape2->has_dad()) {
			shape2 = NULL;
			break;
		}
		shape2 = shape2->get_dad();
	}
	if (shape2 == NULL) shape2 = shape;

	// マテリアル情報を読み込み.
	sxsdk::master_surface_class* pCurrentMasterSurface = masterSurface;
	if (!pCurrentMasterSurface) {
		if (shape2 && shape2->get_master_surface()) {
			pCurrentMasterSurface = shape2->get_master_surface();
		}
	}

	CMaterialInfo material;
	if (masterSurface) material.SetMaterial(scene, *masterSurface);
	else material.SetMaterial(scene, *shape2);

	CRISMaterialInfo risMaterialInfo;
	if (pCurrentMasterSurface) {
		if (StreamCtrl::HasRIBMaterial(*pCurrentMasterSurface)) {
			risMaterialInfo = StreamCtrl::LoadRIBMaterial(*pCurrentMasterSurface);
		}
	}
	if (!risMaterialInfo.useCustom) {
		MaterialCtrl::ConvShade3DToRIS(material, risMaterialInfo);
	}

	//-------------------------------------------------.
	// RISとしてマテリアル情報を出力.
	//-------------------------------------------------.
	m_WriteLine("AttributeBegin");		// マテリアルの開始.

	// 影表現するか.
	{
		std::stringstream s;
		s << "Attribute \"visibility\" \"int transmission\" [" << (int)((material.noShadow) ? 0 : 1) << "]";
		m_WriteLine(s.str().c_str());
	}

	// 反射に対応するか.
	{
		std::stringstream s;
		s << "Attribute \"visibility\" \"int diffuse\" [" << (int)(risMaterialInfo.visibilityDiffuse ? 1 : 0) << "]";
		m_WriteLine(s.str().c_str());
	}
	{
		std::stringstream s;
		s << "Attribute \"visibility\" \"int specular\" [" << (risMaterialInfo.visibilitySpecular ? 1 : 0) << "]";
		m_WriteLine(s.str().c_str());
	}

	// 間接照明を処理するかどうか.
	{
		std::stringstream s;
		s << "Attribute \"visibility\" \"int indirect\" [" << (risMaterialInfo.visibilityIndirect ? 1 : 0) << "]";
		m_WriteLine(s.str().c_str());
	}

	// 可視かどうか.
	{
		std::stringstream s;
		s << "Attribute \"visibility\" \"int camera\" [" << (risMaterialInfo.visibilityCamera ? 1 : 0) << "]";
		m_WriteLine(s.str().c_str());
	}

	if (risMaterialInfo.useDepth) {
		// 拡散反射と鏡面反射の最大.
		{
			std::stringstream s;
			s << "Attribute \"trace\" " << "\"int maxdiffusedepth\" [" << risMaterialInfo.maxdiffusedepth << "] \"int maxspeculardepth\" [" << risMaterialInfo.maxspeculardepth << "]";
			m_WriteLine(s.str());
		}
	}

	const std::string materialName = material.name;

	// テクスチャの繰り返しや色反転が存在する場合は、パターンを再度出力.
	std::string diffusePatternName = "";
	std::string normalPatternName  = "";
	std::string trimPatternName    = "";

	if (pCurrentMasterSurface) {
		// すでに出力したmaster surface情報から参照.
		for (int i = 0; i < m_MaterialList.size(); i++) {
			CMaterialInfo& material = m_MaterialList[i];
			if (material.masterSurface == pCurrentMasterSurface) {
				diffusePatternName = material.ribDiffusePatternName;
				normalPatternName  = material.ribNormalPatternName;
				trimPatternName    = material.ribTrimPatternName;
				break;
			}
		}
	} else {
		std::string diffuseFirst = "";
		diffusePatternName = m_WriteMaterialRGB(materialName, "diffuse", material.diffuseLayer, material.diffuseColor, diffuseFirst);
		normalPatternName  = m_WriteMaterialNormal(materialName, "normal", material.normalLayer);

		std::string alphaTransTexName = "";
		if (material.transparentAlpha) alphaTransTexName = diffuseFirst;
		trimPatternName    = m_WriteMaterialTrim(materialName, "trim", material.trimLayer, alphaTransTexName);
	}

	switch (risMaterialInfo.type) {
	case RIBParam::pxrDiffuse:
		{
			const sxsdk::rgb_class diffuseCol       = m_CalcLinearColor(risMaterialInfo.pxrDiffuse.diffuseColor);
			const sxsdk::rgb_class transmissionCol  = m_CalcLinearColor(risMaterialInfo.pxrDiffuse.transmissionColor);

			if (diffusePatternName.size() > 0) {
				std::stringstream s;
				s << "Bxdf \"PxrDiffuse\" \"" << materialName << "\" \"reference color diffuseColor\" [\"" << diffusePatternName << ":resultRGB\"]";
				m_WriteLine(s.str().c_str());
			} else {
				std::stringstream s;
				s << "Bxdf \"PxrDiffuse\" \"" << materialName << "\" \"color diffuseColor\" [" << diffuseCol.red << " " << diffuseCol.green << " " << diffuseCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}

			if (trimPatternName.size() > 0) {
				std::stringstream s;
				s << "  \"reference float presence\" [\"" << trimPatternName << ":resultR\"]";
				m_WriteLine(s.str().c_str());
			}

			if (!::CheckColorBlack(transmissionCol)) {
				std::stringstream s;
				s << "  \"color transmissionColor\" [" << transmissionCol.red << " " << transmissionCol.green << " " << transmissionCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}

			if (normalPatternName.size() > 0) {
				std::stringstream s;
				s << "  \"reference normal bumpNormal\" [\"" << normalPatternName << ":resultN\"]";
				m_WriteLine(s.str().c_str());
			}
		}
		break;

	case RIBParam::pxrDisney:
		{
			const sxsdk::rgb_class baseCol   = m_CalcLinearColor(risMaterialInfo.pxrDisney.baseColor);
			const sxsdk::rgb_class emitCol   = m_CalcLinearColor(risMaterialInfo.pxrDisney.emitColor);

			if (diffusePatternName.size() > 0) {
				std::stringstream s;
				s << "Bxdf \"PxrDisney\" \"" << materialName << "\" \"reference color baseColor\" [\"" << diffusePatternName << ":resultRGB\"]";
				m_WriteLine(s.str().c_str());
			} else {
				std::stringstream s;
				s << "Bxdf \"PxrDisney\" \"" << materialName << "\" \"color baseColor\" [" << baseCol.red << " " << baseCol.green << " " << baseCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}

			// アルファ透明使用時は、Alpha値を参照.
			if (trimPatternName.size() > 0) {
				std::stringstream s;
				s << "  \"reference float presence\" [\"" << trimPatternName << ":resultR\"]";
				m_WriteLine(s.str().c_str());
			}

			if (!::CheckColorBlack(emitCol)) {
				std::stringstream s;
				s << "  \"color emitColor\" [" << emitCol.red << " " << emitCol.green << " " << emitCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}
			{
				std::stringstream s;
				s << "  \"float metallic\" [" << risMaterialInfo.pxrDisney.metallic << "]";
				m_WriteLine(s.str().c_str());
			}
			{
				std::stringstream s;
				s << "  \"float roughness\" [" << risMaterialInfo.pxrDisney.roughness << "]";
				m_WriteLine(s.str().c_str());
			}
			{
				std::stringstream s;
				s << "  \"float anisotropic\" [" << risMaterialInfo.pxrDisney.anisotropic << "]";
				m_WriteLine(s.str().c_str());
			}
			{
				std::stringstream s;
				s << "  \"float specular\" [" << risMaterialInfo.pxrDisney.specular << "]";
				m_WriteLine(s.str().c_str());
			}
			if (!sx::zero(risMaterialInfo.pxrDisney.specularTint)) {
				std::stringstream s;
				s << "  \"float specularTint\" [" << risMaterialInfo.pxrDisney.specularTint << "]";
				m_WriteLine(s.str().c_str());
			}

			if (!sx::zero(risMaterialInfo.pxrDisney.subsurface)) {
				std::stringstream s;
				s << "  \"float subsurface\" [" << risMaterialInfo.pxrDisney.subsurface << "]";
				m_WriteLine(s.str().c_str());
			}
			if (!::CheckColorBlack(risMaterialInfo.pxrDisney.subsurfaceColor)) {
				std::stringstream s;
				const sxsdk::rgb_class subsurfaceCol = m_CalcLinearColor(risMaterialInfo.pxrDisney.subsurfaceColor);
				s << "  \"color subsurfaceColor\" [" << subsurfaceCol.red << " " << subsurfaceCol.green << " " << subsurfaceCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}

			if (normalPatternName.size() > 0) {
				std::stringstream s;
				s << "  \"reference normal bumpNormal\" [\"" << normalPatternName << ":resultN\"]";
				m_WriteLine(s.str().c_str());
			}
		}
		break;

	case RIBParam::pxrGlass:
		{
			const sxsdk::rgb_class reflectionCol   = m_CalcLinearColor(risMaterialInfo.pxrGlass.reflectionColor);
			const sxsdk::rgb_class transCol        = m_CalcLinearColor(risMaterialInfo.pxrGlass.transmissionColor);
			const sxsdk::rgb_class absorptionCol   = m_CalcLinearColor(risMaterialInfo.pxrGlass.absorptionColor);

			if (diffusePatternName.size() > 0) {
				std::stringstream s;
				s << "Bxdf \"PxrGlass\" \"" << materialName << "\" \"reference color reflectionColor\" [\"" << diffusePatternName << ":resultRGB\"]";
				m_WriteLine(s.str().c_str());
			} else {
				std::stringstream s;
				s << "Bxdf \"PxrGlass\" \"" << materialName << "\" \"color reflectionColor\" [" << reflectionCol.red << " " << reflectionCol.green << " " << reflectionCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}

			if (!sx::zero(risMaterialInfo.pxrGlass.reflectionGain - 1.0f)) {
				std::stringstream s;
				s << "  \"float reflectionGain\" [" << risMaterialInfo.pxrGlass.reflectionGain << "]";
				m_WriteLine(s.str().c_str());
			}

			{
				std::stringstream s;
				s << "  \"float ior\" [" << risMaterialInfo.pxrGlass.ior << "]";
				m_WriteLine(s.str().c_str());
			}
			{
				std::stringstream s;
				s << "  \"float roughness\" [" << risMaterialInfo.pxrGlass.roughness << "]";
				m_WriteLine(s.str().c_str());
			}

			{
				std::stringstream s;
				s << "  \"color transmissionColor\" [" << transCol.red << " " << transCol.green << " " << transCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}

			if (!sx::zero(risMaterialInfo.pxrGlass.transmissionGain - 1.0f)) {
				std::stringstream s;
				s << "  \"float transmissionGain\" [" << risMaterialInfo.pxrGlass.transmissionGain << "]";
				m_WriteLine(s.str().c_str());
			}

			if (!sx::zero(risMaterialInfo.pxrGlass.absorptionGain)) {
				std::stringstream s;
				s << "  \"float absorptionGain\" [" << risMaterialInfo.pxrGlass.absorptionGain << "]";
				m_WriteLine(s.str().c_str());
			}

			if (!::CheckColorWhite(absorptionCol)) {
				std::stringstream s;
				s << "  \"color absorptionColor\" [" << absorptionCol.red << " " << absorptionCol.green << " " << absorptionCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}
			if (normalPatternName.size() > 0) {
				std::stringstream s;
				s << "  \"reference normal bumpNormal\" [\"" << normalPatternName << ":resultN\"]";
				m_WriteLine(s.str().c_str());
			}
		}

		break;

	case RIBParam::pxrConstant:
		{
			const sxsdk::rgb_class emitCol = m_CalcLinearColor(risMaterialInfo.pxrConstant.emitColor);

			if (diffusePatternName.size() > 0) {
				std::stringstream s;
				s << "Bxdf \"PxrConstant\" \"" << materialName << "\" \"reference color emitColor\" [\"" << diffusePatternName << ":resultRGB\"]";
				m_WriteLine(s.str().c_str());
			} else {
				std::stringstream s;
				s << "Bxdf \"PxrConstant\" \"" << materialName << "\" \"color emitColor\" [" << emitCol.red << " " << emitCol.green << " " << emitCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}
		}
		break;

	case RIBParam::pxrVolume:
		{
			const sxsdk::rgb_class diffuseCol  = m_CalcLinearColor(risMaterialInfo.pxrVolume.diffuseColor);
			const sxsdk::rgb_class emitCol     = m_CalcLinearColor(risMaterialInfo.pxrVolume.emitColor);
			const sxsdk::rgb_class densityCol  = m_CalcLinearColor(risMaterialInfo.pxrVolume.densityColor);

			if (diffusePatternName.size() > 0) {
				std::stringstream s;
				s << "Bxdf \"PxrVolume\" \"" << materialName << "\" \"reference color diffuseColor\" [\"" << diffusePatternName << ":resultRGB\"]";
				m_WriteLine(s.str().c_str());
			} else {
				std::stringstream s;
				s << "Bxdf \"PxrVolume\" \"" << materialName << "\" \"color diffuseColor\" [" << diffuseCol.red << " " << diffuseCol.green << " " << diffuseCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}

			if (!::CheckColorBlack(emitCol)) {
				std::stringstream s;
				s << "  \"color emitColor\" [" << emitCol.red << " " << emitCol.green << " " << emitCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}
			if (!::CheckColorWhite(densityCol)) {
				std::stringstream s;
				s << "  \"color densityColor\" [" << densityCol.red << " " << densityCol.green << " " << densityCol.blue << "]";
				m_WriteLine(s.str().c_str());
			}

			if (!sx::zero(risMaterialInfo.pxrVolume.densityFloat - 1.0f)) {
				std::stringstream s;
				s << "  \"float densityFloat\" [" << risMaterialInfo.pxrVolume.densityFloat << "]";
				m_WriteLine(s.str().c_str());
			}
			if (!sx::zero(risMaterialInfo.pxrVolume.densityScale - 1.0f)) {
				std::stringstream s;
				s << "  \"float densityScale\" [" << risMaterialInfo.pxrVolume.densityScale << "]";
				m_WriteLine(s.str().c_str());
			}
			if (!sx::zero(risMaterialInfo.pxrVolume.anisotropy)) {
				std::stringstream s;
				s << "  \"float anisotropy\" [" << risMaterialInfo.pxrVolume.anisotropy << "]";
				m_WriteLine(s.str().c_str());
			}
			if (!sx::zero(risMaterialInfo.pxrVolume.maxDensity - 1.0f)) {
				std::stringstream s;
				s << "  \"float maxDensity\" [" << risMaterialInfo.pxrVolume.maxDensity << "]";
				m_WriteLine(s.str().c_str());
			}

			if (risMaterialInfo.pxrVolume.multiScatter) {
				std::stringstream s;
				s << "  \"int multiScatter\" [" << 1 << "]";
				m_WriteLine(s.str().c_str());
			}
		}

		break;

	case RIBParam::pxrSkin:
		break;
	}
}

/**
 * マテリアル出力時に、テクスチャの繰り返しや色反転などが存在する場合のPxrManifold2D出力.
 * @param[in]  materialName  マスターサーフェス名.
 * @param[in]  typeName      マテリアルの種類（diffuse/bump/normal/trim）.
 * @param[in]  mappingLayer  マテリアルのマッピングレイヤリスト.
 * @param[in]  layerIndex    マテリアルのマッピングレイヤ番号.
 * @return RGBを持つパターン名.
 */
std::string CSaveRIB::m_WriteMaterialTexture (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer, const int layerIndex, const bool isBumpNormal)
{
	if (mappingLayer.size() == 0) return "";
	const CMaterialMappingLayerInfo& layerInfo = mappingLayer[layerIndex];
	if (layerInfo.textureIndex < 0) return "";

	// テクスチャファイル名からテクスチャ名を取得（ファイル拡張子はカット）.
	const std::string textureName = m_GetTextureName(layerInfo.textureIndex, false);

	std::string retName = textureName;
	if (layerInfo.repeatX == 1 && layerInfo.repeatY == 1 && !layerInfo.flipColor) return retName;

	const std::string patternName = materialName + std::string("_") + typeName;

	std::string manifoldName = "";
	{
		std::stringstream s;
		s << patternName << "_" << layerIndex << "_manifold";
		manifoldName = s.str();
	}
	{
		std::stringstream s;
		s << patternName << "_" << layerIndex << "_texture";
		retName = s.str();
	}

	{
		std::stringstream s;
		s << "Pattern \"PxrManifold2D\" \"" << manifoldName << "\" \"float scaleS\" [" << layerInfo.repeatX << "] \"float scaleT\" [" << layerInfo.repeatY << "]";
		s << " \"int invertT\" [0]";
		m_WriteLine(s.str().c_str());
	}

	{
		std::stringstream s;
		s << "Pattern \"PxrTexture\" \"" << retName << "\" \"string filename\" [\"" << textureName << ".tiff\"] \"int linearize\" [1]";
		s << " \"int invertT\" [0]";
		s << " \"reference struct manifold\" [\"" << manifoldName << ":result" << "\"]";
		m_WriteLine(s.str());
	}

	if (!layerInfo.flipColor) return retName;

	// 色反転.
	const std::string invertName = retName + std::string("_invert");
	{
		std::stringstream s;
		s << "Pattern \"PxrInvert\" \"" << invertName << "\" \"reference color inputRGB\" [\"" << retName << ":resultRGB\"]";
		m_WriteLine(s.str().c_str());
	}

	return invertName;
}


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
std::string CSaveRIB::m_WriteMaterialFractal (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer, const int layerIndex, const sxsdk::rgb_class baseColor, const std::string baseTexName, const bool isBumpNormal)
{
	const CMaterialMappingLayerInfo& layerInfo = mappingLayer[layerIndex];
	std::string retMixName = "";
	std::string fractalName = "";
	std::string bumpName = "";

	const std::string patternName = materialName + std::string("_") + typeName;

	const float defaultF   = 0.008f;		// 値が大きいほど細かい.
	const float size       = std::max(0.00001f, layerInfo.size);
	const float frequency  = defaultF / size;
	sxsdk::rgb_class col   = layerInfo.color;

	{
		std::stringstream s;
		s << patternName << "_" << layerIndex << "_fractal";
		fractalName = s.str();
	}
	{
		std::stringstream s;
		s << patternName << "_" << layerIndex << "_mix";
		retMixName = s.str();
	}

	{
		std::stringstream s;
		s << "Pattern \"PxrFractal\" \"" << fractalName << "\" \"float frequency\" [" << frequency << "]";
		m_WriteLine(s.str());
	}
	if (layerInfo.patternType == sxsdk::enums::spotted_pattern) {
		std::stringstream s;
		s << "  \"int layers\" [" << 3 << "]";
		m_WriteLine(s.str());
	}

	if (baseTexName.size() == 0) {
		std::stringstream s;
		s << "Pattern \"PxrMix\" \"" << retMixName << "\" \"color color1\" [" << baseColor.red << " " << baseColor.green << " " << baseColor.blue << "]";
		m_WriteLine(s.str());
	} else {
		std::stringstream s;
		s << "Pattern \"PxrMix\" \"" << retMixName << "\" \"reference color color1\" [\"" << baseTexName << ":resultRGB\"]";
		m_WriteLine(s.str());
	}

	{
		std::stringstream s;
		s << "  \"color color2\" [" << layerInfo.color.red << " " << layerInfo.color.green << " " << layerInfo.color.blue << "]";
		m_WriteLine(s.str());
	}
	{
		std::stringstream s;
		s << "  \"reference float mix\" [\"" << fractalName << ":resultF\"]";
		m_WriteLine(s.str());
	}

	if (isBumpNormal || sx::zero(layerInfo.weight - 1.0f)) return retMixName;

	// layerInfo.weightが1.0ではない場合.
	std::string mixName2 = "";
	{
		std::stringstream s;
		s << patternName << "_" << layerIndex << "_mix2";
		mixName2 = s.str();
	}

	if (baseTexName.size() == 0) {
		std::stringstream s;
		s << "Pattern \"PxrMix\" \"" << mixName2 << "\" \"color color1\" [" << baseColor.red << " " << baseColor.green << " " << baseColor.blue << "]";
		m_WriteLine(s.str());
	} else {
		std::stringstream s;
		s << "Pattern \"PxrMix\" \"" << mixName2 << "\" \"reference color color1\" [\"" << baseTexName << ":resultRGB\"]";
		m_WriteLine(s.str());
	}
	{
		std::stringstream s;
		s << "  \"reference color color2\" [\"" << retMixName << ":resultRGB\"]";
		m_WriteLine(s.str());
	}
	{
		std::stringstream s;
		s << "  \"float mix\" [" << layerInfo.weight << "]";
		m_WriteLine(s.str());
	}

	return mixName2;
}

/**
 * マルチレイヤに対応したマテリアルの出力（Diffuse/Trim）.
 */
std::string CSaveRIB::m_WriteMaterialRGB (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer, const sxsdk::rgb_class baseColor, std::string& retDiffuseFirst)
{
	const std::string patternName = materialName + "_" + typeName;

	retDiffuseFirst = "";

	const int layerCou = mappingLayer.size();
	std::vector<std::string> textureList;
	int mixCount = 0;
	for (int i = 0; i < layerCou; i++) {
		const CMaterialMappingLayerInfo& layerInfo = mappingLayer[i];

		// Pattern情報として出力.
		std::string texName = "";
		if (layerInfo.patternType == sxsdk::enums::image_pattern && layerInfo.textureIndex >= 0) {
			// イメージテクスチャの場合.
			texName = m_WriteMaterialTexture(materialName, typeName, mappingLayer, i);
			if (textureList.size() == 0) {
				textureList.push_back(texName);
				if (retDiffuseFirst.size() == 0) retDiffuseFirst = texName;
				continue;
			}

		} else if (layerInfo.patternType == sxsdk::enums::spotted_pattern || layerInfo.patternType == sxsdk::enums::cloud_pattern) {
			// spot/cloudのProcedual Textureの場合.
			std::string baseTexName = (textureList.size() > 0) ? textureList[0] : "";
			texName = m_WriteMaterialFractal(materialName, typeName, mappingLayer, i, baseColor, baseTexName);
			textureList.clear();
			textureList.push_back(texName);
			continue;
		}

		if (texName.size() > 0 && textureList.size() >= 1) {
			// テクスチャを合成.
			{
				std::string mixName = "";
				{
					std::stringstream s;
					s << patternName << "_mix_" << mixCount;
					mixName = s.str();
				}

				{
					std::stringstream s;
					s << "Pattern \"PxrMix\" \"" << mixName << "\" \"reference color color1\" [\"" << textureList[0] << ":resultRGB\"]";
					m_WriteLine(s.str());
				}
				{
					std::stringstream s;
					s << "  \"reference color color2\" [\"" << texName << ":resultRGB\"]";
					m_WriteLine(s.str());
				}
				{
					std::stringstream s;
					s << "  \"float mix\" [" << layerInfo.weight << "]";
					m_WriteLine(s.str());
				}
				textureList.clear();
				textureList.push_back(mixName);

				mixCount++;
			}
		}
	}

	return (textureList.size() == 0) ? "" : textureList[0];
}

/**
 * マルチレイヤに対応したマテリアルの出力（Trim）.
 */
std::string CSaveRIB::m_WriteMaterialTrim (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer, const std::string diffuseTextureName)
{
	const std::string patternName = materialName + "_" + typeName;

	// トリムとしてのアルファ情報を取得.
	std::string diffuseFirst;
	std::string retName = m_WriteMaterialRGB(materialName, typeName, mappingLayer, sxsdk::rgb_class(1, 1, 1), diffuseFirst);
	if (diffuseTextureName.size() == 0) return retName;

	const std::string trimName = patternName + "_" + "mix_alpha";

	if (retName.size() == 0) {
		std::stringstream s;
		s << "Pattern \"PxrMix\" \"" << trimName << "\" \"color color1\" [0 0 0]";
		m_WriteLine(s.str());
	} else {
		std::stringstream s;
		s << "Pattern \"PxrMix\" \"" << trimName << "\" \"reference color color1\" [\"" << retName << ":resultRGB\"]";
		m_WriteLine(s.str());
	}
	{
		std::stringstream s;
		s << "  \"color color2\" [1 1 1]";
		m_WriteLine(s.str());
	}
	{
		std::stringstream s;
		s << "  \"reference float mix\" [\"" << diffuseTextureName << ":resultA\"]";
		m_WriteLine(s.str());
	}

	return trimName;
}

/**
 * マルチレイヤに対応したマテリアルの出力（Bump or Normal）.
 */
std::string CSaveRIB::m_WriteMaterialNormal (const std::string& materialName, const std::string typeName, const std::vector<CMaterialMappingLayerInfo>& mappingLayer)
{
	const std::string patternName = materialName + "_" + typeName;

	const int layerCou = mappingLayer.size();
	std::vector<std::string> textureList;
	int crossCount = 0;
	for (int i = 0; i < layerCou; i++) {
		const CMaterialMappingLayerInfo& layerInfo = mappingLayer[i];

		// Pattern情報として出力.
		std::string texName  = "";
		std::string texName2 = "";
		if (layerInfo.patternType == sxsdk::enums::image_pattern && layerInfo.textureIndex >= 0) {
			// イメージテクスチャの場合.
			texName  = m_WriteMaterialTexture(materialName, typeName, mappingLayer, i, true);
			texName2 = texName + "_bump";

			if (!layerInfo.normalMap) {
				// bump mapの場合.
				const float scale = 10.0f * layerInfo.weight;
				std::stringstream s;
				s << "Pattern \"PxrBump\" \"" << texName2 << "\" \"reference float inputBump\" [\"" << texName << ":resultR\"] \"float scale\" [" << scale << "] \"int invertT\" [0]"; 
				m_WriteLine(s.str());

			} else {
				// normal mapの場合.
				const float scale = 1.0f * layerInfo.weight;
				std::stringstream s;
				s << "Pattern \"PxrNormalMap\" \"" << texName2 << "\" \"color inputRGB\" [\"" << texName << ":resultRGB\"] \"float scale\" [" << scale << "]";
				m_WriteLine(s.str());
			}
			
		} else if (layerInfo.patternType == sxsdk::enums::spotted_pattern || layerInfo.patternType == sxsdk::enums::cloud_pattern) {
			// spot/cloudのProcedual Textureの場合.
			texName  = m_WriteMaterialFractal(materialName, typeName, mappingLayer, i, sxsdk::rgb_class(1, 1, 1), "", true);
			if (texName.size() == 0) continue;

			texName2 = texName + "_bump";
			{
				const float scale = 10.0f * layerInfo.weight;
				std::stringstream s;
				s << "Pattern \"PxrBump\" \"" << texName2 << "\" \"reference float inputBump\" [\"" << texName << ":resultR\"] \"float scale\" [" << scale << "] \"int invertT\" [0]"; 
				m_WriteLine(s.str());
			}
		}

		if (texName2.size() > 0) {
			// 法線の合成.
			const std::string prevTexName = (textureList.size() == 0) ? "" : textureList[0];
			if (prevTexName.size() > 0) {
				if (layerInfo.normalMap) {
					std::stringstream s;
					s << "  \"reference normal bumpOverlay\" [\"" << prevTexName << ":resultN\"]"; 
					m_WriteLine(s.str());
				} else {
					std::stringstream s;
					s << "  \"reference normal inputN\" [\"" << prevTexName << ":resultN\"]"; 
					m_WriteLine(s.str());
				}
			}

			textureList.clear();
			textureList.push_back(texName2);
		}
	}
	return (textureList.size() == 0) ? "" : textureList[0];
}

/**
 * マテリアルの出力終了.
 */
void CSaveRIB::m_EndWriteMaterial ()
{
	m_WriteLine("AttributeEnd");
}

/**
 * 光源の出力.
 */
void CSaveRIB::m_WriteLights (sxsdk::scene_interface* scene)
{
	// 光源情報をシーンより取得.
	m_lightCtrl.StoreLights(scene);

	m_WriteLine("");
	m_WriteLine("# Lights --------");

	// 背景のIBLを出力.
	if (m_backgroundTextureName.size() > 0) {
		m_WriteLine("AttributeBegin");
		m_indent++;

		m_WriteLine("TransformBegin");
		m_indent++;

		m_WriteLine("Attribute \"identifier\" \"string name\" [\"envLight\"]");
		m_WriteLine("Translate 0 0 0");
		m_WriteLine("Rotate 0 0 0 1");
		m_WriteLine("Rotate -90 0 1 0");
		m_WriteLine("Rotate 90 1 0 0");
		m_WriteLine("Scale -1 1 -1");
      
		{
			const float intensity = m_RIBInfo.backgroundImageIntensity;
			std::stringstream s;
			s << "AreaLightSource \"PxrEnvMapLight\" \"envLight\" \"string envmap\" [\"" << m_backgroundTextureName << ".tiff\"] " << "\"float intensity\" [" << intensity << "]";
			m_WriteLine(s.str());
		}
		{
			std::stringstream s;
			s << "Attribute \"visibility\" \"int indirect\" [0] \"int camera\" [" << (int)(m_RIBInfo.backgroundDraw ? 1 : 0) << "]";
			m_WriteLine(s.str());
		}
		m_WriteLine("ReverseOrientation");
		m_WriteLine("Opacity [1 1 1]");
		m_WriteLine("Sides 1");
		if (m_RIBInfo.backgroundDraw) {
			std::stringstream s;
			s << "Bxdf \"PxrConstant\" \"background_geom\" \"reference color emitColor\" [\"" << m_backgroundTextureName << ":resultRGB\"]";
			m_WriteLine(s.str());
		}
		m_WriteLine("Geometry \"envsphere\" \"constant float radius\" [-1] \"constant int infinite\" [1065353216] \"constant float[2] resolution\" [-1 -1]");

		m_indent--;
		m_WriteLine("TransformEnd");
		m_indent--;
		m_WriteLine("AttributeEnd");
		m_WriteLine("");
	}

	bool useSunLight = false;
	for (int loop = 0; loop < m_lightCtrl.GetLightsCount(); loop++) {
		CLightInfo lightInfo = m_lightCtrl.GetLightInfo(loop);

		const int index = loop + 1;

		double intensity = lightInfo.intensity;
		double ambient   = lightInfo.ambient;

		// 無限遠光源でのDayLight.
		// Physical Skyの日時情報も持ってくる.
		if (m_RIBInfo.lightDayLight && lightInfo.lightType == light_type_distant && !useSunLight) {
			useSunLight = true;
			m_WriteLine("AttributeBegin");
			m_indent++;

			m_WriteLine("TransformBegin");
			m_indent++;

			m_WriteLine("Translate 0 0 0");
			m_WriteLine("Rotate 0 0 0 1");
			m_WriteLine("Rotate 0 0 1 0");
			m_WriteLine("Rotate 90 1 0 0");

			{
				std::stringstream s;
				s << "Attribute \"visibility\" \"int indirect\" [0] \"int transmission\" [0] \"int camera\" [0]";
				m_WriteLine(s.str());
			}

			{
				std::stringstream s;
				s << "AreaLightSource \"PxrEnvDayLight\" \"envDayLightHandle\"";
				m_WriteLine(s.str());
			}

			{
				std::stringstream s;
				s << "  \"vector direction\" [" << -lightInfo.direction.x << " " << -lightInfo.direction.y << " " << lightInfo.direction.z << "]";
				m_WriteLine(s.str());
			}

			// Physical Skyの情報を反映.
			try {
				compointer<sxsdk::distant_light_interface> dLight(scene->get_distant_light_interface());
				if (dLight) {
					sxsdk::physical_sky_class& physical_sky = dLight->physical_sky();
					if (physical_sky.get_use_sun()) {
						const int month       = physical_sky.get_datetime_month();
						const int day         = physical_sky.get_datetime_day();
						const int hour        = physical_sky.get_datetime_hour();
						const int minutes     = physical_sky.get_datetime_minutes();
						const int sec         = physical_sky.get_datetime_second();
						const float utc       = physical_sky.get_utc();
						const float latitude  = physical_sky.get_latitude();
						const float longitude = physical_sky.get_longitude();

						{
							std::stringstream s;
							s << "  \"float month\" [" << (month - 1) << "]";
							m_WriteLine(s.str());
						}
						{
							std::stringstream s;
							s << "  \"float day\" [" << day << "]";
							m_WriteLine(s.str());
						}
						{
							float hourV = (float)hour + ((float)minutes / 60.0f) + ((float)sec / (60.0f * 60.0f));
							std::stringstream s;
							s << "  \"float hour\" [" << hour << "]";
							m_WriteLine(s.str());
						}
						{
							std::stringstream s;
							s << "  \"float zone\" [" << utc << "]";
							m_WriteLine(s.str());
						}
						{
							std::stringstream s;
							s << "  \"float latitude\" [" << latitude << "]";
							m_WriteLine(s.str());
						}
						{
							std::stringstream s;
							s << "  \"float longitude\" [" << longitude << "]";
							m_WriteLine(s.str());
						}
					}
				}
			} catch (...) { }

			m_WriteLine("Geometry \"envsphere\" \"constant float radius\" [-1] \"constant int infinite\" [1065353216] \"constant float[2] resolution\" [-1 -1]");

			m_indent--;
			m_WriteLine("TransformEnd");

			m_indent--;
			m_WriteLine("AttributeEnd");
		}

		// 面光源以外はスキップ.
		if (lightInfo.lightType != light_type_area) continue;

		// PxrAreaLightとしての面光源情報を取得 (streamに情報が保持されている).
		CPxrAreaLight pxrAreaLightInfo;
		if (lightInfo.shape) {
			CLightInfo tmpLInfo = LightCtrl::GetAreaLightInfo(*lightInfo.shape);			// 光源情報を取得.
			LightCtrl::ConvAreaLightShade3DToRIS(shade, tmpLInfo, pxrAreaLightInfo);		// RenderMan向けのパラメータに変換.
			if (StreamCtrl::HasRIBAreaLight(*lightInfo.shape)) {
				pxrAreaLightInfo = StreamCtrl::LoadRIBAreaLight(*lightInfo.shape);			// streamよりPxrAreaLightの光源情報を読み込み.
			}
		}

		// 面光源の場合は、AttributeBegin - AttributeEndで囲む必要あり.
		if (lightInfo.lightType == light_type_area || lightInfo.lightType == light_type_line) {
			m_WriteLine("AttributeBegin");
			m_indent++;

			{
				std::stringstream s;
				s << "Attribute \"visibility\" \"int indirect\" [0] ";					// 間接照明の反映 Off.
				s << "\"int transmission\" [0] ";										// 影の反映 Off.
				s << "\"int camera\" [" << (lightInfo.visible ? 1 : 0) << "]";			// 可視.
				m_WriteLine(s.str());
			}
		}

		if (lightInfo.lightType == light_type_ambient) {
			if (ambient > 0.0f) {
				std::stringstream s;
				s << "LightSource \"ambientlight\" " << index << " \"intensity\" [" << ambient << "]";
				m_WriteLine(s.str());
			}
			continue;
		}

		if (lightInfo.lightType == light_type_distant) {
			std::stringstream s;

			s << "LightSource \"";
			if (sx::zero(lightInfo.shadowValue)) s << "distant";
			else s << "shadowdistant";
			s << "\" " << index << " \"to\" [" << lightInfo.direction.x << " " << lightInfo.direction.y << " " << -lightInfo.direction.z << "]";
			m_WriteLine(s.str());
			intensity = intensity * sx::pi;
			ambient *= intensity;

		} else if (lightInfo.lightType == light_type_point) {
			std::stringstream s;

			s << "LightSource \"";
			if (sx::zero(lightInfo.shadowValue)) s << "pointlight";
			else s << "shadowpoint";
			s << "\" " << index << " \"from\" [" << lightInfo.pos.x << " " << lightInfo.pos.y << " " << -lightInfo.pos.z << "]";
			m_WriteLine(s.str());
			intensity = std::pow(intensity, 2.0) * sx::pi;

		} else if (lightInfo.lightType == light_type_spot) {
			std::stringstream s;

			s << "LightSource \"";
			if (sx::zero(lightInfo.shadowValue)) s << "spotlight";
			else s << "shadowspot";
			s << "\" " << index;

			const sxsdk::vec3 targetV = lightInfo.pos - lightInfo.direction * (float)intensity;
			s << " \"from\" [" << lightInfo.pos.x << " " << lightInfo.pos.y << " " << -lightInfo.pos.z << "]";
			s << " \"to\" [" << targetV.x << " " << targetV.y << " " << -targetV.z << "]";

			m_WriteLine(s.str());
			intensity = std::pow(intensity, 2.0) * sx::pi;
		
		} else if (lightInfo.lightType == light_type_area) {
			std::stringstream s;
#if USE_PRMAN_RIS
			s << "AreaLightSource \"PxrAreaLight\" \"mylighthandle\"";
#else
			s << "AreaLightSource \"plausibleArealight\" \"mylighthandle\"";
#endif
			m_WriteLine(s.str());
		}

		if (lightInfo.lightType != light_type_area && lightInfo.lightType != light_type_line) {
			if (!sx::zero(lightInfo.shadowValue)) {
				if (lightInfo.lightType == light_type_distant || lightInfo.lightType == light_type_directional || lightInfo.lightType == light_type_spot) {
					std::stringstream s;
					s << "  \"string shadowname\" [\"raytrace\"]";
					m_WriteLine(s.str());
				} else {
					std::stringstream s;
					s << "  \"string sfpx\" [\"raytrace\"]\r\n";
					s << "  \"string sfpy\" [\"raytrace\"]\r\n";
					s << "  \"string sfpz\" [\"raytrace\"]\r\n";
					s << "  \"string sfnx\" [\"raytrace\"]\r\n";
					s << "  \"string sfny\" [\"raytrace\"]\r\n";
					s << "  \"string sfnz\" [\"raytrace\"]";
					m_WriteLine(s.str());
				}
			}

			// サンプリング数は速度に影響（デフォルトは16.0）.
			{
				std::stringstream s;
				s << "  \"float samples\" [" << 1.0 << "]";
				m_WriteLine(s.str());
			}

		} else {
#if !USE_PRMAN_RIS
			std::stringstream s;
			s << "  \"float maxSamples\" [" << 4.0 << "]";
			m_WriteLine(s.str());
#endif
			// 面光源の面積を計算.
			//const double area = MathUtil::CalcPolygonArea(shade, lightInfo.areaLightPos);
			//intensity = (intensity * intensity) * sx::pi / area;

			intensity       = pxrAreaLightInfo.intensity;
			lightInfo.color = pxrAreaLightInfo.lightColor;
		}

		const sxsdk::rgb_class lightCol = m_CalcLinearColor(lightInfo.color);

		{
			std::stringstream s;
			s << "  \"float intensity\" [" << intensity << "]";
			m_WriteLine(s.str());
		}
		{
			std::stringstream s;
			s << "  \"color lightcolor\" [" << lightCol.red << " " << lightCol.green << " " << lightCol.blue << "]";
			m_WriteLine(s.str());
		}

		if (lightInfo.lightType == light_type_spot) {
			// スポットライトの角度は、ラジアン表現.
			const  float angle      = (lightInfo.spotConeAngle * 0.5f) * sx::pi / 180.0f;
			const  float deltaAngle = (sx::pi - angle) * lightInfo.spotSoftness;
			{
				std::stringstream s;
				s << "  \"coneangle\" [" << angle + deltaAngle << "]";
				m_WriteLine(s.str());
			}
			{
				std::stringstream s;
				s << "  \"conedeltaangle\" [" << deltaAngle << "]";
				m_WriteLine(s.str());
			}
		}

		if (ambient > 0.0f) {
			std::stringstream s;
			s << "LightSource \"ambientlight\" " << index << " \"intensity\" [" << ambient << "]";
			m_WriteLine(s.str());
		}

		if (lightInfo.lightType == light_type_area || lightInfo.lightType == light_type_line) {
			if (!sx::zero(pxrAreaLightInfo.areaNormalize)) {
					std::stringstream s;
					s << "  \"float areaNormalize\" [" << pxrAreaLightInfo.areaNormalize << "]";
					m_WriteLine(s.str());
			}
			{
				const sxsdk::rgb_class col = m_CalcLinearColor(pxrAreaLightInfo.specAmount);
				if (!::CheckColorWhite(col)) {
					std::stringstream s;
					s << "  \"color specAmount\" [" << col.red << " " << col.green << " " << col.blue << "]";
					m_WriteLine(s.str());
				}
			}
			{
				const sxsdk::rgb_class col = m_CalcLinearColor(pxrAreaLightInfo.diffAmount);
				if (!::CheckColorWhite(col)) {
					std::stringstream s;
					s << "  \"color diffAmount\" [" << col.red << " " << col.green << " " << col.blue << "]";
					m_WriteLine(s.str());
				}
			}
			if (!sx::zero(pxrAreaLightInfo.coneAngle - 20.0f)) {
				std::stringstream s;
				s << "  \"float coneangle\" [" << pxrAreaLightInfo.coneAngle << "]";
				m_WriteLine(s.str());
			}
			if (!sx::zero(pxrAreaLightInfo.penumbraExponent)) {
				std::stringstream s;
				s << "  \"float penumbraexponent\" [" << pxrAreaLightInfo.penumbraExponent << "]";
				m_WriteLine(s.str());
			}
			if (!sx::zero(pxrAreaLightInfo.penumbraAngle - 5.0f)) {
				std::stringstream s;
				s << "  \"float penumbraangle\" [" << pxrAreaLightInfo.penumbraAngle << "]";
				m_WriteLine(s.str());
			}
			if (!sx::zero(pxrAreaLightInfo.profileRange - 180.0f)) {
				std::stringstream s;
				s << "  \"float profilerange\" [" << pxrAreaLightInfo.profileRange << "]";
				m_WriteLine(s.str());
			}
			if (!sx::zero(pxrAreaLightInfo.cosinePower - 90.0f)) {
				std::stringstream s;
				s << "  \"float cosinepower\" [" << pxrAreaLightInfo.cosinePower << "]";
				m_WriteLine(s.str());
			}
			if (!sx::zero(pxrAreaLightInfo.angularVisibility - 1.0f)) {
				std::stringstream s;
				s << "  \"float angularVisibility\" [" << pxrAreaLightInfo.angularVisibility << "]";
				m_WriteLine(s.str());
			}
			{
				const sxsdk::rgb_class col = m_CalcLinearColor(pxrAreaLightInfo.shadowColor);
				if (!::CheckColorBlack(col)) {
					std::stringstream s;
					s << "  \"color shadowColor\" [" << col.red << " " << col.green << " " << col.blue << "]";
					m_WriteLine(s.str());
				}
			}
			if (!sx::zero(pxrAreaLightInfo.traceShadows - 1.0f)) {
				std::stringstream s;
				s << "  \"float traceShadows\" [" << pxrAreaLightInfo.traceShadows << "]";
				m_WriteLine(s.str());
			}
			if (!sx::zero(pxrAreaLightInfo.adaptiveShadows - 1.0f)) {
				std::stringstream s;
				s << "  \"float adaptiveShadows\" [" << pxrAreaLightInfo.adaptiveShadows << "]";
				m_WriteLine(s.str());
			}
			m_WriteLine("Sides 1");

			if (lightInfo.visible) {
				std::stringstream s;
				s << "Bxdf \"PxrConstant\" \"area_light\" \"color emitColor\" [" << lightCol.red << " " << lightCol.green << " " << lightCol.blue << "]";
				m_WriteLine(s.str());
			}

			{
				std::stringstream s;
				s << "Polygon \"P\" [";

				const int vCou = lightInfo.areaLightPos.size();
				for (int i = 0; i < vCou; i++) {
					const sxsdk::vec3& v = lightInfo.areaLightPos[i];
					s << v.x << " " << v.y << " " << -v.z << "  ";
				}
				s << "]";
				m_WriteLine(s.str());
			}
		}

		if (lightInfo.lightType == light_type_area || lightInfo.lightType == light_type_line) {
			m_indent--;
			m_WriteLine("AttributeEnd");
		}
	}
	m_WriteLine("");

}

/**
 * 画像ファイルを保存.
 */
void CSaveRIB::m_OutputTextureFiles (sxsdk::scene_interface* scene)
{
	sxsdk::shape_class& rootShape = scene->get_shape();
	if (!rootShape.has_son()) return;

	m_RIBInfo.textureList.clear();

	sxsdk::shape_class* pImagePart = NULL;
	sxsdk::shape_class* pS = rootShape.get_son();
	while (pS->has_bro()) {
		pS = pS->get_bro();
		if (!pS) break;
		if (pS->get_type() == sxsdk::enums::part) {
			if (pS->get_part().get_part_type() == sxsdk::enums::master_image_part) {
				pImagePart = pS;
				break;
			}
		}
	}
	if (!pImagePart) return;
	if (!pImagePart->has_son()) return;

	pS = pImagePart->get_son();
	int index = 0;
	while (pS->has_bro()) {
		pS = pS->get_bro();
		if (!pS) break;
		m_OutputTexture(scene, pS, index);
		index++;
	}
}

/**
 * 背景をパノラマの画像ファイルとして保存.
 */
void CSaveRIB::m_OutputBackgroundTextureFile (sxsdk::scene_interface* scene)
{
	m_backgroundTextureName = "";
	if (!m_RIBInfo.outputBackgroundImage) return;

	CBackgroundTexture backTexture(shade);

	int texWidth  = 1024;
	int texHeight = 512;
	switch (m_RIBInfo.backgroundImageSize) {
	case RIBParam::size_256x128:
		texWidth  = 256;
		texHeight = 128;
		break;

	case RIBParam::size_512x256:
		texWidth  = 512;
		texHeight = 256;
		break;

	case RIBParam::size_1024x512:
		texWidth  = 1024;
		texHeight = 512;
		break;

	case RIBParam::size_2048x1024:
		texWidth  = 2048;
		texHeight = 1024;
		break;

	case RIBParam::size_4096x2048:
		texWidth  = 4096;
		texHeight = 2048;
		break;

	case RIBParam::size_8192x4096:
		texWidth  = 8192;
		texHeight = 4096;
		break;
	}

	try {
		// 背景画像を生成.
		compointer<sxsdk::image_interface> image(backTexture.CalcBackgroundTextureImage(scene, texWidth, texHeight));
		if (!image) return;

		// ファイルフルパス.
		const std::string& saveFilePath = m_RIBInfo.filePath;
		std::string name = BACKGROUND_TEXTURE_NAME;
		
		// imagesフォルダがない場合は作成.
		{
			struct stat buffer;
			const std::string imagePath = saveFilePath + "/images";
			if (stat(imagePath.c_str(), &buffer) != 0) {
#if SXWINDOWS
				_mkdir(imagePath.c_str());
#else
				mkdir(imagePath.c_str(), S_IRWXU);
#endif
			}
		}

		int cou = 1;
		while (1) {
			bool sameF = false;
			for (int i = 0; i < m_RIBInfo.textureList.size(); i++) {
				CTextureInfo& textureInfo = m_RIBInfo.textureList[i];
				std::string fileName = textureInfo.fileName;
				const int stPos  = fileName.find('/');
				const int endPos = fileName.find_last_of('.');
				if (stPos != std::string::npos && endPos != std::string::npos) {
					fileName = fileName.substr(stPos + 1, endPos - stPos - 1);
				}
				if (fileName == name) {
					sameF = true;
					{
						std::stringstream s;
						s << std::string(BACKGROUND_TEXTURE_NAME) << (int)(++cou);
						name =  s.str();
					}
				}
			}
			if (!sameF) break;
		}

		const std::string saveFileName = saveFilePath + "/images/" + name + ".tiff";
		m_backgroundTextureName = "images/" + name;

		// RenderManが認識できるtiff画像として出力.
		CSaveTiff tiff(scene);
		tiff.SavePRManImage(image, saveFileName, true);		// パノラマのテクスチャとして保存.

	} catch (...) { }
}

/**
 * 画像ファイルをtiffファイルとして保存.
 */
std::string CSaveRIB::m_OutputTexture (sxsdk::scene_interface* scene, sxsdk::shape_class* pShape, const int index)
{
	if (!pShape || pShape->get_type() != sxsdk::enums::master_image) return "";
	const std::string& saveFilePath = m_RIBInfo.filePath;

	// imagesフォルダがない場合は作成.
	{
		struct stat buffer;
		const std::string imagePath = saveFilePath + "/images";
		if (stat(imagePath.c_str(), &buffer) != 0) {
#if SXWINDOWS
			_mkdir(imagePath.c_str());
#else
			mkdir(imagePath.c_str(), S_IRWXU);
#endif
		}
	}

	sxsdk::master_image_class& masterImage = pShape->get_master_image();
	try {
		std::string name = masterImage.get_name();
		int iPos = name.find_last_of(".");
		if (iPos != std::string::npos) name = name.substr(0, iPos);
		const std::string saveFileName = saveFilePath + "/images/" + name + ".tiff";

		compointer<sxsdk::image_interface> image(masterImage.get_image());
		if (!image->has_image()) return "";

		m_RIBInfo.textureList.push_back(CTextureInfo());
		CTextureInfo& textureInfo = m_RIBInfo.textureList.back();
		textureInfo.fileName    = "images/" + name + ".tiff";
		textureInfo.index       = index;
		textureInfo.isRealColor = image->has_real_color();

		// 画像が参照されているか.
		textureInfo.useTexture = textureInfo.IsUseTexture(scene, masterImage);
		if (textureInfo.useTexture) {
			// 指定の画像が法線マップかバンプマップかチェック.
			textureInfo.CheckTexturePattern(scene, masterImage, textureInfo.isBumpMap, textureInfo.isNormalMap, textureInfo.useTransparentAlpha);

			// RenderManが認識できるtiff画像として出力.
			{
				CSaveTiff tiff(scene);
				tiff.SavePRManImage(image, saveFileName, false, textureInfo.useTransparentAlpha);
			}

			return saveFileName;
		}
	} catch (...) { }

	return "";
}

/**
 * ポリゴンメッシュ情報の格納開始.
 */
void CSaveRIB::BeginPolygonMesh (sxsdk::shape_class* shape)
{
	m_polygonMeshCtrl.BeginStore(shape->get_name());
	m_pCurrentShape = shape;
}

/**
 * ポリゴンメッシュ情報の格納終了.
 */
void CSaveRIB::EndPolygonMesh ()
{
	m_polygonMeshCtrl.EndStore();
	if (!m_pCurrentShape) return;

	// faceGroupにより分割されている情報リストを取得.
	std::vector<int> faceGroupIndexList;
	m_polygonMeshCtrl.GetFaceGroupIndexList(faceGroupIndexList);
	const int meshCou = faceGroupIndexList.size();
	if (meshCou == 0) return;

	sxsdk::polygon_mesh_class* pmesh = NULL;
	if (m_pCurrentShape->get_type() == sxsdk::enums::polygon_mesh) {
		pmesh = &(m_pCurrentShape->get_polygon_mesh());
	}

	for (int loop = 0; loop < meshCou; loop++) {
		const int polygonsCou = m_polygonMeshCtrl.GetPolygonsCount(loop);

		std::vector<sxsdk::vec3> vertices;
		std::vector<sxsdk::vec3> normals;
		std::vector<sxsdk::vec2> uvs;
		m_polygonMeshCtrl.GetOutputVertices(loop, vertices);
		m_polygonMeshCtrl.GetOutputNormals(loop, normals);
		m_polygonMeshCtrl.GetOutputUVs(loop, uvs);

		const int verCou = vertices.size();

		// マテリアルの割り当て開始.
		if (faceGroupIndexList[loop] < 0) {
			m_BeginWriteMaterial(m_pScene, *m_pCurrentShape);
		} else {
			if (pmesh) {
				sxsdk::master_surface_class* masterSurface = pmesh->get_face_group_surface(faceGroupIndexList[loop]);
				m_BeginWriteMaterial(m_pScene, *masterSurface);
			}
		}

		{
			std::stringstream s;
			s << "TransformBegin";
			m_WriteLine(s.str());
		}
		m_indent++;

		{
			std::stringstream s;
			if (loop == 0) {
				s << "Attribute \"identifier\" \"name\" [\"" << std::string(m_pCurrentShape->get_name()) << "\"]";
			} else {
				s << "Attribute \"identifier\" \"name\" [\"" << std::string(m_pCurrentShape->get_name()) << "_" << loop << "\"]";
			}
			m_WriteLine(s.str());
		}

		m_WriteLine("PointsPolygons");
		m_indent++;

		// 面の頂点数を格納.
		std::vector<int> faceVersCountList;
		m_polygonMeshCtrl.GetPolygonsVCount(loop, faceVersCountList);

		{
			std::stringstream s;
			s << "[ ";
			for (int i = 0; i < polygonsCou; i++) {
				s << faceVersCountList[i] << " ";
			}
			s << "]";
			m_WriteLine(s.str());
		}

		// 面のインデックスリストの格納.
		{
			std::stringstream s;
			s << "[ ";

			std::vector<int> indices;
			for (int i = 0; i < polygonsCou; i++) {
				m_polygonMeshCtrl.GetPolygonIndices(loop, i, indices);
				const int vCou = faceVersCountList[i];
				for (int j = 0; j < vCou; ++j) {
					s << indices[vCou - j - 1]  << " ";		// 座標系が逆向きになるため、頂点の並びも逆にする.
				}
				s << " ";
			}
			s << "]";
			m_WriteLine(s.str());
		}

		// 頂点座標の格納.
		{
			std::stringstream s;
			s << "\"P\" [ ";
			for (int i = 0; i < verCou; i++) {
				s << vertices[i].x << " " << vertices[i].y << " " << -vertices[i].z << " ";
			}
			s << "]";
			m_WriteLine(s.str());
		}

		// 法線の格納.
		{
			std::stringstream s;
			s << "\"N\" [ ";
			for (int i = 0; i < verCou; i++) {
				s << normals[i].x << " " << normals[i].y << " " << -normals[i].z << " ";
			}
			s << "]";
			m_WriteLine(s.str());
		}

		// UVの格納.
		{
			std::stringstream s;
			s << "\"st\" [ ";
			for (int i = 0; i < verCou; i++) {
				s << uvs[i].x << " " << uvs[i].y << " ";
			}
			s << "]";
			m_WriteLine(s.str());
		}

		m_indent--;

		m_indent--;
		m_WriteLine("TransformEnd");

		// マテリアルの割り当て終了.
		m_EndWriteMaterial();
	}
}

/**
 * 頂点座標追加.
 */
void CSaveRIB::AppendPolygonMeshVertex (const sxsdk::vec3& v)
{
	m_polygonMeshCtrl.AppendVertex(v);
}

/**
 * 面情報追加.
 */
void CSaveRIB::AppendPolygonMeshPolygon (const std::vector<int>& indices, const std::vector<sxsdk::vec3>& normals, const std::vector<sxsdk::vec2>& uvs, const int faceGroupIndex)
{
	m_polygonMeshCtrl.AppendPolygon(indices, normals, uvs, faceGroupIndex);
}
