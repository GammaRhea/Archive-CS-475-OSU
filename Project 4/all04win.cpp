#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>

// NOTE(Zack): for intel intrinsics
#if defined(_WIN64) || defined(_WIN32)
#include <immintrin.h>
#else
#error "No Unix SIMD"
#endif

#include <omp.h>

// SSE stands for Streaming SIMD Extensions

#define SSE_WIDTH	4
#define ALIGNED		__declspec(align(16))


#define NUMTRIES	100

#ifndef ARRAYSIZE
#define ARRAYSIZE	1024*1024
#endif


void	SimdMul(    float *, float *,  float *, int );
void	NonSimdMul( float *, float *,  float *, int );
float	SimdMulSum(    float *, float *, int );
float	NonSimdMulSum( float *, float *, int );

ALIGNED float A[ARRAYSIZE];
ALIGNED float B[ARRAYSIZE];
ALIGNED float C[ARRAYSIZE];

int
main( int argc, char *argv[ ] )
{
	for( int i = 0; i < ARRAYSIZE; i++ )
	{
		A[i] = sqrtf( (float)(i+1) );
		B[i] = sqrtf( (float)(i+1) );
        C[i] = 0.0f;
	}

	fprintf( stderr, "%12d\t", ARRAYSIZE );

	double maxPerformance = 0.;
	for( int t = 0; t < NUMTRIES; t++ )
	{
		double time0 = omp_get_wtime( );
		NonSimdMul( A, B, C, ARRAYSIZE );
		double time1 = omp_get_wtime( );
		double perf = (double)ARRAYSIZE / (time1 - time0);
		if( perf > maxPerformance )
			maxPerformance = perf;
	}
	double megaMults = maxPerformance / 1000000.;
	fprintf( stderr, "N %10.2lf\t", megaMults );
	double mmn = megaMults;


	maxPerformance = 0.;
	for( int t = 0; t < NUMTRIES; t++ )
	{
		double time0 = omp_get_wtime( );
		SimdMul( A, B, C, ARRAYSIZE );
		double time1 = omp_get_wtime( );
		double perf = (double)ARRAYSIZE / (time1 - time0);
		if( perf > maxPerformance )
			maxPerformance = perf;
	}
	megaMults = maxPerformance / 1000000.;
	fprintf( stderr, "S %10.2lf\t", megaMults );
	double mms = megaMults;
	double speedup = mms/mmn;
	fprintf( stderr, "(%6.2lf)\t", speedup );


	maxPerformance = 0.;
	float sumn, sums;
	for( int t = 0; t < NUMTRIES; t++ )
	{
		double time0 = omp_get_wtime( );
		sumn = NonSimdMulSum( A, B, ARRAYSIZE );
		double time1 = omp_get_wtime( );
		double perf = (double)ARRAYSIZE / (time1 - time0);
		if( perf > maxPerformance )
			maxPerformance = perf;
	}
	double megaMultAdds = maxPerformance / 1000000.;
	fprintf( stderr, "N %10.2lf\t", megaMultAdds );
	mmn = megaMultAdds;


	maxPerformance = 0.;
	for( int t = 0; t < NUMTRIES; t++ )
	{
		double time0 = omp_get_wtime( );
		sums = SimdMulSum( A, B, ARRAYSIZE );
		double time1 = omp_get_wtime( );
		double perf = (double)ARRAYSIZE / (time1 - time0);
		if( perf > maxPerformance )
			maxPerformance = perf;
	}
	megaMultAdds = maxPerformance / 1000000.;
	fprintf( stderr, "S %10.2lf\t", megaMultAdds );
	mms = megaMultAdds;
	speedup = mms/mmn;
	fprintf( stderr, "(%6.2lf)\n", speedup );
	// fprintf( stderr, "[ %8.1f , %8.1f , %8.1f ]\n", C[ARRAYSIZE-1], sumn, sums );

	return 0;
}


void
NonSimdMul( float *A, float *B, float *C, int n )
{
	for( int i = 0; i < n; i++ )
		C[i] = A[i] * B[i];
}

float
NonSimdMulSum( float *A, float *B, int n )
{
	float sum = 0.;
	for( int i = 0; i < n; i++ )
		sum += A[i] * B[i];
	
	return sum;
}


void
SimdMul( float *a, float *b, float *c, int len )
{
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;

	for( int i = 0; i < limit; i += SSE_WIDTH )	{
		__m128 xmm0 = _mm_loadu_ps(a + i);
		__m128 xmm1 = _mm_loadu_ps(b + i);
		__m128 xmm2 = _mm_mul_ps(xmm0, xmm1);
        _mm_storeu_ps(c + i, xmm2);
	}

	for( int i = limit; i < len; i++ ) {
		c[i] = a[i] * b[i];
	}
}



float
SimdMulSum( float *a, float *b, int len )
{
	__m128 sum = _mm_set1_ps(0.0f);
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;

	for( int i = 0; i < limit; i += SSE_WIDTH )	{
		__m128 xmm0 = _mm_loadu_ps(a + i);
		__m128 xmm1 = _mm_loadu_ps(b + i);
		__m128 xmm2 = _mm_mul_ps(xmm0, xmm1);
        sum = _mm_add_ps(sum, xmm2);
	}

    float to_float[4];
    _mm_storeu_ps(to_float, sum);
    float result = 
        to_float[0] +
        to_float[1] +
        to_float[2] +
        to_float[3] ;
	
	for( int i = limit; i < len; i++ )
	{
		result += a[i] * b[i];
	}

	return result;
}