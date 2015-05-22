/**
 *  @file   MyPluginInterface.h
 *  @brief  Shade13プラグインSDK、plugin_interface派生クラス.
 */

#ifndef _MYPLUGININTERFACE_H
#define _MYPLUGININTERFACE_H

#include "GlobalHeader.h"

struct CMyPluginInterface : public sxsdk::plugin_interface
{
private:
	sxsdk::shade_interface& shade;

	/**
	 * SDKのビルド番号を指定（これは固定で変更ナシ）。.
	 * ※ これはプラグインインターフェースごとに必ず必要。.
	 */
	virtual int get_shade_version () const { return SHADE_BUILD_NUMBER; }

	/**
	 * UUIDの指定（独自に定義したGUIDを指定）.
	 * ※ これはプラグインインターフェースごとに必ず必要。.
	 */
	virtual sx::uuid_class get_uuid (void * = 0) { return MY_PLUGIN_ID; }

	/**
	 * プラグインメニューより選択された場合に呼ばれる.
	 */
	virtual void do_it (sxsdk::shade_interface *shade, sxsdk::scene_interface *scene, void *aux = 0);

public:
	CMyPluginInterface (sxsdk::shade_interface& shade);
	virtual ~CMyPluginInterface ();

	/**
	 * プラグイン名をSXUL(text.sxul)より取得.
	 */
	static const char *name (sxsdk::shade_interface *shade) { return shade->gettext("title"); }

};

#endif
