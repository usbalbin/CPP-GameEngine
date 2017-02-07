

#include "kernels/containers.h"
#include "kernels/math.h"

#define gid get_global_id(0)

Vertex transformVertex(float16 matrix, Vertex vertex);

void kernel vertexShader(
	//In
	global const Object* objectTypes,
	global const Instance* instances,
	global const Vertex* objectTypesVertices,

	//Out
	global Object* objects,
	global Vertex* vertices
){
	Instance instance = instances[gid];
	Object objectType = objectTypes[instance.meshType];
	float16 modelMatrix = instance.modelMatrix;

	/*if(gid == 0){
		printf("instances[1].startTriangle: %d\n", instances[1].startVertex);
	}*/
	
	Object mesh;
	mesh.startTriangle = objectType.startTriangle;
	mesh.startVertex = instance.startVertex;
	mesh.numTriangles = objectType.numTriangles;
	mesh.numVertices = objectType.numVertices;
	
	objects[gid] = mesh;

	for(int i = 0; i < objectType.numVertices; i++){
		vertices[mesh.startVertex + i] = 
			transformVertex(
				modelMatrix, 
				objectTypesVertices[objectType.startVertex + i]
			);
	}
}
#include "kernels/containers.h"

#define gid get_global_id(0)

void kernel aabb(
	//In
	global const 
	Vertex* vertices,
	
	//In and Out
	global Object* objects
	
){
	
	
	Object mesh = objects[gid];
	
	AABB aabb = mesh.boundingBox;
	aabb.min = aabb.max = vertices[mesh.startVertex].position;
	
	for(int i = 1; i < mesh.numVertices; i++){
		aabb.min = min(aabb.min, vertices[mesh.startVertex + i].position);
		aabb.max = max(aabb.max, vertices[mesh.startVertex + i].position);
	}
	objects[gid].boundingBox = aabb;
	
}
#include "kernels/containers.h"
#include "kernels/oldKernels/rayTracer.h"

#define xIndex get_global_id(0)
#define yIndex get_global_id(1)
#define width get_global_size(0)
#define height get_global_size(1)

float4 testColor();

void kernel rayTracer(
	int objectCount,
	float16 matrix,
	global const Object* objects,
	global const TriangleIndices* triangles,
	global const Vertex* vertices,
	__write_only image2d_t output
){
	
	int2 pos = (int2)(xIndex, height - yIndex);
	float4 color;
	
	//if(pos.x < 15 || pos.x > width - 15 - 1)
		//return;
	
	//float4 color = (float4)(1, 0, 0, 1);
	//float4 color = (float4)(pos.x / 1024.0f, pos.y / 768.0f, 0, 1);
	
	
	color = traceRay(objectCount, objects, triangles, vertices, matrix);
	//color = testColor();
	write_imagef(output, pos, color.xyzw);
}


















float4 testColor(){
	float3 res = mix(
		mix((float3)(1, 0, 0), (float3)(0, 1, 0), xIndex / (float)width),
		mix((float3)(0, 0, 0), (float3)(0, 0, 1), xIndex / (float)width),
		yIndex / (float)height
	);
	return (float4)(res, 1);
}

kernel void debug(
	volatile global /*atomic_int*/int* counter
){

	//atomic_fetch_add(counter, 1);
	/*Vertex v;
	int reflectlLoc = (float*)&v.reflectFactor - (float*)&v.color;
	int refractlLoc = (float*)&v.refractFactor - (float*)&v.color;
	printf(
		"reflectlLoc: %d\n"
		"refractlLoc: %d\n",
		reflectlLoc,
		refractlLoc
	);*/

}
#include "kernels/math.h"

#define xIndex get_global_id(0)
#define yIndex get_global_id(1)
#define width get_global_size(0)
#define height get_global_size(1)

Ray genPerspectiveRay(float16 matrix);

void kernel perspectiveRayGenerator(float16 matrix, global Ray* rays) {
	rays[yIndex * width + xIndex] = genPerspectiveRay(matrix);
}

Ray genPerspectiveRay(float16 matrix) {
	Ray ray;
	
	
	ray.position = mulMatVec(matrix, (float4)(0.0f, 0.0f, 0.0f, 1.0f)).xyz;

	
	
	float sideToSide = (float)xIndex / width;
	float topToBottom = (float)yIndex / height;

	float3 tl = (float3)(-1.0f, +1.0f, -1.0f); float3 tr = (float3)(+1.0f, +1.0f, -1.0f);
	float3 bl = (float3)(-1.0f, -1.0f, -1.0f); float3 br = (float3)(+1.0f, -1.0f, -1.0f);

	ray.direction = normalize(
		mix(
			mix(tl, tr, sideToSide),
			mix(bl, br, sideToSide),
			topToBottom
		)
	);

	ray.direction = mulMatVec(matrix, (float4)(ray.direction, 0.0f)).xyz;
	
	ray.inverseDirection = 1.0f / ray.direction;
	return ray;
}
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
	
	read_only image2d_t texture0
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
		
		
		
		float2 textureCoords = intersectionPoint.position.xz * 0.02f;
		
		float blendWidth = 5;
		float blendDistance = 10;
	
		if(closestInstance.texture[0] > 0){
			intersectionPoint.color += mix(
				((float)closestInstance.texture[0] / SHRT_MAX) * read_imagef(texture0, sampler, textureCoords * 8).xyzw,
				((float)closestInstance.texture[0] / SHRT_MAX) * read_imagef(texture0, sampler, textureCoords).xyzw,
				(float4)(clamp((closestTriangleDist - blendDistance) / blendWidth,0.0f, 1.0f))
			);
		}
		
		
		
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
	hit.vertex.color = (float4)(1.0f, 0.41f, 0.71f, 1.0f);
	hit.vertex.reflectFactor = 0.0f;
	hit.vertex.refractFactor = 0.0f;
	return hit;
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

#if __OPENCL_VERSION__ > 120
void summarizeRays200(global Ray* results, volatile global atomic_int* globalResultCount, Ray result, bool hasResult, int* indexOut);
void summarizeRaysNewer(global Ray* results, volatile global atomic_int* globalResultCount, Ray reflection, Ray refraction, int hasReflection, int hasRefraction, int* reflectIndexOut, int* refractIndexOut);
#else
void summarizeRays120(global Ray* results, volatile global int* globalResultCount, Ray result, bool hasResult, int* indexOut);
#endif


void kernel rayGenerator(
	global const Hit* hits,
#if __OPENCL_VERSION__ > 120
	volatile global atomic_int* rayIndex,
#else
	volatile global int* rayIndex,
#endif
	global Ray* raysOut,
	global RayTree* rayTrees
){
	Hit hit = hits[gid];


	bool hasReflection = hit.vertex.reflectFactor > 0;
	bool hasRefraction = hit.vertex.refractFactor > 0;
	int reflectionIndex = -1;
	int refractionIndex = -1;
	
	
	
	Ray reflection = reflect(hit);
	Ray refraction = refract(hit);
	
	
	/*volatile local atomic_int groupResultCount;
	if(get_local_id(0)==0){																			// First worker will initialize groupResultCount to 0
        atomic_init(&groupResultCount, 0);
		//groupResultCount = 0;
    }
    barrier(CLK_LOCAL_MEM_FENCE);*/
#if __OPENCL_VERSION__ > 120
	summarizeRays200(raysOut, rayIndex, reflection, hasReflection, &reflectionIndex);//, &groupResultCount);
#else
	summarizeRays120(raysOut, rayIndex, reflection, hasReflection, &reflectionIndex);
#endif
	
	
	/*
	if(get_local_id(0)==0){																			// First worker will initialize groupResultCount to 0
        atomic_init(&groupResultCount, 0);
		//groupResultCount = 0;
    }
    barrier(CLK_LOCAL_MEM_FENCE);*/
#if __OPENCL_VERSION__ > 120
	summarizeRays200(raysOut, rayIndex, refraction, hasRefraction, &refractionIndex);//, &groupResultCount);
#else
	summarizeRays120(raysOut, rayIndex, refraction, hasRefraction, &refractionIndex);
#endif
	//summarizeRaysNewer(raysOut, rayIndex, reflection, refraction, hasReflection, hasRefraction, &reflectionIndex, &refractionIndex);
	
	rayTrees[gid].reflectIndex = reflectionIndex;
	rayTrees[gid].refractIndex = refractionIndex;
}


#if __OPENCL_VERSION__ > 120


void summarizeRays200(global Ray* results, volatile global atomic_int* globalResultCount, Ray result, bool hasResult, int* indexOut){
	local int groupIndex;
	int privateIndex;
	
	bool someInGroupHasResult = work_group_any(hasResult);
	
	if(someInGroupHasResult){
		bool allInGroupHasResult = work_group_all(hasResult);
		privateIndex = allInGroupHasResult ?
			get_local_id(0) :
			work_group_scan_exclusive_add(hasResult ? 1 : 0);
		
		barrier(CLK_LOCAL_MEM_FENCE);
			
		if(get_local_id(0) == get_local_size(0) - 1){
			int groupResultCount = privateIndex + (hasResult ? 1 : 0);
			groupIndex = atomic_fetch_add(globalResultCount, groupResultCount);
		}
	}
	
	
	barrier(CLK_LOCAL_MEM_FENCE);
	if(hasResult){
		int index = groupIndex + privateIndex;
		*indexOut = index;
		results[index] = result;
	}
}

void summarizeRaysNewer(global Ray* results, volatile global atomic_int* globalResultCount, Ray reflection, Ray refraction, int hasReflection, int hasRefraction, int* reflectIndexOut, int* refractIndexOut){
	local int groupIndex;
	int privateIndex;
	
	bool someInGroupHasResult = work_group_any(hasReflection | hasRefraction);
	
	if(someInGroupHasResult){
		bool allInGroupHasResult = work_group_all(hasReflection | hasRefraction);
		privateIndex = allInGroupHasResult ?
			get_local_id(0) :
			work_group_scan_exclusive_add(hasReflection + hasRefraction);
		
		barrier(CLK_LOCAL_MEM_FENCE);
			
		if(get_local_id(0) == get_local_size(0) - 1){
			int groupResultCount = privateIndex + (hasReflection + hasRefraction);
			groupIndex = atomic_fetch_add(globalResultCount, groupResultCount);
		}
	}
	
	
	barrier(CLK_LOCAL_MEM_FENCE);
	int index = groupIndex + privateIndex;
	if(hasReflection){
		*reflectIndexOut = index;
		results[index] = reflection;
	}
	if(hasRefraction){
		index += hasRefraction;
		*refractIndexOut = index;
		results[index] = refraction;
	}
}

#else

void summarizeRays120(global Ray* results, volatile global int* globalResultCount, Ray result, bool hasResult, int* indexOut){
	if(hasResult){
		int index = atomic_add(globalResultCount, 1);
		*indexOut = index;
		results[index] = result;
	}
	
}

#endif







void kernel treeTraverser(
	global RayTree* rayTrees,
	global RayTree* childRayTrees
){
	int reflectIndex = rayTrees[gid].reflectIndex;
	int refractIndex = rayTrees[gid].refractIndex;
	
	float4 surfaceColor = rayTrees[gid].color;
	
	if(reflectIndex != -1){
		rayTrees[gid].color += surfaceColor * rayTrees[gid].reflectFactor * childRayTrees[reflectIndex].color;
	}
	
	if(refractIndex != -1){
		rayTrees[gid].color += surfaceColor * rayTrees[gid].refractFactor * childRayTrees[refractIndex].color;
		
	}
	/*
	printf("gid: %d\nreflectIndex: %d\nrefractIndex: %d\n\n",
		get_global_id(0),
		reflectIndex,
		refractIndex
	);*/
}
#define xIndex get_global_id(0)
#define yIndex get_global_id(1)
#define width get_global_size(0)
#define height get_global_size(1)


void kernel colorToPixel(
	global const RayTree* rayTrees,
	__write_only image2d_t output
){
	int2 pos = (int2)(xIndex, height - yIndex);
	float4 color = rayTrees[yIndex * width + xIndex].color;
	
	
	
	
	
	write_imagef(output, pos, color.xyzw);
}

void kernel colorToColor(
	global const RayTree* rayTrees,
	global float4* output
){
	output[(height - yIndex - 1) * width + xIndex] = rayTrees[yIndex * width + xIndex].color;
}