/**
 * 光源情報を一時的に保持するクラス.
 */

/*
  面光源、点光源、スポットライト、平行光源に対応.
   - 点光源の場合は、半径10.0の球から放射されるとする.
   - スポットライトの場合は、 面積 10x10の平面から放射されるとする.

*/

#include "LightCtrl.h"
#include "MathUtil.h"

CLightInfo::CLightInfo ()
{
	Clear();
}

void CLightInfo::Clear ()
{
	shape           = NULL;
	lightType       = light_type_area;
	intensity       = 100.0f;
	color           = sxsdk::rgb_class(1, 1, 1);
	direction       = sxsdk::vec3(0, -1, 0);
	pos             = sxsdk::vec3(0, 0, 0);
	ambient         = 0.0f;
	attenuation     = sxsdk::enums::quadratic_attenuation;
	shadowMap       = false;
	shadowValue     = 1.0f;
	spotConeAngle   = 50.0f;
	spotSoftness    = 0.1f;
	areaLightclosed = true;
	visible         = false;
	pointSphereRadius = 10.0f;

	areaLightPos.clear();
}

//---------------------------------------------------.
/**
 * 面光源.
 */
CPxrAreaLight::CPxrAreaLight ()
{
	Clear();
}

void CPxrAreaLight::Clear ()
{
	intensity         = 1.0f;
	lightColor        = sxsdk::rgb_class(1, 1, 1);
	areaNormalize     = 0.0f;
	specAmount        = sxsdk::rgb_class(1, 1, 1);
	diffAmount        = sxsdk::rgb_class(1, 1, 1);
	coneAngle         = 20.0f;
	penumbraAngle     = 5.0f;
	penumbraExponent  = 0.0f;
	profileRange      = 180.0f;
	cosinePower       = 90.0f;
	angularVisibility = 1.0f;
	shadowColor       = sxsdk::rgb_class(0, 0, 0);
	traceShadows      = 1.0f;
	adaptiveShadows   = 1.0f;
}

//---------------------------------------------------.

CLightCtrl::CLightCtrl (sxsdk::shade_interface& shade) : shade(shade)
{
}

/**
 * 情報のクリア.
 */
void CLightCtrl::Clear ()
{
	m_lights.clear();
}

/**
 * シーンの光源情報をたどって格納.
 */
void CLightCtrl::StoreLights (sxsdk::scene_interface* scene)
{
	// 無限遠光源の取得.
	{
		compointer<sxsdk::distant_light_interface> dLight(scene->get_distant_light_interface());
		const int lightsCou = dLight->get_number_of_lights();

		for (int i = 0; i < lightsCou; i++) {
			sxsdk::distant_light_item_class& item = dLight->distant_light_item(i);
			if (sx::zero(item.get_intensity()) && sx::zero(item.get_ambient())) continue;

			m_lights.push_back(CLightInfo());
			CLightInfo& lightInfo = m_lights.back();
			lightInfo.lightType = light_type_distant;
			lightInfo.ambient   = item.get_ambient();
			lightInfo.color     = item.get_light_color();
			lightInfo.direction = -item.get_direction();
			lightInfo.intensity = item.get_intensity();
			lightInfo.shadowMap = (item.get_shadow_type() == sxsdk::enums::shadowmap_shadow_type);
			lightInfo.shadowValue = item.get_shadow();
			lightInfo.visible     = false;
		}
	}

	// オブジェクト光源の取得.
	m_StoreObjectLight(scene, scene->get_shape());
}

/**
 * 再帰的にシーンをたどってオブジェクト光源を格納.
 */
void CLightCtrl::m_StoreObjectLight (sxsdk::scene_interface* scene, sxsdk::shape_class& shape)
{
	if (!shape.get_rendering()) return;

	// 点光源、スポットライト、平行光源、環境光.
	if (shape.get_type() == sxsdk::enums::light) {
		sxsdk::light_class& light = shape.get_light();
		if (sx::zero(light.get_intensity()) && sx::zero(light.get_ambient())) return;

		// スポットライト、点光源、平行光源、環境光の情報を格納.
		// 環境光の場合は、point_lightでattenuationがsxsdk::enums::no_attenuationであらわす.
		m_lights.push_back(LightCtrl::GetAreaLightInfo(shape));
	}

	// 面光源、線光源.
	if (shape.get_type() == sxsdk::enums::line) {
		if (!sx::zero(shape.get_line().get_light_intensity())) {
			m_lights.push_back(LightCtrl::GetAreaLightInfo(shape));
		}
	}

	if (shape.has_son()) {
		sxsdk::shape_class* pS = shape.get_son();
		while (pS) {
			if (!pS->has_bro()) break;
			pS = pS->get_bro();
			if (!pS) break;
			m_StoreObjectLight(scene, *pS);
		}
	}
}

/**
 * Shade 3Dでの光源情報より、RIS向けにコンバート.
 */
void LightCtrl::ConvAreaLightShade3DToRIS (sxsdk::shade_interface& shade, CLightInfo& lightInfo, CPxrAreaLight& pxrAreaLight)
{
	pxrAreaLight.Clear();

	// 面光源の面積を計算.
	double area = MathUtil::CalcPolygonArea(shade, lightInfo.areaLightPos);

	// 明るさを面積単位で計算.
	double intensity = (double)lightInfo.intensity;
	if (lightInfo.lightType == light_type_point) {
		// 点光源の場合は、半径10.0の球としたときの表面積を計算.
		const double r = (double)lightInfo.pointSphereRadius;
		area = 4.0 * sx::pi * (r * r);
	}
	if (!sx::zero(area)) {
		intensity = (intensity * intensity) * sx::pi / area;
	}

	// スポットライトが作る角度.
	if (lightInfo.lightType == light_type_spot) {
		const  float angle      = (lightInfo.spotConeAngle * 0.5f);
		const  float deltaAngle = (90.0f - angle) * lightInfo.spotSoftness;
		pxrAreaLight.coneAngle     = angle;
		pxrAreaLight.penumbraAngle = deltaAngle;
	}

	pxrAreaLight.intensity   = (float)intensity;
	pxrAreaLight.lightColor  = lightInfo.color;
	if (lightInfo.shadowValue < 1.0f && !sx::zero(lightInfo.shadowValue - 1.0f)) {
		const float shadowVal = 1.0f - lightInfo.shadowValue;
		pxrAreaLight.shadowColor = sxsdk::rgb_class(shadowVal, shadowVal, shadowVal);
	}
}

/**
 * 指定の面光源情報を取得.
 */
CLightInfo LightCtrl::GetAreaLightInfo (sxsdk::shape_class& shape)
{
	CLightInfo lightInfo;

	if (shape.get_type() != sxsdk::enums::line && shape.get_type() != sxsdk::enums::light) return lightInfo;

	if (shape.get_type() == sxsdk::enums::line) {
		sxsdk::line_class& lineC = shape.get_line();
		const float intensity = lineC.get_light_intensity();
		if (sx::zero(intensity)) return lightInfo;

		lightInfo.shape     = &shape;
		lightInfo.lightType = lineC.get_closed() ? light_type_area : light_type_line;
		lightInfo.intensity = intensity;
		lightInfo.color     = lineC.get_light_color();
		lightInfo.areaLightclosed = lineC.get_closed();
		lightInfo.visible         = lineC.get_light_visible();
		lightInfo.shadowValue     = lineC.get_shadow();

		// ベジェの頂点をラインに変換.
		{
			const sxsdk::mat4 lwMat = shape.get_local_to_world_matrix();
			std::vector<CPoint3D> bezierPos;
			const int pCou = lineC.get_number_of_points();
			for (int i = 0; i < pCou; i++) {
				sxsdk::control_point_class& cp = lineC.control_point(i);
				CPoint3D p;
				p.point     = cp.get_position() * lwMat;
				p.inHandle  = cp.get_in_handle() * lwMat;
				p.outHandle = cp.get_out_handle() * lwMat;
				bezierPos.push_back(p);
			}
			MathUtil::GetBezierToLines(bezierPos, lightInfo.areaLightPos, false);
		}

	} else {
		// オブジェクトライトの情報を取得.
		sxsdk::light_class& light = shape.get_light();
		if (sx::zero(light.get_intensity()) && sx::zero(light.get_ambient())) return lightInfo;

		const sxsdk::mat4 lwMat = shape.get_local_to_world_matrix();

		// スポットライト、点光源、平行光源、環境光の情報を格納.
		// 環境光の場合は、point_lightでattenuationがsxsdk::enums::no_attenuationであらわす.
		lightInfo.shape       = &shape;
		lightInfo.ambient     = light.get_ambient();
		lightInfo.color       = light.get_light_color();
		lightInfo.direction   = -light.get_direction();
		lightInfo.intensity   = light.get_intensity();
		lightInfo.pos         = light.get_center() * lwMat;
		lightInfo.attenuation = light.get_attenuation();
		lightInfo.shadowMap   = (light.get_shadow_type() == sxsdk::enums::shadowmap_shadow_type);
		lightInfo.shadowValue = light.get_shadow();
		lightInfo.spotConeAngle = std::min(light.get_angle(), 180.0f);		// スポットライトの角度（度数）.
		lightInfo.spotSoftness  = light.get_softness();
		lightInfo.visible       = false;

		switch (light.get_distribution_type()) {
		case sxsdk::enums::point_light:
			lightInfo.lightType = light_type_point;
			if (lightInfo.attenuation == sxsdk::enums::no_attenuation) {
				lightInfo.lightType = light_type_ambient;
			}
			break;

		case sxsdk::enums::spotlight:
			lightInfo.lightType = light_type_spot;
			break;

		case sxsdk::enums::distribution_light:
			lightInfo.lightType = light_type_distribution;
			break;

		case sxsdk::enums::directional_light:
			lightInfo.lightType = light_type_directional;
			break;
		}

		sxsdk::vec4 dirV = sxsdk::vec4(lightInfo.direction.x, lightInfo.direction.y, lightInfo.direction.z, 0.0f) * lwMat;
		lightInfo.direction = normalize(sxsdk::vec3(dirV.x, dirV.y, dirV.z));

		// スポットライト時もPxrAreaLightで表現する.
		if (lightInfo.lightType == light_type_spot) {
			// +Z方向を向く面がデフォルト.
			lightInfo.areaLightPos.resize(4);
			lightInfo.areaLightPos[0] = sxsdk::vec3(-5,  5, 0);
			lightInfo.areaLightPos[1] = sxsdk::vec3( 5,  5, 0);
			lightInfo.areaLightPos[2] = sxsdk::vec3( 5, -5, 0);
			lightInfo.areaLightPos[3] = sxsdk::vec3(-5, -5, 0);
		}
	}

	return lightInfo;
}
