// Directly copied from "C++ Data Structures for Rigid-Body Physics" by Miguel Gomez
// http://www.gamasutra.com/features/19990702/data_structures_02.htm
// For I'm too lazy to write yet another vector class
#pragma once

#include <cmath>

namespace physics
{
	// A floating point number
	//
	typedef float SCALAR;

	//
	// A 3D vector
	//
	class VECTOR
	{
	public:
		SCALAR x,y,z; //x,y,z coordinates
	public:
		VECTOR()
			: x(0), y(0), z(0)
		{}
		VECTOR( const SCALAR& a, const SCALAR& b, const SCALAR& c )
			: x(a), y(b), z(c)
		{}

		//compare
		const bool operator == ( const VECTOR& v ) const
		{
			return (v.x==x && v.y==y && v.z==z);
		}

		const bool operator != ( const VECTOR& v ) const
		{
			return !(v == *this);
		}

		//negate
		const VECTOR operator - () const
		{
			return VECTOR( -x, -y, -z );
		}

		//assign
		const VECTOR& operator = ( const VECTOR& v )
		{
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}

		//increment
		const VECTOR& operator += ( const VECTOR& v )
		{
			x+=v.x;
			y+=v.y;
			z+=v.z;
			return *this;
		}

		//decrement
		const VECTOR& operator -= ( const VECTOR& v )
		{
			x-=v.x;
			y-=v.y;
			z-=v.z;
			return *this;
		}

		//self-multiply
		const VECTOR& operator *= ( const SCALAR& s )
		{
			x*=s;
			y*=s;
			z*=s;
			return *this;
		}

		//self-divide
		const VECTOR& operator /= ( const SCALAR& s )
		{
			const SCALAR r = 1 / s;
			x *= r;
			y *= r;
			z *= r;
			return *this;
		}

		//add
		const VECTOR operator + ( const VECTOR& v ) const
		{
			return VECTOR(x + v.x, y + v.y, z + v.z);
		}

		//subtract
		const VECTOR operator - ( const VECTOR& v ) const
		{
			return VECTOR(x - v.x, y - v.y, z - v.z);
		}

		//post-multiply by a scalar

		const VECTOR operator * ( const SCALAR& s ) const
		{
			return VECTOR( x*s, y*s, z*s );
		}

		//pre-multiply by a scalar
		friend inline const VECTOR operator * ( const SCALAR& s, const VECTOR& v )
		{
			return v * s;
		}

		//divide
		const VECTOR operator / (SCALAR s) const
		{
			s = 1/s;
			return VECTOR( s*x, s*y, s*z );
		}

		//cross product
		const VECTOR cross( const VECTOR& v ) const
		{
			//Davis, Snider, "Introduction to Vector Analysis", p. 44
			return VECTOR( y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x );
		}

		//scalar dot product
		const SCALAR dot( const VECTOR& v ) const
		{
			return x*v.x + y*v.y + z*v.z;
		}

		//length
		const SCALAR length() const
		{
			return (SCALAR)sqrt( (double)this->dot(*this) );
		}

		//unit vector
		const VECTOR unit() const
		{
			return (*this) / length();
		}

		//make this a unit vector
		void normalize()
		{
			(*this) /= length();
		}

		//equal within an error ‘e’
		const bool nearlyEquals( const VECTOR& v, const SCALAR e ) const
		{
			return fabs(x-v.x)<e && fabs(y-v.y)<e && fabs(z-v.z)<e;
		}
	};

	//
	// A 3D position
	//

	typedef VECTOR POINT;
}