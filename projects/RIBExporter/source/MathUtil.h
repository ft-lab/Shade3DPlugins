/**
 * 数値演算系関数.
 */

#ifndef _MATHUTIL_H
#define _MATHUTIL_H

#include "GlobalHeader.h"

/**
 * 3Dのポイント情報.
 */
class CPoint3D
{
public:
	sxsdk::vec3 point;
	sxsdk::vec3 inHandle;
	sxsdk::vec3 outHandle;

public:
	CPoint3D () {
		point     = sxsdk::vec3(0, 0, 0);
		inHandle  = sxsdk::vec3(0, 0, 0);
		outHandle = sxsdk::vec3(0, 0, 0);
	}
};

namespace MathUtil
{
	/**
	 * ガウスの消去法で逆行列計算.
	 */
	bool InverseMatrix (const sxsdk::mat4& m, sxsdk::mat4& retM);

	/**
	 * 指定のベジェ曲線を直線の集まりにしたものを取得.
	 */
	void GetBezierToLines (const std::vector<CPoint3D>& bezierPosList, std::vector<sxsdk::vec3>& retPosList, const bool closed, const int divideLevel = 2);

	/**
	 * 3Dのラインリストで、リダクションを行う.
	 * 同一位置にある頂点のカット、直線的な位置にある中間の頂点をカット.
	 */
	void LineOptimize3D (std::vector<sxsdk::vec3>& lines);

	/**
	 * 三角形の面積を計算.
	 */
	double CalcTriangleArea (const sxsdk::vec3& v1, const sxsdk::vec3& v2, const sxsdk::vec3& v3);

	/**
	 * 多角形の面積を計算.
	 */
	double CalcPolygonArea (sxsdk::shade_interface& shade, std::vector<sxsdk::vec3>& polygon);

	/**
	 * ガンマ補正した色を計算.
	 */
	sxsdk::rgb_class CalcGamma (const sxsdk::rgb_class& col, const float gamma = 1.0f / 2.2f);
}

#endif
