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

		ResizableBuffer() : buffer(nullptr), capacity(100)
		{
			buffer = new T[capacity];
		}

		void updateBuffer(uintptr_t address, int capacity)
		{
			if (capacity > this->capacity)
			{
				if (buffer)
					delete[] buffer;

				buffer = new T[capacity];
				this->capacity = capacity;
			}

			dma.Read((void*)address, (void*)buffer, sizeof(T) * capacity);
		}

		T& operator[](int index)
		{
			if (index >= capacity)
				throw std::out_of_range("Index out of bounds: forgot to call updateBuffer?");
			return buffer[index];
		}
	};

public:
	// use the same buffer for all transforms. NOTE: if you're multithreaded, probably mutex these or store buffer another way
	inline static ResizableBuffer<trsX> trsBuffer;
	inline static ResizableBuffer<int> parentIndicesBuffer;

	uintptr_t address;
	uintptr_t address_internal;

	TransformAccess transformAccess;

	uintptr_t localTransforms;
	uintptr_t parentIndices;

	void updateTrsXBuffer() const
	{
		trsBuffer.updateBuffer(localTransforms, transformAccess.index + 1);
	}

	void updateParentIndicesBuffer() const
	{
		parentIndicesBuffer.updateBuffer(parentIndices, transformAccess.index + 1);
	}

	Vector3 localPosition() const
	{
		return dma.Read<trsX>(localTransforms + transformAccess.index * sizeof(trsX)).t;
	}

	Vector3 localScale() const
	{
		return dma.Read<trsX>(localTransforms + transformAccess.index * sizeof(trsX)).s;
	}

	Vector4 localRotation() const
	{
		return dma.Read<trsX>(localTransforms + transformAccess.index * sizeof(trsX)).q;
	}

	Vector3 position() const
	{
		Vector3 worldPos = trsBuffer[transformAccess.index].t;
		int index = parentIndicesBuffer[transformAccess.index];
		while (index >= 0)
		{
			auto parent = trsBuffer[index];

			worldPos = parent.q * worldPos;
			worldPos = worldPos * parent.s;
			worldPos = worldPos + parent.t;

			index = parentIndicesBuffer[index];
		}

		return worldPos;
	}

	Vector4 rotation() const
	{
		Vector4 worldRot = trsBuffer[transformAccess.index].q;
		int index = parentIndicesBuffer[transformAccess.index];
		while (index >= 0)
		{
			auto parent = trsBuffer[index];

			worldRot = parent.q * worldRot;

			index = parentIndicesBuffer[index];
		}

		return worldRot.normalized();
	}

	Vector3 right() const
	{
		static Vector3 right = { 1, 0, 0 };
		return rotation() * right;
	}

	Vector3 up() const
	{
		static Vector3 up = { 0, 1, 0 };
		return rotation() * up;
	}

	Vector3 forward() const
	{
		static Vector3 forward = { 0, 0, 1 };
		return rotation() * forward;
	}

	Vector3 TransformDirection(Vector3 localDirection) const
	{
		return rotation() * localDirection;
	}

	Vector3 InverseTransformDirection(Vector3 worldDirection) const
	{
		return rotation().conjugate() * worldDirection;
	}

	Vector3 TransformPoint(Vector3 localPoint) const
	{
		Vector3 worldPos = localPoint;
		int index = transformAccess.index;
		while (index >= 0)
		{
			auto parent = trsBuffer[index];

			worldPos = worldPos * parent.s;
			worldPos = parent.q * worldPos;
			worldPos = worldPos + parent.t;

			index = parentIndicesBuffer[index];
		}

		return worldPos;
	}

	Vector3 InverseTransformPoint(Vector3 worldPoint) const
	{
		Vector3 worldPos = trsBuffer[transformAccess.index].t;
		Vector4 worldRot = trsBuffer[transformAccess.index].q;

		Vector3 localScale = trsBuffer[transformAccess.index].s;

		int index = parentIndicesBuffer[transformAccess.index];
		while (index >= 0)
		{
			auto parent = trsBuffer[index];

			worldPos = parent.q * worldPos;
			worldPos = worldPos * parent.s;
			worldPos = worldPos + parent.t;

			worldRot = parent.q * worldRot;

			index = parentIndicesBuffer[index];
		}

		Vector3 local = worldRot.conjugate() * (worldPoint - worldPos);
		return local / localScale;

		return worldPos;
	}
};