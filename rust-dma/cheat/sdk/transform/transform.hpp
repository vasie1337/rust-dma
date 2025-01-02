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

	void updateBuffer(HANDLE scatter_handle, uintptr_t address, int capacity)
	{
		if (capacity > Capacity) {
			throw std::out_of_range("Requested capacity exceeds buffer size");
		}
		dma.AddScatterRead(scatter_handle, address, buffer, capacity * sizeof(T));
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

	uintptr_t localTransforms;
	uintptr_t parentIndices;

	void UpdateTrsXBuffer(HANDLE scatter_handle);
	void UpdateParentIndicesBuffer(HANDLE scatter_handle);

	Vector3 Position();
	Vector4 Rotation();
};