/**
 * RIBでのMaterial相当の情報を指定するウィンドウ.
 */
#ifndef _ATTRIBUTE_WINDOW_INTERFACE_H
#define _ATTRIBUTE_WINDOW_INTERFACE_H

#include "GlobalHeader.h"
#include "MaterialCtrl.h"

class CAttributeWindowInterface : public sxsdk::window_interface {
private:
	sxsdk::shade_interface &shade;

	sxsdk::shape_class* m_pCurrentShape;	// カレントの形状.
	sxsdk::shape_class* m_pOldCurrentShape;

	std::string m_please_select_master_surface_text;		// マスターサーフェスが選択されていない場合に表示するメッセージ .

	// マテリアル情報.
	CRISMaterialInfo m_materialInfo;
	CRISMaterialInfo m_oldMaterialInfo;		// 古いマテリアル情報（Revert用）.

private:
	virtual int get_shade_version () const { return SHADE_BUILD_NUMBER; }
	virtual sx::uuid_class get_uuid (void *) { return RIB_ATTRIBUTE_WINDOW_ID; }

	virtual int get_placement_flags (void *aux=0) {
		return sxsdk::window_interface::view_menu_placement_flag;
	}
	virtual int get_flags (void *aux=0) {
		return 0;
	}

	/**
	 * 初期化処理.
	 */
	virtual void initialize (void *);

	//------------------------------------------.
	// setup時のコールバック.
	//------------------------------------------.
	virtual bool setup_static_text (sxsdk::window_interface::static_text_class &static_text, void *aux=0);
	virtual bool setup_push_button (sxsdk::window_interface::push_button_class &push_button, void *aux=0);
	virtual bool setup_tab (sxsdk::window_interface::tab_class &tab, void *aux=0);
	virtual bool setup_checkbox (sxsdk::window_interface::checkbox_class &checkbox, void *aux=0);
	virtual bool setup_popup_menu (sxsdk::window_interface::popup_menu_class &popup_menu, void *aux=0);
	virtual bool setup_color_box (sxsdk::window_interface::color_box_class &color_box, void *aux=0);
	virtual bool setup_number (sxsdk::window_interface::number_class &number, void *aux=0);

	//------------------------------------------.
	// イベント処理のコールバック.
	//------------------------------------------.
	virtual void tab_value_changed (sxsdk::window_interface::tab_class &tab, void *aux=0);
	virtual void push_button_clicked (window_interface::push_button_class& push_button, void* aux = 0);
	virtual void popup_menu_value_changed (window_interface::popup_menu_class& popup_menu, void* aux = 0);
	virtual void color_box_value_changed (sxsdk::window_interface::color_box_class &color_box, void *aux=0);
	virtual void number_value_changed (sxsdk::window_interface::number_class &number, void *aux=0);
	virtual void checkbox_value_changed (sxsdk::window_interface::checkbox_class &checkbox, void *aux=0);

	//------------------------------------------.
	// 選択の変更などのコールバック.
	//------------------------------------------.
	virtual void active_shapes_changed (bool &b, sxsdk::scene_interface *scene, int old_n, sxsdk::shape_class *const *old_shapes, int n, sxsdk::shape_class *const *shapes, void *aux=0);

	//------------------------------------------.
	/**
	 * シーンより、カレント形状の参照を取得.
	 */
	sxsdk::shape_class* m_GetCurrentShape (sxsdk::scene_interface* scene);

	/**
	 * 形状の選択が変更された場合に呼ばれる.
	 */
	void m_ChangeShape (sxsdk::scene_interface* scene, sxsdk::shape_class& shape);

	/**
	 * ウィンドウを開いた直後、形状の選択がない状態の場合の設定の再読み込み.
	 */
	void m_Reload ();

	/**
	 * 情報をstreamに保存 (Applyボタンが押された).
	 */
	void m_Apply (sxsdk::shape_class& shape);

	/**
	 * 情報を戻す (Revertボタンが押された).
	 */
	void m_Revert (sxsdk::shape_class& shape);

public:
	explicit CAttributeWindowInterface (sxsdk::shade_interface &shade) : sxsdk::window_interface(shade), shade(shade) { }

	static const char *name (sxsdk::shade_interface *shade) { return shade->gettext("renderman_attribute_title"); }
};

#endif
