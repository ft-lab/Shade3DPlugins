/**
 *  @file   MyPluginInterface.cpp
 *  @brief  Shade13プラグインSDK、plugin_interface派生クラス.
 */

#include "MyPluginInterface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CMyPluginInterface::CMyPluginInterface (sxsdk::shade_interface& shade) : shade(shade)
{
}

CMyPluginInterface::~CMyPluginInterface ()
{
}

/**
 * プラグインメニューより選択された場合に呼ばれる.
 */
void CMyPluginInterface::do_it (sxsdk::shade_interface *shade, sxsdk::scene_interface *scene, void *aux)
{
	shade->message("Hello shade.");
}

