#include "kernels/containers.h"
#include "kernels/intersection.h"

typedef struct{
	int objectIndex;
	float nearDistance, farDistance;
} ObjectInfo;

Hit sky(Ray ray);
bool traceBruteForceColor(int objectCount, global const Object* allObjects, global const TriangleIndices* allTriangles, global const Vertex* allVertices, Ray ray, Vertex* intersectionPoint);

#ifdef BVH	
bool traceBvhColor(int instanceCount, global const Instance* instances,	global const Object* objectTypes, global const BvhNode* triangleBvhNodes, global const TriangleIndices* allTriangles, global const Vertex* allVertices,	Ray ray, Vertex* intersectionPoint);
#endif


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

void kernel rayTraceAdvanced(
	int objectCount,
#ifdef BVH
	global const Instance* instances,
#endif
	global const Object* objects,
	
#ifdef BVH
	global const BvhNode* triangleBvhNodes,
#endif
	
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
	bool wasHit = traceBvhColor(objectCount, instances, objects, triangleBvhNodes, triangles, vertices, ray, &intersectionPoint);
#else
	bool wasHit = traceBruteForceColor(objectCount, objects, triangles, vertices, ray, &intersectionPoint);
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
	global const Instance* instances,
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
		
		Instance instance = instances[instanceIndex];
		Object objectType = objectTypes[instance.meshType];
		
		const global TriangleIndices* triangles = getTrianglesIndices(allTriangles, objectType);
		const global Vertex* vertices = getVertices(allVertices, objectType);
		
		float nearDistance, farDistance;
		
		Ray transformedRay = transformRay(instance.modelMatrix, ray);
		
		
		for (int index = objectType.bvhRootNodeIndex; index < objectType.bvhRootNodeIndex + objectType.bvhTreeSize; ) {
			BvhNode currentNode = triangleBvhNodes[index];
			//currentNode.aabb.max += (float3)(1) * 0.01f;
			//currentNode.aabb.min -= (float3)(1) * 0.01f;
			
			bool isHit = intersectsBox(transformedRay, currentNode.aabb, &nearDistance, &farDistance)/* && (nearDistance < closestTriangleDist)*/;
			bool isLeaf = (currentNode.escapeIndex == -1);
			
			if(/*isHit && */isLeaf){
				Triangle triangle = getTriangle(triangles, vertices, objectType, currentNode.index);

				float distance = 1;
				float2 uv = (float2)(0.5f, 0.5f);
				if (intersectsTriangle(transformedRay, triangle, &distance, &uv)/* && distance < closestTriangleDist*/) {
					closestTriangleDist = distance;
					closestTriangle = triangle;
					closestUv = uv;
					//printf("Current index: %d, escIndex: %d, objectType.bvhTreeSize: %d, objectType.bvhRootNodeIndex: %d, currentNode.index: %d\n", index, currentNode.escapeIndex, objectType.bvhTreeSize, objectType.bvhRootNodeIndex, currentNode.index);
				}
				
			}
			//if(isLeaf)
			//	printf("Current index: %d, escIndex: %d, objectType.bvhTreeSize: %d, objectType.bvhRootNodeIndex: %d, currentNode.index: %d\n", index, currentNode.escapeIndex, objectType.bvhTreeSize, objectType.bvhRootNodeIndex, currentNode.index);
			//if(isHit)
			//	printf("Current index: %d, escIndex: %d, objectType.bvhTreeSize: %d, objectType.bvhRootNodeIndex: %d\n", index, currentNode.escapeIndex, objectType.bvhTreeSize, objectType.bvhRootNodeIndex);
			
			if(isHit || isLeaf){//Proceed to left child if was hit and has child, if is leaf then sibling will be at same index as left child would've been
				index++;
			}else{
				index += currentNode.escapeIndex;
			}
			
		}
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





bool traceBruteForceColor(int objectCount, global const Object* allObjects, global const TriangleIndices* allTriangles, global const Vertex* allVertices, Ray ray, Vertex* intersectionPoint) {
	float closestTriangleDist = FLT_MAX;
	Triangle closestTriangle;
	float2 closestUv;
	
	#if __OPENCL_VERSION__ > 120
	local const TriangleIndices triangles[MAX_INDICES_PER_OBJECT];
	local const Vertex vertices[MAX_VERTICES_PER_OBJECT];
	#endif
		
	
#if 0	//Only for debug
	local ObjectInfo objectInfoList[MAX_OBJECTS_IN_RAY];
	
	int broadPhaseObjectCount;
	sortObjects(ray, objectCount, allObjects, objectInfoList, &broadPhaseObjectCount);
	
#endif

	for (int objectIndex = 0; objectIndex < objectCount; objectIndex++) {
		#if __OPENCL_VERSION__ <= 120
		const global TriangleIndices* triangles;
		const global Vertex* vertices;
		#endif
		
		Object object = allObjects[objectIndex];
		
		float nearDistacnce, farDistance;
		
		
#if __OPENCL_VERSION__ > 120
		if (!work_group_any(intersectsBox(ray, object.boundingBox, &nearDistacnce, &farDistance) && nearDistacnce < closestTriangleDist))/* ))*/
			continue;
#else
		if (!intersectsBox(ray, object.boundingBox, &nearDistacnce, &farDistance) || !(nearDistacnce < closestTriangleDist))
			continue;
#endif
		
#if __OPENCL_VERSION__ > 120
		event_t triEvent =  async_work_group_copy((local float*)triangles, (global float*)getTrianglesIndices(allTriangles, object), sizeof(TriangleIndices) / sizeof(float) * object.numTriangles, 0);
		event_t vertEvent = async_work_group_copy((local float*)vertices, (global float*)getVertices(allVertices, object), sizeof(Vertex) / sizeof(float) * object.numVertices, 0);
		
		wait_group_events(1, &triEvent);
		wait_group_events(1, &vertEvent);
		
#else
		triangles = getTrianglesIndices(allTriangles, object);
		vertices = getVertices(allVertices, object);
#endif
		
		
		
		for (int triangleIndex = 0; triangleIndex < object.numTriangles; triangleIndex++) {
			Triangle triangle = getTriangle(triangles, vertices, object, triangleIndex);

			float distance;
			float2 uv;
			if (intersectsTriangle(ray, triangle, &distance, &uv) && distance < closestTriangleDist) {
				closestTriangleDist = distance;
				closestTriangle = triangle;
				closestUv = uv;
			}
		}
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


Vertex interpolateTriangle(Triangle triangle, float2 uv){
	Vertex result;
	result.position = interpolate3(triangle.a.position, triangle.b.position, triangle.c.position, uv);
	result.normal = interpolate3(triangle.a.normal, triangle.b.normal, triangle.c.normal, uv);
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