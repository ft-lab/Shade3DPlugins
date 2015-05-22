/**
 * 光源情報を一時的に保持するクラス.
 */

#include "LightCtrl.h"
#include "MathUtil.h"

CLightInfo::CLightInfo ()
{
}

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

		const sxsdk::mat4 lwMat = shape.get_local_to_world_matrix();

		// スポットライト、点光源、平行光源、環境光の情報を格納.
		// 環境光の場合は、point_lightでattenuationがsxsdk::enums::no_attenuationであらわす.
		m_lights.push_back(CLightInfo());
		CLightInfo& lightInfo = m_lights.back();
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
	}

	// 面光源、線光源.
	if (shape.get_type() == sxsdk::enums::line) {
		sxsdk::line_class& lineC = shape.get_line();
		const float intensity = lineC.get_light_intensity();
		if (!sx::zero(intensity)) {
			const sxsdk::mat4 lwMat = shape.get_local_to_world_matrix();
			m_lights.push_back(CLightInfo());
			CLightInfo& lightInfo = m_lights.back();
			lightInfo.lightType = lineC.get_closed() ? light_type_area : light_type_line;
			lightInfo.intensity = intensity;
			lightInfo.color     = lineC.get_light_color();
			lightInfo.areaLightclosed = lineC.get_closed();
			lightInfo.visible         = lineC.get_light_visible();

			// ベジェの頂点をラインに変換.
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
