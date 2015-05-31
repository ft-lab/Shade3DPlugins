/**
 * RIBでのMaterial相当の情報を指定するウィンドウ.
 */

#include "AttributeWindowInterface.h"
#include "StreamCtrl.h"

void CAttributeWindowInterface::initialize (void *)
{
	m_pCurrentShape    = NULL;
	m_pOldCurrentShape = NULL;

	m_please_select_master_surface_text = shade.gettext("msg_select_master_surface");

	set_trigger(sxsdk::enums::trigger_enum(sxsdk::enums::active_scene_changed | sxsdk::enums::active_shapes_changed | sxsdk::enums::active_shapes_modified |  sxsdk::enums::shapes_deleted));
	load_sxul("material_window");

	this->set_client_size(get_layout_bounds().size());
	this->set_title(CAttributeWindowInterface::name(&shade));
}

/**
 * シーンより、カレント形状の参照を取得.
 */
sxsdk::shape_class* CAttributeWindowInterface::m_GetCurrentShape (sxsdk::scene_interface* scene)
{
	if (!scene) return NULL;
	const int cou = scene->get_number_of_active_shapes();
	if (cou == 0) return NULL;

	for (int i = 0; i < cou; i++) {
		sxsdk::shape_class& s = scene->active_shape(i);
		if (s.get_type() == sxsdk::enums::master_surface) return &s;
	}
	return NULL;
}

//------------------------------------------.
// setup時のコールバック.
//------------------------------------------.
bool CAttributeWindowInterface::setup_static_text (sxsdk::window_interface::static_text_class &static_text, void *aux)
{
	// ウィンドウを開いた直後など、設定の再読み込み.
	m_Reload();

	// 現在のアクティブな形状名を反映.
	if (strcmp(static_text.get_control_idname(), "target_master_surface_name") == 0) {
		if (m_pCurrentShape && m_pCurrentShape->get_type() == sxsdk::enums::master_surface) {
			try {
				static_text.set_title(m_pCurrentShape->get_name());
			} catch (...) { }
		} else {
				static_text.set_title(m_please_select_master_surface_text.c_str());
		}
		static_text.set_active(true);

		return true;
	}

	static_text.set_active((m_pCurrentShape && m_materialInfo.useCustom) ? true : false);

	return true;
}

bool CAttributeWindowInterface::setup_push_button (sxsdk::window_interface::push_button_class &push_button, void *aux)
{
	// ウィンドウを開いた直後など、設定の再読み込み.
	m_Reload();

	push_button.set_active(m_pCurrentShape ? true : false);
	return true;
}

bool CAttributeWindowInterface::setup_tab (sxsdk::window_interface::tab_class &tab, void *aux)
{
	// ウィンドウを開いた直後など、設定の再読み込み.
	m_Reload();

	tab.set_active((m_pCurrentShape && m_materialInfo.useCustom) ? true : false);

	// タブの変更を反映させるため、setup_tabからの呼び出し.
	if (strcmp(tab.get_control_idname(), "material_tab") == 0) {
		tab.set_value((int)m_materialInfo.type);
		return true;
	}
	return false;
}

bool CAttributeWindowInterface::setup_checkbox (sxsdk::window_interface::checkbox_class &checkbox, void *aux)
{
	// ウィンドウを開いた直後など、設定の再読み込み.
	m_Reload();

	checkbox.set_active((m_pCurrentShape && m_materialInfo.useCustom) ? true : false);

	if (strcmp(checkbox.get_control_idname(), "use_custom") == 0) {
		checkbox.set_value(m_materialInfo.useCustom);
		checkbox.set_active(m_pCurrentShape ? true : false);
		return true;
	}
	if (strcmp(checkbox.get_control_idname(), "use_depth") == 0) {
		checkbox.set_value(m_materialInfo.useDepth);
		return true;
	}
	if (strcmp(checkbox.get_control_idname(), "visibility_camera") == 0) {
		checkbox.set_value(m_materialInfo.visibilityCamera);
		return true;
	}
	if (strcmp(checkbox.get_control_idname(), "visibility_diffuse") == 0) {
		checkbox.set_value(m_materialInfo.visibilityDiffuse);
		return true;
	}
	if (strcmp(checkbox.get_control_idname(), "visibility_specular") == 0) {
		checkbox.set_value(m_materialInfo.visibilitySpecular);
		return true;
	}
	if (strcmp(checkbox.get_control_idname(), "visibility_indirect") == 0) {
		checkbox.set_value(m_materialInfo.visibilityIndirect);
		return true;
	}

	if (strcmp(checkbox.get_control_idname(), "volume_multiScatter") == 0) {
		checkbox.set_value(m_materialInfo.pxrVolume.multiScatter);
		return true;
	}

	return false;
}

bool CAttributeWindowInterface::setup_popup_menu (sxsdk::window_interface::popup_menu_class &popup_menu, void *aux)
{
	// ウィンドウを開いた直後など、設定の再読み込み.
	m_Reload();

	popup_menu.set_active((m_pCurrentShape && m_materialInfo.useCustom) ? true : false);

	if (strcmp(popup_menu.get_control_idname(), "material_type") == 0) {
		popup_menu.set_value((int)m_materialInfo.type);
		return true;
	}

	return false;
}

bool CAttributeWindowInterface::setup_color_box (sxsdk::window_interface::color_box_class &color_box, void *aux)
{
	// ウィンドウを開いた直後など、設定の再読み込み.
	m_Reload();

	color_box.set_active((m_pCurrentShape && m_materialInfo.useCustom) ? true : false);

	if (strcmp(color_box.get_control_idname(), "diffuse_diffuseColor") == 0) {
		color_box.set_value(m_materialInfo.pxrDiffuse.diffuseColor);
		return true;
	}
	
	if (strcmp(color_box.get_control_idname(), "diffuse_transmissionColor") == 0) {
		color_box.set_value(m_materialInfo.pxrDiffuse.transmissionColor);
		return true;
	}

	if (strcmp(color_box.get_control_idname(), "disney_baseColor") == 0) {
		color_box.set_value(m_materialInfo.pxrDisney.baseColor);
		return true;

	}
	
	if (strcmp(color_box.get_control_idname(), "disney_emitColor") == 0) {
		color_box.set_value(m_materialInfo.pxrDisney.emitColor);
		return true;
	}
	
	if (strcmp(color_box.get_control_idname(), "disney_subsurfaceColor") == 0) {
		color_box.set_value(m_materialInfo.pxrDisney.subsurfaceColor);
		return true;
	}

	if (strcmp(color_box.get_control_idname(), "glass_reflectionColor") == 0) {
		color_box.set_value(m_materialInfo.pxrGlass.reflectionColor);
		return true;
	}

	if (strcmp(color_box.get_control_idname(), "glass_transmissionColor") == 0) {
		color_box.set_value(m_materialInfo.pxrGlass.transmissionColor);
		return true;
	}

	if (strcmp(color_box.get_control_idname(), "glass_absorptionColor") == 0) {
		color_box.set_value(m_materialInfo.pxrGlass.absorptionColor);
		return true;
	}

	if (strcmp(color_box.get_control_idname(), "constsnt_emitColor") == 0) {
		color_box.set_value(m_materialInfo.pxrConstant.emitColor);
		return true;
	}

	if (strcmp(color_box.get_control_idname(), "volume_diffuseColor") == 0) {
		color_box.set_value(m_materialInfo.pxrVolume.diffuseColor);
		return true;
	}
	if (strcmp(color_box.get_control_idname(), "volume_emitColor") == 0) {
		color_box.set_value(m_materialInfo.pxrVolume.emitColor);
		return true;
	}
	if (strcmp(color_box.get_control_idname(), "volume_densityColor") == 0) {
		color_box.set_value(m_materialInfo.pxrVolume.densityColor);
		return true;
	}

	return false;
}

bool CAttributeWindowInterface::setup_number (sxsdk::window_interface::number_class &number, void *aux)
{
	// ウィンドウを開いた直後など、設定の再読み込み.
	m_Reload();

	number.set_active((m_pCurrentShape && m_materialInfo.useCustom) ? true : false);

	if (strcmp(number.get_control_idname(), "maxdiffusedepth") == 0) {
		number.set_value(m_materialInfo.maxdiffusedepth);
		number.set_active(m_pCurrentShape && m_materialInfo.useDepth && m_materialInfo.useCustom);
		return true;
	}
	if (strcmp(number.get_control_idname(), "maxspeculardepth") == 0) {
		number.set_value(m_materialInfo.maxspeculardepth);
		number.set_active(m_pCurrentShape && m_materialInfo.useDepth && m_materialInfo.useCustom);
		return true;
	}

	if (strcmp(number.get_control_idname(), "disney_subsurface") == 0) {
		number.set_value(m_materialInfo.pxrDisney.subsurface);
		return true;
	}

	if (strcmp(number.get_control_idname(), "disney_metallic") == 0) {
		number.set_value(m_materialInfo.pxrDisney.metallic);
		return true;

	}
	if (strcmp(number.get_control_idname(), "disney_specular") == 0) {
		number.set_value(m_materialInfo.pxrDisney.specular);
		return true;
	}
	
	if (strcmp(number.get_control_idname(), "disney_specularTint") == 0) {
		number.set_value(m_materialInfo.pxrDisney.specularTint);
		return true;
	}

	if (strcmp(number.get_control_idname(), "disney_roughness") == 0) {
		number.set_value(m_materialInfo.pxrDisney.roughness);
		return true;
	}

	if (strcmp(number.get_control_idname(), "disney_anisotropic") == 0) {
		number.set_value(m_materialInfo.pxrDisney.anisotropic);
		return true;
	}

	if (strcmp(number.get_control_idname(), "disney_sheen") == 0) {
		number.set_value(m_materialInfo.pxrDisney.sheen);
		return true;

	}
	
	if (strcmp(number.get_control_idname(), "disney_sheenTint") == 0) {
		number.set_value(m_materialInfo.pxrDisney.sheenTint);
		return true;
	}

	if (strcmp(number.get_control_idname(), "glass_ior") == 0) {
		number.set_value(m_materialInfo.pxrGlass.ior);
		return true;
	}

	if (strcmp(number.get_control_idname(), "glass_roughness") == 0) {
		number.set_value(m_materialInfo.pxrGlass.roughness);
		return true;
	}

	if (strcmp(number.get_control_idname(), "glass_reflectionGain") == 0) {
		number.set_value(m_materialInfo.pxrGlass.reflectionGain);
		return true;
	}

	if (strcmp(number.get_control_idname(), "glass_transmissionGain") == 0) {
		number.set_value(m_materialInfo.pxrGlass.transmissionGain);
		return true;
	}

	if (strcmp(number.get_control_idname(), "glass_absorptionGain") == 0) {
		number.set_value(m_materialInfo.pxrGlass.absorptionGain);
		return true;
	}

	if (strcmp(number.get_control_idname(), "volume_densityFloat") == 0) {
		number.set_value(m_materialInfo.pxrVolume.densityFloat);
		return true;
	}
	if (strcmp(number.get_control_idname(), "volume_densityScale") == 0) {
		number.set_value(m_materialInfo.pxrVolume.densityScale);
		return true;
	}
	if (strcmp(number.get_control_idname(), "volume_anisotropy") == 0) {
		number.set_value(m_materialInfo.pxrVolume.anisotropy);
		return true;
	}
	if (strcmp(number.get_control_idname(), "volume_maxDensity") == 0) {
		number.set_value(m_materialInfo.pxrVolume.maxDensity);
		return true;
	}

	return false;
}


//------------------------------------------.
// イベント処理のコールバック.
//------------------------------------------.
void CAttributeWindowInterface::tab_value_changed (sxsdk::window_interface::tab_class &tab, void *aux)
{
	if (strcmp(tab.get_control_idname(), "material_tab") == 0) {
		const int type = tab.get_value();
		if (type != (int)m_materialInfo.type) {
			m_materialInfo.type = (RIBParam::MATERIAL_TYPE)type;
			this->obsolete();
		}
		return;
	}
}

void CAttributeWindowInterface::push_button_clicked (window_interface::push_button_class& push_button, void* aux)
{
	if (strcmp(push_button.get_control_idname(), "convert_from_surface_but") == 0) {
		// 「Convert from surface」ボタンが押された場合は、表面材質からパラメータを持ってくる.
		if (m_pCurrentShape && m_pCurrentShape->get_type() == sxsdk::enums::master_surface) {
			compointer<sxsdk::scene_interface> scene(m_pCurrentShape->get_scene_interface());
			CMaterialInfo materialInfo;
			materialInfo.SetMaterial(scene, *(m_pCurrentShape->get_master_surface()));
			MaterialCtrl::ConvShade3DToRIS(materialInfo, m_materialInfo);
			this->obsolete();		// 再更新.
		}
		return;
	}

	if (strcmp(push_button.get_control_idname(), "apply_but") == 0) {
		// Apply ボタンが押された場合は、情報を確定しstreamに保存.
		if (m_pCurrentShape) {
			compointer<sxsdk::scene_interface> scene(m_pCurrentShape->get_scene_interface());
			const int cou = scene->get_number_of_active_shapes();
			for (int i = 0; i < cou; i++) {
				m_Apply(scene->active_shape(i));
			}
		}
		return;
	}

	if (strcmp(push_button.get_control_idname(), "revert_but") == 0) {
		// Revert ボタンが押された場合は、情報を戻す.
		if (m_pCurrentShape) {
			compointer<sxsdk::scene_interface> scene(m_pCurrentShape->get_scene_interface());
			const int cou = scene->get_number_of_active_shapes();
			for (int i = 0; i < cou; i++) {
				m_Revert(scene->active_shape(i));
			}
			this->obsolete();		// 再更新.
		}
		return;
	}
}

void CAttributeWindowInterface::popup_menu_value_changed (window_interface::popup_menu_class& popup_menu, void* aux) {
	// マテリアルの切り替えで、tabを変更.
	if (strcmp(popup_menu.get_control_idname(), "material_type") == 0) {
		const int type = popup_menu.get_value();
		if (type != (int)m_materialInfo.type) {
			m_materialInfo.type = (RIBParam::MATERIAL_TYPE)type;
			this->obsolete();
		}
		return;
	}
}

void CAttributeWindowInterface::color_box_value_changed (sxsdk::window_interface::color_box_class &color_box, void *aux)
{
	if (strcmp(color_box.get_control_idname(), "diffuse_diffuseColor") == 0) {
		m_materialInfo.pxrDiffuse.diffuseColor = color_box.get_value();
		return;
	}
	
	if (strcmp(color_box.get_control_idname(), "diffuse_transmissionColor") == 0) {
		m_materialInfo.pxrDiffuse.transmissionColor = color_box.get_value();
		return;
	}

	if (strcmp(color_box.get_control_idname(), "disney_baseColor") == 0) {
		m_materialInfo.pxrDisney.baseColor = color_box.get_value();
		return;
	}

	if (strcmp(color_box.get_control_idname(), "disney_emitColor") == 0) {
		m_materialInfo.pxrDisney.emitColor = color_box.get_value();
		return;
	}

	if (strcmp(color_box.get_control_idname(), "disney_subsurfaceColor") == 0) {
		m_materialInfo.pxrDisney.subsurfaceColor = color_box.get_value();
		return;
	}

	if (strcmp(color_box.get_control_idname(), "glass_reflectionColor") == 0) {
		m_materialInfo.pxrGlass.reflectionColor = color_box.get_value();
		return;
	}

	if (strcmp(color_box.get_control_idname(), "glass_transmissionColor") == 0) {
		m_materialInfo.pxrGlass.transmissionColor = color_box.get_value();
		return;
	}

	if (strcmp(color_box.get_control_idname(), "glass_absorptionColor") == 0) {
		m_materialInfo.pxrGlass.absorptionColor = color_box.get_value();
		return;
	}

	if (strcmp(color_box.get_control_idname(), "constsnt_emitColor") == 0) {
		m_materialInfo.pxrConstant.emitColor = color_box.get_value();
		return;
	}

	if (strcmp(color_box.get_control_idname(), "volume_diffuseColor") == 0) {
		m_materialInfo.pxrVolume.diffuseColor = color_box.get_value();
		return;
	}
	if (strcmp(color_box.get_control_idname(), "volume_emitColor") == 0) {
		m_materialInfo.pxrVolume.emitColor = color_box.get_value();
		return;
	}
	if (strcmp(color_box.get_control_idname(), "volume_densityColor") == 0) {
		m_materialInfo.pxrVolume.densityColor = color_box.get_value();
		return;
	}
}

void CAttributeWindowInterface::number_value_changed (sxsdk::window_interface::number_class &number, void *aux)
{
	if (strcmp(number.get_control_idname(), "maxdiffusedepth") == 0) {
		m_materialInfo.maxdiffusedepth = number.get_value();
		return;
	}
	
	if (strcmp(number.get_control_idname(), "maxspeculardepth") == 0) {
		m_materialInfo.maxspeculardepth = number.get_value();
		return;
	}
	
	if (strcmp(number.get_control_idname(), "disney_subsurface") == 0) {
		m_materialInfo.pxrDisney.subsurface = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "disney_metallic") == 0) {
		m_materialInfo.pxrDisney.metallic = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "disney_specular") == 0) {
		m_materialInfo.pxrDisney.specular = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "disney_specularTint") == 0) {
		m_materialInfo.pxrDisney.specularTint = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "disney_roughness") == 0) {
		m_materialInfo.pxrDisney.roughness = number.get_value();
		return;
	}
	
	if (strcmp(number.get_control_idname(), "disney_anisotropic") == 0) {
		m_materialInfo.pxrDisney.anisotropic = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "disney_sheen") == 0) {
		m_materialInfo.pxrDisney.sheen = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "disney_sheenTint") == 0) {
		m_materialInfo.pxrDisney.sheenTint = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "glass_ior") == 0) {
		m_materialInfo.pxrGlass.ior = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "glass_roughness") == 0) {
		m_materialInfo.pxrGlass.roughness = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "glass_reflectionGain") == 0) {
		m_materialInfo.pxrGlass.reflectionGain = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "glass_transmissionGain") == 0) {
		m_materialInfo.pxrGlass.transmissionGain = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "glass_absorptionGain") == 0) {
		m_materialInfo.pxrGlass.absorptionGain = number.get_value();
		return;
	}

	if (strcmp(number.get_control_idname(), "volume_densityFloat") == 0) {
		m_materialInfo.pxrVolume.densityFloat = number.get_value();
		return;
	}
	if (strcmp(number.get_control_idname(), "volume_densityScale") == 0) {
		m_materialInfo.pxrVolume.densityScale = number.get_value();
		return;
	}
	if (strcmp(number.get_control_idname(), "volume_anisotropy") == 0) {
		m_materialInfo.pxrVolume.anisotropy = number.get_value();
		return;
	}
	if (strcmp(number.get_control_idname(), "volume_maxDensity") == 0) {
		m_materialInfo.pxrVolume.maxDensity = number.get_value();
		return;
	}
}

void CAttributeWindowInterface::checkbox_value_changed (sxsdk::window_interface::checkbox_class &checkbox, void *aux)
{
	if (strcmp(checkbox.get_control_idname(), "use_custom") == 0) {
		const bool chkF = checkbox.get_value() ? true : false;
		if (m_materialInfo.useCustom != chkF) {
			m_materialInfo.useCustom = chkF;
			this->obsolete();	// 再更新(window内のsetup_xxxを呼ぶ).
		}
		return;
	}

	if (strcmp(checkbox.get_control_idname(), "use_depth") == 0) {
		const bool chkF = checkbox.get_value() ? true : false;
		if (m_materialInfo.useDepth != chkF) {
			m_materialInfo.useDepth = chkF;
			this->obsolete();
		}
		return;
	}

	if (strcmp(checkbox.get_control_idname(), "visibility_camera") == 0) {
		m_materialInfo.visibilityCamera = checkbox.get_value();
		return;
	}

	if (strcmp(checkbox.get_control_idname(), "visibility_diffuse") == 0) {
		m_materialInfo.visibilityDiffuse = checkbox.get_value();
		return;
	}

	if (strcmp(checkbox.get_control_idname(), "visibility_specular") == 0) {
		m_materialInfo.visibilitySpecular = checkbox.get_value();
		return;
	}

	if (strcmp(checkbox.get_control_idname(), "visibility_indirect") == 0) {
		m_materialInfo.visibilityIndirect = checkbox.get_value();
		return;
	}

	if (strcmp(checkbox.get_control_idname(), "volume_multiScatter") == 0) {
		m_materialInfo.pxrVolume.multiScatter = checkbox.get_value();
		return;
	}
}

//------------------------------------------.
// 選択の変更などのコールバック.
//------------------------------------------.
void CAttributeWindowInterface::active_shapes_changed (bool &b, sxsdk::scene_interface *scene, int old_n, sxsdk::shape_class *const *old_shapes, int n, sxsdk::shape_class *const *shapes, void *aux)
{
	m_pCurrentShape = NULL;
	if (!scene || !this->is_shown()) return;

	// 形状を変更して、パラメータを切り替え.
	// active_shapes_changed の後にsetup_xxx 関数が呼ばれるため、ここではm_materialInfoの更新だけでよい.
	if (n >= 1 && shapes && shapes[0] != NULL) {
		m_ChangeShape(scene, *(shapes[0]));
	}
}

/**
 * 形状の選択が変更された場合に呼ばれる.
 */
void CAttributeWindowInterface::m_ChangeShape (sxsdk::scene_interface* scene, sxsdk::shape_class& shape)
{
	m_pCurrentShape    = NULL;
	m_pOldCurrentShape = NULL;
	m_materialInfo.Clear();
	m_oldMaterialInfo.Clear();

	if (shape.get_type() == sxsdk::enums::master_surface) {
		m_pOldCurrentShape = m_pCurrentShape = &shape;

		if (StreamCtrl::HasRIBMaterial(*m_pCurrentShape)) {
			// マテリアル情報をstreamから読み込み.
			m_materialInfo = StreamCtrl::LoadRIBMaterial(*m_pCurrentShape);

		} else {
			// 現在のマスターサーフェスからコンバートした設定を割り当て.
			CMaterialInfo materialInfo;
			materialInfo.SetMaterial(scene, *shape.get_master_surface());
			MaterialCtrl::ConvShade3DToRIS(materialInfo, m_materialInfo);
		}

		m_oldMaterialInfo = m_materialInfo;
	}
}

/**
 * 情報をstreamに保存 (Applyボタンが押された).
 */
void CAttributeWindowInterface::m_Apply (sxsdk::shape_class& shape)
{
	if (shape.get_type() != sxsdk::enums::master_surface) return;

	// streamにマテリアル情報を保存.
	StreamCtrl::SaveRIBMaterial(shape, m_materialInfo);
	m_oldMaterialInfo = m_materialInfo;
}

/**
 * 情報を戻す (Revertボタンが押された).
 */
void CAttributeWindowInterface::m_Revert (sxsdk::shape_class& shape)
{
	if (shape.get_type() != sxsdk::enums::master_surface) return;
	m_materialInfo = m_oldMaterialInfo;
}

/**
 * ウィンドウを開いた直後、形状の選択がない状態の場合の設定の再読み込み.
 */
void CAttributeWindowInterface::m_Reload ()
{
	m_pCurrentShape = NULL;
	try {
		compointer<sxsdk::scene_interface> scene(shade.get_scene_interface());
		if (!scene) return;

		m_pCurrentShape = m_GetCurrentShape(scene);
		if (!m_pCurrentShape) {
			m_pOldCurrentShape = NULL;
			return;
		}
		if (m_pCurrentShape && m_pOldCurrentShape && m_pCurrentShape == m_pOldCurrentShape) {
			return;
		}
		m_ChangeShape(scene, *m_pCurrentShape);
	} catch (...) { }
}
