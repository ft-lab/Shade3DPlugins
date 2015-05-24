/**
 * RIB Exporter.
 */

#include "RIBExporterInterface.h"
#include "AttributeWindowInterface.h"
#include "AreaLightAttributeInterface.h"

//**************************************************//
//	グローバル関数									//
//**************************************************//
/**
 * プラグインインターフェースの生成.
 */
extern "C" void STDCALL create_interface (const IID &iid, int i, void **p, sxsdk::shade_interface *shade, void *) {
	unknown_interface *u = NULL;
	
	if (iid == exporter_iid) {
		if (i == 0) {
			u = new CRIBExporterInterface(*shade);
		}
	}
	if (iid == window_iid) {
		if (i == 0) {
			u = new CAttributeWindowInterface(*shade);
		}
	}
	if (iid == attribute_iid) {
		if (i == 0) {
			u = new CAreaLightAttributeInterface(*shade);
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
	if (iid == exporter_iid) return 1;
	if (iid == window_iid) return 1;
	if (iid == attribute_iid) return 1;

	return 0;
}

/**
 * インターフェース名を返す.
 */
extern "C" const char * STDCALL get_name (const IID &iid, int i, sxsdk::shade_interface *shade, void *) {
	// SXULより、プラグイン名を取得して渡す.
	if (iid == exporter_iid) {
		if (i == 0) {
			return CRIBExporterInterface::name(shade);
		}
	}
	if (iid == window_iid) {
		if (i == 0) {
			return CAttributeWindowInterface::name(shade);
		}
	}
	if (iid == attribute_iid) {
		if (i == 0) {
			return CAreaLightAttributeInterface::name(shade);
		}
	}

	return 0;
}

/**
 * プラグインのUUIDを返す.
 */
extern "C" sx::uuid_class STDCALL get_uuid (const IID &iid, int i, void *) {
	if (iid == exporter_iid) {
		if (i == 0) {
			return RIB_EXPORTER_ID;
		}
	}
	if (iid == window_iid) {
		if (i == 0) {
			return RIB_ATTRIBUTE_WINDOW_ID;
		}
	}
	if (iid == attribute_iid) {
		if (i == 0) {
			return RIB_AREA_LIGHT_ATTRIBUTE_ID;
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
	info.major_version = 1;
	info.minor_version = 0;
	info.micro_version = 0;
	info.build_number =  2;
}

/**
 * 常駐プラグイン.
 */
extern "C" bool STDCALL is_resident (const IID &iid, int i, void *) {
	return true;
}
