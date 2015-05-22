/**
 * 背景をパノラマのテクスチャとして生成.
 */

#include "BackgroundTexture.h"

CBackgroundTexture::CBackgroundTexture (sxsdk::shade_interface& shade) : shade(shade)
{
}

/**
 * 背景のパノラマ画像のテクスチャを生成.
 * returnで取得したsxsdk::image_interface* は、compointerで管理のこと.
 */
sxsdk::image_interface* CBackgroundTexture::CalcBackgroundTextureImage (sxsdk::scene_interface* scene, const int texWidth, const int texHeight)
{
	sxsdk::image_interface* image = NULL;

	try {
		compointer<sxsdk::background_interface> background(scene->get_background_interface());

		image = scene->create_image_interface(sx::vec<int,2>(texWidth, texHeight), 64);

		const int texWidthH  = texWidth >> 1;
		const int texHeightH = texHeight >> 1;

		const float dTheta = sx::pi * 2.0f / (float)texWidth;
		const float dPhy   = sx::pi / (float)texHeight;

		std::vector<sxsdk::rgba_class> lines;
		lines.resize(texWidth);

		const float pxPos = -sx::pi * 0.5f;
		sxsdk::vec3 v;
		float py = 0.0f;
		for (int y = 0; y < texHeight; y++) {
			const float sinT = std::sin(py);
			const float cosT = std::cos(py);

			float px = pxPos;
			for (int x = 0; x < texWidth; x++) {
				const float sinP = std::sin(px);
				const float cosP = std::cos(px);

				// 極座標からデカルト座標に変換.
				v.x = sinT * cosP;
				v.y = cosT;
				v.z = sinT * sinP;

				// 指定の方向での色を計算.
				lines[x] = sxsdk::rgba_class(background->calculate_background_color(v), 1.0f);

				px += dTheta;
			}
			image->set_pixels_rgba_float(0, y, texWidth, 1, &lines[0]);

			py += dPhy;
		}
		image->update();

		return image;

	} catch (...) { }

	return NULL;
}

