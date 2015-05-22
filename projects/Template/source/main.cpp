/**
 *  @file   main.cpp
 *  @brief  Shade13プラグインSDK雛形.
 */

/*
	[mac]	OSX用のプロジェクト一式.
    [win]	Windows用のプロジェクト一式.

	[source]  ソースファイル一式.
        main.cpp           "HelloShade!"と表示するだけのサンプル.
		[resources]        リソースファイルフォルダ.
		     [en.lproj]       英語リソースフォルダ.
		     [ja.lproj]       日本語リソースフォルダ.
		     [zh_CN.lproj]    中国語（簡体字）リソースフォルダ.
*/

#include "GlobalHeader.h"
#include "MyPluginInterface.h"


//**************************************************//
//	グローバル関数									//
//**************************************************//
/**
 * プラグインインターフェースの生成.
 */
extern "C" void STDCALL create_interface (const IID &iid, int i, void **p, sxsdk::shade_interface *shade, void *) {
	unknown_interface *u = NULL;
	
	if (iid == plugin_iid) {
		if (i == 0) {
			u = new CMyPluginInterface(*shade);
		}
	}

	if (u) {
		u->AddRef();
		*p = (void *)u;
	}
}

/**
 * インターフェースの数を返す.
 */
extern "C" int STDCALL has_interface (const IID &iid, sxsdk::shade_interface *shade) {
	if (iid == plugin_iid) return 1;
	return 0;
}

/**
 * インターフェース名を返す.
 */
extern "C" const char * STDCALL get_name (const IID &iid, int i, sxsdk::shade_interface *shade, void *) {
	// SXULより、プラグイン名を取得して渡す.
	if (iid == plugin_iid) {
		if (i == 0) {
			return CMyPluginInterface::name(shade);
		}
	}
	return 0;
}

/**
 * プラグインのUUIDを返す.
 */
extern "C" sx::uuid_class STDCALL get_uuid (const IID &iid, int i, void *) {
	if (iid == plugin_iid) {
		if (i == 0) {
			return MY_PLUGIN_ID;
		}
	}

	return sx::uuid_class(0, 0, 0, 0);
}


/**
 * バージョン情報.
 */
extern "C" void STDCALL get_info (sxsdk::shade_plugin_info &info, sxsdk::shade_interface *shade, void *) {
	info.sdk_version = SHADE_BUILD_NUMBER;
	info.recommended_shade_version = 410000;
	info.major_version = 0;
	info.minor_version = 0;
	info.micro_version = 0;
	info.build_number =  1;
}

/**
 * 常駐プラグイン.
 */
extern "C" bool STDCALL is_resident (const IID &iid, int i, void *) {
	return false;
}


