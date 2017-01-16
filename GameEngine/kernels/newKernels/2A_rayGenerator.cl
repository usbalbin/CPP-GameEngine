
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