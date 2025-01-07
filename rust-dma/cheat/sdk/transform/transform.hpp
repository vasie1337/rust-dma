#pragma once
#include "../../../include.hpp"

class TransformAccess
{
public:
	uintptr_t hierarchyAddr;
	int index;

	operator bool() const
	{
		return hierarchyAddr != 0;
	}
};

class TransformArrays
{
public:
	uintptr_t localTransforms;
	uintptr_t parentIndices;

	operator bool() const
	{
		return localTransforms != 0 && parentIndices != 0;
	}
};

class TRSX
{
public:
	Vector3 t;
private:
	char pad_000C[4];
public:
	Vector4 q;
	Vector3 s;
private:
	char pad_0014[4];
};

template <typename T, int Capacity = 200>
class StackBuffer
{
public:
	T buffer[Capacity]; 
	static constexpr int capacity = Capacity;

	StackBuffer() = default;

	void updateBuffer(HANDLE scatter_handle, uintptr_t address, int capacity) const
	{
		if (capacity > Capacity) {
			return;
		}
		dma.AddScatterRead(scatter_handle, address, (void*)buffer, capacity * sizeof(T));
	}

	T& operator[](int index)
	{
		if (index >= capacity)
			return buffer[0];
		return buffer[index];
	}
};

class Transform
{
public:
	StackBuffer<TRSX> trsBuffer;
	StackBuffer<int> parentIndicesBuffer;

	uintptr_t address;
	uintptr_t address_internal;

	TransformAccess transformAccess;
	TransformArrays transformArrays;

	void UpdateTrsXBuffer(HANDLE scatter_handle);
	void UpdateParentIndicesBuffer(HANDLE scatter_handle);

	void CalculatePosition();
	void CalculateRotation();

	Vector3 cached_position;
	Vector4 cached_rotation;
};