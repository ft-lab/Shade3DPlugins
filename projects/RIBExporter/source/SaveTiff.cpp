/**
 * tiffファイルの保存.
 * RenderManで扱えるテクスチャは、tiffのヘッダにTIFFTAG_PIXAR_TEXTUREFORMATの指定がいる.
 */

#include "SaveTiff.h"
#include "tiff.h"
#include "tiffio.h"

CSaveTiff::CSaveTiff (sxsdk::scene_interface* scene) : m_pScene(scene)
{
}

CSaveTiff::~CSaveTiff ()
{
}

/**
 * 2の累乗にリサイズした画像を生成.
 * returnで取得したsxsdk::image_interface* は、compointerで管理のこと.
 */
sxsdk::image_interface* CSaveTiff::m_ResizeImage (sxsdk::image_interface* image, const bool useAlpha)
{
	sxsdk::image_interface* srcImage = NULL;

	int src_wid = image->get_size().x;
	int src_hei = image->get_size().y;
	int wid = src_wid;
	int hei = src_hei;

	const int maxSize = 8192;
	{
		if (wid >= maxSize) wid = maxSize;
		else {
			int texSize = 8;
			while (texSize <= maxSize) {
				if (wid <= texSize + (texSize >> 1)) {
					wid = texSize;
					break;
				}
				texSize = texSize + texSize;
			}
		}
	}
	{
		if (hei >= maxSize) hei = maxSize;
		else {
			int texSize = 8;
			while (texSize <= maxSize) {
				if (hei <= texSize + (texSize >> 1)) {
					hei = texSize;
					break;
				}
				texSize = texSize + texSize;
			}
		}
	}

	sx::vec<int,2> size(wid, hei);

	if (image->has_real_color()) {
		return image->duplicate_image(&size, true, 64);

	} else {
		return m_DuplicateImage(image, size, useAlpha);
	}

	return NULL;
}

/**
 * リサイズした画像を生成.
 * image_interface::duplicate_imageでは、アルファ成分は無視されるためアルファは別途行う.
 */
sxsdk::image_interface* CSaveTiff::m_DuplicateImage(sxsdk::image_interface* srcImage, const sx::vec<int,2>& dstSize, const bool useAlpha)
{
	sxsdk::image_interface* image2 = srcImage->duplicate_image(&dstSize);
	if (!image2) return NULL;

	if (useAlpha) {
		// アルファ成分を取り出してRedに入れた画像を生成.
		const sx::vec<int,2> srcSize = srcImage->get_size();
		const int srcWidth  = srcSize.x;
		const int srcHeight = srcSize.y;
		compointer<sxsdk::image_interface> alphaImage(m_pScene->create_image_interface(srcSize));
		try {
			std::vector<sx::rgba8_class> lines;
			lines.resize(srcWidth);

			for (int y = 0; y < srcHeight; y++) {
				srcImage->get_pixels_rgba(0, y, srcWidth, 1, &lines[0]);
				for (int x = 0; x < srcWidth; x++) {
					lines[x] = sx::rgba8_class(lines[x].alpha, 0, 0, 255);
				}
				alphaImage->set_pixels_rgba(0, y, srcWidth, 1, &lines[0]);
			}
			alphaImage->update();
		} catch (...) {
			return NULL;
		}

		// alphaImage をリサイズして、image2のアルファ成分にアルファ値を入れる.
		compointer<sxsdk::image_interface> alphaImage2(alphaImage->duplicate_image(&dstSize));
		try {
			const int dstWidth  = dstSize.x;
			const int dstHeight = dstSize.y;

			std::vector<sx::rgba8_class> lines, Alines;
			lines.resize(dstWidth);
			Alines.resize(dstWidth);

			for (int y = 0; y < dstHeight; y++) {
				image2->get_pixels_rgba(0, y, dstWidth, 1, &lines[0]);
				alphaImage2->get_pixels_rgba(0, y, dstWidth, 1, &Alines[0]);
				for (int x = 0; x < dstWidth; x++) {
					lines[x].alpha = Alines[x].red;
				}
				image2->set_pixels_rgba(0, y, srcWidth, 1, &lines[0]);
			}
			image2->update();
		} catch (...) {
			return NULL;
		}
	}
	return image2;
}

/**
 * RenderMan向けのRGB画像を出力.
 * 参考 : http://marc.info/?l=kde-kimageshop&m=118164368923788&w=2
 */
bool CSaveTiff::SavePRManImage (sxsdk::image_interface* image, const std::string& saveFileName, const bool latLongEnvironment, const bool useAlpha)
{
	// floatのRGBを持つものは、float x 3のピクセル情報で格納する.
	if (image->has_real_color()) {
		return m_SavePRManImageFloat(image, saveFileName, latLongEnvironment);
	}

	// 画像は2の累乗サイズである必要あり.
	compointer<sxsdk::image_interface> srcImage(m_ResizeImage(image, useAlpha));

	// tiffの書き込みとしてファイルオープン.
	TIFF* tiffImage = TIFFOpen(saveFileName.c_str(), "w");
	if (tiffImage == NULL) return false;

	const int srcWidth  = srcImage->get_size().x;
	const int srcHeight = srcImage->get_size().y;

	const int tileWidth  = 64;
	const int tileHeight = 64;

	std::vector<unsigned char> tileBuffer;
	tileBuffer.resize(tileWidth * tileHeight);

	std::vector<sx::rgba8_class> srcBuffer;
	srcBuffer.resize(tileWidth * tileHeight);

	const int planarCount = useAlpha ? 4 : 3;

	compointer<sxsdk::image_interface> tempImage = srcImage;

	// mipmapとして複数テクスチャを格納していく.
	int width  = srcWidth;
	int height = srcHeight;
	while (width >= 32 && height >= 32) {

		sx::vec<int,2> size(width, height);
		compointer<sxsdk::image_interface> image2(m_DuplicateImage(tempImage, size, useAlpha));

		TIFFSetField(tiffImage, TIFFTAG_IMAGEWIDTH, width);						// 画像の幅.
		TIFFSetField(tiffImage, TIFFTAG_IMAGELENGTH, height);					// 画像の高さ.
		TIFFSetField(tiffImage, TIFFTAG_BITSPERSAMPLE, 8);						// 1バイトでのビット数 (8固定).
		TIFFSetField(tiffImage, TIFFTAG_SAMPLESPERPIXEL, planarCount);			// 1pixelでの要素数(RGBまたはRGBAの3つ).
	
		TIFFSetField(tiffImage, TIFFTAG_COMPRESSION, COMPRESSION_LZW);			// 圧縮方式.

		TIFFSetField(tiffImage, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);			// RGBカラーを持つ.
		TIFFSetField(tiffImage, TIFFTAG_XRESOLUTION, 1.0);
		TIFFSetField(tiffImage, TIFFTAG_YRESOLUTION, 1.0);
		TIFFSetField(tiffImage, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
		TIFFSetField(tiffImage, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(tiffImage, TIFFTAG_SOFTWARE, "libtiff");
	
		//--------------------------------------.
		// 以下、RenderMan向けの指定.
		// PLANARCONFIG_SEPARATEの場合は、Redを先に格納、Greenを次にまとめて、Blueを次にまとめて、という順番に格納する.
		TIFFSetField(tiffImage, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE);
		TIFFSetField(tiffImage, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);

		// タイル状に、64x64 pixelごとに左上から右下に格納.
		TIFFSetField(tiffImage, TIFFTAG_TILEWIDTH, tileWidth);
		TIFFSetField(tiffImage, TIFFTAG_TILELENGTH, tileHeight);

		TIFFSetField(tiffImage, TIFFTAG_PIXAR_TEXTUREFORMAT, "Plain Texture");		// RenderMan向けのテクスチャとして出力.
		TIFFSetField(tiffImage, TIFFTAG_PIXAR_WRAPMODES, "periodic,periodic");		// テクスチャのWrap情報.

		TIFFSetField(tiffImage, TIFFTAG_PIXAR_FOVCOT, 1.0);							// Plain Textureでも必須.

		// 画像は、Red/Green/Blue/(Alpha) の順番に、tileWidth x tileHeightのブロックごとに格納していく.
		for (int colorLoop = 0; colorLoop < planarCount; colorLoop++) {
			for (int iy = 0; iy < height; iy += tileHeight) {
				for (int ix = 0; ix < width; ix += tileWidth) {

					// 画像からピクセル情報を読み込み.
					image2->get_pixels_rgba(ix, iy, tileWidth, tileHeight, &srcBuffer[0]);

					// (ix, iy)の位置から、wid x heiの画像をコピー.
					int iPos = 0;
					if (colorLoop == 0) {
						for (int y = 0; y < tileHeight; y++) {
							if (y + iy >= height) break;
							for (int x = 0; x < tileWidth; x++) {
								if (x + ix < width) {
									tileBuffer[iPos] = (unsigned char)srcBuffer[iPos].red;
								}
								iPos++;
							}
						}
					} else if (colorLoop == 1) {
						for (int y = 0; y < tileHeight; y++) {
							if (y + iy >= height) break;
							for (int x = 0; x < tileWidth; x++) {
								if (x + ix < width) {
									tileBuffer[iPos] = (unsigned char)srcBuffer[iPos].green;
								}
								iPos++;
							}
						}
					} else if (colorLoop == 2) {
						for (int y = 0; y < tileHeight; y++) {
							if (y + iy >= height) break;
							for (int x = 0; x < tileWidth; x++) {
								if (x + ix < width) {
									tileBuffer[iPos] = (unsigned char)srcBuffer[iPos].blue;
								}
								iPos++;
							}
						}
					} else {
						for (int y = 0; y < tileHeight; y++) {
							if (y + iy >= height) break;
							for (int x = 0; x < tileWidth; x++) {
								if (x + ix < width) {
									tileBuffer[iPos] = (unsigned char)srcBuffer[iPos].alpha;
								}
								iPos++;
							}
						}
					}

					TIFFWriteTile(tiffImage, &tileBuffer[0], ix, iy, 0, colorLoop);
				}
			}
		}
		TIFFWriteDirectory(tiffImage);

		width  >>= 1;
		height >>= 1;

		tempImage = image2;
	}

	TIFFClose(tiffImage);

	return true;
}

/**
 * hdrなどのfloatのピクセル情報を持つ場合のRGB出力.
 * latLongEnvironment がtrueの場合は、「LatLong Environment」のパノラマ画像として使用.
 */
bool CSaveTiff::m_SavePRManImageFloat (sxsdk::image_interface* image, const std::string& saveFileName, const bool latLongEnvironment)
{
	// 画像は2の累乗サイズである必要あり.
	compointer<sxsdk::image_interface> srcImage(m_ResizeImage(image));

	// tiffの書き込みとしてファイルオープン.
	TIFF* tiffImage = TIFFOpen(saveFileName.c_str(), "w");
	if (tiffImage == NULL) return false;

	const int srcWidth  = srcImage->get_size().x;
	const int srcHeight = srcImage->get_size().y;
	const int depth     = 64;

	const int tileWidth  = 32;		// 変更 : 64 ==> 32.
	const int tileHeight = 32;		// 変更 : 64 ==> 32.

	std::vector<float> tileBuffer;
	tileBuffer.resize(tileWidth * tileHeight);

	std::vector<sxsdk::rgba_class> srcBuffer;
	srcBuffer.resize(tileWidth * tileHeight);

	compointer<sxsdk::image_interface> tempImage = srcImage;

	// mipmapとして複数テクスチャを格納していく.
	int width  = srcWidth;
	int height = srcHeight;
	while (width >= 32 && height >= 32) {
		sx::vec<int,2> size(width, height);
		compointer<sxsdk::image_interface> image2(tempImage->duplicate_image(&size, true, depth));

		TIFFSetField(tiffImage, TIFFTAG_IMAGEWIDTH, width);						// 画像の幅.
		TIFFSetField(tiffImage, TIFFTAG_IMAGELENGTH, height);					// 画像の高さ.
		TIFFSetField(tiffImage, TIFFTAG_BITSPERSAMPLE, 32);						// 変更 : 1バイトでのビット数 (float型).
		TIFFSetField(tiffImage, TIFFTAG_SAMPLESPERPIXEL, 3);					// 1pixelでの要素数(RGBの3つ).
	
		TIFFSetField(tiffImage, TIFFTAG_COMPRESSION, COMPRESSION_LZW);			// 圧縮方式.

		TIFFSetField(tiffImage, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);			// RGBカラーを持つ.
		TIFFSetField(tiffImage, TIFFTAG_XRESOLUTION, 1.0);
		TIFFSetField(tiffImage, TIFFTAG_YRESOLUTION, 1.0);
		TIFFSetField(tiffImage, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
		TIFFSetField(tiffImage, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(tiffImage, TIFFTAG_SOFTWARE, "libtiff");

		TIFFSetField(tiffImage, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);		// 追加 : float値で格納.

		//--------------------------------------.
		// 以下、RenderMan向けの指定.
		// PLANARCONFIG_SEPARATEの場合は、Redを先に格納、Greenを次にまとめて、Blueを次にまとめて、という順番に格納する.
		TIFFSetField(tiffImage, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE);
		TIFFSetField(tiffImage, TIFFTAG_PREDICTOR, PREDICTOR_NONE);				// 変更 : floatで格納する際はPREDICTOR_NONEを指定.

		// タイル状に、64x64 pixelごとに左上から右下に格納.
		TIFFSetField(tiffImage, TIFFTAG_TILEWIDTH, tileWidth);
		TIFFSetField(tiffImage, TIFFTAG_TILELENGTH, tileHeight);

		if (latLongEnvironment) {
			TIFFSetField(tiffImage, TIFFTAG_PIXAR_TEXTUREFORMAT, "LatLong Environment");		// RenderMan向けのテクスチャとして出力.
		} else {
			TIFFSetField(tiffImage, TIFFTAG_PIXAR_TEXTUREFORMAT, "Plain Texture");				// RenderMan向けのテクスチャとして出力.
		}
		TIFFSetField(tiffImage, TIFFTAG_PIXAR_WRAPMODES, "periodic,periodic");				// テクスチャのWrap情報.

		TIFFSetField(tiffImage, TIFFTAG_PIXAR_FOVCOT, 1.0);							// Plain Textureでも必須.

		// 画像は、Red/Green/Blueの順番に、tileWidth x tileHeightのブロックごとに格納していく.
		for (int colorLoop = 0; colorLoop < 3; colorLoop++) {
			for (int iy = 0; iy < height; iy += tileHeight) {
				for (int ix = 0; ix < width; ix += tileWidth) {

					// 画像からピクセル情報を読み込み.
					image2->get_pixels_rgba_float(ix, iy, tileWidth, tileHeight, &srcBuffer[0]);

					// (ix, iy)の位置から、wid x heiの画像をコピー.
					int iPos = 0;
					if (colorLoop == 0) {
						for (int y = 0; y < tileHeight; y++) {
							if (y + iy >= height) break;
							for (int x = 0; x < tileWidth; x++) {
								if (x + ix < width) {
									tileBuffer[iPos] = srcBuffer[iPos].red;
								}
								iPos++;
							}
						}
					} else if (colorLoop == 1) {
						for (int y = 0; y < tileHeight; y++) {
							if (y + iy >= height) break;
							for (int x = 0; x < tileWidth; x++) {
								if (x + ix < width) {
									tileBuffer[iPos] = srcBuffer[iPos].green;
								}
								iPos++;
							}
						}
					} else {
						for (int y = 0; y < tileHeight; y++) {
							if (y + iy >= height) break;
							for (int x = 0; x < tileWidth; x++) {
								if (x + ix < width) {
									tileBuffer[iPos] = srcBuffer[iPos].blue;
								}
								iPos++;
							}
						}
					}

					TIFFWriteTile(tiffImage, &tileBuffer[0], ix, iy, 0, colorLoop);
				}
			}
		}
		TIFFWriteDirectory(tiffImage);

		width  >>= 1;
		height >>= 1;

		tempImage = image2;
	}

	TIFFClose(tiffImage);

	return true;
}

/**
 * RenderMan向けのR値のみで画像を出力(Gray Scale).
 */
bool CSaveTiff::SavePRManImageGrayScale (sxsdk::image_interface* image, const std::string& saveFileName)
{
	// 画像は2の累乗サイズである必要あり.
	compointer<sxsdk::image_interface> srcImage(m_ResizeImage(image));

	// tiffの書き込みとしてファイルオープン.
	TIFF* tiffImage = TIFFOpen(saveFileName.c_str(), "w");
	if (tiffImage == NULL) return false;

	const int srcWidth  = srcImage->get_size().x;
	const int srcHeight = srcImage->get_size().y;

	const int tileWidth  = 64;
	const int tileHeight = 64;

	std::vector<unsigned char> tileBuffer;
	tileBuffer.resize(tileWidth * tileHeight);

	std::vector<sx::rgba8_class> srcBuffer;
	srcBuffer.resize(tileWidth * tileHeight);

	compointer<sxsdk::image_interface> tempImage = srcImage;

	// mipmapとして複数テクスチャを格納していく.
	int width  = srcWidth;
	int height = srcHeight;
	while (width >= 32 && height >= 32) {
		sx::vec<int,2> size(width, height);
		compointer<sxsdk::image_interface> image2(tempImage->duplicate_image(&size));

		TIFFSetField(tiffImage, TIFFTAG_IMAGEWIDTH, width);						// 画像の幅.
		TIFFSetField(tiffImage, TIFFTAG_IMAGELENGTH, height);					// 画像の高さ.
		TIFFSetField(tiffImage, TIFFTAG_BITSPERSAMPLE, 8);						// 1バイトでのビット数 (8固定).
		TIFFSetField(tiffImage, TIFFTAG_SAMPLESPERPIXEL, 1);					// 1pixelでの要素数(Gray scaleなので1つ).
	
		TIFFSetField(tiffImage, TIFFTAG_COMPRESSION, COMPRESSION_LZW);			// 圧縮方式.

		TIFFSetField(tiffImage, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MASK);			// Maskを持つ.
		TIFFSetField(tiffImage, TIFFTAG_XRESOLUTION, 1.0);
		TIFFSetField(tiffImage, TIFFTAG_YRESOLUTION, 1.0);
		TIFFSetField(tiffImage, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
		TIFFSetField(tiffImage, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(tiffImage, TIFFTAG_SOFTWARE, "libtiff");
	
		//--------------------------------------.
		// 以下、RenderMan向けの指定.
		TIFFSetField(tiffImage, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE);
		TIFFSetField(tiffImage, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);

		// タイル状に、64x64 pixelごとに左上から右下に格納.
		TIFFSetField(tiffImage, TIFFTAG_TILEWIDTH, tileWidth);
		TIFFSetField(tiffImage, TIFFTAG_TILELENGTH, tileHeight);

		TIFFSetField(tiffImage, TIFFTAG_PIXAR_TEXTUREFORMAT, "Plain Texture");		// RenderMan向けのテクスチャとして出力.
		TIFFSetField(tiffImage, TIFFTAG_PIXAR_WRAPMODES, "periodic,periodic");		// テクスチャのWrap情報.

		TIFFSetField(tiffImage, TIFFTAG_PIXAR_FOVCOT, 1.0);							// Plain Textureでも必須.

		// 画像は、tileWidth x tileHeightのブロックごとに格納していく.
		for (int iy = 0; iy < height; iy += tileHeight) {
			for (int ix = 0; ix < width; ix += tileWidth) {

				// 画像からピクセル情報を読み込み.
				image2->get_pixels_rgba(ix, iy, tileWidth, tileHeight, &srcBuffer[0]);

				// (ix, iy)の位置から、wid x heiの画像をコピー.
				int iPos = 0;
				for (int y = 0; y < tileHeight; y++) {
					if (y + iy >= height) break;
					for (int x = 0; x < tileWidth; x++) {
						if (x + ix < width) {
							tileBuffer[iPos] = (unsigned char)srcBuffer[iPos].red;
						}
						iPos++;
					}
				}
				TIFFWriteTile(tiffImage, &tileBuffer[0], ix, iy, 0, 0);
			}
		}
		TIFFWriteDirectory(tiffImage);

		width  >>= 1;
		height >>= 1;

		tempImage = image2;
	}

	TIFFClose(tiffImage);

	return true;
}
