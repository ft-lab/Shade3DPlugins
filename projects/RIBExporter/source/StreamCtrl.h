/**
 * 情報保存用.
 */
#ifndef _STREAMCTRL_H
#define _STREAMCTRL_H

#include "GlobalHeader.h"
#include "MaterialCtrl.h"

namespace StreamCtrl
{
	/**
	 * RIBエクスポートのダイアログパラメータをstreamに保存.
	 */
	void SaveRIBExportDlg (sxsdk::scene_interface* scene, const RIBExportData& data);

	/**
	 * RIBエクスポートのダイアログパラメータをstreamから呼び出し.
	 */
	RIBExportData LoadRIBExportDlg (sxsdk::scene_interface* scene);

	/**
	 * RenderManのマテリアルパラメータを持つか.
	 */
	bool HasRIBMaterial (sxsdk::shape_class& shape);

	/**
	 * 指定の形状（マスターサーフェス）にRenderManのパラメータを保存.
	 */
	void SaveRIBMaterial (sxsdk::shape_class& shape, const CRISMaterialInfo& data);

	/**
	 * 指定の形状（マスターサーフェス）のRenderManのパラメータを取得.
	 */
	CRISMaterialInfo LoadRIBMaterial (sxsdk::shape_class& shape);
}

#endif
