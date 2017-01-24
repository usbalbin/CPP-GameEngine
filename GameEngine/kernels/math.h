#pragma once
#include "kernels/containers.h"

/*//Based on glm's matrix inversion
float16 matInverse(float16 m){
	
	float4 Fac0;
	Fac0.x = Fac0.y = m.sa * m.sf - m.se * m.sb;
	Fac0.z = m.s6 * m.sf - m.se * m.s7;
	Fac0.w = m.s6 * m.sb - m.sa * m.s7;

	float4 Fac1;
	Fac1.x = Fac0.y = m.s9 * m.sf - m.sd * m.sb;
	Fac1.z = m.s5 * m.sf - m.sd * m.s7;
	Fac1.w = m.s5 * m.sb - m.s9 * m.s7;

	float4 Fac2;
	Fac1.x = Fac0.y = m.s9 * m.se - m.sd * m.sa;
	Fac1.z = m.s5 * m.se - m.sd * m.s6;
	Fac1.w = m.s5 * m.sa - m.s9 * m.s6;

	float4 Fac3;
	Fac1.x = Fac0.y = m.s8 * m.sf - m.sc * m.sb;
	Fac1.z = m.s4 * m.sf - m.sc * m.s7;
	Fac1.w = m.s4 * m.sb - m.s8 * m.s7;

	float4 Fac4;
	Fac1.x = Fac0.y = m.s8 * m.se - m.sc * m.sa;
	Fac1.z = m.s4 * m.se - m.sc * m.s6;
	Fac1.w = m.s4 * m.sa - m.s8 * m.s6;

	float4 Fac5;
	Fac1.x = Fac0.y = m.s8 * m.sd - m.sc * m.s9;
	Fac1.z = m.s4 * m.sd - m.sc * m.s5;
	Fac1.w = m.s4 * m.s9 - m.s8 * m.s5;
  
  
	

	 
	

	float4 Vec0 = (float4)(m.s4, m.s0, m.s0, m.s0);
	float4 Vec1 = (float4)(m.s5, m.s1, m.s1, m.s1);
	float4 Vec2 = (float4)(m.s6, m.s2, m.s2, m.s2);
	float4 Vec3 = (float4)(m.s7, m.s3, m.s3, m.s3);

	float4 Inv0 = (float4)(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
	float4 Inv1 = (float4)(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
	float4 Inv2 = (float4)(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
	float4 Inv3 = (float4)(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

	float4 SignA = (float4)(+1, -1, +1, -1);
	float4 SignB = (float4)(-1, +1, -1, +1);
	float16 Inverse = (float16)(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

	float4 Row0 = (float4)(Inverse.s0, Inverse.s4, Inverse.s8, Inverse.sc);

	float4 Dot0 = (float4)(m[0] * Row0);
	float Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

	float OneOverDeterminant = 1 / Dot1;

	return Inverse * OneOverDeterminant;
}*/

float4 mulMatVec(float16 m, float4 v){
	/*return (float4)(
		dot(m.s048c, v),
		dot(m.s159d, v),
		dot(m.s26ae, v),
		dot(m.s37bf, v)
	);*/
	return (float4)(
		dot(m.s0123, v),
		dot(m.s4567, v),
		dot(m.s89ab, v),
		dot(m.scdef, v)
	);
}

float4 mulMatVec123(float16 m, float4 v){
	return (float4)(
		dot(m.s048c, v),
		dot(m.s159d, v),
		dot(m.s26ae, v),
		dot(m.s37bf, v)
	);

}

Ray transformRay(float16 matrix, Ray ray){
	Ray result;
	result.position = mulMatVec(matrix, (float4)(ray.position, 1.0f)).xyz;
	result.direction = mulMatVec(matrix, (float4)(ray.direction, 0.0f)).xyz;
	result.inverseDirection = 1.0f / result.direction;
	return result;
}

Vertex transformVertex(float16 matrix, Vertex vertex){
	Vertex result = vertex;
	result.position = mulMatVec(matrix,	(float4)(vertex.position, 1.0f)).xyz;
	result.normal = mulMatVec(matrix,	(float4)(vertex.normal, 0.0f)).xyz;
	return result;
}

Vertex transformVertex123(float16 matrix, Vertex vertex){
	Vertex result = vertex;
	result.position = mulMatVec123(matrix,	(float4)(vertex.position, 1.0f)).xyz;
	result.normal = mulMatVec123(matrix,	(float4)(vertex.normal, 0.0f)).xyz;
	return result;
}

/*
//TODO check wether m1 and m2 should swap places
float16 mulMat_old(float16 m1, float16 m2){
	return (float16)(
		dot(m1.s0123, m2.s048c), dot(m1.s0123, m2.s159d), dot(m1.s0123, m2.s26ae), dot(m1.s0123, m2.s37bf),
		dot(m1.s4567, m2.s048c), dot(m1.s4567, m2.s159d), dot(m1.s4567, m2.s26ae), dot(m1.s4567, m2.s37bf),
		dot(m1.s89ab, m2.s048c), dot(m1.s89ab, m2.s159d), dot(m1.s89ab, m2.s26ae), dot(m1.s89ab, m2.s37bf),
		dot(m1.scdef, m2.s048c), dot(m1.scdef, m2.s159d), dot(m1.scdef, m2.s26ae), dot(m1.scdef, m2.s37bf)
	);
}
*/

Ray reflect(Hit hit){
	const float epsilon = 1e-6f;
	const Ray rayIn = hit.ray;
	const Vertex poi = hit.vertex;
	
	
	Ray result;
	
	//Move position along normal slightly to prevent collision with reflector
	result.position = poi.position + epsilon * poi.normal;
	result.direction = rayIn.direction - 2.0f * dot(rayIn.direction, poi.normal) * poi.normal;
	result.inverseDirection = 1.0f / result.direction;
	
	return result;
}

/**
 * TODO add support for refraction index
 */
Ray refract(Hit hit){
	const float epsilon = 1e-6f;
	const Ray rayIn = hit.ray;
	const Vertex poi = hit.vertex;
	
	Ray result;
	
	//Move position along normal slightly to prevent collision with refractor
	result.position = poi.position - epsilon * poi.normal;
	result.direction = rayIn.direction;
	result.inverseDirection = rayIn.inverseDirection;
	
	return result;
}