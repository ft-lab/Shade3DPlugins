﻿/**
 * RIBExporterInterface.h
 * RenderManのRIB形式出力.
 */

#ifndef _RIBEXPORTERINTERFACE_H
#define _RIBEXPORTERINTERFACE_H

#include "GlobalHeader.h"
#include "ShapeStack.h"

class CSaveRIB;

class CRIBExporterInterface : public sxsdk::exporter_interface
{
private:
	sxsdk::shade_interface& shade;
	compointer<sxsdk::plugin_exporter_interface> m_pluginExporter;
	compointer<sxsdk::stream_interface> m_stream;
	compointer<sxsdk::text_stream_interface> m_text_stream;

	RIBExportData m_data;						// ダイアログパラメータ.

	CShapeStack m_shapeStack;

	sxsdk::shape_class* m_pCurrentShape;		// カレントの形状クラスのポインタ.
	int m_currentDepth;							// エクスポート時の階層の深さ.

	sxsdk::scene_interface* m_pScene;
	sxsdk::mat4 m_currentLWMatrix;				// カレントのローカルワールド変換行列.
	sxsdk::mat4 m_LWMat;
	sxsdk::mat4 m_spMat;						// 掃引体時の変換行列.

	CSaveRIB* m_pSaveRIB;						// RIB出力クラス.

	bool m_skip;								// 処理を飛ばす場合.
	bool m_dlgOK;								// ダイアログでOKボタンを押して進めたか.

	int m_currentFaceGroupIndex;				// faceGroup番号.

	virtual sx::uuid_class get_uuid (void *) { return RIB_EXPORTER_ID; }
	virtual int get_shade_version () const { return SHADE_BUILD_NUMBER; }

	/**
	 * ファイル拡張子.
	 */
	virtual const char *get_file_extension (void *aux = 0);

	/**
	 * ファイルの説明文.
	 */
	virtual const char *get_file_description (void *aux = 0);

	/**
	 * エクスポート処理を行う.
	 */
	virtual void do_export (sxsdk::plugin_exporter_interface *plugin_exporter, void *);

	/**
	 * 開いた線形状を受け付けるかどうか.
	 */
	virtual bool can_accept_polyline (void *) { return false; }

	/**
	 * 閉じた線形状を受け付けるかどうか.
	 */
	virtual bool can_accept_polygon (void *) { return false; }

	/**
	 * 球を受け付けるかどうか.
	 */
	virtual bool can_accept_sphere (void *) { return false; }

	/**
	 * 自由曲面を受け付けるかどうか.
	 */
	virtual bool can_accept_bezier_surface (void *) { return false; }

	/**
	 * ポリゴンメッシュを受け付けるかどうか.
	 */
	virtual bool can_accept_polymesh (void *) { return true; }

	/**
	 * ダイアログ表示のスキップするかどうか.
	 */
	virtual bool skips_dialog (void *) { return false; }

	/**
	 * リソースに埋め込むSXULを指定.
	 */
	virtual const char *get_include_resource_name (const int index, void * aux = 0) {
		return "export_dlg";
	}

	/**
	 * 受け付けることのできるポリゴンメッシュ面の頂点の最大数.
	 */
	virtual int get_max_vertices_per_face (void *) { return 4; }

	/**
	 * ポリゴンメッシュの面は分割しない.
	 */
	virtual bool must_triangulate_polymesh (void *) { return false; }

	/**
	 * メッシュを受け付けるかどうか.
	 */
	virtual bool can_accept_meshes (void *aux=0) { return false; }

	/**
	 * ポリゴンメッシュの面を分割するか.
	 * ここをfalseにしget_max_vertices_per_faceが4の場合は、極力4角形は保つ。5角形以上は4角形と3角形に分割される.
	 */
	virtual bool must_divide_polymesh (void *aux=0) { return false; }

	/**
	 * バイナリで出力.
	 */
	virtual bool can_export_binary (void * = 0) { return false; }
	virtual bool can_export_text (void * = 0) { return true; }

	virtual bool cannot_select_eol (void *aux=0) { return true; }
	virtual bool can_select_filter_objects (void *aux=0) { return false; }

	/**
	 * 面をサブディビジョンするか.
	 */
	virtual bool must_round_polymesh (void *aux=0) { return m_data.doSubdivision; }

	/********************************************************************/
	/* エクスポートのコールバックとして呼ばれる							*/
	/********************************************************************/

	/**
	 * エクスポートの開始.
	 */
	virtual void start (void * = 0);

	/**
	 * エクスポートの終了
	 */
	virtual void finish (void * = 0);

	/**
	 * カレント形状の処理の開始.
	 */
	virtual void begin (void * = 0);

	/**
	 * カレント形状の処理の終了.
	 */
	virtual void end (void * = 0);

	/**
	 * カレント形状が掃引体の上面部分の場合、掃引に相当する変換マトリクスが渡される.
	 */
	virtual void set_transformation (const sxsdk::mat4 &t, void * = 0);

	/**
	 * カレント形状が掃引体の上面部分の場合の行列クリア.
	 */
	virtual void clear_transformation (void * = 0);

	/**
	 * ポリゴンメッシュの開始時に呼ばれる.
	 */
	virtual void begin_polymesh (void * = 0);

	/**
	 * ポリゴンメッシュの頂点情報格納時に呼ばれる.
	 */
	virtual void begin_polymesh_vertex (int n, void * = 0);

	/**
	 * 頂点が格納されるときに呼ばれる.
	 */
	virtual void polymesh_vertex (int i, const sxsdk::vec3 &v, const sxsdk::skin_class *skin);

	/**
	 * ポリゴンメッシュの面情報が格納されるときに呼ばれる（Shade12の追加機能）.
	 */
	virtual void polymesh_face_uvs (int n_list, const int list[], const sxsdk::vec3 *normals, const sxsdk::vec4 *plane_equation, const int n_uvs, const sxsdk::vec2 *uvs, void *aux=0);

	/**
	 * ポリゴンメッシュの終了時に呼ばれる.
	 */
	virtual void end_polymesh (void * = 0);

	/**
	 * 面情報格納前に呼ばれる.
	 */
	virtual void begin_polymesh_face2 (int n, int number_of_face_groups, void *aux=0);

	/**
	 * フェイスグループごとの面列挙前に呼ばれる.
	 */
	virtual void begin_polymesh_face_group (int face_group_index, void *aux=0);

	/**
	 * フェイスグループごとの面列挙後に呼ばれる.
	 */
	virtual void end_polymesh_face_group (void *aux=0);

	/****************************************************************/
	/* ダイアログイベント											*/
	/****************************************************************/
	virtual void initialize_dialog (sxsdk::dialog_interface& dialog, void* aux = 0);
	virtual void load_dialog_data (sxsdk::dialog_interface &d,void * = 0);
	virtual void save_dialog_data (sxsdk::dialog_interface &dialog,void * = 0);
	virtual bool respond (sxsdk::dialog_interface &dialog, sxsdk::dialog_item_class &item, int action, void *);

public:
	CRIBExporterInterface (sxsdk::shade_interface& shade);
	~CRIBExporterInterface ();

	/**
	 * プラグイン名
	 */
	static const char *name (sxsdk::shade_interface *shade) { return shade->gettext("rib_exporter_title"); }
};

#endif
