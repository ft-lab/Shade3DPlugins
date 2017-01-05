/**
 *  @file   GlobalHeader.h
 *  @brief  共通して使用する変数など.
 */

#ifndef _GLOBALHEADER_H
#define _GLOBALHEADER_H

#include "sxsdk.cxx"

/**
 * RIB ExporterのプラグインID.
 */
#define RIB_EXPORTER_ID                      sx::uuid_class("4CA75EFA-BDF7-4B85-9E97-8427FB3451B3")		// ExporterのID.
#define RIB_ATTRIBUTE_WINDOW_ID              sx::uuid_class("84CE7D1A-5EFB-4581-ABA7-9864A43DEC2F")		// Attributeを指定するWindow ID.
#define RIB_AREA_LIGHT_ATTRIBUTE_ID	         sx::uuid_class("54B8005D-7361-4BA2-ACB5-2A88F01A92FB")		// 面光源のAttributeを指定するID.

// TODO : バージョン上がるたびに、RIB_ATTRIBUTE_WINDOW_IDのuuidを変更してあげないと、.
// CAttributeWindowInterfaceのthis->set_client_size(get_layout_bounds().size());でのウィンドウサイズが更新できない.

#define RIB_MATERIAL_ID			sx::uuid_class("10B96B1E-93A5-402B-BFE4-BADA6B7F504B")			// マテリアルとしてマスターサーフェスに保持するためのID.
#define RIB_AREA_LIGHT_ID		sx::uuid_class("FE518A69-B953-4C7F-B155-60AAD3C276DB")			// 面光源情報を保持するためのID.


#define RIB_EXPORT_DLG_VERSION_100		0x100		// RIB Export Dialogのバージョン.
#define RIB_EXPORT_DLG_VERSION_101		0x101		// ver.1.0.0.1 - .
#define RIB_EXPORT_DLG_VERSION_104		0x104		// ver.1.0.0.4 - .
#define RIB_EXPORT_DLG_VERSION_105		0x105		// ver.1.0.0.5 - .
#define RIB_EXPORT_DLG_VERSION			0x105		// current (ver.1.0.0.5 - ).

#define RIB_MATERIAL_VERSION_100		0x100		// Materialのバージョン.
#define RIB_MATERIAL_VERSION_102		0x102		// ver.1.0.0.2 - 
#define RIB_MATERIAL_VERSION			0x102		// current (ver.1.0.0.2 -).

#define RIB_AREA_LIGHT_VERSION			0x100		// 面光源情報のバージョン.


namespace RIBParam
{
	/**
	 * レンダリングの種類.
	 */
	enum INTEGRATORS_TYPE {
		pxrPathTracer = 0,			// 単方向パストレース.
		pxrVCM,						// 双方向パストレース.
		pxrDirectLighting,			// 直接照明のみ.
		pxrOcclusion,				// Ambient Occlusuion.
	};

	/**
	 * 出力ファイルのフォーマット.
	 */
	enum RENDERING_FORMAT_TYPE {
		tiff = 0,					// TIFF.
		openexr,					// OpenEXR.
	};

	/**
	 * 出力画像の種類.
	 */
	enum RENDERING_IMAGE_TYPE {
		image_rgb = 0,				// RGB.
		image_rgba,					// RGBA.
	};

	/**
	 * マテリアルの種類.
	 * これは、マテリアル設定ウィンドウのpopup_menuと同じ順番.
	 */
	enum MATERIAL_TYPE {
		pxrDiffuse = 0,
		pxrDisney,
		pxrGlass,
		pxrConstant,
		pxrVolume,
		pxrSkin,
	};

	/**
	 * 背景画像のサイズ.
	 */
	enum BACKGROUND_IMAGE_SIZE {
		size_256x128 = 0,
		size_512x256,
		size_1024x512,
		size_2048x1024,
		size_4096x2048,
		size_8192x4096,
	};
}

/**
 * RIBエクスポートの際の情報.
 */
class RIBExportData
{
public:
	RIBParam::INTEGRATORS_TYPE type;		// レンダリングの種類.
	float pixelVariance;
	float minSamples, maxSamples;			// レンダリング時のピクセルごとのサンプリング数の最小と最大.
	bool incremental;
	float bias;								// レイを浮かせるバイアス.
	int maxDiffuseDepth;					// 拡散反射の反射回数最大.
	int maxSpecularDepth;					// 鏡面反射（映り込み含む）の反射回数最大.

	RIBParam::RENDERING_FORMAT_TYPE renderingFormatType;	// レンダリングファイルの種類.
	RIBParam::RENDERING_IMAGE_TYPE renderingImageType;		// レンダリング画像の種類.

	bool outputBackgroundImage;									// 背景画像を出力.
	RIBParam::BACKGROUND_IMAGE_SIZE backgroundImageSize;		// 背景画像のサイズ.
	float backgroundImageIntensity;								// 背景画像の明るさ.
	bool backgroundDraw;										// 背景の反映.

	bool colorColorToLinear;									// 色情報をリニアに計算して渡す.
	bool colorTextureToLinear;									// テクスチャ情報をリニアに計算して渡す.

	bool lightDayLight;											// 太陽光の有効化.

	bool statisticsEndOfFrame;									// レンダリング時間を表示.
	bool statisticsXMLFile;										// XMLファイルを出力.

	bool doSubdivision;											// サブディビジョンする場合はtrue.
	int prmanVersion;											// RenderManのバージョン（0:20.x、1:21.x）.

public:
	RIBExportData () {
		Clear();
	}

	void Clear () {
		type                = RIBParam::pxrPathTracer;
		pixelVariance       = 0.01f;
		minSamples          = 1;
		maxSamples          = 16;
		incremental         = true;
		bias                = 0.001f;
		maxDiffuseDepth     = 5;
		maxSpecularDepth    = 10;
		prmanVersion        = 1;

		renderingFormatType = RIBParam::tiff;
		renderingImageType  = RIBParam::image_rgb;

		outputBackgroundImage    = true;
		backgroundImageSize      = RIBParam::size_1024x512;
		backgroundImageIntensity = 1.0f;
		backgroundDraw           = true;

		colorColorToLinear   = true;
		colorTextureToLinear = true;

		lightDayLight = false;

		statisticsEndOfFrame = true;
		statisticsXMLFile    = false;
		doSubdivision        = true;
	}
};

#endif
