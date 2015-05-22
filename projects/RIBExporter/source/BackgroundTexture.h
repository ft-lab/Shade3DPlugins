/**
 * 背景をパノラマのテクスチャとして生成.
 */
#ifndef _BACKGROUNDTEXTURE_H
#define _BACKGROUNDTEXTURE_H

#include "GlobalHeader.h"

class CBackgroundTexture
{
private:
	sxsdk::shade_interface& shade;

public:
	CBackgroundTexture (sxsdk::shade_interface& shade);

	/**
	 * 背景のパノラマ画像のテクスチャを生成.
	 * returnで取得したsxsdk::image_interface* は、compointerで管理のこと.
	 */
	sxsdk::image_interface* CalcBackgroundTextureImage (sxsdk::scene_interface* scene, const int texWidth = 1024, const int texHeight = 512);
};

#endif
