/*
 * Copyright (C) 2022 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "TuningHelper"

#include "platform/mtkisp7/Tuning_Helper.h"

#include <algorithm>
#include <cstring>
#include <math.h>
#include <memory>
#include <stdint.h>
#include <string>
#include <time.h>

#if defined MTKCAM_USER_LOAD
#define TIME_START(start) \
	do {              \
	} while (0);

#define TIME_END(end) \
	do {          \
	} while (0);

#define TIME_DIFF_TO_US(start, end) ({ \
	int64_t retval = 0;            \
	retval;                        \
})

#else
#define TIME_START(start)                               \
	do {                                            \
		clock_gettime(CLOCK_MONOTONIC, &start); \
	} while (0);

#define TIME_END(end)                                 \
	do {                                          \
		clock_gettime(CLOCK_MONOTONIC, &end); \
	} while (0);

#define TIME_DIFF_TO_US(start, end) ({       \
	int64_t retval = 0;                  \
	struct timespec diff;                \
	time_diff_time(&start, &end, &diff); \
	retval = time_to_micro_sec(&diff);   \
	retval;                              \
})

#endif

#define TIME_ACC(start, end, acc)                    \
	do {                                         \
		time_accumulate(&start, &end, &acc); \
	} while (0);

#define TIME_RESET(acc)          \
	do {                     \
		acc.tv_sec = 0;  \
		acc.tv_nsec = 0; \
	} while (0);

#define TIME_CALL_V(acc, func, ...)       \
	do {                              \
		struct timespec start;    \
		struct timespec end;      \
		TIME_START(start);        \
		func(__VA_ARGS__);        \
		TIME_END(end);            \
		TIME_ACC(start, end, acc) \
	} while (0);

#define TIME_CALL_R(acc, ret, func, ...)  \
	do {                              \
		struct timespec start;    \
		struct timespec end;      \
		TIME_START(start);        \
		ret = func(__VA_ARGS__);  \
		TIME_END(end);            \
		TIME_ACC(start, end, acc) \
	} while (0);

#define TIME_TO_MS(period) time_to_mili_sec(&period)

#define TIME_TO_US(period) time_to_micro_sec(&period)

void time_diff_time(struct timespec *start,
		    struct timespec *end,
		    struct timespec *diff);

void time_accumulate(struct timespec *start,
		     struct timespec *end,
		     struct timespec *acc);

int64_t time_to_mili_sec(struct timespec *period);

int64_t time_to_micro_sec(struct timespec *period);

void time_diff_time(struct timespec *start,
		    struct timespec *end,
		    struct timespec *diff)
{
	if ((end->tv_nsec - start->tv_nsec) < 0) {
		diff->tv_sec = end->tv_sec - start->tv_sec - 1;
		diff->tv_nsec = 1000000000L + end->tv_nsec - start->tv_nsec;
	} else {
		diff->tv_sec = end->tv_sec - start->tv_sec;
		diff->tv_nsec = end->tv_nsec - start->tv_nsec;
	}
}

void time_accumulate(struct timespec *start,
		     struct timespec *end,
		     struct timespec *acc)
{
	struct timespec diff;

	time_diff_time(start, end, &diff);

	if ((acc->tv_nsec + diff.tv_nsec) >= 1000000000L) {
		acc->tv_sec = acc->tv_sec + diff.tv_sec + 1;
		acc->tv_nsec = acc->tv_nsec - 1000000000L + diff.tv_nsec;
	} else {
		acc->tv_sec = acc->tv_sec + diff.tv_sec;
		acc->tv_nsec = acc->tv_nsec + diff.tv_nsec;
	}
}

int64_t time_to_mili_sec(struct timespec *period)
{
	int64_t time_val = 0;

	if (period->tv_sec > 0) {
		time_val += ((int64_t)(period->tv_sec) * 1000);
	}

	if (period->tv_nsec > 0) {
		int64_t temp = (period->tv_nsec / 1000000);
		time_val += temp;

		temp = (int64_t)(period->tv_nsec) - temp * 1000000;
		if (temp >= 500000) {
			time_val++;
		}
	}

	return time_val;
}

int64_t time_to_micro_sec(struct timespec *period)
{
	int64_t time_val = 0;

	if (period->tv_sec > 0) {
		time_val += ((int64_t)(period->tv_sec) * 1000000);
	}

	if (period->tv_nsec > 0) {
		int64_t temp = ((int64_t)(period->tv_nsec) / 1000);
		time_val += temp;

		temp = (int64_t)(period->tv_nsec) - temp * 1000;
		if (temp >= 500) {
			time_val++;
		}
	}

	return time_val;
}

#define PROPERTY_VALUE_MAX 92

#define LOG_ERR(...) std::printf(__VA_ARGS__)
#define LOG_WRN(...) std::printf(__VA_ARGS__)
#define LOG_ADBDBG(...) std::printf(__VA_ARGS__)
#define LOG_INF(...) std::printf(__VA_ARGS__)
#define LOG_VRB(...) std::printf(__VA_ARGS__)
#define LOG_DBG(...) std::printf(__VA_ARGS__)
#define CAT_LOGD(...)
#define AEE_ASSERT(Module, String)

#define FUNCTION_LOG_START LOG_INF("+");
#define FUNCTION_LOG_END LOG_INF("-");

/*************************************************************************************
 * SLK Utility
 *************************************************************************************/
struct FwSlkParam {
	int CENTR_X;
	int CENTR_Y;
	int R_0;
	int R_1;
	int R_2;
	int GAIN_0;
	int GAIN_1;
	int GAIN_2;
	int GAIN_3;
	int GAIN_4;
	int IN_WD;
	int IN_HT;
	bool CROP_EN;
	int CROP_WD;
	int CROP_HT;
	int CROP_X;
	int CROP_Y;
	int OUT_WD;
	int OUT_HT;
	bool CRT_EN;
	int CRT_IN_WD;
	int CRT_IN_HT;
	int CRT_CROP_WD;
	int CRT_CROP_HT;
	int CRT_POINT_X0;
	int CRT_POINT_Y0;
	int CRT_POINT_X1;
	int CRT_POINT_Y1;
	int CRT_POINT_X2;
	int CRT_POINT_Y2;
	int CRT_POINT_X3;
	int CRT_POINT_Y3;
	bool PQ_CROP_EN;
	int PQ_IN_WD;
	int PQ_IN_HT;
	int PQ_CROP_WD;
	int PQ_CROP_HT;
	int PQ_CROP_X;
	int PQ_CROP_Y;
};

struct SlkParam {
	int CENTR_X; // Q0.16.0
	int CENTR_Y; // Q0.16.0
	int R_0; // Q0.16.0
	int R_1; // Q0.16.0
	int R_2; // Q0.16.0
	int GAIN_0; // Q0.8.0
	int GAIN_1; // Q0.8.0
	int GAIN_2; // Q0.8.0
	int GAIN_3; // Q0.8.0
	int GAIN_4; // Q0.8.0
	int SET_ZERO; // Q0.1.0
	int SLP_1; // Q0.8.16
	int SLP_2; // Q0.8.16
	int SLP_3; // Q0.8.16
	int SLP_4; // Q0.8.16
	int HRZ_COMP; // Q0.14.11
	int VRZ_COMP; // Q0.14.11
	int CROP_X; // Q0.16.11
	int CROP_Y; // Q0.16.11
};

/*************************************************************************************
 * module struct
 *************************************************************************************/
struct FwLscParam {
	uint32_t img_w;
	uint32_t img_h;
};

struct LscParam {
	uint32_t block_xofst;
	uint32_t block_yofst;
	uint32_t block_num_x;
	uint32_t block_num_y;
	uint32_t block_width;
	uint32_t block_height;
	uint32_t block_fwidth;
	uint32_t block_fheight;
	uint32_t block_lwidth;
	uint32_t block_lheight;
};

struct FwLtmParam {
	uint32_t img_w;
	uint32_t img_h;
	uint32_t block_num_x;
	uint32_t block_num_y;
};

struct LtmParam {
	uint32_t ltm_r1_ltm_blk_sz_x;
	uint32_t ltm_r1_ltm_blk_sz_y;
	uint32_t ltm_r1_ltm_blk_divx_apha_base;
	uint32_t ltm_r1_ltm_blk_divx_apha_base_shift_bit;
	uint32_t ltm_r1_ltm_blk_divy_apha_base;
	uint32_t ltm_r1_ltm_blk_divy_apha_base_shift_bit;
};

struct FwTncParam {
	uint32_t img_w;
	uint32_t img_h;
	uint32_t block_num_x;
	uint32_t block_num_y;
	uint32_t block_width;
	uint32_t block_height;
};

struct TncParam {
	/*
    uint32_t block_num_x;
    uint32_t block_num_y;*/
	uint32_t bce_blk_sz_x;
	uint32_t bce_blk_sz_y;

	uint32_t bce_blk_divx_apha_base;
	uint32_t bce_blk_divx_apha_base_shift_bit;
	uint32_t bce_blk_divy_apha_base;
	uint32_t bce_blk_divy_apha_base_shift_bit;
};

/************************************************
 *utils
 *************************************************/
UINT32 diff(const timespec &from, const timespec &to)
{
	UINT32 diff = 0;
	if (to.tv_sec || to.tv_nsec || from.tv_sec || from.tv_nsec) {
		diff = ((to.tv_sec - from.tv_sec) * 1000) +
		       ((to.tv_nsec - from.tv_nsec) / 1000000);
	}
	return diff;
}

////////////////////////////////////////////////////////
/*
template <class T>
T floor(T a) {
  double t = (int64_t)a;

  if (a == t || a > static_cast<double>(0.0))
    return t;
  else
    return t - static_cast<double>(1.0);
}
*/
template<class T>
T abs(T a)
{
	return (a < 0) ? -a : a;
}

template<class T>
T round(T num, int shift)
{
	bool neg = (num < 0);
	if (neg) {
		num = -num;
	}

	T ret = 0;
	if (shift == 0) {
		ret = num;
	} else if (shift < 0) {
		ret = num << (-shift);
	} else if (shift >= (int)sizeof(T) * 8) {
		ret = 0;
	} else {
		// Round half away from zero
		T inc = (((T)1) << shift) >> 1;
		ret = (num + inc) >> shift;
	}
	return neg ? -ret : ret;
}

template<class T>
T divInt(T n, T d, int prec)
{
	if (d == 0) {
		return 0;
	}
	bool sgn = (n ^ d) < 0;
	n = abs(n);
	d = abs(d);
	T q = (n * (((T)1) << prec) + (d >> 1)) / d;
	return sgn ? -q : q;
}

template<class T>
int msbPos(T a)
{
	// pos = sizeof(T) * 8 - 1 if a < 0
	// pos = -1 if a == 0
	int pos = sizeof(T) * 8 - 1;
	while ((pos >= 0) && ((a & (((T)1) << pos)) == 0)) {
		--pos;
	}
	return pos;
}

template<class T>
T approxL2Norm(T x, T y)
{
	if (x == 0 && y == 0) {
		return 0;
	}
	x = abs(x);
	y = abs(y);
	if (y > x) {
		return approxL2Norm(y, x);
	}
	T n = 0;
	if (2 * y <= x) {
		n = x + (y * y + x) / (2 * x);
	} else {
		n = round<T>(836 * x + 603 * y, 10);
	}
	return n;
}

/************************************************
 * utils for slk_algo_core
 *************************************************/
const int SLK_SLP_PREC = 16;
const int SLK_RZ_COMP_PREC = 11;

struct CropParam {
	int CROP_WD;
	int CROP_HT;
	int CROP_X;
	int CROP_Y;
};

struct CrtParam {
	int cenX; // Center of mass
	int cenY;
	int slpX; // Slope of regression line
	int slpY;
};

int calcOctDist(int cenX, int cenY, int x, int y)
{
	int distX = abs(x - cenX);
	int distY = abs(y - cenY);
	int dist =
		((distX + distY) * 1448) >> 11; // Using floor, consistent with HW
	dist = std::max(dist, distX);
	dist = std::max(dist, distY);
	return dist;
}

int calcMaxR(int cenX, int cenY, int wd, int ht)
{
	int dist1 = calcOctDist(cenX, cenY, 0, 0);
	int dist2 = calcOctDist(cenX, cenY, wd - 1, 0);
	int dist3 = calcOctDist(cenX, cenY, 0, ht - 1);
	int dist4 = calcOctDist(cenX, cenY, wd - 1, ht - 1);
	int maxR = std::max(dist1, dist2);
	maxR = std::max(maxR, dist3);
	maxR = std::max(maxR, dist4);
	return maxR;
}

CrtParam calcCrtParam(const FwSlkParam &fp)
{
	CrtParam param;
	int px[4] = {
		fp.CRT_POINT_X0,
		fp.CRT_POINT_X1,
		fp.CRT_POINT_X2,
		fp.CRT_POINT_X3
	};
	int py[4] = {
		fp.CRT_POINT_Y0,
		fp.CRT_POINT_Y1,
		fp.CRT_POINT_Y2,
		fp.CRT_POINT_Y3
	};

	// Center of mass
	param.cenX = 0;
	param.cenY = 0;
	for (int i = 0; i < 4; ++i) {
		param.cenX += px[i];
		param.cenY += py[i];
	}
	param.cenX = round(param.cenX, 2);
	param.cenY = round(param.cenY, 2);

	// Slope of regression line
	// TODO(MTK): Check sufficiency of precision of slpX and slpY. (Use int64?)
	param.slpX = 0;
	param.slpY = 0;
	for (int i = 0; i < 4; ++i) {
		int dx = px[i] - param.cenX;
		int dy = py[i] - param.cenY;
		param.slpX += dx * dx;
		param.slpY += dx * dy;
	}

	// Preserve 8 MSBs for slpX and slpY
	int slpX = abs(param.slpX);
	int slpY = abs(param.slpY);
	int maxSlp = std::max(slpX, slpY);
	int sh = std::max(msbPos(maxSlp) - 8, 0);
	param.slpX = round(param.slpX, sh); // Range: [-256,256]
	param.slpY = round(param.slpY, sh); // Range: [-256,256]

	return param;
}

void setSlope(const FwSlkParam &fp, SlkParam &sp)
{
	int maxR = calcMaxR(sp.CENTR_X, sp.CENTR_Y, fp.IN_WD, fp.IN_HT);
	int dR1 = sp.R_0;
	int dR2 = sp.R_1 - sp.R_0;
	int dR3 = sp.R_2 - sp.R_1;
	int dR4 = maxR - sp.R_2;
	int dG1 = sp.GAIN_1 - sp.GAIN_0;
	int dG2 = sp.GAIN_2 - sp.GAIN_1;
	int dG3 = sp.GAIN_3 - sp.GAIN_2;
	int dG4 = sp.GAIN_4 - sp.GAIN_3;
	if (dR1 > 0 && dG1 >= 0) {
		sp.SLP_1 = divInt(dG1, dR1, SLK_SLP_PREC);
	}
	if (dR2 > 0 && dG2 >= 0) {
		sp.SLP_2 = divInt(dG2, dR2, SLK_SLP_PREC);
	}
	if (dR3 > 0 && dG3 >= 0) {
		sp.SLP_3 = divInt(dG3, dR3, SLK_SLP_PREC);
	}
	if (dR4 > 0 && dG4 >= 0) {
		sp.SLP_4 = divInt(dG4, dR4, SLK_SLP_PREC);
	}
}

void setCrop(const FwSlkParam &fp, SlkParam &sp, const CropParam &cp)
{
	(void)fp;
	sp.CROP_X = cp.CROP_X << SLK_RZ_COMP_PREC;
	sp.CROP_Y = cp.CROP_Y << SLK_RZ_COMP_PREC;
}

void setScaling(const FwSlkParam &fp, SlkParam &sp, const CropParam &cp)
{
	sp.HRZ_COMP = divInt(cp.CROP_WD, fp.OUT_WD, SLK_RZ_COMP_PREC);
	sp.VRZ_COMP = divInt(cp.CROP_HT, fp.OUT_HT, SLK_RZ_COMP_PREC);
}

void setCrt(const FwSlkParam &fp, SlkParam &sp, const CropParam &cp)
{
	// CRT parameters
	CrtParam param = calcCrtParam(fp);

	// Coordinate conversion
	int rzRatioX = divInt(cp.CROP_WD, fp.CRT_IN_WD, SLK_RZ_COMP_PREC);
	int rzRatioY = divInt(cp.CROP_HT, fp.CRT_IN_HT, SLK_RZ_COMP_PREC);
	int imCenX = round((fp.CRT_CROP_WD >> 1) * rzRatioX, SLK_RZ_COMP_PREC);
	int imCenY = round((fp.CRT_CROP_HT >> 1) * rzRatioY, SLK_RZ_COMP_PREC);
	param.cenX = round(param.cenX * rzRatioX, SLK_RZ_COMP_PREC);
	param.cenY = round(param.cenY * rzRatioY, SLK_RZ_COMP_PREC);
	param.slpX = round(param.slpX * rzRatioX, SLK_RZ_COMP_PREC);
	param.slpY = round(param.slpY * rzRatioY, SLK_RZ_COMP_PREC);

	// Crop & translation
	int ofstX = param.cenX - imCenX;
	int ofstY = param.cenY - imCenY;
	int slkCenX = sp.CENTR_X - ofstX - cp.CROP_X;
	int slkCenY = sp.CENTR_Y - ofstY - cp.CROP_Y;

	// Rotation
	int dx = slkCenX - imCenX;
	int dy = slkCenY - imCenY;
	int slpR = approxL2Norm(param.slpX, param.slpY);
	slkCenX = imCenX + divInt(dx * param.slpX, slpR, 0) + divInt(dy * param.slpY, slpR, 0);
	slkCenY = imCenY - divInt(dx * param.slpY, slpR, 0) + divInt(dy * param.slpX, slpR, 0);

	// Center & offset
	if (slkCenX >= 0) {
		sp.CENTR_X = slkCenX;
		sp.CROP_X = 0;
	} else {
		sp.CENTR_X = 0;
		sp.CROP_X = (-slkCenX) << SLK_RZ_COMP_PREC;
	}
	if (slkCenY >= 0) {
		sp.CENTR_Y = slkCenY;
		sp.CROP_Y = 0;
	} else {
		sp.CENTR_Y = 0;
		sp.CROP_Y = (-slkCenY) << SLK_RZ_COMP_PREC;
	}

	// RzRatio = (CROP_SIZE / CRT_IN_SIZE) * (CRT_CROP_SIZE / OUT_SIZE)
	int64_t tmpH =
		static_cast<int64_t>(sp.HRZ_COMP) * static_cast<int64_t>(fp.CRT_CROP_WD);
	int64_t tmpV =
		static_cast<int64_t>(sp.VRZ_COMP) * static_cast<int64_t>(fp.CRT_CROP_HT);
	sp.HRZ_COMP = divInt<int64_t>(tmpH, fp.CRT_IN_WD, 0);
	sp.VRZ_COMP = divInt<int64_t>(tmpV, fp.CRT_IN_HT, 0);
}

void setPqCrop(const FwSlkParam &fp, SlkParam &sp)
{
	int64_t tmpH =
		static_cast<int64_t>(sp.HRZ_COMP) * static_cast<int64_t>(fp.OUT_WD);
	int64_t tmpV =
		static_cast<int64_t>(sp.VRZ_COMP) * static_cast<int64_t>(fp.OUT_HT);
	int tmpHrzComp = divInt<int64_t>(tmpH, fp.PQ_IN_WD, 0);
	int tmpVrzComp = divInt<int64_t>(tmpV, fp.PQ_IN_HT, 0);
	sp.CROP_X += fp.PQ_CROP_X * tmpHrzComp;
	sp.CROP_Y += fp.PQ_CROP_Y * tmpVrzComp;
	tmpH =
		static_cast<int64_t>(sp.HRZ_COMP) * static_cast<int64_t>(fp.PQ_CROP_WD);
	tmpV =
		static_cast<int64_t>(sp.VRZ_COMP) * static_cast<int64_t>(fp.PQ_CROP_HT);
	sp.HRZ_COMP = divInt<int64_t>(tmpH, fp.PQ_IN_WD, 0);
	sp.VRZ_COMP = divInt<int64_t>(tmpV, fp.PQ_IN_HT, 0);
}

/************************************************************************
 *
 ************************************************************************/
TuningHelper::TuningHelper()
{
	LOG_INF("TuningHelper(+)\n");
}

TuningHelper::~TuningHelper()
{
	LOG_INF("TuningHelper(-)\n");
}

void TuningHelper::init()
{
	FUNCTION_LOG_START;
	dbgEnable = 0;
	moduleLSCEnable = 0;
	moduleLTMEnable = 0;
	moduleTNCEnable = 0;
	moduleMEEnable = 0;
	moduleDMEnable = 0;
	moduleSNRSEnable = 0;
	moduleTNREnable = 0;
	moduleSNREnable = 0;
	moduleEEEnable = 0;
	moduleCNREnable = 0;
	moduleTDSHAPAEnable = 0;
	moduleTDSHAPBEnable = 0;
	moduleRegDump = 0;

	//dbgEnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.dbgth.enable", 0);

	//moduleLSCEnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thlscen.enable", 0);

	//moduleLTMEnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thltmen.enable", 0);

	//moduleTNCEnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thtncen.enable", 0);

	//moduleMEEnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thmeen.enable", 0);

	//moduleDMEnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thdmen.enable", 0);

	//moduleSNRSEnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thsnrsen.enable", 0);

	//moduleTNREnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thtnren.enable", 0);

	//moduleSNREnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thsnren.enable", 0);

	//moduleEEEnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.theeen.enable", 0);

	//moduleCNREnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thcnren.enable", 0);

	//moduleTDSHAPAEnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thtdshapaen.enable", 0);

	//moduleTDSHAPBEnable = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thtdshapben.enable", 0);

	//moduleRegDump = NSCam::Utils::Properties::property_get_int32(
	//"vendor.debug.thmoduleregdump.enable", 0);

	TIME_RESET(dohelper_st);
	TIME_RESET(dohelper_et);
	TIME_RESET(lsc_st);
	TIME_RESET(lsc_et);
	TIME_RESET(ltm_st);
	TIME_RESET(ltm_et);
	TIME_RESET(tnc_st);
	TIME_RESET(tnc_et);
	TIME_RESET(me_st);
	TIME_RESET(me_et);
	TIME_RESET(dm_st);
	TIME_RESET(dm_et);
	TIME_RESET(dipallslk_st);
	TIME_RESET(dipallslk_et);
	TIME_RESET(tdshapa_st);
	TIME_RESET(tdshapa_et);
	TIME_RESET(tdshapb_st);
	TIME_RESET(tdshapb_et);

	FUNCTION_LOG_END;
}

/************************************************
 * fw_slk_algo
 *************************************************/
void fwSlkCore(const FwSlkParam &fp, SlkParam &sp)
{
	// Initialization
	sp.CENTR_X = fp.CENTR_X;
	sp.CENTR_Y = fp.CENTR_Y;
	sp.R_0 = fp.R_0;
	sp.R_1 = fp.R_1;
	sp.R_2 = fp.R_2;
	sp.GAIN_0 = fp.GAIN_0;
	sp.GAIN_1 = fp.GAIN_1;
	sp.GAIN_2 = fp.GAIN_2;
	sp.GAIN_3 = fp.GAIN_3;
	sp.GAIN_4 = fp.GAIN_4;
	sp.SET_ZERO = 0;
	sp.SLP_1 = 0;
	sp.SLP_2 = 0;
	sp.SLP_3 = 0;
	sp.SLP_4 = 0;
	sp.HRZ_COMP = 1 << SLK_RZ_COMP_PREC;
	sp.VRZ_COMP = 1 << SLK_RZ_COMP_PREC;
	sp.CROP_X = 0;
	sp.CROP_Y = 0;

	// Crop information
	CropParam cp;
	cp.CROP_WD = fp.CROP_EN ? fp.CROP_WD : fp.IN_WD;
	cp.CROP_HT = fp.CROP_EN ? fp.CROP_HT : fp.IN_HT;
	cp.CROP_X = fp.CROP_EN ? fp.CROP_X : 0;
	cp.CROP_Y = fp.CROP_EN ? fp.CROP_Y : 0;

	// Slope
	setSlope(fp, sp);

	// Crop
	setCrop(fp, sp, cp);

	// Scaling
	setScaling(fp, sp, cp);

	// CRT
	if (fp.CRT_EN) {
		setCrt(fp, sp, cp);
	}

	// PQ_DIP crop
	if (fp.PQ_CROP_EN) {
		setPqCrop(fp, sp);
	}
}

/************************************************
 * fw_lsc_algo
 *************************************************/
void fwLscCore(const FwLscParam &fwlsc, LscParam &lscP)
{
	uint32_t img_w = fwlsc.img_w;
	uint32_t img_h = fwlsc.img_h;

	lscP.block_xofst = 0;
	lscP.block_yofst = 0;
	lscP.block_num_x = 16;
	lscP.block_num_y = 16;
	lscP.block_width = floor(img_w / 2 / lscP.block_num_x);
	lscP.block_height = floor(img_h / 2 / lscP.block_num_y);
	lscP.block_fwidth =
		floor((img_w / 2 - lscP.block_width * (lscP.block_num_x - 2)) / 2);
	lscP.block_fheight =
		floor((img_h / 2 - lscP.block_height * (lscP.block_num_y - 2)) / 2);
	lscP.block_lwidth =
		img_w / 2 - lscP.block_fwidth - lscP.block_width * (lscP.block_num_x - 2);
	lscP.block_lheight = img_h / 2 - lscP.block_fheight -
			     lscP.block_height * (lscP.block_num_y - 2);
}

/************************************************
 * fw_ltm_algo
 *************************************************/
void fwLtmCore(const FwLtmParam &fwltm, LtmParam &ltmP)
{
	uint32_t blk_x_num, blk_y_num, img_width, img_height, tmp, shift_bit;
	uint32_t blk_width, blk_height, half_blk_width, half_blk_height;
	img_width = fwltm.img_w;
	img_height = fwltm.img_h;

	blk_x_num = fwltm.block_num_x;
	blk_y_num = fwltm.block_num_y;
	blk_width =
		img_width / blk_x_num + ((img_width % blk_x_num != 0UL) ? 1UL : 0UL);
	blk_height =
		img_height / blk_y_num + ((img_height % blk_y_num != 0UL) ? 1UL : 0UL);

	/* ltm_r1_ltm_blk_sz */
	ltmP.ltm_r1_ltm_blk_sz_x = blk_width;
	ltmP.ltm_r1_ltm_blk_sz_y = blk_height;

	/* ltm_r1_ltm_blk_divx */
	tmp = (img_width + 256UL) >> 9UL;
	shift_bit = 4UL;
	while (tmp != 0UL) {
		tmp = tmp >> 1UL;
		shift_bit++;
	}
	ltmP.ltm_r1_ltm_blk_divx_apha_base_shift_bit = shift_bit;

	/* add half_blk_W calc. for 2-pixel mode */
	half_blk_width = (blk_width + 1) >> 1;
	ltmP.ltm_r1_ltm_blk_divx_apha_base =
		((1 << (shift_bit + 9)) + half_blk_width) / blk_width;

	/* ltm_r1_ltm_blk_divy */
	tmp = (img_height + 256UL) >> 9UL;
	shift_bit = 4UL;
	while (tmp != 0UL) {
		tmp = tmp >> 1UL;
		shift_bit++;
	}
	ltmP.ltm_r1_ltm_blk_divy_apha_base_shift_bit = shift_bit;

	/* add half_blk_H calc. for 2-pixel mode */
	half_blk_height = (blk_height + 1) >> 1;
	ltmP.ltm_r1_ltm_blk_divy_apha_base =
		((1 << (shift_bit + 9)) + half_blk_height) / blk_height;
}

/************************************************
 * fw_tnc_algo
 *************************************************/
void fwTncCore(const FwTncParam &fwtnc, TncParam &tncP)
{
	uint32_t blk_x_num, blk_y_num, img_width, img_height, tmp, shift_bit;
	uint32_t blk_width, blk_height, half_blk_width, half_blk_height;
	img_width = fwtnc.img_w;
	img_height = fwtnc.img_h;

	/*
  tncP.block_num_x = 12;  // Set by SW, default value = 12, (imageWidth +
                            // (TNC_BCE_BLK_X_NUM - 1)) / TNC_BCE_BLK_X_NUM >=32
  tncP.block_num_y = 9;   // Set by SW, default value = 9
  */
	blk_x_num = fwtnc.block_num_x;
	blk_y_num = fwtnc.block_num_y;
	blk_width =
		img_width / blk_x_num + ((img_width % blk_x_num != 0UL) ? 1UL : 0UL);
	blk_height =
		img_height / blk_y_num + ((img_height % blk_y_num != 0UL) ? 1UL : 0UL);

	if (blk_width < 32) {
		blk_width = 32; // HW constraint
		LOG_WRN("[K]%s: HW constraint blk_width = 32!!!", __func__);
	}

	tncP.bce_blk_sz_x = blk_width;
	tncP.bce_blk_sz_y = blk_height;

	/*
    blk_x_num = fwtnc.block_num_x;
    blk_y_num = fwtnc.block_num_y;
    blk_width = fwtnc.block_width;
    blk_height = fwtnc.block_height;
  */
	tmp = (img_width + 256UL) >> 9UL;
	shift_bit = 4UL;
	while (tmp != 0UL) {
		tmp = tmp >> 1UL;
		shift_bit++;
	}
	tncP.bce_blk_divx_apha_base_shift_bit = shift_bit;

	/* add half_blk_W calc. for 2-pixel mode */
	half_blk_width = (blk_width + 1) >> 1;
	tncP.bce_blk_divx_apha_base =
		((1 << (shift_bit + 9)) + half_blk_width) / blk_width;

	tmp = (img_height + 256UL) >> 9UL;
	shift_bit = 4UL;
	while (tmp != 0UL) {
		tmp = tmp >> 1UL;
		shift_bit++;
	}
	tncP.bce_blk_divy_apha_base_shift_bit = shift_bit;

	/* add half_blk_H calc. for 2-pixel mode */
	half_blk_height = (blk_height + 1) >> 1;
	tncP.bce_blk_divy_apha_base =
		((1 << (shift_bit + 9)) + half_blk_height) / blk_height;
}

/************************************************
 * dump_slk_input
 *************************************************/
void dump_slk_input(const FwSlkParam &fwslk, const char *module_name)
{
	LOG_INF(
		"[K]%s-%s:\n"
		"CENTR_X: %d\n"
		"CENTR_Y: %d\n"
		"R_0: %d\n"
		"R_1: %d\n"
		"R_2: %d\n"
		"GAIN_0: %d\n"
		"GAIN_1: %d\n"
		"GAIN_2: %d\n"
		"GAIN_3: %d\n"
		"GAIN_4: %d\n"
		"IN_WD: %d\n"
		"IN_HT: %d\n"
		"CROP_EN: %d\n"
		"CROP_WD: %d\n"
		"CROP_HT: %d\n"
		"CROP_X: %d\n"
		"CROP_Y: %d\n"
		"CRT_EN: %d\n"
		"OUT_WD: %d\n"
		"OUT_HT: %d\n"
		"CRT_IN_WD: %d\n"
		"CRT_IN_HT: %d\n"
		"CRT_CROP_WD: %d\n"
		"CRT_CROP_HT: %d\n"
		"CRT_POINT_X0: %d\n"
		"CRT_POINT_Y0: %d\n"
		"CRT_POINT_X1: %d\n"
		"CRT_POINT_Y1: %d\n"
		"CRT_POINT_X2: %d\n"
		"CRT_POINT_Y2: %d\n"
		"CRT_POINT_X3: %d\n"
		"CRT_POINT_Y3: %d\n"
		"PQ_CROP_EN: %d\n"
		"PQ_IN_WD: %d\n"
		"PQ_IN_HT: %d\n"
		"PQ_CROP_WD: %d\n"
		"PQ_CROP_HT: %d\n"
		"PQ_CROP_X: %d\n"
		"PQ_CROP_Y: %d\n",
		__func__, module_name, fwslk.CENTR_X, fwslk.CENTR_Y, fwslk.R_0, fwslk.R_1,
		fwslk.R_2, fwslk.GAIN_0, fwslk.GAIN_1, fwslk.GAIN_2, fwslk.GAIN_3,
		fwslk.GAIN_4, fwslk.IN_WD, fwslk.IN_HT, fwslk.CROP_EN, fwslk.CROP_WD,
		fwslk.CROP_HT, fwslk.CROP_X, fwslk.CROP_Y, fwslk.CRT_EN, fwslk.OUT_WD,
		fwslk.OUT_HT, fwslk.CRT_IN_WD, fwslk.CRT_IN_HT, fwslk.CRT_CROP_WD,
		fwslk.CRT_CROP_HT, fwslk.CRT_POINT_X0, fwslk.CRT_POINT_Y0,
		fwslk.CRT_POINT_X1, fwslk.CRT_POINT_Y1, fwslk.CRT_POINT_X2,
		fwslk.CRT_POINT_Y2, fwslk.CRT_POINT_X3, fwslk.CRT_POINT_Y3,
		fwslk.PQ_CROP_EN, fwslk.PQ_IN_WD, fwslk.PQ_IN_HT, fwslk.PQ_CROP_WD,
		fwslk.PQ_CROP_HT, fwslk.PQ_CROP_X, fwslk.PQ_CROP_Y);
}

/************************************************
 * module cfg
 *************************************************/
void cfg_lsc(struct mtk_img_uapi_meta_raw_stats_cfg *metai,
	     struct dltb_t *dl_table,
	     struct MwCtrlParams *ctrli)
{
	(void)ctrli;
	struct FwLscParam fwlsc;
	struct LscParam lscP;
	// prepare fw input data
	fwlsc.img_w = dl_table->src_wd;
	fwlsc.img_h = dl_table->src_ht;
	// do fw_lsc_algo
	fwLscCore(fwlsc, lscP);
	// patch output data 2 metai
	metai->prot.lsc_t1.LSC_CTL1.bits.LSC_SDBLK_YOFST = lscP.block_yofst;
	metai->prot.lsc_t1.LSC_CTL1.bits.LSC_SDBLK_XOFST = lscP.block_xofst;
	metai->prot.lsc_t1.LSC_CTL2.bits.LSC_SDBLK_XNUM = lscP.block_num_x;
	metai->prot.lsc_t1.LSC_CTL2.bits.LSC_SDBLK_WIDTH = lscP.block_width;
	metai->prot.lsc_t1.LSC_CTL3.bits.LSC_SDBLK_YNUM = lscP.block_num_y;
	metai->prot.lsc_t1.LSC_CTL3.bits.LSC_SDBLK_HEIGHT = lscP.block_height;
	metai->prot.lsc_t1.LSC_FBLOCK.bits.LSC_SDBLK_FWIDTH = lscP.block_fwidth;
	metai->prot.lsc_t1.LSC_FBLOCK.bits.LSC_SDBLK_FHEIGHT = lscP.block_fheight;
	metai->prot.lsc_t1.LSC_LBLOCK.bits.LSC_SDBLK_lWIDTH = lscP.block_lwidth;
	metai->prot.lsc_t1.LSC_LBLOCK.bits.LSC_SDBLK_lHEIGHT = lscP.block_lheight;
}

void cfg_ltm(struct mtk_img_uapi_meta_raw_stats_cfg *metai,
	     struct dltb_t *dl_table,
	     struct MwCtrlParams *ctrli)
{
	(void)ctrli;
	struct FwLtmParam fwltm;
	struct LtmParam ltmP;
	// prepare input data for ltm
	fwltm.img_w = dl_table->src_wd;
	fwltm.img_h = dl_table->src_ht;

	fwltm.block_num_x =
		metai->prot.ltm_t1.LTM_BLK_NUM.bits.LTM_BLK_X_NUM; // Xnum

	fwltm.block_num_y = metai->prot.ltm_t1.LTM_BLK_NUM.bits.LTM_BLK_Y_NUM;
	// do
	fwLtmCore(fwltm, ltmP);
	// patch output data 2 metai
	metai->prot.ltm_t1.LTM_BLK_SZ.bits.LTM_BLK_WIDTH = ltmP.ltm_r1_ltm_blk_sz_x;
	metai->prot.ltm_t1.LTM_BLK_SZ.bits.LTM_BLK_HEIGHT = ltmP.ltm_r1_ltm_blk_sz_y;
	metai->prot.ltm_t1.LTM_BLK_DIVX.bits.LTM_X_ALPHA_BASE =
		ltmP.ltm_r1_ltm_blk_divx_apha_base;
	metai->prot.ltm_t1.LTM_BLK_DIVX.bits.LTM_X_ALPHA_BASE_SHIFT_BIT =
		ltmP.ltm_r1_ltm_blk_divx_apha_base_shift_bit;
	metai->prot.ltm_t1.LTM_BLK_DIVY.bits.LTM_Y_ALPHA_BASE =
		ltmP.ltm_r1_ltm_blk_divy_apha_base;
	metai->prot.ltm_t1.LTM_BLK_DIVY.bits.LTM_Y_ALPHA_BASE_SHIFT_BIT =
		ltmP.ltm_r1_ltm_blk_divy_apha_base_shift_bit;
}

void cfg_me(struct mtk_img_uapi_meta_raw_stats_cfg *metai,
	    struct MwCtrlParams *ctrli,
	    int moduleRegDump)
{
	struct FwSlkParam fwslk;
	struct SlkParam slkP;
	std::memset(&fwslk, 0, sizeof(struct FwSlkParam));
	std::memset(&slkP, 0, sizeof(struct SlkParam));
	fwslk.CENTR_X = metai->prot.slk_param.center_x;
	fwslk.CENTR_Y = metai->prot.slk_param.center_y;
	fwslk.R_0 = metai->prot.slk_param.radius_0;
	fwslk.R_1 = metai->prot.slk_param.radius_1;
	fwslk.R_2 = metai->prot.slk_param.radius_2;
	fwslk.GAIN_0 = metai->prot.slk_param.gain0;
	fwslk.GAIN_1 = metai->prot.slk_param.gain1;
	fwslk.GAIN_2 = metai->prot.slk_param.gain2;
	fwslk.GAIN_3 = metai->prot.slk_param.gain3;
	fwslk.GAIN_4 = metai->prot.slk_param.gain4;
	fwslk.IN_WD = metai->sensor_param.tg_width;
	fwslk.IN_HT = metai->sensor_param.tg_height;
	fwslk.CROP_EN = metai->prot.drzs8t_crop_param.is_enable;
	fwslk.CROP_WD = metai->prot.drzs8t_crop_param.input_width;
	fwslk.CROP_HT = metai->prot.drzs8t_crop_param.input_height;
	fwslk.CROP_X = metai->prot.drzs8t_crop_param.start_ofst_x;
	fwslk.CROP_Y = metai->prot.drzs8t_crop_param.start_ofst_y;
	fwslk.CRT_EN = metai->prot.wraping_param.CRT_EN;
	fwslk.OUT_WD = ctrli->me_ctrl_slk.IN_WD;
	fwslk.OUT_HT = ctrli->me_ctrl_slk.IN_HT;
	fwslk.CRT_IN_WD = metai->prot.wraping_param.CRT_IN_WD;
	fwslk.CRT_IN_HT = metai->prot.wraping_param.CRT_IN_HT;
	fwslk.CRT_CROP_WD = metai->prot.wraping_param.CRT_CROP_WD;
	fwslk.CRT_CROP_HT = metai->prot.wraping_param.CRT_CROP_HT;
	fwslk.CRT_POINT_X0 = metai->prot.wraping_param.CRT_POINT_X0;
	fwslk.CRT_POINT_Y0 = metai->prot.wraping_param.CRT_POINT_Y0;
	fwslk.CRT_POINT_X1 = metai->prot.wraping_param.CRT_POINT_X1;
	fwslk.CRT_POINT_Y1 = metai->prot.wraping_param.CRT_POINT_Y1;
	fwslk.CRT_POINT_X2 = metai->prot.wraping_param.CRT_POINT_X2;
	fwslk.CRT_POINT_Y2 = metai->prot.wraping_param.CRT_POINT_Y2;
	fwslk.CRT_POINT_X3 = metai->prot.wraping_param.CRT_POINT_X3;
	fwslk.CRT_POINT_Y3 = metai->prot.wraping_param.CRT_POINT_Y3;
	/*fwslk.PQ_CROP_EN;
  fwslk.PQ_IN_WD;
  fwslk.PQ_IN_HT;
  fwslk.PQ_CROP_WD;
  fwslk.PQ_CROP_HT;
  fwslk.PQ_CROP_X;
  fwslk.PQ_CROP_Y;*/
	if (moduleRegDump)
		dump_slk_input(fwslk, "ME_SLK");
	// do slk algo core
	fwSlkCore(fwslk, slkP);
	// patch output data 2 metai
	metai->prot.me_e1.ME_SLK_CEN.bits.ME_SLK_CENTR_X = slkP.CENTR_X;
	metai->prot.me_e1.ME_SLK_CEN.bits.ME_SLK_CENTR_Y = slkP.CENTR_Y;
	metai->prot.me_e1.ME_SLK_RR_CON0.bits.ME_SLK_R_0 = slkP.R_0;
	metai->prot.me_e1.ME_SLK_RR_CON0.bits.ME_SLK_R_1 = slkP.R_1;
	metai->prot.me_e1.ME_SLK_RR_CON1.bits.ME_SLK_R_2 = slkP.R_2;
	//
	metai->prot.me_e1.ME_SLK_RR_CON1.bits.ME_SLK_GAIN_0 = slkP.GAIN_0;
	metai->prot.me_e1.ME_SLK_RR_CON1.bits.ME_SLK_GAIN_1 = slkP.GAIN_1;
	metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_GAIN_2 = slkP.GAIN_2;
	metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_GAIN_3 = slkP.GAIN_3;
	metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_GAIN_4 = slkP.GAIN_4;
	metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_SET_ZERO = slkP.SET_ZERO;
	//
	metai->prot.me_e1.ME_SLK_CROPX.bits.ME_SLK_CROP_X = slkP.CROP_X;
	metai->prot.me_e1.ME_SLK_CROPY.bits.ME_SLK_CROP_Y = slkP.CROP_Y;
	//
	metai->prot.me_e1.ME_SLK_HRZ.bits.ME_SLK_HRZ_COMP = slkP.HRZ_COMP;
	metai->prot.me_e1.ME_SLK_VRZ.bits.ME_SLK_VRZ_COMP = slkP.VRZ_COMP;
	metai->prot.me_e1.ME_SLK_SLP_CON0.bits.ME_SLK_SLP_1 = slkP.SLP_1;
	metai->prot.me_e1.ME_SLK_SLP_CON1.bits.ME_SLK_SLP_2 = slkP.SLP_2;
	metai->prot.me_e1.ME_SLK_SLP_CON2.bits.ME_SLK_SLP_3 = slkP.SLP_3;
	metai->prot.me_e1.ME_SLK_SLP_CON3.bits.ME_SLK_SLP_4 = slkP.SLP_4;
}

void cfg_dm(struct mtk_img_uapi_meta_raw_stats_cfg *metai,
	    struct dltb_t *dl_table, struct MwCtrlParams *ctrli,
	    int moduleRegDump)
{
	(void)ctrli;
	struct FwSlkParam fwslk;
	struct SlkParam slkP;
	std::memset(&fwslk, 0, sizeof(struct FwSlkParam));
	std::memset(&slkP, 0, sizeof(struct SlkParam));
	fwslk.CENTR_X = metai->prot.slk_param.center_x;
	fwslk.CENTR_Y = metai->prot.slk_param.center_y;
	fwslk.R_0 = metai->prot.slk_param.radius_0;
	fwslk.R_1 = metai->prot.slk_param.radius_1;
	fwslk.R_2 = metai->prot.slk_param.radius_2;
	fwslk.GAIN_0 = metai->prot.slk_param.gain0;
	fwslk.GAIN_1 = metai->prot.slk_param.gain1;
	fwslk.GAIN_2 = metai->prot.slk_param.gain2;
	fwslk.GAIN_3 = metai->prot.slk_param.gain3;
	fwslk.GAIN_4 = metai->prot.slk_param.gain4;
	fwslk.IN_WD = metai->sensor_param.tg_width;
	fwslk.IN_HT = metai->sensor_param.tg_height;
	fwslk.CROP_EN = 0;
	fwslk.CROP_WD = metai->prot.drzs8t_crop_param.input_width;
	fwslk.CROP_HT = metai->prot.drzs8t_crop_param.input_height;
	fwslk.CROP_X = metai->prot.drzs8t_crop_param.start_ofst_x;
	fwslk.CROP_Y = metai->prot.drzs8t_crop_param.start_ofst_y;
	fwslk.CRT_EN = 0;
	fwslk.OUT_WD = dl_table->src_wd;
	fwslk.OUT_HT = dl_table->src_ht;
	fwslk.CRT_IN_WD = metai->prot.wraping_param.CRT_IN_WD;
	fwslk.CRT_IN_HT = metai->prot.wraping_param.CRT_IN_HT;
	fwslk.CRT_CROP_WD = metai->prot.wraping_param.CRT_CROP_WD;
	fwslk.CRT_CROP_HT = metai->prot.wraping_param.CRT_CROP_HT;
	fwslk.CRT_POINT_X0 = metai->prot.wraping_param.CRT_POINT_X0;
	fwslk.CRT_POINT_Y0 = metai->prot.wraping_param.CRT_POINT_Y0;
	fwslk.CRT_POINT_X1 = metai->prot.wraping_param.CRT_POINT_X1;
	fwslk.CRT_POINT_Y1 = metai->prot.wraping_param.CRT_POINT_Y1;
	fwslk.CRT_POINT_X2 = metai->prot.wraping_param.CRT_POINT_X2;
	fwslk.CRT_POINT_Y2 = metai->prot.wraping_param.CRT_POINT_Y2;
	fwslk.CRT_POINT_X3 = metai->prot.wraping_param.CRT_POINT_X3;
	fwslk.CRT_POINT_Y3 = metai->prot.wraping_param.CRT_POINT_Y3;
	/*fwslk.PQ_CROP_EN;
  fwslk.PQ_IN_WD;
  fwslk.PQ_IN_HT;
  fwslk.PQ_CROP_WD;
  fwslk.PQ_CROP_HT;
  fwslk.PQ_CROP_X;
  fwslk.PQ_CROP_Y;*/
	if (moduleRegDump)
		dump_slk_input(fwslk, "DM_SLK");
	// do slk algo core
	fwSlkCore(fwslk, slkP);
	// patch output data 2 metai
	metai->prot.dm_t1.DM_SLK_CEN.bits.DM_SLK_CENTR_X = slkP.CENTR_X;
	metai->prot.dm_t1.DM_SLK_CEN.bits.DM_SLK_CENTR_Y = slkP.CENTR_Y;
	metai->prot.dm_t1.DM_SLK_RR_CON0.bits.DM_SLK_R_0 = slkP.R_0;
	metai->prot.dm_t1.DM_SLK_RR_CON0.bits.DM_SLK_R_1 = slkP.R_1;
	metai->prot.dm_t1.DM_SLK_RR_CON1.bits.DM_SLK_R_2 = slkP.R_2;
	//
	metai->prot.dm_t1.DM_SLK_RR_CON1.bits.DM_SLK_GAIN_0 = slkP.GAIN_0;
	metai->prot.dm_t1.DM_SLK_RR_CON1.bits.DM_SLK_GAIN_1 = slkP.GAIN_1;
	metai->prot.dm_t1.DM_SLK_GAIN.bits.DM_SLK_GAIN_2 = slkP.GAIN_2;
	metai->prot.dm_t1.DM_SLK_GAIN.bits.DM_SLK_GAIN_3 = slkP.GAIN_3;
	metai->prot.dm_t1.DM_SLK_GAIN.bits.DM_SLK_GAIN_4 = slkP.GAIN_4;
	metai->prot.dm_t1.DM_SLK_GAIN.bits.DM_SLK_SET_ZERO = slkP.SET_ZERO;
	//
	metai->prot.dm_t1.DM_SLK_CROPX.bits.DM_SLK_CROP_X = slkP.CROP_X;
	metai->prot.dm_t1.DM_SLK_CROPY.bits.DM_SLK_CROP_Y = slkP.CROP_Y;
	//
	metai->prot.dm_t1.DM_SLK_HRZ.bits.DM_SLK_HRZ_COMP = slkP.HRZ_COMP;
	metai->prot.dm_t1.DM_SLK_VRZ.bits.DM_SLK_VRZ_COMP = slkP.VRZ_COMP;
	metai->prot.dm_t1.DM_SLK_SLP_CON0.bits.DM_SLK_SLP_1 = slkP.SLP_1;
	metai->prot.dm_t1.DM_SLK_SLP_CON1.bits.DM_SLK_SLP_2 = slkP.SLP_2;
	metai->prot.dm_t1.DM_SLK_SLP_CON2.bits.DM_SLK_SLP_3 = slkP.SLP_3;
	metai->prot.dm_t1.DM_SLK_SLP_CON3.bits.DM_SLK_SLP_4 = slkP.SLP_4;
}

void cfg_allDipSlk(struct mtk_img_uapi_meta_raw_stats_cfg *metai,
		   struct dltb_t *dl_table, struct MwCtrlParams *ctrli,
		   int moduleRegDump)
{
	(void)ctrli;
	struct FwSlkParam fwslk;
	struct SlkParam slkP;
	std::memset(&fwslk, 0, sizeof(struct FwSlkParam));
	std::memset(&slkP, 0, sizeof(struct SlkParam));

	fwslk.CENTR_X = metai->prot.slk_param.center_x;
	fwslk.CENTR_Y = metai->prot.slk_param.center_y;
	fwslk.R_0 = metai->prot.slk_param.radius_0;
	fwslk.R_1 = metai->prot.slk_param.radius_1;
	fwslk.R_2 = metai->prot.slk_param.radius_2;
	fwslk.GAIN_0 = metai->prot.slk_param.gain0;
	fwslk.GAIN_1 = metai->prot.slk_param.gain1;
	fwslk.GAIN_2 = metai->prot.slk_param.gain2;
	fwslk.GAIN_3 = metai->prot.slk_param.gain3;
	fwslk.GAIN_4 = metai->prot.slk_param.gain4;
	fwslk.IN_WD = metai->sensor_param.tg_width;
	fwslk.IN_HT = metai->sensor_param.tg_height;
	fwslk.CROP_EN = metai->prot.drzs8t_crop_param.is_enable;
	fwslk.CROP_WD = metai->prot.drzs8t_crop_param.input_width;
	fwslk.CROP_HT = metai->prot.drzs8t_crop_param.input_height;
	fwslk.CROP_X = metai->prot.drzs8t_crop_param.start_ofst_x;
	fwslk.CROP_Y = metai->prot.drzs8t_crop_param.start_ofst_y;
	fwslk.CRT_EN = metai->prot.wraping_param.CRT_EN;
	fwslk.OUT_WD = dl_table->src_wd;
	fwslk.OUT_HT = dl_table->src_ht;
	fwslk.CRT_IN_WD = metai->prot.wraping_param.CRT_IN_WD;
	fwslk.CRT_IN_HT = metai->prot.wraping_param.CRT_IN_HT;
	fwslk.CRT_CROP_WD = metai->prot.wraping_param.CRT_CROP_WD;
	fwslk.CRT_CROP_HT = metai->prot.wraping_param.CRT_CROP_HT;
	fwslk.CRT_POINT_X0 = metai->prot.wraping_param.CRT_POINT_X0;
	fwslk.CRT_POINT_Y0 = metai->prot.wraping_param.CRT_POINT_Y0;
	fwslk.CRT_POINT_X1 = metai->prot.wraping_param.CRT_POINT_X1;
	fwslk.CRT_POINT_Y1 = metai->prot.wraping_param.CRT_POINT_Y1;
	fwslk.CRT_POINT_X2 = metai->prot.wraping_param.CRT_POINT_X2;
	fwslk.CRT_POINT_Y2 = metai->prot.wraping_param.CRT_POINT_Y2;
	fwslk.CRT_POINT_X3 = metai->prot.wraping_param.CRT_POINT_X3;
	fwslk.CRT_POINT_Y3 = metai->prot.wraping_param.CRT_POINT_Y3;
	/*fwslk.PQ_CROP_EN;
  fwslk.PQ_IN_WD;
  fwslk.PQ_IN_HT;
  fwslk.PQ_CROP_WD;
  fwslk.PQ_CROP_HT;
  fwslk.PQ_CROP_X;
  fwslk.PQ_CROP_Y;*/
	if (moduleRegDump)
		dump_slk_input(fwslk, "DIP_ALL_SLK");
	// do slk algo core
	fwSlkCore(fwslk, slkP);
	if (metai->prot.snrs_d1.SNRS_CON1.bits.SNRS_SLK_LINK) {
		// patch output data 2 metai (SNRS)
		metai->prot.snrs_d1.SNRS_SLK_CEN.bits.SNRS_SLK_CENTR_X = slkP.CENTR_X;
		metai->prot.snrs_d1.SNRS_SLK_CEN.bits.SNRS_SLK_CENTR_Y = slkP.CENTR_Y;
		metai->prot.snrs_d1.SNRS_SLK_RR_CON0.bits.SNRS_SLK_R_0 = slkP.R_0;
		metai->prot.snrs_d1.SNRS_SLK_RR_CON0.bits.SNRS_SLK_R_1 = slkP.R_1;
		metai->prot.snrs_d1.SNRS_SLK_RR_CON1.bits.SNRS_SLK_R_2 = slkP.R_2;
		//
		metai->prot.snrs_d1.SNRS_SLK_RR_CON1.bits.SNRS_SLK_GAIN_0 = slkP.GAIN_0;
		metai->prot.snrs_d1.SNRS_SLK_RR_CON1.bits.SNRS_SLK_GAIN_1 = slkP.GAIN_1;
		metai->prot.snrs_d1.SNRS_SLK_GAIN.bits.SNRS_SLK_GAIN_2 = slkP.GAIN_2;
		metai->prot.snrs_d1.SNRS_SLK_GAIN.bits.SNRS_SLK_GAIN_3 = slkP.GAIN_3;
		metai->prot.snrs_d1.SNRS_SLK_GAIN.bits.SNRS_SLK_GAIN_4 = slkP.GAIN_4;
		metai->prot.snrs_d1.SNRS_SLK_GAIN.bits.SNRS_SLK_SET_ZERO = slkP.SET_ZERO;
		//
		metai->prot.snrs_d1.SNRS_SLK_CROPX.bits.SNRS_SLK_CROP_X = slkP.CROP_X;
		metai->prot.snrs_d1.SNRS_SLK_CROPY.bits.SNRS_SLK_CROP_Y = slkP.CROP_Y;
		//
		metai->prot.snrs_d1.SNRS_SLK_HRZ.bits.SNRS_SLK_HRZ_COMP = slkP.HRZ_COMP;
		metai->prot.snrs_d1.SNRS_SLK_VRZ.bits.SNRS_SLK_VRZ_COMP = slkP.VRZ_COMP;
		metai->prot.snrs_d1.SNRS_SLK_SLP_CON0.bits.SNRS_SLK_SLP_1 = slkP.SLP_1;
		metai->prot.snrs_d1.SNRS_SLK_SLP_CON1.bits.SNRS_SLK_SLP_2 = slkP.SLP_2;
		metai->prot.snrs_d1.SNRS_SLK_SLP_CON2.bits.SNRS_SLK_SLP_3 = slkP.SLP_3;
		metai->prot.snrs_d1.SNRS_SLK_SLP_CON3.bits.SNRS_SLK_SLP_4 = slkP.SLP_4;
	} else {
		// reset to default value if no need slk_link
		metai->prot.snrs_d1.SNRS_SLK_HRZ.bits.SNRS_SLK_HRZ_COMP = 2048;
		metai->prot.snrs_d1.SNRS_SLK_VRZ.bits.SNRS_SLK_VRZ_COMP = 2048;
	}
	if (metai->prot.tnr_d1.TNR_TEMPORAL.bits.TNR_SLK_EN) {
		// patch output data 2 metai (TNR)
		metai->prot.tnr_d1.TNR_SLK_CEN.bits.TNR_SLK_CENTR_X = slkP.CENTR_X;
		metai->prot.tnr_d1.TNR_SLK_CEN.bits.TNR_SLK_CENTR_Y = slkP.CENTR_Y;
		metai->prot.tnr_d1.TNR_SLK_RR_CON0.bits.TNR_SLK_R_0 = slkP.R_0;
		metai->prot.tnr_d1.TNR_SLK_RR_CON0.bits.TNR_SLK_R_1 = slkP.R_1;
		metai->prot.tnr_d1.TNR_SLK_RR_CON1.bits.TNR_SLK_R_2 = slkP.R_2;
		//
		metai->prot.tnr_d1.TNR_SLK_RR_CON1.bits.TNR_SLK_GAIN_0 = slkP.GAIN_0;
		metai->prot.tnr_d1.TNR_SLK_RR_CON1.bits.TNR_SLK_GAIN_1 = slkP.GAIN_1;
		metai->prot.tnr_d1.TNR_SLK_GAIN.bits.TNR_SLK_GAIN_2 = slkP.GAIN_2;
		metai->prot.tnr_d1.TNR_SLK_GAIN.bits.TNR_SLK_GAIN_3 = slkP.GAIN_3;
		metai->prot.tnr_d1.TNR_SLK_GAIN.bits.TNR_SLK_GAIN_4 = slkP.GAIN_4;
		metai->prot.tnr_d1.TNR_SLK_GAIN.bits.TNR_SLK_SET_ZERO = slkP.SET_ZERO;
		//
		metai->prot.tnr_d1.TNR_SLK_CROPX.bits.TNR_SLK_CROP_X = slkP.CROP_X;
		metai->prot.tnr_d1.TNR_SLK_CROPY.bits.TNR_SLK_CROP_Y = slkP.CROP_Y;
		//
		metai->prot.tnr_d1.TNR_SLK_HRZ.bits.TNR_SLK_HRZ_COMP = slkP.HRZ_COMP;
		metai->prot.tnr_d1.TNR_SLK_VRZ.bits.TNR_SLK_VRZ_COMP = slkP.VRZ_COMP;
		metai->prot.tnr_d1.TNR_SLK_SLP_CON0.bits.TNR_SLK_SLP_1 = slkP.SLP_1;
		metai->prot.tnr_d1.TNR_SLK_SLP_CON1.bits.TNR_SLK_SLP_2 = slkP.SLP_2;
		metai->prot.tnr_d1.TNR_SLK_SLP_CON2.bits.TNR_SLK_SLP_3 = slkP.SLP_3;
		metai->prot.tnr_d1.TNR_SLK_SLP_CON3.bits.TNR_SLK_SLP_4 = slkP.SLP_4;
	} else {
		// reset to default value if no need slk_link
		metai->prot.tnr_d1.TNR_SLK_HRZ.bits.TNR_SLK_HRZ_COMP = 2048;
		metai->prot.tnr_d1.TNR_SLK_VRZ.bits.TNR_SLK_VRZ_COMP = 2048;
	}
	if (metai->prot.snr_d1.SNR_CON1.bits.SNR_SLK_LINK) {
		// patch output data 2 metai (SNR)
		metai->prot.snr_d1.SNR_SLK_CEN.bits.SNR_SLK_CENTR_X = slkP.CENTR_X;
		metai->prot.snr_d1.SNR_SLK_CEN.bits.SNR_SLK_CENTR_Y = slkP.CENTR_Y;
		metai->prot.snr_d1.SNR_SLK_RR_CON0.bits.SNR_SLK_R_0 = slkP.R_0;
		metai->prot.snr_d1.SNR_SLK_RR_CON0.bits.SNR_SLK_R_1 = slkP.R_1;
		metai->prot.snr_d1.SNR_SLK_RR_CON1.bits.SNR_SLK_R_2 = slkP.R_2;
		//
		metai->prot.snr_d1.SNR_SLK_RR_CON1.bits.SNR_SLK_GAIN_0 = slkP.GAIN_0;
		metai->prot.snr_d1.SNR_SLK_RR_CON1.bits.SNR_SLK_GAIN_1 = slkP.GAIN_1;
		metai->prot.snr_d1.SNR_SLK_GAIN.bits.SNR_SLK_GAIN_2 = slkP.GAIN_2;
		metai->prot.snr_d1.SNR_SLK_GAIN.bits.SNR_SLK_GAIN_3 = slkP.GAIN_3;
		metai->prot.snr_d1.SNR_SLK_GAIN.bits.SNR_SLK_GAIN_4 = slkP.GAIN_4;
		metai->prot.snr_d1.SNR_SLK_GAIN.bits.SNR_SLK_SET_ZERO = slkP.SET_ZERO;
		//
		metai->prot.snr_d1.SNR_SLK_CROPX.bits.SNR_SLK_CROP_X = slkP.CROP_X;
		metai->prot.snr_d1.SNR_SLK_CROPY.bits.SNR_SLK_CROP_Y = slkP.CROP_Y;
		//
		metai->prot.snr_d1.SNR_SLK_HRZ.bits.SNR_SLK_HRZ_COMP = slkP.HRZ_COMP;
		metai->prot.snr_d1.SNR_SLK_VRZ.bits.SNR_SLK_VRZ_COMP = slkP.VRZ_COMP;
		metai->prot.snr_d1.SNR_SLK_SLP_CON0.bits.SNR_SLK_SLP_1 = slkP.SLP_1;
		metai->prot.snr_d1.SNR_SLK_SLP_CON1.bits.SNR_SLK_SLP_2 = slkP.SLP_2;
		metai->prot.snr_d1.SNR_SLK_SLP_CON2.bits.SNR_SLK_SLP_3 = slkP.SLP_3;
		metai->prot.snr_d1.SNR_SLK_SLP_CON3.bits.SNR_SLK_SLP_4 = slkP.SLP_4;
	} else {
		// reset to default value if no need slk_link
		metai->prot.snr_d1.SNR_SLK_HRZ.bits.SNR_SLK_HRZ_COMP = 2048;
		metai->prot.snr_d1.SNR_SLK_VRZ.bits.SNR_SLK_VRZ_COMP = 2048;
	}
	if ((metai->prot.ee_d1.EE_LUMA_SLNK_CTRL.bits.EE_GLUT_LINK_EN) ||
	    (metai->prot.ee_d1.EE_CE_SL_CTRL.bits.EE_CE_SLMOD_EN)) {
		// patch output data 2 metai (EE)
		metai->prot.ee_d1.EE_SLK_CEN.bits.EE_SLK_CENTR_X = slkP.CENTR_X;
		metai->prot.ee_d1.EE_SLK_CEN.bits.EE_SLK_CENTR_Y = slkP.CENTR_Y;
		metai->prot.ee_d1.EE_SLK_RR_CON0.bits.EE_SLK_R_0 = slkP.R_0;
		metai->prot.ee_d1.EE_SLK_RR_CON0.bits.EE_SLK_R_1 = slkP.R_1;
		metai->prot.ee_d1.EE_SLK_RR_CON1.bits.EE_SLK_R_2 = slkP.R_2;
		//
		metai->prot.ee_d1.EE_SLK_RR_CON1.bits.EE_SLK_GAIN_0 = slkP.GAIN_0;
		metai->prot.ee_d1.EE_SLK_RR_CON1.bits.EE_SLK_GAIN_1 = slkP.GAIN_1;
		metai->prot.ee_d1.EE_SLK_GAIN.bits.EE_SLK_GAIN_2 = slkP.GAIN_2;
		metai->prot.ee_d1.EE_SLK_GAIN.bits.EE_SLK_GAIN_3 = slkP.GAIN_3;
		metai->prot.ee_d1.EE_SLK_GAIN.bits.EE_SLK_GAIN_4 = slkP.GAIN_4;
		metai->prot.ee_d1.EE_SLK_GAIN.bits.EE_SLK_SET_ZERO = slkP.SET_ZERO;
		//
		metai->prot.ee_d1.EE_SLK_CROPX.bits.EE_SLK_CROP_X = slkP.CROP_X;
		metai->prot.ee_d1.EE_SLK_CROPY.bits.EE_SLK_CROP_Y = slkP.CROP_Y;
		//
		metai->prot.ee_d1.EE_SLK_HRZ.bits.EE_SLK_HRZ_COMP = slkP.HRZ_COMP;
		metai->prot.ee_d1.EE_SLK_VRZ.bits.EE_SLK_VRZ_COMP = slkP.VRZ_COMP;
		metai->prot.ee_d1.EE_SLK_SLP_CON0.bits.EE_SLK_SLP_1 = slkP.SLP_1;
		metai->prot.ee_d1.EE_SLK_SLP_CON1.bits.EE_SLK_SLP_2 = slkP.SLP_2;
		metai->prot.ee_d1.EE_SLK_SLP_CON2.bits.EE_SLK_SLP_3 = slkP.SLP_3;
		metai->prot.ee_d1.EE_SLK_SLP_CON3.bits.EE_SLK_SLP_4 = slkP.SLP_4;
	} else {
		// reset to default value if no need slk_link
		metai->prot.ee_d1.EE_SLK_HRZ.bits.EE_SLK_HRZ_COMP = 2048;
		metai->prot.ee_d1.EE_SLK_VRZ.bits.EE_SLK_VRZ_COMP = 2048;
	}
	if ((metai->prot.cnr_d1.CNR_CNR_CTRL.bits.CNR_CNR_SLK_LINK) ||
	    (metai->prot.cnr_d1.CNR_CNR_MED11.bits.CNR_SPK_SLK_LINK) ||
	    (metai->prot.cnr_d1.CNR_CCR_CON.bits.CNR_CCR_SLK_LINK)) {
		// patch output data 2 metai (CNR)
		metai->prot.cnr_d1.CNR_SLK_CEN.bits.CNR_SLK_CENTR_X = slkP.CENTR_X;
		metai->prot.cnr_d1.CNR_SLK_CEN.bits.CNR_SLK_CENTR_Y = slkP.CENTR_Y;
		metai->prot.cnr_d1.CNR_SLK_RR_CON0.bits.CNR_SLK_R_0 = slkP.R_0;
		metai->prot.cnr_d1.CNR_SLK_RR_CON0.bits.CNR_SLK_R_1 = slkP.R_1;
		metai->prot.cnr_d1.CNR_SLK_RR_CON1.bits.CNR_SLK_R_2 = slkP.R_2;
		//
		metai->prot.cnr_d1.CNR_SLK_RR_CON1.bits.CNR_SLK_GAIN_0 = slkP.GAIN_0;
		metai->prot.cnr_d1.CNR_SLK_RR_CON1.bits.CNR_SLK_GAIN_1 = slkP.GAIN_1;
		metai->prot.cnr_d1.CNR_SLK_GAIN.bits.CNR_SLK_GAIN_2 = slkP.GAIN_2;
		metai->prot.cnr_d1.CNR_SLK_GAIN.bits.CNR_SLK_GAIN_3 = slkP.GAIN_3;
		metai->prot.cnr_d1.CNR_SLK_GAIN.bits.CNR_SLK_GAIN_4 = slkP.GAIN_4;
		metai->prot.cnr_d1.CNR_SLK_GAIN.bits.CNR_SLK_SET_ZERO = slkP.SET_ZERO;
		//
		metai->prot.cnr_d1.CNR_SLK_CROPX.bits.CNR_SLK_CROP_X = slkP.CROP_X;
		metai->prot.cnr_d1.CNR_SLK_CROPY.bits.CNR_SLK_CROP_Y = slkP.CROP_Y;
		//
		metai->prot.cnr_d1.CNR_SLK_HRZ.bits.CNR_SLK_HRZ_COMP = slkP.HRZ_COMP;
		metai->prot.cnr_d1.CNR_SLK_VRZ.bits.CNR_SLK_VRZ_COMP = slkP.VRZ_COMP;
		metai->prot.cnr_d1.CNR_SLK_SLP_CON0.bits.CNR_SLK_SLP_1 = slkP.SLP_1;
		metai->prot.cnr_d1.CNR_SLK_SLP_CON1.bits.CNR_SLK_SLP_2 = slkP.SLP_2;
		metai->prot.cnr_d1.CNR_SLK_SLP_CON2.bits.CNR_SLK_SLP_3 = slkP.SLP_3;
		metai->prot.cnr_d1.CNR_SLK_SLP_CON3.bits.CNR_SLK_SLP_4 = slkP.SLP_4;
	} else {
		// reset to default value if no need slk_link
		metai->prot.cnr_d1.CNR_SLK_HRZ.bits.CNR_SLK_HRZ_COMP = 2048;
		metai->prot.cnr_d1.CNR_SLK_VRZ.bits.CNR_SLK_VRZ_COMP = 2048;
	}
}

void cfg_tnc(struct mtk_img_uapi_meta_raw_stats_cfg *metai,
	     struct dltb_t *dl_table,
	     struct MwCtrlParams *ctrli)
{
	(void)ctrli;
	struct FwTncParam fwtnc;
	struct TncParam tncP;
	// prepare input data for ltm
	fwtnc.img_w = dl_table->src_wd;
	fwtnc.img_h = dl_table->src_ht;
	fwtnc.block_num_x = metai->prot.tnc_d1.TNC_BCE_BLK_NUM.bits.TNC_BCE_BLK_X_NUM;
	fwtnc.block_num_y = metai->prot.tnc_d1.TNC_BCE_BLK_NUM.bits.TNC_BCE_BLK_Y_NUM;
	/*fwtnc.block_width = metai->prot.tnc_d1.TNC_BCE_BLK_SIZE.bits.TNC_BCE_BLK_WD;
  fwtnc.block_height =
  metai->prot.tnc_d1.TNC_BCE_BLK_SIZE.bits.TNC_BCE_BLK_HT;*/
	// do
	fwTncCore(fwtnc, tncP);
	// patch output data 2 metai
	/*metai->prot.tnc_d1.TNC_BCE_BLK_NUM.bits.TNC_BCE_BLK_X_NUM =
  tncP.block_num_x;
  metai->prot.tnc_d1.TNC_BCE_BLK_NUM.bits.TNC_BCE_BLK_Y_NUM =
  tncP.block_num_y;*/
	metai->prot.tnc_d1.TNC_BCE_BLK_SIZE.bits.TNC_BCE_BLK_WD = tncP.bce_blk_sz_x;
	metai->prot.tnc_d1.TNC_BCE_BLK_SIZE.bits.TNC_BCE_BLK_HT = tncP.bce_blk_sz_y;
	metai->prot.tnc_d1.TNC_BCE_BLK_ALPHA.bits.TNC_BCE_X_ALPHA_BASE =
		tncP.bce_blk_divx_apha_base;
	metai->prot.tnc_d1.TNC_BCE_BLK_ALPHA.bits.TNC_BCE_X_ALPHA_SHIFT_BIT =
		tncP.bce_blk_divx_apha_base_shift_bit;
	metai->prot.tnc_d1.TNC_BCE_BLK_ALPHA.bits.TNC_BCE_Y_ALPHA_BASE =
		tncP.bce_blk_divy_apha_base;
	metai->prot.tnc_d1.TNC_BCE_BLK_ALPHA.bits.TNC_BCE_Y_ALPHA_SHIFT_BIT =
		tncP.bce_blk_divy_apha_base_shift_bit;
}

void cfg_tdshp(struct mtk_img_uapi_meta_raw_stats_cfg *metai,
	       struct dltb_t *dl_table, int pqdip_id,
	       struct MwCtrlParams *ctrli, int moduleRegDump)
{
	struct FwSlkParam fwslk;
	struct SlkParam slkP;
	int pqdip_num = 0;
	std::memset(&fwslk, 0, sizeof(struct FwSlkParam));
	std::memset(&slkP, 0, sizeof(struct SlkParam));

	pqdip_num = (pqdip_id == HW_PQDIP_A) ? 0 : 1;

	fwslk.CENTR_X = metai->prot.slk_param.center_x;
	fwslk.CENTR_Y = metai->prot.slk_param.center_y;
	fwslk.R_0 = metai->prot.slk_param.radius_0;
	fwslk.R_1 = metai->prot.slk_param.radius_1;
	fwslk.R_2 = metai->prot.slk_param.radius_2;
	fwslk.GAIN_0 = metai->prot.slk_param.gain0;
	fwslk.GAIN_1 = metai->prot.slk_param.gain1;
	fwslk.GAIN_2 = metai->prot.slk_param.gain2;
	fwslk.GAIN_3 = metai->prot.slk_param.gain3;
	fwslk.GAIN_4 = metai->prot.slk_param.gain4;
	fwslk.IN_WD = metai->sensor_param.tg_width;
	fwslk.IN_HT = metai->sensor_param.tg_height;
	fwslk.CROP_EN = metai->prot.drzs8t_crop_param.is_enable;
	fwslk.CROP_WD = metai->prot.drzs8t_crop_param.input_width;
	fwslk.CROP_HT = metai->prot.drzs8t_crop_param.input_height;
	fwslk.CROP_X = metai->prot.drzs8t_crop_param.start_ofst_x;
	fwslk.CROP_Y = metai->prot.drzs8t_crop_param.start_ofst_y;
	fwslk.CRT_EN = metai->prot.wraping_param.CRT_EN;
	fwslk.OUT_WD = ctrli->pqdip_ctrl_slk[pqdip_num].PQ_OUT_WD;
	fwslk.OUT_HT = ctrli->pqdip_ctrl_slk[pqdip_num].PQ_OUT_HT;
	fwslk.CRT_IN_WD = metai->prot.wraping_param.CRT_IN_WD;
	fwslk.CRT_IN_HT = metai->prot.wraping_param.CRT_IN_HT;
	fwslk.CRT_CROP_WD = metai->prot.wraping_param.CRT_CROP_WD;
	fwslk.CRT_CROP_HT = metai->prot.wraping_param.CRT_CROP_HT;
	fwslk.CRT_POINT_X0 = metai->prot.wraping_param.CRT_POINT_X0;
	fwslk.CRT_POINT_Y0 = metai->prot.wraping_param.CRT_POINT_Y0;
	fwslk.CRT_POINT_X1 = metai->prot.wraping_param.CRT_POINT_X1;
	fwslk.CRT_POINT_Y1 = metai->prot.wraping_param.CRT_POINT_Y1;
	fwslk.CRT_POINT_X2 = metai->prot.wraping_param.CRT_POINT_X2;
	fwslk.CRT_POINT_Y2 = metai->prot.wraping_param.CRT_POINT_Y2;
	fwslk.CRT_POINT_X3 = metai->prot.wraping_param.CRT_POINT_X3;
	fwslk.CRT_POINT_Y3 = metai->prot.wraping_param.CRT_POINT_Y3;
	fwslk.PQ_CROP_EN = ctrli->pqdip_ctrl_slk[pqdip_num].PQ_CROP_EN;
	fwslk.PQ_IN_WD = dl_table->src_wd;
	fwslk.PQ_IN_HT = dl_table->src_ht;
	fwslk.PQ_CROP_WD = ctrli->pqdip_ctrl_slk[pqdip_num].PQ_CROP_WD;
	fwslk.PQ_CROP_HT = ctrli->pqdip_ctrl_slk[pqdip_num].PQ_CROP_HT;
	fwslk.PQ_CROP_X = ctrli->pqdip_ctrl_slk[pqdip_num].PQ_CROP_X;
	fwslk.PQ_CROP_Y = ctrli->pqdip_ctrl_slk[pqdip_num].PQ_CROP_Y;
	// do slk algo core
	fwSlkCore(fwslk, slkP);
	// patch output data 2 metai
	// pqdip_a
	if (pqdip_id == HW_PQDIP_A) {
		if (moduleRegDump)
			dump_slk_input(fwslk, "TDSHAP_A_SLK");
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_CEN.bits.TDSHP_SLK_CENTR_X =
			slkP.CENTR_X;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_CEN.bits.TDSHP_SLK_CENTR_Y =
			slkP.CENTR_Y;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_RR_CON0.bits.TDSHP_SLK_R_0 = slkP.R_0;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_RR_CON0.bits.TDSHP_SLK_R_1 = slkP.R_1;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_R_2 = slkP.R_2;
		//
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_GAIN_0 =
			slkP.GAIN_0;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_GAIN_1 =
			slkP.GAIN_1;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_2 =
			slkP.GAIN_2;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_3 =
			slkP.GAIN_3;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_4 =
			slkP.GAIN_4;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_SET_ZERO =
			slkP.SET_ZERO;
		//
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_CROPX.bits.TDSHP_SLK_CROP_X =
			slkP.CROP_X;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_CROPY.bits.TDSHP_SLK_CROP_Y =
			slkP.CROP_Y;
		//
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_HRZ.bits.TDSHP_SLK_HRZ_COMP =
			slkP.HRZ_COMP;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_VRZ.bits.TDSHP_SLK_VRZ_COMP =
			slkP.VRZ_COMP;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_SLP_CON0.bits.TDSHP_SLK_SLP_1 =
			slkP.SLP_1;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_SLP_CON1.bits.TDSHP_SLK_SLP_2 =
			slkP.SLP_2;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_SLP_CON2.bits.TDSHP_SLK_SLP_3 =
			slkP.SLP_3;
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_SLP_CON3.bits.TDSHP_SLK_SLP_4 =
			slkP.SLP_4;
	}
	// pqdip_b
	if (pqdip_id == HW_PQDIP_B) {
		if (moduleRegDump)
			dump_slk_input(fwslk, "TDSHAP_B_SLK");
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_CEN.bits.TDSHP_SLK_CENTR_X =
			slkP.CENTR_X;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_CEN.bits.TDSHP_SLK_CENTR_Y =
			slkP.CENTR_Y;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_RR_CON0.bits.TDSHP_SLK_R_0 = slkP.R_0;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_RR_CON0.bits.TDSHP_SLK_R_1 = slkP.R_1;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_R_2 = slkP.R_2;
		//
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_GAIN_0 =
			slkP.GAIN_0;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_GAIN_1 =
			slkP.GAIN_1;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_2 =
			slkP.GAIN_2;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_3 =
			slkP.GAIN_3;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_4 =
			slkP.GAIN_4;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_SET_ZERO =
			slkP.SET_ZERO;
		//
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_CROPX.bits.TDSHP_SLK_CROP_X =
			slkP.CROP_X;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_CROPY.bits.TDSHP_SLK_CROP_Y =
			slkP.CROP_Y;
		//
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_HRZ.bits.TDSHP_SLK_HRZ_COMP =
			slkP.HRZ_COMP;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_VRZ.bits.TDSHP_SLK_VRZ_COMP =
			slkP.VRZ_COMP;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_SLP_CON0.bits.TDSHP_SLK_SLP_1 =
			slkP.SLP_1;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_SLP_CON1.bits.TDSHP_SLK_SLP_2 =
			slkP.SLP_2;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_SLP_CON2.bits.TDSHP_SLK_SLP_3 =
			slkP.SLP_3;
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_SLP_CON3.bits.TDSHP_SLK_SLP_4 =
			slkP.SLP_4;
	}
}

/************************************************
 * dbg dump reg
 *************************************************/
void dump_lsc_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_lsc dump_reg:\n"
		"LSC_SDBLK_YOFST: %d\n"
		"LSC_SDBLK_XOFST: %d\n"
		"LSC_SDBLK_XNUM: %d\n"
		"LSC_SDBLK_WIDTH: %d\n"
		"LSC_SDBLK_YNUM: %d\n"
		"LSC_SDBLK_HEIGHT: %d\n"
		"LSC_SDBLK_FWIDTH: %d\n"
		"LSC_SDBLK_FHEIGHT: %d\n"
		"LSC_SDBLK_lWIDTH: %d\n"
		"LSC_SDBLK_lHEIGHT: %d\n",
		metai->prot.lsc_t1.LSC_CTL1.bits.LSC_SDBLK_YOFST,
		metai->prot.lsc_t1.LSC_CTL1.bits.LSC_SDBLK_XOFST,
		metai->prot.lsc_t1.LSC_CTL2.bits.LSC_SDBLK_XNUM,
		metai->prot.lsc_t1.LSC_CTL2.bits.LSC_SDBLK_WIDTH,
		metai->prot.lsc_t1.LSC_CTL3.bits.LSC_SDBLK_YNUM,
		metai->prot.lsc_t1.LSC_CTL3.bits.LSC_SDBLK_HEIGHT,
		metai->prot.lsc_t1.LSC_FBLOCK.bits.LSC_SDBLK_FWIDTH,
		metai->prot.lsc_t1.LSC_FBLOCK.bits.LSC_SDBLK_FHEIGHT,
		metai->prot.lsc_t1.LSC_LBLOCK.bits.LSC_SDBLK_lWIDTH,
		metai->prot.lsc_t1.LSC_LBLOCK.bits.LSC_SDBLK_lHEIGHT);
}

void dump_ltm_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_ltm dump_reg:\n"
		"[I]LTM_BLK_X_NUM: %d\n"
		"[I]LTM_BLK_Y_NUM: %d\n"
		"LTM_BLK_WIDTH: %d\n"
		"LTM_BLK_HEIGHT: %d\n"
		"LTM_X_ALPHA_BASE: %d\n"
		"LTM_X_ALPHA_BASE_SHIFT_BIT: %d\n"
		"LTM_Y_ALPHA_BASE: %d\n"
		"LTM_Y_ALPHA_BASE_SHIFT_BIT: %d\n",
		metai->prot.ltm_t1.LTM_BLK_NUM.bits.LTM_BLK_X_NUM,
		metai->prot.ltm_t1.LTM_BLK_NUM.bits.LTM_BLK_Y_NUM,
		metai->prot.ltm_t1.LTM_BLK_SZ.bits.LTM_BLK_WIDTH,
		metai->prot.ltm_t1.LTM_BLK_SZ.bits.LTM_BLK_HEIGHT,
		metai->prot.ltm_t1.LTM_BLK_DIVX.bits.LTM_X_ALPHA_BASE,
		metai->prot.ltm_t1.LTM_BLK_DIVX.bits.LTM_X_ALPHA_BASE_SHIFT_BIT,
		metai->prot.ltm_t1.LTM_BLK_DIVY.bits.LTM_Y_ALPHA_BASE,
		metai->prot.ltm_t1.LTM_BLK_DIVY.bits.LTM_Y_ALPHA_BASE_SHIFT_BIT);
}

void dump_me_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_me dump_reg:\n"
		"ME_SLK_CENTR_X:%d\n"
		"ME_SLK_CENTR_Y:%d\n"
		"ME_SLK_R_0:%d\n"
		"ME_SLK_R_1:%d\n"
		"ME_SLK_R_2:%d\n"
		"ME_SLK_GAIN_0:%d\n"
		"ME_SLK_GAIN_1:%d\n"
		"ME_SLK_GAIN_2:%d\n"
		"ME_SLK_GAIN_3:%d\n"
		"ME_SLK_GAIN_4:%d\n"
		"ME_SLK_SET_ZERO:%d\n"
		"ME_SLK_HRZ_COMP:%d\n"
		"ME_SLK_VRZ_COMP:%d\n"
		"ME_SLK_SLP_1:%d\n"
		"ME_SLK_SLP_2:%d\n"
		"ME_SLK_SLP_3:%d\n"
		"ME_SLK_SLP_4:%d\n"
		"ME_SLK_CROP_X:%d\n"
		"ME_SLK_CROP_Y:%d\n",
		metai->prot.me_e1.ME_SLK_CEN.bits.ME_SLK_CENTR_X,
		metai->prot.me_e1.ME_SLK_CEN.bits.ME_SLK_CENTR_Y,
		metai->prot.me_e1.ME_SLK_RR_CON0.bits.ME_SLK_R_0,
		metai->prot.me_e1.ME_SLK_RR_CON0.bits.ME_SLK_R_1,
		metai->prot.me_e1.ME_SLK_RR_CON1.bits.ME_SLK_R_2,
		metai->prot.me_e1.ME_SLK_RR_CON1.bits.ME_SLK_GAIN_0,
		metai->prot.me_e1.ME_SLK_RR_CON1.bits.ME_SLK_GAIN_1,
		metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_GAIN_2,
		metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_GAIN_3,
		metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_GAIN_4,
		metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_SET_ZERO,
		metai->prot.me_e1.ME_SLK_HRZ.bits.ME_SLK_HRZ_COMP,
		metai->prot.me_e1.ME_SLK_VRZ.bits.ME_SLK_VRZ_COMP,
		metai->prot.me_e1.ME_SLK_SLP_CON0.bits.ME_SLK_SLP_1,
		metai->prot.me_e1.ME_SLK_SLP_CON1.bits.ME_SLK_SLP_2,
		metai->prot.me_e1.ME_SLK_SLP_CON2.bits.ME_SLK_SLP_3,
		metai->prot.me_e1.ME_SLK_SLP_CON3.bits.ME_SLK_SLP_4,
		metai->prot.me_e1.ME_SLK_CROPX.bits.ME_SLK_CROP_X,
		metai->prot.me_e1.ME_SLK_CROPY.bits.ME_SLK_CROP_Y);
}

void cat_dump_me_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai,
		     struct MwCtrlParams *ctrli)
{
	(void)metai;
	(void)ctrli;
	CAT_LOGD(
		"[CAT][MCNR] ME_REQID:%d.%ld "
		"ME_SLK_CENTR_X:%d "
		"ME_SLK_CENTR_Y:%d "
		"ME_SLK_R_0:%d "
		"ME_SLK_R_1:%d "
		"ME_SLK_R_2:%d "
		"ME_SLK_GAIN_0:%d "
		"ME_SLK_GAIN_1:%d "
		"ME_SLK_GAIN_2:%d "
		"ME_SLK_GAIN_3:%d "
		"ME_SLK_GAIN_4:%d "
		"ME_SLK_SET_ZERO:%d "
		"ME_SLK_HRZ_COMP:%d "
		"ME_SLK_VRZ_COMP:%d "
		"ME_SLK_SLP_1:%d "
		"ME_SLK_SLP_2:%d "
		"ME_SLK_SLP_3:%d "
		"ME_SLK_SLP_4:%d "
		"ME_SLK_CROP_X:%d "
		"ME_SLK_CROP_Y:%d ",
		ctrli->mRequestNo, ctrli->frm_owner,
		metai->prot.me_e1.ME_SLK_CEN.bits.ME_SLK_CENTR_X,
		metai->prot.me_e1.ME_SLK_CEN.bits.ME_SLK_CENTR_Y,
		metai->prot.me_e1.ME_SLK_RR_CON0.bits.ME_SLK_R_0,
		metai->prot.me_e1.ME_SLK_RR_CON0.bits.ME_SLK_R_1,
		metai->prot.me_e1.ME_SLK_RR_CON1.bits.ME_SLK_R_2,
		metai->prot.me_e1.ME_SLK_RR_CON1.bits.ME_SLK_GAIN_0,
		metai->prot.me_e1.ME_SLK_RR_CON1.bits.ME_SLK_GAIN_1,
		metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_GAIN_2,
		metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_GAIN_3,
		metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_GAIN_4,
		metai->prot.me_e1.ME_SLK_GAIN.bits.ME_SLK_SET_ZERO,
		metai->prot.me_e1.ME_SLK_HRZ.bits.ME_SLK_HRZ_COMP,
		metai->prot.me_e1.ME_SLK_VRZ.bits.ME_SLK_VRZ_COMP,
		metai->prot.me_e1.ME_SLK_SLP_CON0.bits.ME_SLK_SLP_1,
		metai->prot.me_e1.ME_SLK_SLP_CON1.bits.ME_SLK_SLP_2,
		metai->prot.me_e1.ME_SLK_SLP_CON2.bits.ME_SLK_SLP_3,
		metai->prot.me_e1.ME_SLK_SLP_CON3.bits.ME_SLK_SLP_4,
		metai->prot.me_e1.ME_SLK_CROPX.bits.ME_SLK_CROP_X,
		metai->prot.me_e1.ME_SLK_CROPY.bits.ME_SLK_CROP_Y);
}

void dump_dm_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_dm dump_reg:\n"
		"DM_SLK_CENTR_X:%d\n"
		"DM_SLK_CENTR_Y:%d\n"
		"DM_SLK_R_0:%d\n"
		"DM_SLK_R_1:%d\n"
		"DM_SLK_R_2:%d\n"
		"DM_SLK_GAIN_0:%d\n"
		"DM_SLK_GAIN_1:%d\n"
		"DM_SLK_GAIN_2:%d\n"
		"DM_SLK_GAIN_3:%d\n"
		"DM_SLK_GAIN_4:%d\n"
		"DM_SLK_SET_ZERO:%d\n"
		"DM_SLK_HRZ_COMP:%d\n"
		"DM_SLK_VRZ_COMP:%d\n"
		"DM_SLK_SLP_1:%d\n"
		"DM_SLK_SLP_2:%d\n"
		"DM_SLK_SLP_3:%d\n"
		"DM_SLK_SLP_4:%d\n"
		"DM_SLK_CROP_X:%d\n"
		"DM_SLK_CROP_Y:%d\n",
		metai->prot.dm_t1.DM_SLK_CEN.bits.DM_SLK_CENTR_X,
		metai->prot.dm_t1.DM_SLK_CEN.bits.DM_SLK_CENTR_Y,
		metai->prot.dm_t1.DM_SLK_RR_CON0.bits.DM_SLK_R_0,
		metai->prot.dm_t1.DM_SLK_RR_CON0.bits.DM_SLK_R_1,
		metai->prot.dm_t1.DM_SLK_RR_CON1.bits.DM_SLK_R_2,
		metai->prot.dm_t1.DM_SLK_RR_CON1.bits.DM_SLK_GAIN_0,
		metai->prot.dm_t1.DM_SLK_RR_CON1.bits.DM_SLK_GAIN_1,
		metai->prot.dm_t1.DM_SLK_GAIN.bits.DM_SLK_GAIN_2,
		metai->prot.dm_t1.DM_SLK_GAIN.bits.DM_SLK_GAIN_3,
		metai->prot.dm_t1.DM_SLK_GAIN.bits.DM_SLK_GAIN_4,
		metai->prot.dm_t1.DM_SLK_GAIN.bits.DM_SLK_SET_ZERO,
		metai->prot.dm_t1.DM_SLK_HRZ.bits.DM_SLK_HRZ_COMP,
		metai->prot.dm_t1.DM_SLK_VRZ.bits.DM_SLK_VRZ_COMP,
		metai->prot.dm_t1.DM_SLK_SLP_CON0.bits.DM_SLK_SLP_1,
		metai->prot.dm_t1.DM_SLK_SLP_CON1.bits.DM_SLK_SLP_2,
		metai->prot.dm_t1.DM_SLK_SLP_CON2.bits.DM_SLK_SLP_3,
		metai->prot.dm_t1.DM_SLK_SLP_CON3.bits.DM_SLK_SLP_4,
		metai->prot.dm_t1.DM_SLK_CROPX.bits.DM_SLK_CROP_X,
		metai->prot.dm_t1.DM_SLK_CROPY.bits.DM_SLK_CROP_Y);
}

void dump_snrs_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_snrs dump_reg:\n"
		"SNRS_SLK_CENTR_X:%d\n"
		"SNRS_SLK_CENTR_Y:%d\n"
		"SNRS_SLK_R_0:%d\n"
		"SNRS_SLK_R_1:%d\n"
		"SNRS_SLK_R_2:%d\n"
		"SNRS_SLK_GAIN_0:%d\n"
		"SNRS_SLK_GAIN_1:%d\n"
		"SNRS_SLK_GAIN_2:%d\n"
		"SNRS_SLK_GAIN_3:%d\n"
		"SNRS_SLK_GAIN_4:%d\n"
		"SNRS_SLK_SET_ZERO:%d\n"
		"SNRS_SLK_HRZ_COMP:%d\n"
		"SNRS_SLK_VRZ_COMP:%d\n"
		"SNRS_SLK_SLP_1:%d\n"
		"SNRS_SLK_SLP_2:%d\n"
		"SNRS_SLK_SLP_3:%d\n"
		"SNRS_SLK_SLP_4:%d\n"
		"SNRS_SLK_CROP_X:%d\n"
		"SNRS_SLK_CROP_Y:%d\n",
		metai->prot.snrs_d1.SNRS_SLK_CEN.bits.SNRS_SLK_CENTR_X,
		metai->prot.snrs_d1.SNRS_SLK_CEN.bits.SNRS_SLK_CENTR_Y,
		metai->prot.snrs_d1.SNRS_SLK_RR_CON0.bits.SNRS_SLK_R_0,
		metai->prot.snrs_d1.SNRS_SLK_RR_CON0.bits.SNRS_SLK_R_1,
		metai->prot.snrs_d1.SNRS_SLK_RR_CON1.bits.SNRS_SLK_R_2,
		metai->prot.snrs_d1.SNRS_SLK_RR_CON1.bits.SNRS_SLK_GAIN_0,
		metai->prot.snrs_d1.SNRS_SLK_RR_CON1.bits.SNRS_SLK_GAIN_1,
		metai->prot.snrs_d1.SNRS_SLK_GAIN.bits.SNRS_SLK_GAIN_2,
		metai->prot.snrs_d1.SNRS_SLK_GAIN.bits.SNRS_SLK_GAIN_3,
		metai->prot.snrs_d1.SNRS_SLK_GAIN.bits.SNRS_SLK_GAIN_4,
		metai->prot.snrs_d1.SNRS_SLK_GAIN.bits.SNRS_SLK_SET_ZERO,
		metai->prot.snrs_d1.SNRS_SLK_HRZ.bits.SNRS_SLK_HRZ_COMP,
		metai->prot.snrs_d1.SNRS_SLK_VRZ.bits.SNRS_SLK_VRZ_COMP,
		metai->prot.snrs_d1.SNRS_SLK_SLP_CON0.bits.SNRS_SLK_SLP_1,
		metai->prot.snrs_d1.SNRS_SLK_SLP_CON1.bits.SNRS_SLK_SLP_2,
		metai->prot.snrs_d1.SNRS_SLK_SLP_CON2.bits.SNRS_SLK_SLP_3,
		metai->prot.snrs_d1.SNRS_SLK_SLP_CON3.bits.SNRS_SLK_SLP_4,
		metai->prot.snrs_d1.SNRS_SLK_CROPX.bits.SNRS_SLK_CROP_X,
		metai->prot.snrs_d1.SNRS_SLK_CROPY.bits.SNRS_SLK_CROP_Y);
}

void dump_snr_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_snr dump_reg:\n"
		"SNR_SLK_CENTR_X:%d\n"
		"SNR_SLK_CENTR_Y:%d\n"
		"SNR_SLK_R_0:%d\n"
		"SNR_SLK_R_1:%d\n"
		"SNR_SLK_R_2:%d\n"
		"SNR_SLK_GAIN_0:%d\n"
		"SNR_SLK_GAIN_1:%d\n"
		"SNR_SLK_GAIN_2:%d\n"
		"SNR_SLK_GAIN_3:%d\n"
		"SNR_SLK_GAIN_4:%d\n"
		"SNR_SLK_SET_ZERO:%d\n"
		"SNR_SLK_HRZ_COMP:%d\n"
		"SNR_SLK_VRZ_COMP:%d\n"
		"SNR_SLK_SLP_1:%d\n"
		"SNR_SLK_SLP_2:%d\n"
		"SNR_SLK_SLP_3:%d\n"
		"SNR_SLK_SLP_4:%d\n"
		"SNR_SLK_CROP_X:%d\n"
		"SNR_SLK_CROP_Y:%d\n",
		metai->prot.snr_d1.SNR_SLK_CEN.bits.SNR_SLK_CENTR_X,
		metai->prot.snr_d1.SNR_SLK_CEN.bits.SNR_SLK_CENTR_Y,
		metai->prot.snr_d1.SNR_SLK_RR_CON0.bits.SNR_SLK_R_0,
		metai->prot.snr_d1.SNR_SLK_RR_CON0.bits.SNR_SLK_R_1,
		metai->prot.snr_d1.SNR_SLK_RR_CON1.bits.SNR_SLK_R_2,
		metai->prot.snr_d1.SNR_SLK_RR_CON1.bits.SNR_SLK_GAIN_0,
		metai->prot.snr_d1.SNR_SLK_RR_CON1.bits.SNR_SLK_GAIN_1,
		metai->prot.snr_d1.SNR_SLK_GAIN.bits.SNR_SLK_GAIN_2,
		metai->prot.snr_d1.SNR_SLK_GAIN.bits.SNR_SLK_GAIN_3,
		metai->prot.snr_d1.SNR_SLK_GAIN.bits.SNR_SLK_GAIN_4,
		metai->prot.snr_d1.SNR_SLK_GAIN.bits.SNR_SLK_SET_ZERO,
		metai->prot.snr_d1.SNR_SLK_HRZ.bits.SNR_SLK_HRZ_COMP,
		metai->prot.snr_d1.SNR_SLK_VRZ.bits.SNR_SLK_VRZ_COMP,
		metai->prot.snr_d1.SNR_SLK_SLP_CON0.bits.SNR_SLK_SLP_1,
		metai->prot.snr_d1.SNR_SLK_SLP_CON1.bits.SNR_SLK_SLP_2,
		metai->prot.snr_d1.SNR_SLK_SLP_CON2.bits.SNR_SLK_SLP_3,
		metai->prot.snr_d1.SNR_SLK_SLP_CON3.bits.SNR_SLK_SLP_4,
		metai->prot.snr_d1.SNR_SLK_CROPX.bits.SNR_SLK_CROP_X,
		metai->prot.snr_d1.SNR_SLK_CROPY.bits.SNR_SLK_CROP_Y);
}

void dump_ee_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_ee dump_reg:\n"
		"EE_SLK_CENTR_X:%d\n"
		"EE_SLK_CENTR_Y:%d\n"
		"EE_SLK_R_0:%d\n"
		"EE_SLK_R_1:%d\n"
		"EE_SLK_R_2:%d\n"
		"EE_SLK_GAIN_0:%d\n"
		"EE_SLK_GAIN_1:%d\n"
		"EE_SLK_GAIN_2:%d\n"
		"EE_SLK_GAIN_3:%d\n"
		"EE_SLK_GAIN_4:%d\n"
		"EE_SLK_SET_ZERO:%d\n"
		"EE_SLK_HRZ_COMP:%d\n"
		"EE_SLK_VRZ_COMP:%d\n"
		"EE_SLK_SLP_1:%d\n"
		"EE_SLK_SLP_2:%d\n"
		"EE_SLK_SLP_3:%d\n"
		"EE_SLK_SLP_4:%d\n"
		"EE_SLK_CROP_X:%d\n"
		"EE_SLK_CROP_Y:%d\n",
		metai->prot.ee_d1.EE_SLK_CEN.bits.EE_SLK_CENTR_X,
		metai->prot.ee_d1.EE_SLK_CEN.bits.EE_SLK_CENTR_Y,
		metai->prot.ee_d1.EE_SLK_RR_CON0.bits.EE_SLK_R_0,
		metai->prot.ee_d1.EE_SLK_RR_CON0.bits.EE_SLK_R_1,
		metai->prot.ee_d1.EE_SLK_RR_CON1.bits.EE_SLK_R_2,
		metai->prot.ee_d1.EE_SLK_RR_CON1.bits.EE_SLK_GAIN_0,
		metai->prot.ee_d1.EE_SLK_RR_CON1.bits.EE_SLK_GAIN_1,
		metai->prot.ee_d1.EE_SLK_GAIN.bits.EE_SLK_GAIN_2,
		metai->prot.ee_d1.EE_SLK_GAIN.bits.EE_SLK_GAIN_3,
		metai->prot.ee_d1.EE_SLK_GAIN.bits.EE_SLK_GAIN_4,
		metai->prot.ee_d1.EE_SLK_GAIN.bits.EE_SLK_SET_ZERO,
		metai->prot.ee_d1.EE_SLK_HRZ.bits.EE_SLK_HRZ_COMP,
		metai->prot.ee_d1.EE_SLK_VRZ.bits.EE_SLK_VRZ_COMP,
		metai->prot.ee_d1.EE_SLK_SLP_CON0.bits.EE_SLK_SLP_1,
		metai->prot.ee_d1.EE_SLK_SLP_CON1.bits.EE_SLK_SLP_2,
		metai->prot.ee_d1.EE_SLK_SLP_CON2.bits.EE_SLK_SLP_3,
		metai->prot.ee_d1.EE_SLK_SLP_CON3.bits.EE_SLK_SLP_4,
		metai->prot.ee_d1.EE_SLK_CROPX.bits.EE_SLK_CROP_X,
		metai->prot.ee_d1.EE_SLK_CROPY.bits.EE_SLK_CROP_Y);
}

void dump_cnr_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_cnr dump_reg:\n"
		"CNR_SLK_CENTR_X:%d\n"
		"CNR_SLK_CENTR_Y:%d\n"
		"CNR_SLK_R_0:%d\n"
		"CNR_SLK_R_1:%d\n"
		"CNR_SLK_R_2:%d\n"
		"CNR_SLK_GAIN_0:%d\n"
		"CNR_SLK_GAIN_1:%d\n"
		"CNR_SLK_GAIN_2:%d\n"
		"CNR_SLK_GAIN_3:%d\n"
		"CNR_SLK_GAIN_4:%d\n"
		"CNR_SLK_SET_ZERO:%d\n"
		"CNR_SLK_HRZ_COMP:%d\n"
		"CNR_SLK_VRZ_COMP:%d\n"
		"CNR_SLK_SLP_1:%d\n"
		"CNR_SLK_SLP_2:%d\n"
		"CNR_SLK_SLP_3:%d\n"
		"CNR_SLK_SLP_4:%d\n"
		"CNR_SLK_CROP_X:%d\n"
		"CNR_SLK_CROP_Y:%d\n",
		metai->prot.cnr_d1.CNR_SLK_CEN.bits.CNR_SLK_CENTR_X,
		metai->prot.cnr_d1.CNR_SLK_CEN.bits.CNR_SLK_CENTR_Y,
		metai->prot.cnr_d1.CNR_SLK_RR_CON0.bits.CNR_SLK_R_0,
		metai->prot.cnr_d1.CNR_SLK_RR_CON0.bits.CNR_SLK_R_1,
		metai->prot.cnr_d1.CNR_SLK_RR_CON1.bits.CNR_SLK_R_2,
		metai->prot.cnr_d1.CNR_SLK_RR_CON1.bits.CNR_SLK_GAIN_0,
		metai->prot.cnr_d1.CNR_SLK_RR_CON1.bits.CNR_SLK_GAIN_1,
		metai->prot.cnr_d1.CNR_SLK_GAIN.bits.CNR_SLK_GAIN_2,
		metai->prot.cnr_d1.CNR_SLK_GAIN.bits.CNR_SLK_GAIN_3,
		metai->prot.cnr_d1.CNR_SLK_GAIN.bits.CNR_SLK_GAIN_4,
		metai->prot.cnr_d1.CNR_SLK_GAIN.bits.CNR_SLK_SET_ZERO,
		metai->prot.cnr_d1.CNR_SLK_HRZ.bits.CNR_SLK_HRZ_COMP,
		metai->prot.cnr_d1.CNR_SLK_VRZ.bits.CNR_SLK_VRZ_COMP,
		metai->prot.cnr_d1.CNR_SLK_SLP_CON0.bits.CNR_SLK_SLP_1,
		metai->prot.cnr_d1.CNR_SLK_SLP_CON1.bits.CNR_SLK_SLP_2,
		metai->prot.cnr_d1.CNR_SLK_SLP_CON2.bits.CNR_SLK_SLP_3,
		metai->prot.cnr_d1.CNR_SLK_SLP_CON3.bits.CNR_SLK_SLP_4,
		metai->prot.cnr_d1.CNR_SLK_CROPX.bits.CNR_SLK_CROP_X,
		metai->prot.cnr_d1.CNR_SLK_CROPY.bits.CNR_SLK_CROP_Y);
}

void dump_tnr_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_tnr dump_reg:\n"
		"TNR_SLK_CENTR_X:%d\n"
		"TNR_SLK_CENTR_Y:%d\n"
		"TNR_SLK_R_0:%d\n"
		"TNR_SLK_R_1:%d\n"
		"TNR_SLK_R_2:%d\n"
		"TNR_SLK_GAIN_0:%d\n"
		"TNR_SLK_GAIN_1:%d\n"
		"TNR_SLK_GAIN_2:%d\n"
		"TNR_SLK_GAIN_3:%d\n"
		"TNR_SLK_GAIN_4:%d\n"
		"TNR_SLK_SET_ZERO:%d\n"
		"TNR_SLK_HRZ_COMP:%d\n"
		"TNR_SLK_VRZ_COMP:%d\n"
		"TNR_SLK_SLP_1:%d\n"
		"TNR_SLK_SLP_2:%d\n"
		"TNR_SLK_SLP_3:%d\n"
		"TNR_SLK_SLP_4:%d\n"
		"TNR_SLK_CROP_X:%d\n"
		"TNR_SLK_CROP_Y:%d\n",
		metai->prot.tnr_d1.TNR_SLK_CEN.bits.TNR_SLK_CENTR_X,
		metai->prot.tnr_d1.TNR_SLK_CEN.bits.TNR_SLK_CENTR_Y,
		metai->prot.tnr_d1.TNR_SLK_RR_CON0.bits.TNR_SLK_R_0,
		metai->prot.tnr_d1.TNR_SLK_RR_CON0.bits.TNR_SLK_R_1,
		metai->prot.tnr_d1.TNR_SLK_RR_CON1.bits.TNR_SLK_R_2,
		metai->prot.tnr_d1.TNR_SLK_RR_CON1.bits.TNR_SLK_GAIN_0,
		metai->prot.tnr_d1.TNR_SLK_RR_CON1.bits.TNR_SLK_GAIN_1,
		metai->prot.tnr_d1.TNR_SLK_GAIN.bits.TNR_SLK_GAIN_2,
		metai->prot.tnr_d1.TNR_SLK_GAIN.bits.TNR_SLK_GAIN_3,
		metai->prot.tnr_d1.TNR_SLK_GAIN.bits.TNR_SLK_GAIN_4,
		metai->prot.tnr_d1.TNR_SLK_GAIN.bits.TNR_SLK_SET_ZERO,
		metai->prot.tnr_d1.TNR_SLK_HRZ.bits.TNR_SLK_HRZ_COMP,
		metai->prot.tnr_d1.TNR_SLK_VRZ.bits.TNR_SLK_VRZ_COMP,
		metai->prot.tnr_d1.TNR_SLK_SLP_CON0.bits.TNR_SLK_SLP_1,
		metai->prot.tnr_d1.TNR_SLK_SLP_CON1.bits.TNR_SLK_SLP_2,
		metai->prot.tnr_d1.TNR_SLK_SLP_CON2.bits.TNR_SLK_SLP_3,
		metai->prot.tnr_d1.TNR_SLK_SLP_CON3.bits.TNR_SLK_SLP_4,
		metai->prot.tnr_d1.TNR_SLK_CROPX.bits.TNR_SLK_CROP_X,
		metai->prot.tnr_d1.TNR_SLK_CROPY.bits.TNR_SLK_CROP_Y);
}

void dump_allDipSlk_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	if (metai->prot.snrs_d1.SNRS_CON1.bits.SNRS_SLK_LINK)
		dump_snrs_reg(metai);
	if (metai->prot.tnr_d1.TNR_TEMPORAL.bits.TNR_SLK_EN)
		dump_tnr_reg(metai);
	if (metai->prot.snr_d1.SNR_CON1.bits.SNR_SLK_LINK)
		dump_snr_reg(metai);
	if ((metai->prot.ee_d1.EE_LUMA_SLNK_CTRL.bits.EE_GLUT_LINK_EN) ||
	    (metai->prot.ee_d1.EE_CE_SL_CTRL.bits.EE_CE_SLMOD_EN))
		dump_ee_reg(metai);
	if ((metai->prot.cnr_d1.CNR_CNR_CTRL.bits.CNR_CNR_SLK_LINK) ||
	    (metai->prot.cnr_d1.CNR_CNR_MED11.bits.CNR_SPK_SLK_LINK) ||
	    (metai->prot.cnr_d1.CNR_CCR_CON.bits.CNR_CCR_SLK_LINK))
		dump_cnr_reg(metai);
}

void dump_tnc_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_tnc dump_reg:\n"
		"[I]TNC_BCE_BLK_X_NUM: %d\n"
		"[I]TNC_BCE_BLK_Y_NUM: %d\n"
		"TNC_BCE_BLK_WD: %d\n"
		"TNC_BCE_BLK_HT: %d\n"
		"TNC_BCE_X_ALPHA_BASE: %d\n"
		"TNC_BCE_X_ALPHA_SHIFT_BIT: %d\n"
		"TNC_BCE_Y_ALPHA_BASE: %d\n"
		"TNC_BCE_Y_ALPHA_SHIFT_BIT: %d\n",
		metai->prot.tnc_d1.TNC_BCE_BLK_NUM.bits.TNC_BCE_BLK_X_NUM,
		metai->prot.tnc_d1.TNC_BCE_BLK_NUM.bits.TNC_BCE_BLK_Y_NUM,
		metai->prot.tnc_d1.TNC_BCE_BLK_SIZE.bits.TNC_BCE_BLK_WD,
		metai->prot.tnc_d1.TNC_BCE_BLK_SIZE.bits.TNC_BCE_BLK_HT,
		metai->prot.tnc_d1.TNC_BCE_BLK_ALPHA.bits.TNC_BCE_X_ALPHA_BASE,
		metai->prot.tnc_d1.TNC_BCE_BLK_ALPHA.bits.TNC_BCE_X_ALPHA_SHIFT_BIT,
		metai->prot.tnc_d1.TNC_BCE_BLK_ALPHA.bits.TNC_BCE_Y_ALPHA_BASE,
		metai->prot.tnc_d1.TNC_BCE_BLK_ALPHA.bits.TNC_BCE_Y_ALPHA_SHIFT_BIT);
}

void dump_tdshpA_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_tdshp_p1a dump_reg:\n"
		"TDSHP_SLK_CENTR_X:%d\n"
		"TDSHP_SLK_CENTR_Y:%d\n"
		"TDSHP_SLK_R_0:%d\n"
		"TDSHP_SLK_R_1:%d\n"
		"TDSHP_SLK_R_2:%d\n"
		"TDSHP_SLK_GAIN_0:%d\n"
		"TDSHP_SLK_GAIN_1:%d\n"
		"TDSHP_SLK_GAIN_2:%d\n"
		"TDSHP_SLK_GAIN_3:%d\n"
		"TDSHP_SLK_GAIN_4:%d\n"
		"TDSHP_SLK_SET_ZERO:%d\n"
		"TDSHP_SLK_HRZ_COMP:%d\n"
		"TDSHP_SLK_VRZ_COMP:%d\n"
		"TDSHP_SLK_SLP_1:%d\n"
		"TDSHP_SLK_SLP_2:%d\n"
		"TDSHP_SLK_SLP_3:%d\n"
		"TDSHP_SLK_SLP_4:%d\n"
		"TDSHP_SLK_CROP_X:%d\n"
		"TDSHP_SLK_CROP_Y:%d\n",
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_CEN.bits.TDSHP_SLK_CENTR_X,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_CEN.bits.TDSHP_SLK_CENTR_Y,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_RR_CON0.bits.TDSHP_SLK_R_0,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_RR_CON0.bits.TDSHP_SLK_R_1,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_R_2,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_GAIN_0,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_GAIN_1,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_2,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_3,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_4,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_SET_ZERO,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_HRZ.bits.TDSHP_SLK_HRZ_COMP,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_VRZ.bits.TDSHP_SLK_VRZ_COMP,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_SLP_CON0.bits.TDSHP_SLK_SLP_1,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_SLP_CON1.bits.TDSHP_SLK_SLP_2,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_SLP_CON2.bits.TDSHP_SLK_SLP_3,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_SLP_CON3.bits.TDSHP_SLK_SLP_4,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_CROPX.bits.TDSHP_SLK_CROP_X,
		metai->prot.tdshp_p1a.TDSHP_TDSHP_SLK_CROPY.bits.TDSHP_SLK_CROP_Y);
}

void dump_tdshpB_reg(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"cfg_tdshp_p1b dump_reg:\n"
		"TDSHP_SLK_CENTR_X:%d\n"
		"TDSHP_SLK_CENTR_Y:%d\n"
		"TDSHP_SLK_R_0:%d\n"
		"TDSHP_SLK_R_1:%d\n"
		"TDSHP_SLK_R_2:%d\n"
		"TDSHP_SLK_GAIN_0:%d\n"
		"TDSHP_SLK_GAIN_1:%d\n"
		"TDSHP_SLK_GAIN_2:%d\n"
		"TDSHP_SLK_GAIN_3:%d\n"
		"TDSHP_SLK_GAIN_4:%d\n"
		"TDSHP_SLK_SET_ZERO:%d\n"
		"TDSHP_SLK_VRZ_COMP:%d\n"
		"TNR_SLK_VRZ_COMP:%d\n"
		"TDSHP_SLK_SLP_1:%d\n"
		"TDSHP_SLK_SLP_2:%d\n"
		"TDSHP_SLK_SLP_3:%d\n"
		"TDSHP_SLK_SLP_4:%d\n"
		"TDSHP_SLK_CROP_X:%d\n"
		"TDSHP_SLK_CROP_Y:%d\n",
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_CEN.bits.TDSHP_SLK_CENTR_X,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_CEN.bits.TDSHP_SLK_CENTR_Y,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_RR_CON0.bits.TDSHP_SLK_R_0,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_RR_CON0.bits.TDSHP_SLK_R_1,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_R_2,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_GAIN_0,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_RR_CON1.bits.TDSHP_SLK_GAIN_1,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_2,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_3,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_GAIN_4,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_GAIN.bits.TDSHP_SLK_SET_ZERO,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_HRZ.bits.TDSHP_SLK_HRZ_COMP,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_VRZ.bits.TDSHP_SLK_VRZ_COMP,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_SLP_CON0.bits.TDSHP_SLK_SLP_1,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_SLP_CON1.bits.TDSHP_SLK_SLP_2,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_SLP_CON2.bits.TDSHP_SLK_SLP_3,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_SLP_CON3.bits.TDSHP_SLK_SLP_4,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_CROPX.bits.TDSHP_SLK_CROP_X,
		metai->prot.tdshp_p1b.TDSHP_TDSHP_SLK_CROPY.bits.TDSHP_SLK_CROP_Y);
}

void dump_dlInfo(struct dltb_t *dl_table)
{
	LOG_INF(
		"[K] DL-info:\n"
		"traw-on(%d):   fmt(0x%x),W(%d),H(%d)\n"
		"dip-on(%d):    fmt(0x%x),W(%d),H(%d)\n"
		"pqdipA-on(%d): fmt(0x%x),W(%d),H(%d)\n"
		"pqdipB-on(%d): fmt(0x%x),W(%d),H(%d)\n",
		dl_table[HW_TRAW].on, dl_table[HW_TRAW].src_fmt, dl_table[HW_TRAW].src_wd,
		dl_table[HW_TRAW].src_ht, dl_table[HW_DIP].on, dl_table[HW_DIP].src_fmt,
		dl_table[HW_DIP].src_wd, dl_table[HW_DIP].src_ht, dl_table[HW_PQDIP_A].on,
		dl_table[HW_PQDIP_A].src_fmt, dl_table[HW_PQDIP_A].src_wd,
		dl_table[HW_PQDIP_A].src_ht, dl_table[HW_PQDIP_B].on,
		dl_table[HW_PQDIP_B].src_fmt, dl_table[HW_PQDIP_B].src_wd,
		dl_table[HW_PQDIP_B].src_ht);
}

void dump_ctrlEn(struct mtk_img_uapi_meta_raw_stats_cfg *metai)
{
	LOG_INF(
		"[K]dump_ctrlEn:\n"
		"lsc_t1_enable:%d ,"
		"ltm_t1_enable:%d ,"
		"tnc_d1_enable:%d ,"
		"ME_SLK_EN:%d ,"
		"DM_SL_EN:%d ,"
		"SNRS_SLK_LINK:%d ,"
		"TNR_SLK_EN:%d ,"
		"SNR_SLK_LINK:%d ,"
		"EE_GLUT_LINK_EN:%d ,"
		"EE_CE_SLMOD_EN:%d ,"
		"CNR_CNR_SLK_LINK:%d ,"
		"CNR_SPK_SLK_LINK:%d ,"
		"CNR_CCR_SLK_LINK:%d ,"
		"TDSHP_HFC_SLK_LINK_EN(tdshp_p1a):%d ,"
		"TDSHP_HFC_SLK_LINK_EN(tdshp_p1b):%d \n",
		(metai->prot.lsc_t1_enable), (metai->prot.ltm_t1_enable),
		(metai->prot.tnc_d1_enable), (metai->prot.me_e1.ME_TOP.bits.ME_SLK_EN),
		(metai->prot.dm_t1.DM_SL_CTL.bits.DM_SL_EN),
		(metai->prot.snrs_d1.SNRS_CON1.bits.SNRS_SLK_LINK),
		(metai->prot.tnr_d1.TNR_TEMPORAL.bits.TNR_SLK_EN),
		(metai->prot.snr_d1.SNR_CON1.bits.SNR_SLK_LINK),
		(metai->prot.ee_d1.EE_LUMA_SLNK_CTRL.bits.EE_GLUT_LINK_EN),
		(metai->prot.ee_d1.EE_CE_SL_CTRL.bits.EE_CE_SLMOD_EN),
		(metai->prot.cnr_d1.CNR_CNR_CTRL.bits.CNR_CNR_SLK_LINK),
		(metai->prot.cnr_d1.CNR_CNR_MED11.bits.CNR_SPK_SLK_LINK),
		(metai->prot.cnr_d1.CNR_CCR_CON.bits.CNR_CCR_SLK_LINK),
		(metai->prot.tdshp_p1a.TDSHP_TDSHP_HFC_SLK_0.bits.TDSHP_HFC_SLK_LINK_EN),
		(metai->prot.tdshp_p1b.TDSHP_TDSHP_HFC_SLK_0.bits.TDSHP_HFC_SLK_LINK_EN));
}

void dump_ctrlME(struct MwCtrlParams *ctrli)
{
	LOG_INF(
		"[K]dump_ctrlME:\n"
		"me_ctrl_slk.IN_HT: %d ,"
		"me_ctrl_slk.IN_WD: %d ,"
		"me_on: %d \n",
		ctrli->me_ctrl_slk.IN_HT, ctrli->me_ctrl_slk.IN_WD, ctrli->me_on);
}

void dump_ctrlPQDIP(struct MwCtrlParams *ctrli, int pqdip_id)
{
	int pqdip_num = (pqdip_id == HW_PQDIP_A) ? 0 : 1;
	LOG_INF(
		"[K]dump_ctrlPQDIP:\n"
		"PQ_CROP_EN[%d]: %d ,"
		"PQ_CROP_X: %d ,"
		"PQ_CROP_Y: %d ,"
		"PQ_CROP_WD: %d ,"
		"PQ_CROP_HT: %d ,"
		"PQ_OUT_WD: %d ,"
		"PQ_OUT_HT: %d \n",
		pqdip_num, ctrli->pqdip_ctrl_slk[pqdip_num].PQ_CROP_EN,
		ctrli->pqdip_ctrl_slk[pqdip_num].PQ_CROP_X,
		ctrli->pqdip_ctrl_slk[pqdip_num].PQ_CROP_Y,
		ctrli->pqdip_ctrl_slk[pqdip_num].PQ_CROP_WD,
		ctrli->pqdip_ctrl_slk[pqdip_num].PQ_CROP_HT,
		ctrli->pqdip_ctrl_slk[pqdip_num].PQ_OUT_WD,
		ctrli->pqdip_ctrl_slk[pqdip_num].PQ_OUT_HT);
}

/************************************************
 * tuning_helper main flow
 *************************************************/
void TuningHelper::do_helper(struct mtk_img_uapi_meta_raw_stats_cfg *metai,
			     struct dltb_t dl_table[HW_TDR_MAX],
			     struct MwCtrlParams *ctrli)
{
	// TIME_START(dohelper_st);
	/* moduleRegDump = NSCam::Utils::Properties::property_get_int32(
      "vendor.debug.thmoduleregdump.enable", 0); */
	if (moduleRegDump) {
		dump_dlInfo(dl_table);
		dump_ctrlEn(metai);
	}
	// reassign top control
	if (dbgEnable) {
		(metai->prot.lsc_t1_enable) = (moduleLSCEnable);
		(metai->prot.ltm_t1_enable) = (moduleLTMEnable);
		(metai->prot.tnc_d1_enable) = (moduleTNCEnable);
		(metai->prot.me_e1.ME_TOP.bits.ME_SLK_EN) = (moduleMEEnable);
		(metai->prot.dm_t1.DM_SL_CTL.bits.DM_SL_EN) = (moduleDMEnable);
		(metai->prot.snrs_d1.SNRS_CON1.bits.SNRS_SLK_LINK) = (moduleSNRSEnable);
		(metai->prot.tnr_d1.TNR_TEMPORAL.bits.TNR_SLK_EN) = (moduleTNREnable);
		(metai->prot.snr_d1.SNR_CON1.bits.SNR_SLK_LINK) = (moduleSNREnable);

		{
			(metai->prot.ee_d1.EE_LUMA_SLNK_CTRL.bits.EE_GLUT_LINK_EN) =
				(moduleEEEnable);
			(metai->prot.ee_d1.EE_CE_SL_CTRL.bits.EE_CE_SLMOD_EN) = (moduleEEEnable);
		}

		{
			(metai->prot.cnr_d1.CNR_CNR_CTRL.bits.CNR_CNR_SLK_LINK) =
				(moduleCNREnable);
			(metai->prot.cnr_d1.CNR_CNR_MED11.bits.CNR_SPK_SLK_LINK) =
				(moduleCNREnable);
			(metai->prot.cnr_d1.CNR_CCR_CON.bits.CNR_CCR_SLK_LINK) =
				(moduleCNREnable);
		}

		(metai->prot.tdshp_p1a.TDSHP_TDSHP_HFC_SLK_0.bits.TDSHP_HFC_SLK_LINK_EN) =
			(moduleTDSHAPAEnable);
		(metai->prot.tdshp_p1b.TDSHP_TDSHP_HFC_SLK_0.bits.TDSHP_HFC_SLK_LINK_EN) =
			(moduleTDSHAPBEnable);
		if (moduleRegDump) {
			LOG_INF("after dbgEnable:");
			dump_ctrlEn(metai);
		}
	}
	// for ME_slk
	if (ctrli->me_on) {
		// 1.me register remap in metai
		// 2.metai has me related lsc_tsf info/ rz_info/crt_info
		if (metai->prot.me_e1.ME_TOP.bits.ME_SLK_EN) {
			if (moduleRegDump)
				dump_ctrlME(ctrli);
			// TIME_START(me_st);
			cfg_me(metai, ctrli, moduleRegDump);
			// TIME_END(me_et);
			cat_dump_me_reg(metai, ctrli);
			if (moduleRegDump)
				dump_me_reg(metai);
		}
	}
	// Traw
	if (dl_table[HW_TRAW].on) {
		/* 0.CAC
       1.LSC // TODO: another add config function!!! */
		if (metai->prot.lsc_t1_enable == 1) {
			// TIME_START(lsc_st);
			cfg_lsc(metai, &dl_table[HW_TRAW], ctrli);
			//  TIME_END(lsc_et);
			if (moduleRegDump)
				dump_lsc_reg(metai);
		}
		// 2.LTM
		if (metai->prot.ltm_t1_enable == 1) {
			// TIME_START(ltm_st);
			cfg_ltm(metai, &dl_table[HW_TRAW], ctrli);
			// TIME_END(ltm_et);
			if (moduleRegDump)
				dump_ltm_reg(metai);
		}
		// 3.DM(SLK) always do slk cor no metter enable or not
		if (metai->prot.dm_t1.DM_SL_CTL.bits.DM_SL_EN) {
			/* always do slk cor no metter enable or not */
			// TIME_START(dm_st);
			cfg_dm(metai, &dl_table[HW_TRAW], ctrli, moduleRegDump);
			// TIME_END(dm_et);
			if (moduleRegDump)
				dump_dm_reg(metai);
		}
	}
	// Dip
	if (dl_table[HW_DIP].on) {
		{ /* always do slk cor no metter enable or not */
			// TIME_START(dipallslk_st);
			cfg_allDipSlk(metai, &dl_table[HW_DIP], ctrli, moduleRegDump);
			// TIME_END(dipallslk_et);
			if (moduleRegDump)
				dump_allDipSlk_reg(metai);
		}
		// TNC
		if (metai->prot.tnc_d1_enable == 1) {
			// TIME_START(tnc_st);
			cfg_tnc(metai, &dl_table[HW_DIP], ctrli);
			// TIME_END(tnc_et);
			if (moduleRegDump)
				dump_tnc_reg(metai);
		}
	}
	// PQDip-A
	if (dl_table[HW_PQDIP_A].on) {
		// TDSHP(SLK)
		int tdbEn =
			((metai->prot.tdshp_p1a.TDSHP_TDSHP_HFC_SLK_0.bits
				  .TDSHP_HFC_SLK_LINK_EN) &&
			 (metai->prot.tdshp_p1a.TDSHP_TDSHP_HFG_CTRL.bits.TDSHP_HFG_SLK_EN));
		if (tdbEn) { /* always do slk cor no metter enable or not */
			if (moduleRegDump)
				dump_ctrlPQDIP(ctrli, HW_PQDIP_A);
			// TIME_START(tdshapa_st);
			cfg_tdshp(metai, &dl_table[HW_PQDIP_A], HW_PQDIP_A, ctrli, moduleRegDump);
			// TIME_END(tdshapa_et);
			if (moduleRegDump)
				dump_tdshpA_reg(metai);
		}
	}
	// PQDip-B
	if (dl_table[HW_PQDIP_B].on) {
		// TDSHP(SLK)
		int tdbEn =
			((metai->prot.tdshp_p1b.TDSHP_TDSHP_HFC_SLK_0.bits
				  .TDSHP_HFC_SLK_LINK_EN) &&
			 (metai->prot.tdshp_p1b.TDSHP_TDSHP_HFG_CTRL.bits.TDSHP_HFG_SLK_EN));
		if (tdbEn) { /* always do slk cor no metter enable or not */
			if (moduleRegDump)
				dump_ctrlPQDIP(ctrli, HW_PQDIP_B);
			// TIME_START(tdshapb_st);
			cfg_tdshp(metai, &dl_table[HW_PQDIP_B], HW_PQDIP_B, ctrli, moduleRegDump);
			// TIME_END(tdshapb_et);
			if (moduleRegDump)
				dump_tdshpB_reg(metai);
		}
	}
	// TIME_END(dohelper_et);

	//LOG_INF(
	//"doHelperTime= %lld, meTime= %lld, lscTime= %lld,"
	//"ltmTime= %lld, dmTime= %lld, dipallslkTime= %lld,"
	//"tncTime= %lld, tdshapATime= %lld, tdshapBTime= %lld\n",
	//TIME_DIFF_TO_US(dohelper_st, dohelper_et),
	//TIME_DIFF_TO_US(me_st, me_et),
	//TIME_DIFF_TO_US(lsc_st, lsc_et),
	//TIME_DIFF_TO_US(ltm_st, ltm_et),
	//TIME_DIFF_TO_US(dm_st, dm_et),
	//TIME_DIFF_TO_US(dipallslk_st, dipallslk_et),
	//TIME_DIFF_TO_US(tnc_st, tnc_et),
	//TIME_DIFF_TO_US(tdshapa_st, tdshapa_et),
	//TIME_DIFF_TO_US(tdshapb_st, tdshapb_et));
}
