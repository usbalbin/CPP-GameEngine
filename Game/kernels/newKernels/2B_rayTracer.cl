#include "kernels/containers.h"
#include "kernels/intersection.h"






constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_MIRRORED_REPEAT | CLK_FILTER_LINEAR;//CLK_NORMALIZED_COORDS_TRUE  | CLK_FILTER_LINEAR | CLK_ADDRESS_MIRRORED_REPEAT;











typedef struct{
	int objectIndex;
	float nearDistance, farDistance;
} ObjectInfo;

Hit sky(Ray ray);
bool traceBruteForceColor(int instanceCount,
	global const Instance* instances,
	global const Object* objectTypes,
	
	global const TriangleIndices* allTriangles,
	global const Vertex* allVertices,
	Ray ray,
	Vertex* intersectionPoint);
bool traceBruteForceColorNew(
	int instanceCount,
	global const Instance* instances,
	global const Object* objectTypes,
	
	global const TriangleIndices* allTriangles,
	global const Vertex* allVertices,
	Ray ray,
	Vertex* intersectionPoint
	);

#ifdef BVH	
bool traceBvhColor(int instanceCount, global const Instance* instances,	global const Object* objectTypes, global const BvhNode* triangleBvhNodes, global const TriangleIndices* allTriangles, global const Vertex* allVertices,	Ray ray, Vertex* intersectionPoint);
#endif


void sortObjects(Ray ray, int objectCount, global const Object* allObjects, local ObjectInfo* objectInfoListOut, int* objectCountOut);

Vertex interpolateTriangle(Triangle triangle, float2 uv);
float4 interpolate4(float4 a, float4 b, float4 c, float2 uv);
float3 interpolate3(float3 a, float3 b, float3 c, float2 uv);
float2 interpolate2(float2 a, float2 b, float2 c, float2 uv);
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

void kernel rayTraceAdvanced(
	int instanceCount,

	global const Instance* instances,

	global const Object* objectTypes,
	
	global const BvhNode* triangleBvhNodes,

	
	global const TriangleIndices* allTriangles,
	global const Vertex* allVertices,
	global const Ray* rays,
	global Hit* hits,
	global RayTree* rayTrees,
	
	TEXTURE_ARGS
){
	Vertex intersectionPoint;
	Ray ray = rays[gid];
	Hit hit;
	
	Instance closestInstance;
	float closestTriangleDist = FLT_MAX;
	Triangle closestTriangle;
	float2 closestUv;
	
	Instance instance;
	for (int instanceIndex = 0; instanceIndex < instanceCount; instanceIndex++) {
		
		instance = instances[instanceIndex];
		Object objectType = objectTypes[instance.meshType];
		
		const global TriangleIndices* triangles = getTrianglesIndices(allTriangles, objectType);
		const global Vertex* vertices = getVertices(allVertices, objectType);
		
		float nearDistance, farDistance;
		
		Ray transformedRay = transformRay(instance.invModelMatrix, ray);
		
		
		for (int index = objectType.bvhRootNodeIndex; index < objectType.bvhRootNodeIndex + objectType.bvhTreeSize; ) {
			BvhNode currentNode = triangleBvhNodes[index];
			
			bool isHit = intersectsBox(transformedRay, currentNode.aabb, &nearDistance, &farDistance) && (nearDistance < closestTriangleDist);
			bool isLeaf = (currentNode.escapeIndex == -1);
			
			if(isHit && isLeaf){
				Triangle triangle = getTriangle(triangles, vertices, objectType, currentNode.index);

				float distance;// = 1;
				float2 uv;// = (float2)(0.5f, 0.5f);
				if (intersectsTriangle(transformedRay, triangle, &distance, &uv) && distance < closestTriangleDist) {
					closestTriangleDist = distance;
					closestTriangle = triangle;
					closestUv = uv;
					closestInstance = instance;
				}
				
			}
			
			if(isHit || isLeaf){//Proceed to left child if was hit and has child, if is leaf then sibling will be at same index as left child would've been
				index++;
			}else{
				index += currentNode.escapeIndex;
			}
			
		}
		
	}
	
	
	if(closestTriangleDist == FLT_MAX){
		hit = sky(ray);
	}else{

		intersectionPoint = transformVertex(closestInstance.modelMatrix, interpolateTriangle(closestTriangle, closestUv));
		
		
		
		
		float blendWidth = 5;
		float blendDistance = 10;
	
		GET_TEXTURE_COLOR(intersectionPoint.color, closestInstance.textureId, intersectionPoint.uv);
		
		//int printOkidok = printf("texId: %d\n", closestInstance.textureId);
		
		
		float3 lightDir = normalize((float3)(-0.9f, -0.5f, 0.2f));
		float dotProduct = dot(normalize(intersectionPoint.normal), -lightDir);
		intersectionPoint.color *= dotProduct * 0.5f + 0.5f;									//Diffuse
		intersectionPoint.color += (float4)(1.0f) * max(pow(dotProduct, 51), 0.0f) * 0.25f;	//Specular
	
	
		hit.vertex = intersectionPoint;
		hit.ray = ray;
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
	hit.vertex.color = (float4)(0.4f, 0.4f, 1.0f, 1.0f);
	hit.vertex.reflectFactor = 0.0f;
	hit.vertex.refractFactor = 0.0f;
	return hit;
}










Vertex interpolateTriangle(Triangle triangle, float2 uv){
	Vertex result;
	result.position = interpolate3(triangle.a.position, triangle.b.position, triangle.c.position, uv);
	result.normal = interpolate3(triangle.a.normal, triangle.b.normal, triangle.c.normal, uv);
	result.uv = interpolate2(triangle.a.uv, triangle.b.uv, triangle.c.uv, uv);
	
	result.color = interpolate4(triangle.a.color, triangle.b.color, triangle.c.color, uv);

	result.reflectFactor = interpolate1(triangle.a.reflectFactor, triangle.b.reflectFactor, triangle.c.reflectFactor, uv);
	result.refractFactor = interpolate1(triangle.a.refractFactor, triangle.b.refractFactor, triangle.c.refractFactor, uv);
	
	return result;
}


float4 interpolate4(float4 a, float4 b, float4 c, float2 uv){
	float bFactor = uv.x;
	float cFactor = uv.y;
	float aFactor = 1 - uv.x - uv.y;
	
	return a * aFactor + b * bFactor + c * cFactor;
}

float3 interpolate3(float3 a, float3 b, float3 c, float2 uv){
	float bFactor = uv.x;
	float cFactor = uv.y;
	float aFactor = 1 - uv.x - uv.y;
	
	return a * aFactor + b * bFactor + c * cFactor;
}

float2 interpolate2(float2 a, float2 b, float2 c, float2 uv){
	float bFactor = uv.x;
	float cFactor = uv.y;
	float aFactor = 1 - uv.x - uv.y;
	
	return a * aFactor + b * bFactor + c * cFactor;
}

float interpolate1(float a, float b, float c, float2 uv){
	float bFactor = uv.x;
	float cFactor = uv.y;
	float aFactor = 1 - uv.x - uv.y;
	
	return a * aFactor + b * bFactor + c * cFactor;
}

global const Vertex* getVertices(global const Vertex* allVertices, Object object) {
	return &allVertices[object.startVertex];
}

global const TriangleIndices* getTrianglesIndices(global const TriangleIndices* allTriangles, Object object) {
	return &allTriangles[object.startTriangle];
}



#if __OPENCL_VERSION__ > 120
Triangle getTriangle(const TriangleIndices* trianglesIndices, const Vertex* vertices, Object object, int index) {
	TriangleIndices triangleIndices = trianglesIndices[index];
	return getTriangle2(vertices, triangleIndices);
}

Triangle getTriangle2(const Vertex* vertices, TriangleIndices triangleIndices) {
	Triangle triangle;
	triangle.a = vertices[triangleIndices.a];
	triangle.b = vertices[triangleIndices.b];
	triangle.c = vertices[triangleIndices.c];
	return triangle;
}
#else
Triangle getTriangle(const global TriangleIndices* trianglesIndices, const global Vertex* vertices, Object object, int index) {
	TriangleIndices triangleIndices = trianglesIndices[index];
	return getTriangle2(vertices, triangleIndices);
}

Triangle getTriangle2(const global Vertex* vertices, TriangleIndices triangleIndices) {
	Triangle triangle;
	triangle.a = vertices[triangleIndices.a];
	triangle.b = vertices[triangleIndices.b];
	triangle.c = vertices[triangleIndices.c];
	return triangle;
}
#endif