/**
 * 数値演算系関数.
 */

#include "MathUtil.h"

namespace {
	/**
	 * 2点のベジェでの制御点が与えられたときの対応点を計算.
	 */
	sxsdk::vec3 GetBezierPos (const CPoint3D& v1, const CPoint3D& v2, const double fPos)
	{
		double fPos2, fx, fy, fz;
		double t, t2, td, t2d;
		double b1, b2, b3, b4;
		sxsdk::vec3 rPos;

		typedef  struct {
			double x, y, z;
		} FPOINT3;
		FPOINT3 cPos[5];
		FPOINT3 *pC;
		const double fMin = 1e-6;

		cPos[0].x = (double)(v1.point.x);
		cPos[0].y = (double)(v1.point.y);
		cPos[0].z = (double)(v1.point.z);
		cPos[1].x = (double)(v1.outHandle.x);
		cPos[1].y = (double)(v1.outHandle.y);
		cPos[1].z = (double)(v1.outHandle.z);
		cPos[2].x = (double)(v2.inHandle.x);
		cPos[2].y = (double)(v2.inHandle.y);
		cPos[2].z = (double)(v2.inHandle.z);
		cPos[3].x = (double)(v2.point.x);
		cPos[3].y = (double)(v2.point.y);
		cPos[3].z = (double)(v2.point.z);

		fPos2 = fPos;
		if (fPos2 < 0.0) fPos2 = 0.0;
		if (fPos2 > 1.0) fPos2 = 1.0;

		if (std::abs(fPos2) < fMin) {
			rPos.x = (float)cPos[0].x;
			rPos.y = (float)cPos[0].y;
			rPos.z = (float)cPos[0].z;
			return rPos;
		}
		if (std::abs(fPos2 - 1.0) < fMin) {
			rPos.x = (float)cPos[3].x;
			rPos.y = (float)cPos[3].y;
			rPos.z = (float)cPos[3].z;
			return rPos;
		}

		pC = cPos;
		// ベジェ曲線の計算.
		// B1 = (1 - T) ^ 3
		// B2 = 3 * T * (1 - T) ^ 2
		// B3 = 3 * (T^2) * (1 - T)
		// B4 = T^3
		t   = fPos2;
		t2  = 1.0 - t;
		t2d = t2 * t2;
		td  = t * t;
		b1  = t2d * t2;
		b2  = 3.0 * t * t2d;
		b3  = 3.0 * td * t2;
		b4  = t * td;
		fx = b1 * (pC->x) + b2 * ((pC + 1)->x) + b3 * ((pC + 2)->x) + b4 * ((pC + 3)->x);
		fy = b1 * (pC->y) + b2 * ((pC + 1)->y) + b3 * ((pC + 2)->y) + b4 * ((pC + 3)->y);
		fz = b1 * (pC->z) + b2 * ((pC + 1)->z) + b3 * ((pC + 2)->z) + b4 * ((pC + 3)->z);

		rPos.x = (float)fx;
		rPos.y = (float)fy;
		rPos.z = (float)fz;
		return rPos;
	}

	/**
	 * 多角形の三角形分割クラス.
	 */
	std::vector<int> m_triangleIndex; 

	class CDivideTrianglesOutput : public sxsdk::shade_interface::output_function_class {
	private:
	public:
		virtual void output (int i0 , int i1 , int i2 , int i3) {
			const int offset = m_triangleIndex.size();
			m_triangleIndex.resize(offset + 3);
			m_triangleIndex[offset + 0] = i0;
			m_triangleIndex[offset + 1] = i1;
			m_triangleIndex[offset + 2] = i2;
		}
	};
}

/**
 * ガウスの消去法で逆行列計算.
 */
bool MathUtil::InverseMatrix (const sxsdk::mat4& m, sxsdk::mat4& retM)
{
	sxsdk::mat4 mat;
	double fDat, fDat2;
	double mat_8x4[4][8];
	bool flag;

	// 8 x 4行列に値を入れる.
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) mat_8x4[i][j] = (double)(m[i][j]);
		for (int j = 0; j < 4; j++) {
			if (i == j)  mat_8x4[i][j + 4] = 1.0;
			else         mat_8x4[i][j + 4] = 0.0;
		}
	}

	flag = true;
	for (int loop = 0; loop < 4; loop++) {
		fDat = mat_8x4[loop][loop];
		if (fDat != 1.0) {
			if (fDat == 0.0) {
				int i = loop + 1;
				for (; i < 4; i++) {
					fDat = mat_8x4[i][loop];
					if(fDat != 0.0) break;
				}
				if (i >= 4) {
					flag = false;
					break;
				}

				// 行を入れ替える.
				for (int j = 0; j < 8; j++) {
					fDat = mat_8x4[i][j];
					mat_8x4[i][j]    = mat_8x4[loop][j];
					mat_8x4[loop][j] = fDat;
				}
				fDat = mat_8x4[loop][loop];
			}
			for (int i = 0; i < 8; i++) mat_8x4[loop][i] /= fDat;
		}
		for (int i = 0; i < 4; i++) {
			if (i != loop) {
				fDat = mat_8x4[i][loop];
				if (fDat != 0.0f) {
					//mat[i][loop]をmat[loop]の行にかけて.
					//(mat[j] - mat[loop] * fDat)を計算.
					for (int j = 0; j < 8; j++) {
						fDat2 = mat_8x4[loop][j] * fDat;
						mat_8x4[i][j] -= fDat2;
					}
				}
			}
		}
	}

	if (flag) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				mat[i][j] = (float)mat_8x4[i][j + 4];
			}
		}
	} else {
		//単位行列を求める
		mat = sxsdk::mat4::identity;
	}

	retM = mat;

	return flag;
}

/**
 * 指定のベジェ曲線を直線の集まりにしたものを取得.
 */
void MathUtil::GetBezierToLines (const std::vector<CPoint3D>& bezierPosList, std::vector<sxsdk::vec3>& retPosList, const bool closed, const int divideLevel)
{
	// ベジェを直線に変換.
	const int bCou = bezierPosList.size();

	const int divCou = 1 << divideLevel;
	double dPos;
	const double dD = 1.0 / (double)divCou;

	const int bCou2 = closed ? bCou : (bCou - 1);

	retPosList.clear();
	for (int loop = 0; loop < bCou2; loop++) {
		const CPoint3D& p0 = bezierPosList[loop + 0];
		const CPoint3D& p1 = bezierPosList[(loop + 1) % bCou];

		float fPos = (float)loop;

		dPos = 0.0;
		for (int i = 0; i < divCou; i++) {
			const sxsdk::vec3 v = ::GetBezierPos(p0, p1, dPos);
			retPosList.push_back(v);

			dPos += dD;
			fPos += (float)dD;
		}
		if (loop + 1 >= bCou2) {
			retPosList.push_back(p1.point);
		}
	}

	// 不要な頂点を削除.
	LineOptimize3D(retPosList);
}

/**
 * 3Dのラインリストで、リダクションを行う.
 * 同一位置にある頂点のカット、直線的な位置にある中間の頂点をカット.
 */
void MathUtil::LineOptimize3D (std::vector<sxsdk::vec3>& lines)
{
	const float fMax = 1.0f - (float)(1e-4);
	const float fMin = (float)(1e-4);

	{
		for (int i = lines.size() - 2; i >= 0; i--) {
			const sxsdk::vec3& v0 = lines[i + 0];
			const sxsdk::vec3& v1 = lines[i + 1];

			const sxsdk::vec3 vv = v1 - v0;
			if (std::abs(vv.x) < fMin && std::abs(vv.y) < fMin && std::abs(vv.z) < fMin) {
				lines.erase(lines.begin() + (i + 1));
			}
		}
	}

	int pos = 0;
	while (lines.size() >= 3) {
		bool chkF = false;
		sxsdk::vec3 ev0, ev1;
		for (int i = pos; i < lines.size() - 2; i++) {
			const sxsdk::vec3& v0 = lines[i + 0];
			const sxsdk::vec3& v1 = lines[i + 1];
			const sxsdk::vec3& v2 = lines[i + 2];

			if (i == pos) {
				ev0 = v1 - v0;
				ev0 = normalize(ev0);
			}

			ev1 = v2 - v1;
			ev1 = normalize(ev1);

			if (sx::inner_product(ev0, ev1) >= fMax) {
				lines.erase(lines.begin() + (i + 1));		// 中間点を削除.
				chkF = true;
				pos = i - 2;
				if (pos < 0) pos = 0;
				break;
			}
			ev0 = ev1;
		}
		if (!chkF) break;
	}
}


/**
 * 三角形の面積を計算.
 */
double MathUtil::CalcTriangleArea (const sxsdk::vec3& v1, const sxsdk::vec3& v2, const sxsdk::vec3& v3)
{
	double ax, ay, az, bx, by, bz;
	double fDat1, fDat2, fDat3;
	double S;

	ax = (double)(v2.x - v1.x);
	ay = (double)(v2.y - v1.y);
	az = (double)(v2.z - v1.z);
	bx = (double)(v3.x - v1.x);
	by = (double)(v3.y - v1.y);
	bz = (double)(v3.z - v1.z);

	fDat1 = ax * ax + ay * ay + az * az;
	fDat2 = bx * bx + by * by + bz * bz;
	fDat3 = ax * bx + ay * by + az * bz;
	fDat3 *= fDat3;

	fDat1 = fDat1 * fDat2 - fDat3;
	if (fDat1 < (1e-5)) return 0.0;
	S = sqrt(fDat1) * 0.5;
	return S;
}

/**
 * 多角形の面積を計算.
 */
double MathUtil::CalcPolygonArea (sxsdk::shade_interface& shade, std::vector<sxsdk::vec3>& polygon)
{
	// 三角形分割を行う.
	::m_triangleIndex.clear();
	::CDivideTrianglesOutput divC;
	shade.divide_polygon(divC, polygon.size(), &(polygon[0]), true);
	const int triCou = ::m_triangleIndex.size() / 3;

	double area = 0.0;
	int iPos = 0;
	for (int i = 0; i < triCou; i++, iPos += 3) {
		const int i0 = ::m_triangleIndex[iPos + 0];
		const int i1 = ::m_triangleIndex[iPos + 1];
		const int i2 = ::m_triangleIndex[iPos + 2];

		area += CalcTriangleArea(polygon[i0], polygon[i1], polygon[i2]);
	}
	return area;
}

/**
 * ガンマ補正した色を計算.
 */
sxsdk::rgb_class MathUtil::CalcGamma (const sxsdk::rgb_class& col, const float gamma)
{
	sxsdk::rgb_class retCol = col;

	const float gammaInv = 1.0f / gamma;
	if (retCol.red   > 0.0f && retCol.red   < 1.0f) retCol.red   = std::pow(retCol.red,   gammaInv);
	if (retCol.green > 0.0f && retCol.green < 1.0f) retCol.green = std::pow(retCol.green, gammaInv);
	if (retCol.blue  > 0.0f && retCol.blue  < 1.0f) retCol.blue  = std::pow(retCol.blue,  gammaInv);
	return retCol;
}
