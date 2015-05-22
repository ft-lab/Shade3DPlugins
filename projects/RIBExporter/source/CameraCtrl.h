/**
 * Shade 3Dのカメラ情報をRIBの形に変換.
 */
#ifndef _CAMERACTRL_H
#define _CAMERACTRL_H

#include "GlobalHeader.h"

class CCameraCtrl
{
private:
	sxsdk::scene_interface* m_pScene;

	sxsdk::vec3 m_translate;				// 移動情報.
	sxsdk::vec3 m_rotateDegrees;			// 回転情報 (度数).
	float m_fovDegrees;						// 視野角度 (度数).
	sx::vec<int,2> m_renderingImageSize;	// レンダリング画像サイズ.

	/**
	 * RenderMan向けの情報の計算.
	 */
	void m_Init ();

public:
	CCameraCtrl (sxsdk::scene_interface* scene);

	/**
	 * 視野角度 (度数)を取得.
	 */
	float GetFOVDegress () { return m_fovDegrees; }

	/**
	 * 移動情報を取得.
	 */
	sxsdk::vec3 GetTranslate () { return m_translate; }

	/**
	 * 回転情報 (度数)を取得.
	 */
	sxsdk::vec3 GetRotateDegress () { return m_rotateDegrees; }

	/**
	 * 画像サイズ.
	 */
	sx::vec<int,2> GetRenderingImageSize() { return m_renderingImageSize; }
};

#endif
