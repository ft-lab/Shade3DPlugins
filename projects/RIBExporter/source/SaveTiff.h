/**
 * tiffファイルの保存.
 * RenderManで扱えるテクスチャは、tiffのヘッダにTIFFTAG_PIXAR_TEXTUREFORMATの指定がいる.
 */

#ifndef _SAVETIFF_H
#define _SAVETIFF_H

#include "GlobalHeader.h"

class CSaveTiff
{
private:
	sxsdk::scene_interface* m_pScene;

	/**
	 * 2の累乗にリサイズした画像を生成.
	 * returnで取得したsxsdk::image_interface* は、compointerで管理のこと.
	 */
	sxsdk::image_interface* m_ResizeImage (sxsdk::image_interface* image, const bool useAlpha = false);

	/**
	 * hdrなどのfloatのピクセル情報を持つ場合のRGB出力.
	 */
	bool m_SavePRManImageFloat (sxsdk::image_interface* image, const std::string& saveFileName, const bool latLongEnvironment = false);

	/**
	 * リサイズした画像を生成.
	 * image_interface::duplicate_imageでは、アルファ成分は無視されるためアルファは別途行う.
	 */
	sxsdk::image_interface* m_DuplicateImage(sxsdk::image_interface* srcImage, const sx::vec<int,2>& dstSize, const bool useAlpha);

public:
	CSaveTiff (sxsdk::scene_interface* scene);
	~CSaveTiff ();

	/**
	 * RenderMan向けのRGB画像を出力.
	 */
	bool SavePRManImage (sxsdk::image_interface* image, const std::string& saveFileName, const bool latLongEnvironment = false, const bool useAlpha = false);

	/**
	 * RenderMan向けのR値のみで画像を出力(Gray Scale).
	 */
	bool SavePRManImageGrayScale (sxsdk::image_interface* image, const std::string& saveFileName);
};

#endif
