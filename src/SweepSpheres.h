#pragma once

namespace physics
{
typedef float SCALAR;
class VECTOR;

  // Function to find point in time where two moving spheres intersect
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
 );
}
