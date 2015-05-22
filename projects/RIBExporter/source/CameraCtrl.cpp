/**
 * Shade 3Dのカメラ情報をRIBの形に変換.
 */

#include "CameraCtrl.h"
#include "MathUtil.h"

CCameraCtrl::CCameraCtrl (sxsdk::scene_interface* scene) : m_pScene(scene)
{
	m_Init();
}

/**
 * RenderMan向けの情報の計算.
 */
void CCameraCtrl::m_Init ()
{
	m_renderingImageSize = sx::vec<int,2>(640, 480);
	m_fovDegrees    = 45.0f;
	m_translate     = sxsdk::vec3(0, 0, 1000.0f);
	m_rotateDegrees = sxsdk::vec3(0, 0, 0);
	if (!m_pScene) return;

	// レンダリング画像サイズを取得.
	try {
		compointer<sxsdk::rendering_interface> rendering(m_pScene->get_rendering_interface());
		if (!rendering) return;
		m_renderingImageSize = rendering->get_image_size();
	} catch (...) { }

	// カメラの変換行列より、移動と回転値を取得.
	{
		sxsdk::mat4 wvMat = m_pScene->get_world_to_view_matrix();

		sxsdk::vec3 scale, translate, rotate, shear;
		wvMat.unmatrix(scale, shear, rotate, translate);

		float flipF = 1.0f;
		if (scale.x < 0.0f && scale.y < 0.0f && scale.z < 0.0f) {
			flipF = -1.0f;
			scale = -scale;
		}

		// 右手系から左手系の回転補正.
		if (flipF < 0.0f) {
			translate.z *= flipF;
			rotate.x *= flipF;
			rotate.y *= flipF;
			rotate.z -= sx::pi;
		}

		m_translate     = translate;
		m_rotateDegrees = sxsdk::vec3(rotate.x * 180.0f / sx::pi, rotate.y * 180.0f / sx::pi, rotate.z * 180.0f / sx::pi);
	}

	// 視野角度の計算.
	const float screenDSize  = 36.0f;		// スクリーンの長辺は36mm固定.
	const float screenDSizeH = screenDSize * 0.5f;

	sxsdk::camera_class& camera = m_pScene->get_camera();
	const float cameraZoom  = camera.get_zoom();

	const float aspect = (float)m_renderingImageSize.x / (float)m_renderingImageSize.y;
	float dH = 0.0f;
	if (m_renderingImageSize.x > m_renderingImageSize.y) {
		dH = screenDSizeH / aspect;
	} else {
		dH = screenDSizeH * aspect;
	}

	const float cosV = cameraZoom / std::sqrt(dH * dH + cameraZoom * cameraZoom);
	m_fovDegrees = 2.0f * std::acos(cosV) * 180.0f / sx::pi;
}
