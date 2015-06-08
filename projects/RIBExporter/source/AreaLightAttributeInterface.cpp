/**
 * 面光源の情報を設定する.
 */

#include "AreaLightAttributeInterface.h"
#include "StreamCtrl.h"

// SXULダイアログボックスでのイベントID.
enum
{
	dlg_intensity_id = 101,
	dlg_light_color_id = 102,
	dlg_area_normalize_id = 103,
	dlg_spec_amount_id = 104,
	dlg_diff_amount_id = 105,
	dlg_cone_angle_id = 106,
	dlg_penumbra_angle_id = 107,
	dlg_penumbra_exponent_id = 108,
	dlg_profile_range_id = 109,
	dlg_cosine_power_id = 110,
	dlg_angular_visibility_id = 111,
	dlg_shadow_color_id = 112,
	dlg_trace_shadows_id = 113,
	dlg_adaptive_shadows_id = 114,
};

CAreaLightAttributeInterface::CAreaLightAttributeInterface (sxsdk::shade_interface& shade) : shade(shade)
{
}

// カスタム情報が選択された.
bool CAreaLightAttributeInterface::ask_shape (sxsdk::shape_class &shape, void *)
{
	if ((shape.get_type() != sxsdk::enums::line || sx::zero(shape.get_line().get_light_intensity())) && shape.get_type() != sxsdk::enums::light) {
		shade.show_message_box(shade.gettext("msg_select_area_light"), false);
		return false;
	}

	compointer<sxsdk::dialog_interface> dlg(shade.create_dialog_interface_with_uuid(RIB_AREA_LIGHT_ATTRIBUTE_ID));
	dlg->set_resource_name("area_light_dlg");
	dlg->set_responder(this);
	this->AddRef();			// set_responder()に合わせて、参照カウンタを増やす。 .

	// パラメータを取得.
	m_InitAreaLightData(shape, true);

	// ダイアログの表示.
	if (dlg->ask()) {
		StreamCtrl::SaveRIBAreaLight(shape, m_data);
		return true;
	}

	return false;
}

/**
 * 面光源情報よりm_dataを初期化.
 */
void CAreaLightAttributeInterface::m_InitAreaLightData (sxsdk::shape_class &shape, const bool loadStream)
{
	// shape形状から、RenderMan向けの面光源情報を取得.
	CLightInfo lightInfo = LightCtrl::GetAreaLightInfo(shape);			// 光源情報を取得.
	LightCtrl::ConvAreaLightShade3DToRIS(shade, lightInfo, m_data);		// RenderMan向けのパラメータに変換.
	if (loadStream && StreamCtrl::HasRIBAreaLight(shape)) {
		m_data = StreamCtrl::LoadRIBAreaLight(shape);
	}
}

//--------------------------------------------------//
//	ダイアログのイベント処理用						//
//--------------------------------------------------//
// ダイアログの初期化.
void CAreaLightAttributeInterface::initialize_dialog (sxsdk::dialog_interface &d, void *)
{
}

// ダイアログのイベントを受け取る.
bool CAreaLightAttributeInterface::respond (sxsdk::dialog_interface &d, sxsdk::dialog_item_class &item, int action, void *)
{
	const int id = item.get_id();		// アクションがあったダイアログアイテムのID.

	if (id == sx::iddefault) {
		// パラメータを初期化.
		compointer<sxsdk::scene_interface> scene(shade.get_scene_interface());
		sxsdk::shape_class& activeShape = scene->active_shape();
		m_InitAreaLightData(activeShape, false);

		// ダイアログのパラメータを更新.
		load_dialog_data(d);

		return true;
	}

	if (id == dlg_intensity_id) {
		m_data.intensity = item.get_float();
		return true;
	}
	if (id == dlg_light_color_id) {
		m_data.lightColor = item.get_rgb();
		return true;
	}
	if (id == dlg_area_normalize_id) {
		m_data.areaNormalize = item.get_float();
		return true;
	}
	if (id == dlg_spec_amount_id) {
		m_data.specAmount = item.get_rgb();
		return true;
	}
	if (id == dlg_diff_amount_id) {
		m_data.diffAmount = item.get_rgb();
		return true;
	}
	if (id == dlg_cone_angle_id) {
		m_data.coneAngle = item.get_float();
		return true;
	}
	if (id == dlg_penumbra_angle_id) {
		m_data.penumbraAngle = item.get_float();
		return true;
	}
	if (id == dlg_penumbra_exponent_id) {
		m_data.penumbraExponent = item.get_float();
		return true;
	}
	if (id == dlg_profile_range_id) {
		m_data.profileRange = item.get_float();
		return true;
	}
	if (id == dlg_cosine_power_id) {
		m_data.cosinePower = item.get_float();
		return true;
	}
	if (id == dlg_angular_visibility_id) {
		m_data.angularVisibility = item.get_float();
		return true;
	}
	if (id == dlg_shadow_color_id) {
		m_data.shadowColor = item.get_rgb();
		return true;
	}
	if (id == dlg_trace_shadows_id) {
		m_data.traceShadows = item.get_float();
		return true;
	}
	if (id == dlg_adaptive_shadows_id) {
		m_data.adaptiveShadows = item.get_float();
		return true;
	}

	return false;
}


// ダイアログのデータを設定する.
void CAreaLightAttributeInterface::load_dialog_data (sxsdk::dialog_interface &d, void *)
{
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_intensity_id));
		item->set_float(m_data.intensity);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_light_color_id));
		item->set_rgb(m_data.lightColor);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_area_normalize_id));
		item->set_float(m_data.areaNormalize);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_spec_amount_id));
		item->set_rgb(m_data.specAmount);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_diff_amount_id));
		item->set_rgb(m_data.diffAmount);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_cone_angle_id));
		item->set_float(m_data.coneAngle);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_penumbra_angle_id));
		item->set_float(m_data.penumbraAngle);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_penumbra_exponent_id));
		item->set_float(m_data.penumbraExponent);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_profile_range_id));
		item->set_float(m_data.profileRange);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_cosine_power_id));
		item->set_float(m_data.cosinePower);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_angular_visibility_id));
		item->set_float(m_data.angularVisibility);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_shadow_color_id));
		item->set_rgb(m_data.shadowColor);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_trace_shadows_id));
		item->set_float(m_data.traceShadows);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_adaptive_shadows_id));
		item->set_float(m_data.adaptiveShadows);
	}
}

// 値の変更を保存するときに呼ばれる.
void CAreaLightAttributeInterface::save_dialog_data (sxsdk::dialog_interface &d, void *)
{
}

