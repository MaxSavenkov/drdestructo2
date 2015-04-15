// Directly copied from "Simple Intersection Tests For Games" by Miguel Gomez
// http://www.gamasutra.com/features/19991018/Gomez_2.htm
#include "stdafx.h"
#include "Vector.h"

#undef B0
#undef B1

namespace physics
{
	template< class T >
	inline void SWAP( T& a, T& b )
		//swap the values of a and b
	{
		const T temp = a;
		a = b;
		b = temp;
	}

	// Return true if r1 and r2 are real
	inline bool QuadraticFormula( const SCALAR a, const SCALAR b, const SCALAR c,
		SCALAR& r1, //first
		SCALAR& r2 //and second roots
		)
	{
		const SCALAR q = b*b - 4*a*c;
		if( q >= 0 )
		{
			const SCALAR sq = sqrt(q);
			const SCALAR d = 1 / (2*a);
			r1 = ( -b + sq ) * d;
			r2 = ( -b - sq ) * d;
			return true;//real roots
		}
		else
		{
			return false;//complex roots
		}
	}

	const bool SphereSphereSweep
		(
		const SCALAR ra, //radius of sphere A
		const VECTOR& A0, //previous position of sphere A
		const VECTOR& A1, //current position of sphere A
		const SCALAR rb, //radius of sphere B
		const VECTOR& B0, //previous position of sphere B
		const VECTOR& B1, //current position of sphere B
		SCALAR& u0, //normalized time of first collision
		SCALAR& u1 //normalized time of second collision
		)
	{
		const VECTOR va = A1 - A0;
		//vector from A0 to A1

		const VECTOR vb = B1 - B0;
		//vector from B0 to B1

		const VECTOR AB = B0 - A0;
		//vector from A0 to B0

		const VECTOR vab = vb - va;
		//relative velocity (in normalized time)

		const SCALAR rab = ra + rb;

		const SCALAR a = vab.dot(vab);
		//u*u coefficient

		const SCALAR b = 2*vab.dot(AB);
		//u coefficient

		const SCALAR c = AB.dot(AB) - rab*rab;
		//constant term

		//check if they're currently overlapping
		if( AB.dot(AB) <= rab*rab )
		{
			u0 = 0;
			u1 = 0;
			return true;
		}

		//check if they hit each other
		// during the frame
		if( QuadraticFormula( a, b, c, u0, u1 ) )
		{
			if( u0 > u1 )
				SWAP( u0, u1 );
			return true;
		}

		return false;
	}
}
