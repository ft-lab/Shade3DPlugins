/**
 * ポリゴンメッシュ情報を一時的に保持するクラス.
 * 法線やUVを頂点ごとに分離して管理.
 */

#ifndef _POLYGONMESHCTRL_H
#define _POLYGONMESHCTRL_H

#include "GlobalHeader.h"

/**
 * ポリゴンメッシュの面情報.
 */
class CMeshPolygon
{
public:
	std::vector<int> indices;					// 面を構成する頂点インデックス.
	int faceGroupIndex;							// faceGroup番号.

	std::vector<sxsdk::rgba_class> colors;		// 頂点カラー.
	std::vector<sxsdk::vec3> normals;			// 法線.
	std::vector<sxsdk::vec2> uvs;				// UV.

public:
	CMeshPolygon () {
		faceGroupIndex = -1;
	}
};

/**
 * faceGroupごとに分解した出力用ポリゴンメッシュ情報.
 */
class COutputMeshInfo
{
public:
	int faceGroupIndex;								// faceGroup番号.
	std::vector< std::vector<int> > faceIndices;	// 面ごとの頂点インデックス.

	std::vector<sxsdk::vec3> vertices;				// 頂点座標リスト.
	std::vector<sxsdk::vec3> normals;				// 法線リスト.
	std::vector<sxsdk::vec2> uvs;					// UVリスト.

public:
	COutputMeshInfo () {
		faceGroupIndex = -1;
	}
};

/**
 * ポリゴンメッシュ情報.
 */
class CPolygonMeshCtrl
{
private:
	sxsdk::shade_interface& shade;

	std::string m_name;									// 形状名.
	std::vector<sxsdk::vec3> m_vertices;				// 頂点座標の格納.
	std::vector<CMeshPolygon> m_polygons;				// 面情報.

	std::vector<COutputMeshInfo> m_outputMeshList;		// 出力用のポリゴンメッシュ情報.

	int m_currentFaceGroupIndex;						// 格納中のfaceGroup番号.
	int m_faceGroupCount;								// faceGroupの数.

	bool m_separateUV;									// UVが異なる場合に頂点を増やして面を分けるか.
	bool m_separateNormal;								// 法線が異なる場合に頂点を増やして面を分けるか.

	/**
	 * faceGroupごとに分離.
	 */
	void m_Classification();

	/**
	 * 法線とUVを頂点ごとに一意になるように分離(頂点を増やす).
	 */
	void m_AdjustmentPoints (const int faceGroupIndex);

public:
	CPolygonMeshCtrl (sxsdk::shade_interface& shade);

	/**
	 * 情報クリア.
	 */
	void Clear ();

	/**
	 * 格納開始.
	 */
	void BeginStore (const std::string name, const bool separateUV, const bool separateNormal);

	/**
	 * 格納終了.
	 */
	void EndStore ();

	/**
	 * 頂点座標追加.
	 */
	void AppendVertex (const sxsdk::vec3& v);

	/**
	 * 面情報追加.
	 */
	void AppendPolygon (const std::vector<int>& indices, const std::vector<sxsdk::vec3>& normals, const std::vector<sxsdk::vec2>& uvs, const int faceGroupIndex);

	/**
	 * 格納しているfaceGroupごとのMesh数を取得.
	 */
	int GetMeshsCount () { return m_outputMeshList.size(); }

	/**
	 * 処理済の面数を返す.
	 */
	int GetPolygonsCount (const int faceGroupIndex);

	/**
	 * faceGroup情報を取得.
	 * -1の場合はfaceGroupのない箇所(元の割り当て参照).
	 */
	void GetFaceGroupIndexList (std::vector<int>& faceGroupIndexList);

	/**
	 * 処理済の面ごとの頂点数リストを返す.
	 */
	bool GetPolygonsVCount (const int meshIndex, std::vector<int>& retPolygonIndices);

	/**
	 * 指定の面での頂点インデックスを返す.
	 */
	bool GetPolygonIndices (const int meshIndex, const int polyIndex, std::vector<int>& retIndices);

	/**
	 * 処理済の頂点を返す.
	 */
	bool GetOutputVertices (const int meshIndex, std::vector<sxsdk::vec3>& retV);

	/**
	 * 処理済の法線を返す.
	 */
	bool GetOutputNormals (const int meshIndex, std::vector<sxsdk::vec3>& retN);

	/**
	 * 処理済のUVを返す.
	 */
	bool GetOutputUVs (const int meshIndex, std::vector<sxsdk::vec2>& retUVs);
};

#endif
