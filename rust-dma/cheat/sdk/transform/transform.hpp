#pragma once
#include "../../../include.hpp"

class Transform
{
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

	class trsX
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

	template <typename T>
	class ResizableBuffer
	{
	public:
		T* buffer;
		int capacity;

		ResizableBuffer() : buffer(nullptr), capacity(200)
		{
			buffer = new T[capacity];
		}

		void updateBuffer(HANDLE scatter_handle, uintptr_t address, int capacity)
		{
			if (capacity > this->capacity)
			{
				if (buffer)
					delete[] buffer;
			
				buffer = new T[capacity];
				this->capacity = capacity;
			}

			dma.AddScatterRead(scatter_handle, address, buffer, capacity * sizeof(T));
		}

		T& operator[](int index)
		{
			if (index >= capacity)
				throw std::out_of_range("Index out of bounds: forgot to call updateBuffer?");
			return buffer[index];
		}
	};

public:
	ResizableBuffer<trsX> trsBuffer;
	ResizableBuffer<int> parentIndicesBuffer;

	uintptr_t address;
	uintptr_t address_internal;

	TransformAccess transformAccess;

	uintptr_t localTransforms;
	uintptr_t parentIndices;


	void updateTrsXBuffer(HANDLE scatter_handle)
	{
		trsBuffer.updateBuffer(scatter_handle, localTransforms, transformAccess.index + 1);
	}

	void updateParentIndicesBuffer(HANDLE scatter_handle)
	{
		parentIndicesBuffer.updateBuffer(scatter_handle, parentIndices, transformAccess.index + 1);
	}

	Vector3 position()
	{
		Vector3 worldPos = trsBuffer[transformAccess.index].t;
		int index = parentIndicesBuffer[transformAccess.index];
		int max = 0;
		while (index >= 0)
		{
			if (max++ > 10000) 
				break;
			auto& parent = trsBuffer[index];

			worldPos = parent.q * worldPos;
			worldPos = worldPos * parent.s;
			worldPos = worldPos + parent.t;

			index = parentIndicesBuffer[index];
		}
		return worldPos;
	}

	Vector4 rotation()
	{
		Vector4 worldRot = trsBuffer[transformAccess.index].q;
		int index = parentIndicesBuffer[transformAccess.index];
		int max = 0;
		while (index >= 0)
		{
			if (max++ > 10000)
				break;
			auto& parent = trsBuffer[index];

			worldRot = parent.q * worldRot;

			index = parentIndicesBuffer[index];
		}
		return worldRot;
	}
};