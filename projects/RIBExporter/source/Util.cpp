/**
 *  @brief  便利機能.
 */

#include "Util.h"

/**
 * テキストをSJISに変換.
 */
std::string Util::ConvUTF8ToSJIS (sxsdk::shade_interface& shade, const std::string str)
{
	std::string str2 = shade.encode(str.c_str(), sxsdk::enums::shift_jis_encoding);
	return str2;
}

/**
 * テキストをSJISからUTF-8に変換.
 */
std::string Util::ConvSJISToUTF8 (sxsdk::shade_interface& shade, const std::string str)
{
	std::string str2 = shade.decode(str.c_str(), sxsdk::enums::shift_jis_encoding);
	return str2;
}

/**
 * streamより、ファイル名を取得.
 */
std::string Util::GetFileNameToStream (sxsdk::stream_interface* stream)
{
	char *pPos;
	char szFilePath[512];

	strcpy(szFilePath, stream->get_file_path());

	// 作業用のファイル名の場合.
	pPos = strrchr(szFilePath, '.');
	if (pPos) {
		if (strcmp(pPos, ".sxfiletmp") == 0) {
			*pPos = '\0';
		}
	}

	std::string name = "";

	name = szFilePath;

#if SXWINDOWS
	pPos = strrchr(szFilePath, '\\');
	if(pPos) name = (pPos + 1);
#else
	pPos = strrchr(szFilePath, '/');
	if(pPos) name = (pPos + 1);
#endif

	return name;
}

/**
 * 保存パスを取得.
 */
std::string Util::GetFilePath (sxsdk::stream_interface* stream)
{
	std::string name(stream->get_file_path());

#if SXWINDOWS
	const int iPos = name.find_last_of("\\");
#else
	const int iPos = name.find_last_of("/");
#endif

	if (iPos == std::string::npos) return "";
	return name.substr(0, iPos);
}

/**
 * 指定の画像が、マスターイメージの何番目か.
 */
int Util::GetMasterImageIndex (sxsdk::scene_interface* scene, sxsdk::image_interface* image)
{
	sxsdk::shape_class& rootShape = scene->get_shape();
	if (!rootShape.has_son()) return -1;

	sxsdk::shape_class* pImagePart = NULL;
	sxsdk::shape_class* pS = rootShape.get_son();
	while (pS->has_bro()) {
		pS = pS->get_bro();
		if (!pS) break;
		if (pS->get_type() == sxsdk::enums::part) {
			if (pS->get_part().get_part_type() == sxsdk::enums::master_image_part) {
				pImagePart = pS;
				break;
			}
		}
	}
	if (!pImagePart) return -1;
	if (!pImagePart->has_son()) return -1;

	int retIndex = -1;
	int index = 0;
	pS = pImagePart->get_son();
	while (pS->has_bro()) {
		pS = pS->get_bro();
		if (!pS) break;
		if (pS->get_type() == sxsdk::enums::master_image) {
			sxsdk::master_image_class& masterImage = pS->get_master_image();
			compointer<sxsdk::image_interface> image2(masterImage.get_image());
			if (image2->has_image()) {
				if (image2->is_same_as(image)) {
					retIndex = index;
					break;
				}
			}
		}
		index++;
	}

	return retIndex;
}
