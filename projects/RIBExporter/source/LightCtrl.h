/**
 * 光源情報を一時的に保持するクラス.
 */

#ifndef _LIGHTCTRL_H
#define _LIGHTCTRL_H

#include "GlobalHeader.h"

/**
 * ライトの種類.
 */
enum LIGHT_TYPE {
	light_type_distant = 0,				// 無限遠光源.
	light_type_point,					// 点光源.
	light_type_spot,					// スポットライト.
	light_type_distribution,			// 配光.
	light_type_directional,				// 平行光源.
	light_type_ambient,					// 環境光.
	light_type_area,					// 面光源.
	light_type_line,					// 線光源.
};

/**
 * ライト情報の格納用.
 */
class CLightInfo
{
public:
	sxsdk::shape_class* shape;				// 対応する形状.

	LIGHT_TYPE lightType;					// 光源の種類.
	float intensity;						// 明るさ.
	sxsdk::rgb_class color;					// 光源の色.
	sxsdk::vec3 direction;					// 平行光源の場合の向き.
	sxsdk::vec3 pos;						// 点光源、スポットライトの場合の位置.
	float ambient;							// 環境光.
	int attenuation;						// 減衰の種類.

	bool shadowMap;							// シャドウマップかどうか.
	float shadowValue;						// 影の影響度.
	sxsdk::rgb_class shadowColor;			// 影の色.

	float spotConeAngle;					// スポットライトの角度.
	float spotSoftness;						// スポットライトのソフトネス値.
	float pointSphereRadius;				// 点光源のときの球の半径.
	float diskRadius;						// スポット/平行光源時のdiskの半径.

	std::vector<sxsdk::vec3> areaLightPos;		// 光源を構成する頂点座標.
	bool areaLightclosed;						// 面光源が閉じた線形状か.

	bool visible;								// 面光源の可視.

public:
	CLightInfo ();

	void Clear ();
};

//-------------------------------------------------------------------.
// RenderMan用の光源情報.
//-------------------------------------------------------------------.
/**
 * 面光源.
 */
class CPxrAreaLight
{
public:
	float intensity;
	sxsdk::rgb_class lightColor;
	float areaNormalize;
	sxsdk::rgb_class specAmount;
	sxsdk::rgb_class diffAmount;
	float coneAngle;
	float penumbraAngle;
	float penumbraExponent;
	float profileRange;
	float cosinePower;
	float angularVisibility;

	sxsdk::rgb_class shadowColor;
	float traceShadows;
	float adaptiveShadows;

public:
	CPxrAreaLight ();

	void Clear ();
};

/**
 * ライト管理クラス.
 */
class CLightCtrl
{
private:
	sxsdk::shade_interface& shade;

	std::vector<CLightInfo> m_lights;

	/**
	 * 再帰的にシーンをたどってオブジェクト光源を格納.
	 */
	void m_StoreObjectLight (sxsdk::scene_interface* scene, sxsdk::shape_class& shape);

public:
	CLightCtrl (sxsdk::shade_interface& shade);

	/**
	 * 情報のクリア.
	 */
	void Clear ();

	/**
	 * シーンの光源情報をたどって格納.
	 */
	void StoreLights (sxsdk::scene_interface* scene);

	/**
	 * 光源数を取得.
	 */
	int GetLightsCount () { return m_lights.size(); }

	/**
	 * 光源情報を取得.
	 */
	CLightInfo GetLightInfo (const int index) { return m_lights[index]; }
};

//-------------------------------------------------------------------.
// 光源情報を、 Shade 3D ==> RenderMan(RIS)でコンバート.
//-------------------------------------------------------------------.
namespace LightCtrl
{
	/**
	 * Shade 3Dでの光源情報より、RIS向けにコンバート.
	 */
	void ConvAreaLightShade3DToRIS (sxsdk::shade_interface& shade, CLightInfo& lightInfo, CPxrAreaLight& pxrAreaLight, const bool ver21 = false);

	/**
	 * 指定の面光源情報を取得.
	 */
	CLightInfo GetAreaLightInfo (sxsdk::shape_class& shape);
}

#endif
