/*
 * Project : SIMD_Utils
 * Version : 0.1.2
 * Author  : JishinMaster
 * Licence : BSD-2
 */

#ifdef OMP
#include <omp.h>
#endif

#define INVLN10 0.4342944819
#define IMM8_FLIP_VEC  0x1B // change m128 from abcd to dcba
#define IMM8_LO_HI_VEC 0x1E // change m128 from abcd to cdab
#define IMM8_PERMUTE_128BITS_LANES 0x1 // reverse abcd efgh to efgh abcd
#define M_PI 3.14159265358979323846

#warning "TODO : add better alignment checks"

/* LATENCIES
SSE
_mm_store_ps     lat 1, cpi 1 (ivy ) 0.5 (broadwell)
_mm_storeu_ps    lat 1, cpi 1 (ivy ) 0.5 (broadwell)
_mm_load_ps      lat 1, cpi 1 (ivy ) 0.5 (broadwell)
_mm_loadu_ps     lat 1, cpi 1 (ivy ) 0.5 (broadwell)
_mm_min_ps	 lat 3, cpi 1 (ivy ) 1   (broadwell)
_mm_max_ps       lat 3, cpi 1 (ivy ) 1   (broadwell)
_mm_cvtpd_ps     lat 4, cpi 1 (ivy ) 1   (broadwell)
_mm_mul_ps	 lat 5 (ivy) 3 (broadwell), cpi 1 (ivy) 0.5 (broadwell)
_mm_div_ps	 lat 11-14 (ivy) <11 (broadwell), cpi 6 (ivy) 4 (broadwell)
_mm_movelh_ps    lat 1, cpi 1
_mm_hadd_ps		 lat 5, cpi 2 => useful for reduction!
_mm_shuffle_ps lat 1, cpi 1
_mm_cvtps_epi32 lat 3, cpi 1
_mm_round_ps
_mm_castsi128_ps


AVX/AVX2
_mm256_store_ps  lat 1, cpi 1 (ivy ) 0.5 (broadwell)
_mm256_storeu_ps lat 1, cpi 1 (ivy ) 0.5 (broadwell)
_mm256_load_ps   lat 1, cpi 1 (ivy ) 0.5 (broadwell)
_mm256_loadu_ps  lat 1, cpi 1 (ivy ) 0.5 (broadwell)
_mm256_min_ps	 lat 3, cpi 1 (ivy ) 1   (broadwell)
_mm256_max_ps	 lat 3, cpi 1 (ivy ) 1   (broadwell)
_mm256_cvtpd_ps  lat 4 (ivy) 6 (broadwell), cpi 1 (ivy ) 1  (broadwell)
_mm256_mul_ps	 lat 5 (ivy) 3 (broadwell), cpi 1 (ivy) 0.5 (broadwell)
_mm256_div_ps	 lat 18-21 (ivy) 13-17 (broadwell), cpi 14 (ivy) 10 (broadwell)
_mm256_set_m128  lat 3, cpi 1
_mm256_hadd_ps
_mm256_permute_ps lat 1, cpi 1
_mm256_permute2f128_ps lat 2(ivy) 3 (broadwell) , cpi 1	
 */

typedef struct {
	float re;
	float im;
} complex32_t;


typedef struct {
	double re;
	double im;
} complex64_t;

#ifdef SSE
#define SSE_LEN_BYTES 16 // Size of SSE lane
#define SSE_LEN_INT32  4 // number of int32 with an SSE lane
#define SSE_LEN_FLOAT  4 // number of float with an SSE lane
#define SSE_LEN_DOUBLE 2 // number of double with an SSE lane
#include "sse_mathfun.h"

#define _PD_CONST(Name, Val)                                            \
		static const ALIGN16_BEG double _pd_##Name[2] ALIGN16_END = { Val, Val}
#define _PI64_CONST(Name, Val)                                            \
		static const ALIGN16_BEG int64_t _pi64_##Name[2] ALIGN16_END = { Val, Val}
#define _PD_CONST_TYPE(Name, Type, Val)                                 \
		static const ALIGN16_BEG Type _pd_##Name[2] ALIGN16_END = { Val, Val}

_PD_CONST_TYPE(min_norm_pos, long int, 0x380ffff83ce549caL);
_PD_CONST_TYPE(mant_mask, long int, 0xFFFFFFFFFFFFFL);
_PD_CONST_TYPE(inv_mant_mask, long int, ~0xFFFFFFFFFFFFFL);
_PD_CONST_TYPE(sign_mask, long int, (long int)0x8000000000000000L);
_PD_CONST_TYPE(inv_sign_mask, long int, ~0x8000000000000000L);

_PD_CONST(1 , 1.0);
_PD_CONST(0p5, 0.5);

_PI64_CONST(1, 1);
_PI64_CONST(inv1, ~1);
_PI64_CONST(2, 2);
_PI64_CONST(4, 4);
_PI64_CONST(0x7f, 0x7f);

_PD_CONST(cephes_SQRTHF, 0.70710678118654752440);
_PD_CONST(cephes_log_p0, 1.01875663804580931796E-4);
_PD_CONST(cephes_log_p1, - 4.97494994976747001425E-1);
_PD_CONST(cephes_log_p2, 4.70579119878881725854E0);
_PD_CONST(cephes_log_p3, - 1.44989225341610930846E1);
_PD_CONST(cephes_log_p4, + 1.79368678507819816313E1);
_PD_CONST(cephes_log_p5, - 7.70838733755885391666E0);

_PD_CONST(cephes_log_q1, - 1.12873587189167450590E1);
_PD_CONST(cephes_log_q2, 4.52279145837532221105E1);
_PD_CONST(cephes_log_q3, - 8.29875266912776603211E1);
_PD_CONST(cephes_log_q4, 7.11544750618563894466E1);
_PD_CONST(cephes_log_q5, 4.52279145837532221105E1);
_PD_CONST(cephes_log_q6, - 2.31251620126765340583E1);

_PD_CONST(exp_hi,  709.437);
_PD_CONST(exp_lo, -709.436139303);

_PD_CONST(cephes_LOG2EF, 1.4426950408889634073599);

_PD_CONST(cephes_exp_p0, 1.26177193074810590878e-4);
_PD_CONST(cephes_exp_p1, 3.02994407707441961300e-2);
_PD_CONST(cephes_exp_p2, 9.99999999999999999910e-1);

_PD_CONST(cephes_exp_q0, 3.00198505138664455042e-6);
_PD_CONST(cephes_exp_q1, 2.52448340349684104192e-3);
_PD_CONST(cephes_exp_q2, 2.27265548208155028766e-1);
_PD_CONST(cephes_exp_q3, 2.00000000000000000009e0);

_PD_CONST(cephes_exp_C1, 0.693145751953125);
_PD_CONST(cephes_exp_C2, 1.42860682030941723212e-6);

#include "simd_utils_sse_float.h"
#ifndef ARM
#include "simd_utils_sse_double.h"
#endif
#include "simd_utils_sse_int32.h"
#endif

#ifdef AVX
__m256 _mm256_set_m128 ( __m128 H, __m128 L) //not present on every GCC version
{
	return _mm256_insertf128_ps(_mm256_castps128_ps256(L), H, 1);
}
#define AVX_LEN_BYTES 32 // Size of AVX lane
#define AVX_LEN_INT32  4 // number of int32 with an AVX lane
#define AVX_LEN_FLOAT  8 // number of float with an AVX lane
#define AVX_LEN_DOUBLE 4 // number of double with an AVX lane
#include "avx_mathfun.h"
#include "simd_utils_avx_float.h"
#include "simd_utils_avx_double.h"
#include "simd_utils_avx_int32.h"
#endif


#ifdef WINDOWS // TODO : find a way to align on Windows
void posix_memalign( void** inout , int alignement, size_t len){
	*inout = malloc(len);
}	
#endif 



//////////  C Test functions ////////////////
void log10f_C(float* src, float* dst, int len)
{
	for(int i = 0; i < len; i++) dst[i] = log10f(src[i]);
}

void lnf_C(float* src, float* dst, int len)
{
	for(int i = 0; i < len; i++) dst[i] = logf(src[i]);
}

void fabsf_C(float* src, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = fabsf(src[i]);
	}		
}

void setf_C( float* src, float value, int len)
{
	for(int i = 0; i < len; i++){
		src[i] = value;
	}		
}

void addcf_C( float* src, float value, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = src[i] + value;
	}		
}

void mulcf_C( float* src, float value, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = src[i] * value;
	}		
}

void divf_C( float* src1, float* src2, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = src1[i] / src2[i];
	}		
}

void cplxtorealf_C( float* src, float* dstRe, float* dstIm, int len)
{
	int j = 0;
	for(int i = 0; i < 2*len; i+=2){
		dstRe[j]   = src[i];
		dstIm[j]   = src[i+1];
		j++;
	}		
}

void convert_64f32f_C(double* src, float* dst, int len)
{
#ifdef OMP
#pragma omp simd
#endif
	for(int i = 0; i < len; i++){
		dst[i] = (float)src[i];
	}
}

void convert_32f64f_C(float* src, double* dst, int len)
{
#ifdef OMP
#pragma omp simd
#endif
	for(int i = 0; i < len; i++){
		dst[i] = (double)src[i];
	}
}

void threshold_lt_f_C( float* src, float* dst, float value, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = src[i]<value?src[i]:value;
	}	
}


void threshold_gt_f_C( float* src, float* dst, float value, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = src[i]>value?src[i]:value;
	}
}

void magnitudef_C_interleaved( complex32_t* src, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = sqrtf(src[i].re*src[i].re + src[i].im*src[i].im);
	}
}

void magnitudef_C_split( float* srcRe, float* srcIm, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = sqrtf(srcRe[i]*srcRe[i] + srcIm[i]*srcIm[i]);
	}
}

void meanf_C(float* src, float* dst, int len)
{
	float acc = 0.0f;
	int i;
#ifdef OMP
#pragma omp simd reduction(+:acc)
#endif
	for(i = 0; i < len; i++){
		acc += src[i];
	}

	acc  = acc/(float)len;
	*dst = acc;
}

void flipf_C(float* src, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[len  -i - 1] = src[i];
	}
}

void asinf_C( float* src, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = asinf(src[i]);
	}
}

void tanf_C( float* src, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = tanf(src[i]);
	}
}

void sinf_C( float* src, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = sinf(src[i]);
	}
}

void cosf_C( float* src, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = cosf(src[i]);
	}
}

void floorf_C( float* src, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = floorf(src[i]);
	}
}

void ceilf_C( float* src, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = ceilf(src[i]);
	}
}

void roundf_C( float* src, float* dst, int len)
{
	for(int i = 0; i < len; i++){
		dst[i] = roundf(src[i]);
	}
}

////////////////////////
