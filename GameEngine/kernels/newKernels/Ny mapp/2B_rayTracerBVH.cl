#include "kernels/containers.h"
#include "kernels/intersection.h"

typedef struct{
	int objectIndex;
	float nearDistance, farDistance;
} ObjectInfo;

Hit sky(Ray ray);
bool traceBvhColor(int objectCount, global const Object* allObjects, global const TriangleIndices* allTriangles, global const Vertex* allVertices, Ray ray, Vertex* intersectionPoint);
void sortObjects(Ray ray, int objectCount, global const Object* allObjects, local ObjectInfo* objectInfoListOut, int* objectCountOut);

Vertex interpolateTriangle(Triangle triangle, float2 uv);
float4 interpolate4(float4 a, float4 b, float4 c, float2 uv);
float3 interpolate3(float3 a, float3 b, float3 c, float2 uv);
float interpolate1(float a, float b, float c, float2 uv);


global const Vertex* getVertices(global const Vertex* allVertices, Object object);
global const TriangleIndices* getTrianglesIndices(global const TriangleIndices* allTriangles, Object object);


#if __OPENCL_VERSION__ > 120
Triangle getTriangle(const TriangleIndices* trianglesIndices, const Vertex* vertices, Object object, int index);
Triangle getTriangle2(const Vertex* vertices, TriangleIndices triangleIndices);
#else
Triangle getTriangle(const global TriangleIndices* trianglesIndices, const global Vertex* vertices, Object object, int index);
Triangle getTriangle2(const global Vertex* vertices, TriangleIndices triangleIndices);
#endif


Ray genOrthogonalRay();
Ray genPerspectiveRay(float16 matrix);






#define gid get_global_id(0)

float4 testColor();

void kernel rayTraceAdvancedBvh(
	int objectCount,
	global const Object* objects,
	
	int triangleBvhSize,
	global const BvhNode* triangleBvh,
	global const TriangleIndices* triangles,
	global const Vertex* vertices,
	global const Ray* rays,
	global Hit* hits,
	global RayTree* rayTrees 
){
	Vertex intersectionPoint;
	Ray ray = rays[gid];
	Hit hit;

#ifdef BVH	
	bool wasHit = traceBvhColor(objectCount, objects, triangles, vertices, ray, &intersectionPoint);
#else
	
#endif
	
	if(wasHit){
		hit.vertex = intersectionPoint;
		hit.ray = ray;
	}
	else{
		hit = sky(ray);
	}
	
	hits[gid] = hit;
	
	RayTree rayTree;
	rayTree.color = hit.vertex.color;
	rayTree.reflectFactor = hit.vertex.reflectFactor;
	rayTree.refractFactor = hit.vertex.refractFactor;
	rayTrees[gid] = rayTree;
}

Hit sky(Ray ray){
	Hit hit;
	hit.ray = ray;
	hit.vertex.color = (float4)(1.0f, 0.41f, 0.71f, 1.0f);
	hit.vertex.reflectFactor = 0.0f;
	hit.vertex.refractFactor = 0.0f;
	return hit;
}

#ifdef BVH
bool traceBvhColor(
	int instanceCount,
	global const Instance* instances
	global const Object* objectTypes,
	
	
	global const BvhNode* triangleBvhNodes,
	global const TriangleIndices* allTriangles,
	global const Vertex* allVertices,
	Ray ray,
	Vertex* intersectionPoint
	
	) {
		
		
	float closestTriangleDist = FLT_MAX;
	Triangle closestTriangle;
	float2 closestUv;
	
	

	for (int instanceIndex = 0; instanceIndex < instanceCount; instanceIndex++) {
		
		const global TriangleIndices* triangles = getTrianglesIndices(allTriangles, objectType);
		const global Vertex* vertices = getVertices(allVertices, objectType);
		
		Bvh triangleBvh = triangleBvhs[instanceIndex];
		Instance instance = instances[instanceIndex];
		Object objectType = allObjects[instance.meshType];
		
		float nearDistance, farDistance;
		
		Ray transformedRay = transformRay(instance.invertedModelMatrix, ray);
		
		/*
		for (int index = triangleBvh.rootIndex; index < triangleBvh.size; ) {
			BvhNode currentNode = triangleBvhNodes[index];
			
			bool isHit = intersectsBox(transformedRay, currentNode.aabb, &nearDistance, &farDistance) && (nearDistance < closestTriangleDist)
			bool isLeaf = (currentNode.escapeIndex == -1);
			
			if(isHit && isLeaf){
				Triangle triangle = getTriangle(triangles, vertices, objectType, currentNode.index));

				float distance;
				float2 uv;
				if (intersectsTriangle(transformedRay, triangle, &distance, &uv) && distance < closestTriangleDist) {
					closestTriangleDist = distance;
					closestTriangle = triangle;
					closestUv = uv;
				}
			}
			
			if(isHit || isLeaf){//Proceed to left child if was hit and has child, if is leaf then sibling will be at same index as left child would've been
				index++;
			}else{
				index += currentNode.escapeIndex;
			}
			
		}*/
	}
	
	if(closestTriangleDist == FLT_MAX){
		//printf("Ray did not hit anything!!!");
		return false;
	}

	*intersectionPoint = interpolateTriangle(closestTriangle, closestUv);
	
	
	float3 lightDir = normalize((float3)(-0.9f, -0.5f, 0.2f));
	float dotProduct = dot(normalize((*intersectionPoint).normal), -lightDir);
	(*intersectionPoint).color *= dotProduct * 0.5f + 0.5f;									//Diffuse
	(*intersectionPoint).color += (float4)(1.0f) * max(pow(dotProduct, 51), 0.0f) * 0.25f;	//Specular
	
	//printf(" - Ray actually hit something!!!");
	return true;//(float4)((float)(int)closestTriangleDist);
}
#endif