#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

const float RYEGRASS_GROWS_PER_MONTH =		20.0;
const float ONE_RABBITS_EATS_PER_MONTH =	 1.0;
const float RUSSIAN_BLUE_EATS_PER_MONTH = 	0.75;

const float AVG_PRECIP_PER_MONTH =	       12.0;	// average
const float AMP_PRECIP_PER_MONTH =		4.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				60.0;
const float MIDPRECIP =				14.0;

int		NowYear;		// 2023 - 2028
int		NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// rye grass height in inches
int		NowNumRabbits;		// number of rabbits in the current population

float 	NowCatWeight;	// weight of a cat that disrupts the ecosystem in lbs

unsigned int seed = 0;

omp_lock_t	Lock;
volatile int	NumInThreadTeam;
volatile int	NumAtBarrier;
volatile int	NumGone;

// specify how many threads will be in the barrier:
//	(also init's the Lock)

void
InitBarrier( int n )
{
        NumInThreadTeam = n;
        NumAtBarrier = 0;
	omp_init_lock( &Lock );
}


// have the calling thread wait here until all the other threads catch up:

void
WaitBarrier( )
{
        omp_set_lock( &Lock );
        {
                NumAtBarrier++;
                if( NumAtBarrier == NumInThreadTeam )
                {
                        NumGone = 0;
                        NumAtBarrier = 0;
                        // let all other threads get back to what they were doing
			// before this one unlocks, knowing that they might immediately
			// call WaitBarrier( ) again:
                        while( NumGone != NumInThreadTeam-1 );
                        omp_unset_lock( &Lock );
                        return;
                }
        }
        omp_unset_lock( &Lock );

        while( NumAtBarrier != 0 );	// this waits for the nth thread to arrive

        #pragma omp atomic
        NumGone++;			// this flags how many threads have returned
}

float
Sqr( float x )
{
        return x*x;
}

float
Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX

        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}

void setEnv() {
	float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

	float temp = AVG_TEMP - AMP_TEMP * cos( ang );
	NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

	float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
	NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
	if( NowPrecip < 0. )
		NowPrecip = 0.;
}

void Rabbits() {
	while( NowYear < 2029 ) {
		int nextNumRabbits = NowNumRabbits;
		int carryingCapacity = (int)( NowHeight );
		if( nextNumRabbits < carryingCapacity )
			nextNumRabbits++;
		else
			if( nextNumRabbits > carryingCapacity )
                nextNumRabbits--;
			
		if( NowCatWeight >= 13.0 ) {
			// if the cat is away the rabbits will play, +1 bunny
			nextNumRabbits++;
		}
		else if( NowCatWeight < 13.0 && NowNumRabbits > 1 ) {
			// that cat is going to eat a rabbit, -1 bunny
			nextNumRabbits--;
		}

		if( nextNumRabbits < 0 )
			nextNumRabbits = 0;
		
		// DoneComputing Barrier:
		WaitBarrier();
		
		NowNumRabbits = nextNumRabbits;
		
		// DoneAssigning Barrier:
		WaitBarrier();
		
		// DonePrinting Barrier:
		WaitBarrier();
	}
}

void RyeGrass() {
	while( NowYear < 2029 ) {
		float tempFactor = exp(   -Sqr(  ( NowTemp - MIDTEMP ) / 10.  )   );
		float precipFactor = exp(   -Sqr(  ( NowPrecip - MIDPRECIP ) / 10.  )   );
		
		float nextHeight = NowHeight;
		nextHeight += tempFactor * precipFactor * RYEGRASS_GROWS_PER_MONTH;
		nextHeight -= (float)NowNumRabbits * ONE_RABBITS_EATS_PER_MONTH;
		
		// if the cat is fat, it'll eat the grass to sooth its stomach
		if( NowCatWeight >= 13.0 )
			nextHeight -= RUSSIAN_BLUE_EATS_PER_MONTH;
		
		if( nextHeight < 0. ) nextHeight = 0.;
		
		// DoneComputing Barrier:
		WaitBarrier();
		
		NowHeight = nextHeight;
		
		// DoneAssigning Barrier:
		WaitBarrier();
		
		// DonePrinting Barrier:
		WaitBarrier();
	}
}

void Watcher() {
	while( NowYear < 2029 ) {
		// DoneComputing Barrier:
		WaitBarrier();
		
		// DoneAssigning Barrier:
		WaitBarrier();
		
		if( NowMonth >= 11) {
			NowMonth = 0;
			NowYear++;
		}
		else {
			NowMonth++;
		}
		
		setEnv();
		
		printf("%.2f, %.2f, %.2f, %d, %.2f\n", (NowPrecip * 2.54), ((5./9.)*(NowTemp - 32)), (NowHeight * 2.54), NowNumRabbits, NowCatWeight);
		
		// DonePrinting Barrier:
		WaitBarrier();
	}
}

void RussianBlueCat() {
	while( NowYear < 2029 ) {
		float nextCatWeight = NowCatWeight;
		if( nextCatWeight >= 13.0 ) {
			// cat has eaten too much and can't catch the rabbits
			// the cat will eat the grass and lose weight
			nextCatWeight = nextCatWeight - Ranf( &seed, 0.5f, 1.5f );
		}
		else if( nextCatWeight < 13.0 && NowNumRabbits > 1 ) {
			// the cat can catch rabbits to eat them, and will do so
			// the cat will gain weight
			nextCatWeight = nextCatWeight + Ranf( &seed, 1.f, 2.f );
		}
		else {
			// there aren't enough rabbits to eat, so the cat goes hungry
			// poor kitty cat
			nextCatWeight = nextCatWeight - Ranf( &seed, 1.f, 2.f );
		}
		
		if( nextCatWeight < 7.0 )
			nextCatWeight = 7.0; // the crafty cat ends up getting food from nearby homes to avoid starving
		
		// DoneComputing Barrier:
		WaitBarrier();
		
		NowCatWeight = nextCatWeight;
		
		// DoneAssigning Barrier:
		WaitBarrier();
		
		// DonePrinting Barrier:
		WaitBarrier();
	}
}

int main ( )
{
	float x = Ranf( &seed, -1.f, 1.f );
	seed = x;
	
	// starting date and time:
	NowMonth =    0;
	NowYear  = 2023;

	// starting state (feel free to change this if you want):
	NowNumRabbits = 1;
	NowHeight =  5.;
	NowCatWeight = 10.;
	
	setEnv();
	
	printf ("Rain, Temp, Height, Rabbit, Cat Weight\n");
	printf("%.2f, %.2f, %.2f, %d, %.2f\n", (NowPrecip * 2.54), ((5./9.)*(NowTemp - 32)), (NowHeight * 2.54), NowNumRabbits, NowCatWeight);
	
	omp_set_num_threads( 4 );	// or 4
	InitBarrier( 3 );		// or 4
	
	#pragma omp parallel sections
	{
		#pragma omp section
		{
			Rabbits( );
		}

		#pragma omp section
		{
			RyeGrass( );
		}

		#pragma omp section
		{
			Watcher( );
		}
		
		#pragma omp section
		{
			RussianBlueCat( );
		}
	}       // implied barrier -- all functions must return in order
			// to allow any of them to get past here
	printf("Current Year: %d\n", NowYear);
}