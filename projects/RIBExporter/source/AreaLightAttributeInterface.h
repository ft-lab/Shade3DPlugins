/**
 * 面光源の情報を設定する.
 */

#ifndef _AREALIGHT_ATTRIBUTE_INTERFACE_H
#define _AREALIGHT_ATTRIBUTE_INTERFACE_H

#include "GlobalHeader.h"
#include "LightCtrl.h"

class CAreaLightAttributeInterface : public sxsdk::attribute_interface
{
private:
	sxsdk::shade_interface& shade;
	CPxrAreaLight m_data;				// 面光源情報.

	virtual int get_shade_version () const { return SHADE_BUILD_NUMBER; }
	virtual sx::uuid_class get_uuid (void * = 0) { return RIB_AREA_LIGHT_ATTRIBUTE_ID; }

	// 形状のカスタム情報を有効化.
	virtual void accepts_shape (bool &accept, void *) { accept = true; }

	// カスタム情報が選択された.
	virtual bool ask_shape (sxsdk::shape_class &shape, void * = 0);

	/**
	 * 面光源情報よりm_dataを初期化.
	 */
	void m_InitAreaLightData (sxsdk::shape_class &shape, const bool loadStream);

	//--------------------------------------------------//
	//	ダイアログのイベント処理用						//
	//--------------------------------------------------//
	// ダイアログの初期化.
	virtual void initialize_dialog (sxsdk::dialog_interface &d, void * = 0);

	// ダイアログのイベントを受け取る.
	virtual bool respond (sxsdk::dialog_interface &d, sxsdk::dialog_item_class &item, int action, void * = 0);

	// ダイアログのデータを設定する.
	virtual void load_dialog_data (sxsdk::dialog_interface &d, void * = 0);

	// 値の変更を保存するときに呼ばれる.
	virtual void save_dialog_data (sxsdk::dialog_interface &d, void * = 0);

public:
	CAreaLightAttributeInterface (sxsdk::shade_interface& shade);

	static const char *name(sxsdk::shade_interface *shade) { return shade->gettext("rib_attribute_title"); }
};

#endif
