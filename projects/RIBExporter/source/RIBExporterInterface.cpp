/**
 * RIBExporterInterface.cpp
 * RenderManのRIB形式出力.
 */

#include "RIBExporterInterface.h"
#include "SaveRIB.h"
#include "StreamCtrl.h"

enum
{
	dlg_integrators_id = 201,					// レンダリングの種類.
	dlg_pixel_variance_id = 202,
	dlg_min_samples_id = 203,					// ピクセルのサンプリング数最小.
	dlg_max_samples_id = 204,					// ピクセルのサンプリング数最大.
	dlg_incremental_id = 205,
	dlg_bias_id = 206,
	dlg_max_diffuse_depth_id = 207,
	dlg_max_specular_depth_id = 208,
	dlg_version_id = 209,						// Version : 0(ver.20.x)、1(ver.21.x).
	dlg_rendering_format_type_id = 221,			// レンダリング画像のフォーマットの種類.
	dlg_rendering_image_type_id = 222,			// レンダリング画像のピクセルの種類.

	dlg_output_background_image_id = 301,		// 背景画像を出力.
	dlg_background_image_size_id = 302,			// 背景画像サイズ.
	dlg_background_image_intensity_id = 303,	// 背景画像の明るさ.
	dlg_background_draw_id = 304,				// 背景の描画.

	dlg_color_color_to_linear_id = 401,			// 色をリニアにする.
	dlg_color_texture_to_linear_id = 402,		// テクスチャをリニアにする.

	dlg_light_day_light_id = 501,				// 太陽光.

	dlg_statistics_end_of_frame_id = 231,			// 経過時間を表示.
	dlg_statistics_xml_file_id = 232,				// XMLファイル出力.

	dlg_subdivision_id = 240,						// Subdivision.
};

enum {
	binary_text_id = 10002,
	eol_id = 10003,
	all_objects_id = 10004,
	subdivision_level_id = 10005,
	surface_attributes_id = 10006,
};


CRIBExporterInterface::CRIBExporterInterface (sxsdk::shade_interface& shade) : shade(shade)
{
	m_pCurrentShape = NULL;
	m_pSaveRIB = NULL;
	m_dlgOK = false;
}

CRIBExporterInterface::~CRIBExporterInterface ()
{
	if (m_pSaveRIB) delete m_pSaveRIB;
	m_pSaveRIB = NULL;
}

/**
 * ファイル拡張子.
 */
const char *CRIBExporterInterface::get_file_extension (void *aux)
{
	return "rib";
}

/**
 * ファイルの説明文.
 */
const char *CRIBExporterInterface::get_file_description (void *aux)
{
	return "RIB(RenderMan)";
}

/**
 * エクスポート処理を行う.
 */
void CRIBExporterInterface::do_export (sxsdk::plugin_exporter_interface *plugin_exporter, void *)
{
	m_shapeStack.Clear();
	m_currentDepth = 0;

	shade.message("RIB Exporter ------");
	compointer<sxsdk::scene_interface> scene(shade.get_scene_interface());

	try {
		m_pluginExporter = plugin_exporter;
		m_pluginExporter->AddRef();

		m_stream      = m_pluginExporter->get_stream_interface();
		m_text_stream = m_pluginExporter->get_text_stream_interface();
	} catch(...) { }

	m_pScene = scene;

	m_pSaveRIB = new CSaveRIB(shade, m_stream, m_text_stream, m_data);

	m_pSaveRIB->SetSceneInfo(scene);

	// エクスポート開始.
	m_pSaveRIB->BeginExport(scene);

	// エクスポートの開始.
	m_pluginExporter->do_export();

	// エクスポート終了.
	m_pSaveRIB->EndExport();

	{
		std::stringstream s;
		s << "Export " << m_pSaveRIB->GetRIBFileName() << ".";
		shade.message(s.str().c_str());
	}

	// 破棄処理.
	if (m_pSaveRIB) delete m_pSaveRIB;
	m_pSaveRIB = NULL;
}

/********************************************************************/
/* エクスポートのコールバックとして呼ばれる							*/
/********************************************************************/

/**
 * エクスポートの開始.
 */
void CRIBExporterInterface::start (void *)
{
}

/**
 * エクスポートの終了.
 */
void CRIBExporterInterface::finish (void *)
{
}

/**
 * カレント形状の処理の開始.
 */
void CRIBExporterInterface::begin (void *)
{
	m_pCurrentShape = NULL;

	m_skip = false;

	// カレントの形状管理クラスのポインタを取得.
	m_pCurrentShape        = m_pluginExporter->get_current_shape();
	const sxsdk::mat4 gMat = m_pluginExporter->get_transformation();

	m_shapeStack.Push(m_currentDepth, m_pCurrentShape, gMat);

	m_spMat = sxsdk::mat4::identity;
	m_currentLWMatrix = gMat;

	m_currentDepth++;

	// 面光源/線光源の場合はスキップ.
	if (m_pCurrentShape->get_type() == sxsdk::enums::line) {
		sxsdk::line_class& lineC = m_pCurrentShape->get_line();
		if (lineC.get_light_intensity() > 0.0f) {
			m_skip = true;
		}
	}
}

/**
 * カレント形状の処理の終了.
 */
void CRIBExporterInterface::end (void *)
{
	m_skip = false;
	m_pCurrentShape = NULL;

	m_shapeStack.Pop();

	sxsdk::shape_class *pShape;
	sxsdk::mat4 lwMat;
	int depth = 0;

	m_shapeStack.GetShape(&pShape, &lwMat, &depth);
	if (pShape && depth > 0) {
		m_pCurrentShape  = pShape;
		m_currentDepth   = depth;
		m_currentLWMatrix = inv(pShape->get_transformation()) * m_shapeStack.GetLocalToWorldMatrix();
	}
}

/**
 * カレント形状が掃引体の上面部分の場合、掃引に相当する変換マトリクスが渡される.
 */
void CRIBExporterInterface::set_transformation (const sxsdk::mat4 &t, void *)
{
	m_spMat = t;
}

/**
 * カレント形状が掃引体の上面部分の場合の行列クリア.
 */
void CRIBExporterInterface::clear_transformation (void *)
{
	m_spMat = sxsdk::mat4::identity;
}

/**
 * ポリゴンメッシュの開始時に呼ばれる.
 */
void CRIBExporterInterface::begin_polymesh (void *)
{
	if (m_skip) return;

	m_LWMat = m_spMat * m_currentLWMatrix;
	m_currentFaceGroupIndex = -1;
	m_pSaveRIB->BeginPolygonMesh(m_pCurrentShape);
}

/**
 * ポリゴンメッシュの頂点情報格納時に呼ばれる.
 */
void CRIBExporterInterface::begin_polymesh_vertex (int n, void *)
{
	if (m_skip) return;
}

/**
 * 頂点が格納されるときに呼ばれる.
 */
void CRIBExporterInterface::polymesh_vertex (int i, const sxsdk::vec3 &v, const sxsdk::skin_class *skin)
{
	if (m_skip) return;

	// 座標値をワールド座標変換する.
	sxsdk::vec3 pos = v;
	//if (skin) pos = pos * (skin->get_skin_world_matrix());
	pos = pos * m_LWMat;

	m_pSaveRIB->AppendPolygonMeshVertex(pos);
}

/**
 * ポリゴンメッシュの面情報が格納されるときに呼ばれる.
 */
void CRIBExporterInterface::polymesh_face_uvs (int n_list, const int list[], const sxsdk::vec3 *normals, const sxsdk::vec4 *plane_equation, const int n_uvs, const sxsdk::vec2 *uvs, void *)
{
	if (m_skip) return;

	std::vector<int> indicesList;
	std::vector<sxsdk::vec3> normalsList;
	std::vector<sxsdk::vec2> uvsList;

	indicesList.resize(n_list);
	normalsList.resize(n_list);
	uvsList.resize(n_list, sxsdk::vec2(0, 0));

	sxsdk::vec4 v4;
	for (int i = 0; i < n_list; i++) {
		indicesList[i] = list[i];
		v4 = sxsdk::vec4(normals[i], 0) * m_LWMat;
		normalsList[i] = normalize(sxsdk::vec3(v4.x, v4.y, v4.z));
	}

	if (n_uvs > 0 && uvs != NULL) {
		int iPos = 0;
		for (int i = 0; i < n_list; i++) {
			uvsList[i] = uvs[iPos];
			iPos += n_uvs;
		}
	}

	m_pSaveRIB->AppendPolygonMeshPolygon(indicesList, normalsList, uvsList, m_currentFaceGroupIndex);
}

/**
 * ポリゴンメッシュの終了時に呼ばれる.
 */
void CRIBExporterInterface::end_polymesh (void *)
{
	if (m_skip) return;

	m_pSaveRIB->EndPolygonMesh();
}

/**
 * 面情報格納前に呼ばれる.
 */
void CRIBExporterInterface::begin_polymesh_face2 (int n, int number_of_face_groups, void *)
{
	if (m_skip) return;
}

/**
 * フェイスグループごとの面列挙前に呼ばれる.
 */
void CRIBExporterInterface::begin_polymesh_face_group (int face_group_index, void *)
{
	if (m_skip) return;
	m_currentFaceGroupIndex = face_group_index;
}

/**
 * フェイスグループごとの面列挙後に呼ばれる.
 */
void CRIBExporterInterface::end_polymesh_face_group (void *)
{
	if (m_skip) return;
	m_currentFaceGroupIndex = -1;
}

/****************************************************************/
/* ダイアログイベント											*/
/****************************************************************/
void CRIBExporterInterface::initialize_dialog (sxsdk::dialog_interface& dialog, void*)
{
	// streamからデータを取得.
	try {
		compointer<sxsdk::scene_interface> scene(shade.get_scene_interface());
		m_data = StreamCtrl::LoadRIBExportDlg(scene);
	} catch (...) { }
}

void CRIBExporterInterface::load_dialog_data (sxsdk::dialog_interface &d, void *)
{
	// 「バイナリ/テキスト」を無効化.
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(binary_text_id));
		item->set_enabled(false);
	}

	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_integrators_id));
		item->set_selection((int)m_data.type);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_pixel_variance_id));
		item->set_float(m_data.pixelVariance);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_min_samples_id));
		item->set_int(m_data.minSamples);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_max_samples_id));
		item->set_int(m_data.maxSamples);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_incremental_id));
		item->set_bool(m_data.incremental);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_bias_id));
		item->set_float(m_data.bias);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_max_diffuse_depth_id));
		item->set_int(m_data.maxDiffuseDepth);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_max_specular_depth_id));
		item->set_int(m_data.maxSpecularDepth);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_version_id));
		item->set_selection(m_data.prmanVersion);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_rendering_format_type_id));
		item->set_selection((int)m_data.renderingFormatType);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_rendering_image_type_id));
		item->set_selection((int)m_data.renderingImageType);
	}

	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_output_background_image_id));
		item->set_bool(m_data.outputBackgroundImage);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_background_image_size_id));
		item->set_selection((int)m_data.backgroundImageSize);
		item->set_enabled(m_data.outputBackgroundImage);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_background_image_intensity_id));
		item->set_float(m_data.backgroundImageIntensity);
		item->set_enabled(m_data.outputBackgroundImage);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_background_draw_id));
		item->set_bool(m_data.backgroundDraw);
		item->set_enabled(m_data.outputBackgroundImage);
	}

	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_color_color_to_linear_id));
		item->set_bool(m_data.colorColorToLinear);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_color_texture_to_linear_id));
		item->set_bool(m_data.colorTextureToLinear);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_light_day_light_id));
		item->set_bool(m_data.lightDayLight);
	}

	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_statistics_end_of_frame_id));
		item->set_bool(m_data.statisticsEndOfFrame);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_statistics_xml_file_id));
		item->set_bool(m_data.statisticsXMLFile);
	}
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_subdivision_id));
		item->set_bool(m_data.doSubdivision);
	}
}

void CRIBExporterInterface::save_dialog_data (sxsdk::dialog_interface &dialog,void *)
{
	// streamにデータを保存.
	if (m_dlgOK) {
		try {
			compointer<sxsdk::scene_interface> scene(shade.get_scene_interface());
			StreamCtrl::SaveRIBExportDlg(scene, m_data);
		} catch (...) { }
	}
}

bool CRIBExporterInterface::respond (sxsdk::dialog_interface &dialog, sxsdk::dialog_item_class &item, int action, void *)
{
	const int id = item.get_id();		// アクションがあったダイアログアイテムのID.

	if (id == sx::iddefault) {
		m_data.Clear();
		load_dialog_data(dialog);
		return true;
	}

	if (id == sx::idok) {
		m_dlgOK = true;
	}

	if (id == dlg_integrators_id) {
		m_data.type = (RIBParam::INTEGRATORS_TYPE)item.get_selection();
		return true;
	}
	if (id == dlg_pixel_variance_id) {
		m_data.pixelVariance = item.get_float();
		return true;
	}
	if (id == dlg_min_samples_id) {
		m_data.minSamples = item.get_int();
		return true;
	}
	if (id == dlg_max_samples_id) {
		m_data.maxSamples = item.get_int();
		return true;
	}
	if (id == dlg_incremental_id) {
		m_data.incremental = item.get_bool();
		return true;
	}
	if (id == dlg_bias_id) {
		m_data.bias = item.get_float();
		return true;
	}
	if (id == dlg_max_diffuse_depth_id) {
		m_data.maxDiffuseDepth = item.get_int();
		return true;
	}
	if (id == dlg_max_specular_depth_id) {
		m_data.maxSpecularDepth = item.get_int();
		return true;
	}
	if (id == dlg_version_id) {
		m_data.prmanVersion = item.get_selection();
		return true;
	}
	if (id == dlg_rendering_format_type_id) {
		m_data.renderingFormatType = (RIBParam::RENDERING_FORMAT_TYPE)item.get_selection();
		return true;
	}
	if (id == dlg_rendering_image_type_id) {
		m_data.renderingImageType = (RIBParam::RENDERING_IMAGE_TYPE)item.get_selection();
		return true;
	}

	if (id == dlg_output_background_image_id) {
		m_data.outputBackgroundImage = item.get_bool();
		{
			sxsdk::dialog_item_class &item2 = dialog.get_dialog_item(dlg_background_image_size_id);
			item2.set_enabled(m_data.outputBackgroundImage);
		}
		{
			sxsdk::dialog_item_class &item2 = dialog.get_dialog_item(dlg_background_image_intensity_id);
			item2.set_enabled(m_data.outputBackgroundImage);
		}
		{
			sxsdk::dialog_item_class &item2 = dialog.get_dialog_item(dlg_background_draw_id);
			item2.set_enabled(m_data.outputBackgroundImage);
		}
		return true;
	}
	if (id == dlg_background_image_size_id) {
		m_data.backgroundImageSize = (RIBParam::BACKGROUND_IMAGE_SIZE)item.get_selection();
		return true;
	}
	if (id == dlg_background_image_intensity_id) {
		m_data.backgroundImageIntensity = item.get_float();
		return true;
	}
	if (id == dlg_background_draw_id) {
		m_data.backgroundDraw = item.get_bool();
		return true;
	}
	if (id == dlg_color_color_to_linear_id) {
		m_data.colorColorToLinear = item.get_bool();
		return true;
	}
	if (id == dlg_color_texture_to_linear_id) {
		m_data.colorTextureToLinear = item.get_bool();
		return true;
	}
	if (id == dlg_light_day_light_id) {
		m_data.lightDayLight = item.get_bool();
		return true;
	}
	if (id == dlg_statistics_end_of_frame_id) {
		m_data.statisticsEndOfFrame = item.get_bool();
		return true;
	}
	if (id == dlg_statistics_xml_file_id) {
		m_data.statisticsXMLFile = item.get_bool();
		return true;
	}
	if (id == dlg_subdivision_id) {
		m_data.doSubdivision = item.get_bool();
		return true;
	}

	return false;
}

