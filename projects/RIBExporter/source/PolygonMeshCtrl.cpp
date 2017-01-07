/**
 * ポリゴンメッシュ情報を一時的に保持するクラス.
 * 法線やUVを頂点ごとに分離して管理.
 */

#include "PolygonMeshCtrl.h"
#include <cmath>

CPolygonMeshCtrl::CPolygonMeshCtrl (sxsdk::shade_interface& shade): shade(shade)
{
	Clear();
}

/**
 * 情報クリア.
 */
void CPolygonMeshCtrl::Clear ()
{
	m_name = "";
	m_vertices.clear();
	m_polygons.clear();

	m_currentFaceGroupIndex = -1;
	m_faceGroupCount = 0;
}

/**
 * 格納開始.
 */
void CPolygonMeshCtrl::BeginStore (const std::string name, const bool separateUV, const bool separateNormal)
{
	Clear();
	m_separateUV     = separateUV;
	m_separateNormal = separateNormal;

	m_name = name;
}

/**
 * 格納終了.
 */
void CPolygonMeshCtrl::EndStore ()
{
	// faceGroupごとに分解。頂点ごとの法線、UV参照になるようにする.
	m_Classification();
}

/**
 * 頂点座標追加.
 */
void CPolygonMeshCtrl::AppendVertex (const sxsdk::vec3& v)
{
	m_vertices.push_back(v);
}

/**
 * 面情報追加.
 */
void CPolygonMeshCtrl::AppendPolygon (const std::vector<int>& indices, const std::vector<sxsdk::vec3>& normals, const std::vector<sxsdk::vec2>& uvs, const int faceGroupIndex)
{
	// 面積のない面の場合/NANの頂点を持つ面の場合はスキップ.
	{
		const int vCou = (int)indices.size();
		if (vCou <= 2) return;
		if (vCou == 3 || vCou == 4) {
			sxsdk::vec3 versA[4];
			bool nanF = false;
			for (int i = 0; i < vCou; i++) {
				versA[i] = m_vertices[indices[i]];
				if (std::isnan(versA[i].x) || std::isnan(versA[i].y) || std::isnan(versA[i].z)) {
					nanF = true;
					break;
				}
			}
			if (nanF) return;

			{
				int cou = vCou;
				for (int i = 0; i < vCou; i++) {
					const sxsdk::vec3 v = versA[i] - versA[(i + 1) % vCou];
					if (sx::zero(v)) cou--;
				}
				if (cou <= 2) return;
			}
		}
	}

	m_polygons.push_back(CMeshPolygon());
	CMeshPolygon& polygon = m_polygons.back();

	polygon.indices  = indices;
	polygon.normals  = normals;
	polygon.uvs      = uvs;
	polygon.faceGroupIndex = faceGroupIndex;

	if (faceGroupIndex >= 0) {
		if (m_currentFaceGroupIndex != faceGroupIndex) {
			m_currentFaceGroupIndex = faceGroupIndex;
			m_faceGroupCount++;
		}
	}
}

/**
 * faceGroupごとに分離.
 */
void CPolygonMeshCtrl::m_Classification()
{
	m_outputMeshList.clear();

	const int faceCou = m_polygons.size();
	if (faceCou == 0) return;

	// faceGroupを参照していない面を格納.
	m_AdjustmentPoints(-1);
	if (m_faceGroupCount == 0) return;

	// faceGroupを参照する面を格納.
	for (int i = 0; i < m_faceGroupCount; i++) m_AdjustmentPoints(i);
}

/**
 * 法線とUVを頂点ごとに一意になるように分離(頂点を増やす).
 */
void CPolygonMeshCtrl::m_AdjustmentPoints (const int faceGroupIndex)
{
	const int orgVCou = m_vertices.size();
	const int faceCou = m_polygons.size();

	// faceGroupごとの使用している面番号を取得.
	std::vector<int> faceIndexList;
	for (int loop = 0; loop < faceCou; loop++) {
		CMeshPolygon& poly = m_polygons[loop];
		if (poly.faceGroupIndex != faceGroupIndex) {
			if (faceIndexList.size() > 0) break;
			continue;
		}
		faceIndexList.push_back(loop);
	}
	const int newFaceCou = faceIndexList.size();
	if (newFaceCou == 0) return;

	m_outputMeshList.push_back(COutputMeshInfo());
	COutputMeshInfo& outputMeshInfo = m_outputMeshList.back();
	outputMeshInfo.faceGroupIndex = faceGroupIndex;

	// 使用頂点をチェック.
	std::vector<int> chkVertices;
	int preVerCou = 0;
	{
		chkVertices.resize(orgVCou, -1);
		for (int loop = 0; loop < newFaceCou; loop++) {
			CMeshPolygon& poly = m_polygons[ faceIndexList[loop] ];
			const int vCou = poly.indices.size();
			for (int i = 0; i < vCou; i++) chkVertices[ poly.indices[i] ] = 0;
		}
		for (int loop = 0; loop < orgVCou; loop++) {
			if (chkVertices[loop] < 0) continue;
			chkVertices[loop] = preVerCou;
			preVerCou++;
		}
	}

	std::vector< std::vector<int> > newVerticesRef;
	newVerticesRef.resize(preVerCou);

	outputMeshInfo.vertices.resize(preVerCou);
	outputMeshInfo.normals.resize(preVerCou);
	outputMeshInfo.uvs.resize(preVerCou);
	outputMeshInfo.faceIndices.resize(newFaceCou);

	for (int loop = 0; loop < newFaceCou; loop++) {
		CMeshPolygon& poly = m_polygons[ faceIndexList[loop] ];
		const int vCou = poly.indices.size();
		outputMeshInfo.faceIndices[loop].resize(vCou);

		for (int i = 0; i < vCou; i++) {
			const int orgIndex = poly.indices[i];
			const int index    = chkVertices[orgIndex];
			const int cou      = newVerticesRef[index].size();

			if (cou == 0) {
				outputMeshInfo.vertices[index] = m_vertices[orgIndex];
				outputMeshInfo.normals[index]  = poly.normals[i];
				outputMeshInfo.uvs[index]      = poly.uvs[i];
				outputMeshInfo.faceIndices[loop][i] = index;
				newVerticesRef[index].push_back(index);

			} else {
				int searchIndex = -1;
				const sxsdk::vec3& v  = m_vertices[orgIndex];
				const sxsdk::vec3& n  = poly.normals[i];
				const sxsdk::vec2& uv = poly.uvs[i];

				for (int j = 0; j < cou; j++) {
					const int index2 = newVerticesRef[index][j];
					if (sx::zero(outputMeshInfo.vertices[index2] - v) && (!m_separateNormal || sx::zero(outputMeshInfo.normals[index2] - n)) && (!m_separateUV || sx::zero(outputMeshInfo.uvs[index2] - uv))) {
						searchIndex = index2;
						break;
					}
				}
				if (searchIndex >= 0) {
					outputMeshInfo.faceIndices[loop][i] = searchIndex;
				} else {
					searchIndex = outputMeshInfo.vertices.size();
					outputMeshInfo.vertices.push_back(v);
					outputMeshInfo.normals.push_back(n);
					outputMeshInfo.uvs.push_back(uv);
					outputMeshInfo.faceIndices[loop][i] = searchIndex;
					newVerticesRef[index].push_back(searchIndex);
				}
			}
		}
	}
}

/**
 * faceGroup情報を取得.
 * -1の場合はfaceGroupのない箇所(元の割り当て参照).
 */
void CPolygonMeshCtrl::GetFaceGroupIndexList (std::vector<int>& faceGroupIndexList)
{
	const int cou = m_outputMeshList.size();
	faceGroupIndexList.resize(cou);
	for (int i = 0; i < cou; i++) {
		faceGroupIndexList[i] = m_outputMeshList[i].faceGroupIndex;
	}
}

/**
 * 処理済の面数を返す.
 */
int CPolygonMeshCtrl::GetPolygonsCount (const int meshIndex)
{
	return m_outputMeshList[meshIndex].faceIndices.size();
}

/**
 * 指定の面での頂点インデックスを返す.
 */
bool CPolygonMeshCtrl::GetPolygonIndices (const int meshIndex, const int polyIndex, std::vector<int>& retIndices)
{
	COutputMeshInfo& meshInfo = m_outputMeshList[meshIndex];

	if (polyIndex < 0 || polyIndex >= meshInfo.faceIndices.size()) return false;

	const int vCou = meshInfo.faceIndices[polyIndex].size();
	retIndices.resize(vCou);
	for (int i = 0; i < vCou; i++) retIndices[i] = meshInfo.faceIndices[polyIndex][i];

	return true;
}

/**
 * 処理済の面ごとの頂点数リストを返す.
 */
bool CPolygonMeshCtrl::GetPolygonsVCount (const int meshIndex, std::vector<int>& retPolygonIndices)
{
	COutputMeshInfo& meshInfo = m_outputMeshList[meshIndex];

	const int polyCou = meshInfo.faceIndices.size();
	retPolygonIndices.resize(polyCou);

	for (int i = 0; i < polyCou; i++) retPolygonIndices[i] = meshInfo.faceIndices[i].size();
	return true;
}

/**
 * 処理済の頂点を返す.
 */
bool CPolygonMeshCtrl::GetOutputVertices (const int meshIndex, std::vector<sxsdk::vec3>& retV)
{
	COutputMeshInfo& meshInfo = m_outputMeshList[meshIndex];

	retV.resize(meshInfo.vertices.size());
	for (int i = 0; i < meshInfo.vertices.size(); i++)  retV[i] = meshInfo.vertices[i];
	return true;
}

/**
 * 処理済の法線を返す.
 */
bool CPolygonMeshCtrl::GetOutputNormals (const int meshIndex, std::vector<sxsdk::vec3>& retN)
{
	COutputMeshInfo& meshInfo = m_outputMeshList[meshIndex];

	retN.resize(meshInfo.normals.size());
	for (int i = 0; i < meshInfo.normals.size(); i++)  retN[i] = meshInfo.normals[i];
	return true;
}

/**
 * 処理済のUVを返す.
 */
bool CPolygonMeshCtrl::GetOutputUVs (const int meshIndex, std::vector<sxsdk::vec2>& retUVs)
{
	COutputMeshInfo& meshInfo = m_outputMeshList[meshIndex];

	retUVs.resize(meshInfo.uvs.size());
	for (int i = 0; i < meshInfo.uvs.size(); i++)  retUVs[i] = meshInfo.uvs[i];
	return true;
}


