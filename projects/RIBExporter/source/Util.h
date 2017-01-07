/**
 *  @brief  便利機能.
 */

#ifndef _UTIL_H
#define _UTIL_H

#include "GlobalHeader.h"

namespace Util {
	/**
	 * テキストをSJISに変換.
	 */
	std::string ConvUTF8ToSJIS (sxsdk::shade_interface& shade, const std::string str);

	/**
	 * テキストをSJISからUTF-8に変換.
	 */
	std::string ConvSJISToUTF8 (sxsdk::shade_interface& shade, const std::string str);

	/**
	 * streamより、ファイル名を取得.
	 */
	std::string GetFileNameToStream (sxsdk::stream_interface* stream);

	/**
	 * 保存パスを取得.
	 */
	std::string GetFilePath (sxsdk::stream_interface* stream);

	/**
	 * 指定の画像が、マスターイメージの何番目か.
	 */
	int GetMasterImageIndex (sxsdk::scene_interface* scene, sxsdk::image_interface* image);

	/**
	 * 形状名に、「;:- " '」が含まれる場合は、「_」に置き換え.
	 */
	std::string ReplaceName (const std::string& str);
}

#endif
