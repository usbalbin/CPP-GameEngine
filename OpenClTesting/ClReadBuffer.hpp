#pragma once


#include "CL\cl2.hpp"
#include <set>

/*
 * Buffer only readable from CL-Device, only writable from host
 */
template<typename T>
class ClReadBuffer
{
public:
	ClReadBuffer();
	ClReadBuffer(cl::CommandQueue& commandQueue, cl::Context& context);
	~ClReadBuffer();

	/*
	 * Because of operator returning reference, the function has to assume the element is affected.
	 * Keeping reference after flush() might lead to undefined behaviour
	 */
	T& operator[](size_t index);
	const T& operator[](size_t index)const { return hostData[index]; }
	cl::Buffer& operator() ();

	void setup(cl::CommandQueue& commandQueue, cl::Context& context);
	void push_back(T elem);
	void swap_remove(size_t index);

	void insert_back(std::vector<T>& data);
	void updateClBuffer(size_t start, size_t size);
	void flush();
	size_t capacity() { return clCapacity; }
	size_t size() { return hostData.size(); }
	void reserve(size_t capacity);
	void resize(size_t size);
	void clear();

	/*
	 * Because of function returning reference, the function has to assume the element is affected.
	 * Keeping reference after flush() might lead to undefined behaviour
	 */
	std::vector<T>& getHostData();

private:
	const double EXPAND_FACTOR = 2;
	cl::Context context;
	cl_mem_flags flags = CL_MEM_READ_ONLY;
	size_t clCapacity;
	cl::Buffer clBuffer;

	std::vector<T> hostData;
	std::set<size_t> affectedElements;
	cl::CommandQueue* commandQueue = nullptr;
	bool allAffected = false;
};

template<typename T>
inline ClReadBuffer<T>::ClReadBuffer()
{
}

template<typename T>
ClReadBuffer<T>::ClReadBuffer(cl::CommandQueue& commandQueue, cl::Context& context) :
	commandQueue(commandQueue),
	context(context),
	flags(flags),
	clCapacity(0) {}

template<typename T>
inline ClReadBuffer<T>::~ClReadBuffer()
{
}

template<typename T>
inline T & ClReadBuffer<T>::operator[](size_t index)
{
	affectedElements.insert(index);
	return hostData[index];
}

template<typename T>
inline cl::Buffer& ClReadBuffer<T>::operator()()
{
	flush();
	return clBuffer;
}

template<typename T>
inline void ClReadBuffer<T>::setup(cl::CommandQueue& commandQueue, cl::Context& context)
{
	this->commandQueue = &commandQueue;
	this->context = context;
}

template<typename T>
inline void ClReadBuffer<T>::push_back(T elem)
{
	affectedElements.insert(hostData.size());
	hostData.push_back(elem);
}

template<typename T>
inline void ClReadBuffer<T>::swap_remove(size_t index)
{
	std::swap(hostData[index], hostData.back());
	hostData.pop_back();

	affectedElements.add(index);
}

template<typename T>
inline void ClReadBuffer<T>::insert_back(std::vector<T>& data)
{
	affectedElements.insert(affectedElements.end(), size());
	hostData.insert(std::end(hostData), std::begin(data), std::end(data));
	affectedElements.insert(affectedElements.end(), size());
}

template<typename T>
inline void ClReadBuffer<T>::updateClBuffer(size_t start, size_t size)
{
	if (!size)
		return;
	if (start + size > clCapacity)
		reserve(std::max(size_t(EXPAND_FACTOR * clCapacity), size));

	commandQueue->enqueueWriteBuffer(clBuffer, CL_TRUE, sizeof(T) * start, sizeof(T) * size, &hostData[start]);
	affectedElements.clear();
	allAffected = false;
}

template<typename T>
inline void ClReadBuffer<T>::flush()
{
	if (allAffected) {
		updateClBuffer(0, size());
	}
	else {
		if (affectedElements.empty())
			return;
		
		const size_t first = *affectedElements.begin();
		const size_t last = *affectedElements.rbegin();

		const size_t size = last - first + 1;
		updateClBuffer(first, size);
	}
}

template<typename T>
inline void ClReadBuffer<T>::reserve(size_t capacity)
{
	clBuffer = cl::Buffer(context, flags, sizeof(T) * capacity);
	clCapacity = capacity;
	updateClBuffer(0, hostData.size());
}

template<typename T>
inline void ClReadBuffer<T>::resize(size_t size)
{
	hostData.resize(size);

	if (size > clSize)
		reserve(size);
}

template<typename T>
inline void ClReadBuffer<T>::clear()
{
	hostData.clear();
}

template<typename T>
inline std::vector<T>& ClReadBuffer<T>::getHostData()
{
	allAffected = true;			//All elements are to be considered affected
	return hostData;
}