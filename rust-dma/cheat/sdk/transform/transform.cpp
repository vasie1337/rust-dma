#include "../../../include.hpp"

void Transform::UpdateTrsXBuffer(HANDLE scatter_handle) 
{
	trsBuffer.updateBuffer(scatter_handle, transformArrays.localTransforms, transformAccess.index + 1);
}

void Transform::UpdateParentIndicesBuffer(HANDLE scatter_handle)
{
	parentIndicesBuffer.updateBuffer(scatter_handle, transformArrays.parentIndices, transformAccess.index + 1);
}

void Transform::CalculatePosition()
{
	Vector3 worldPos = trsBuffer[transformAccess.index].t;
	int index = parentIndicesBuffer[transformAccess.index];
	int max = 0;

	while (index >= 0 && index <= 200)
	{
		if (max++ > 200)
			break;

		auto& parent = trsBuffer[index];

		worldPos = parent.q * worldPos;
		worldPos = worldPos * parent.s;
		worldPos = worldPos + parent.t;

		index = parentIndicesBuffer[index];
	}

	cached_position = worldPos;
}

void Transform::CalculateRotation()
{
	Vector4 worldRot = trsBuffer[transformAccess.index].q;
	int index = parentIndicesBuffer[transformAccess.index];
	int max = 0;

	while (index >= 0 && index <= 200)
	{
		if (max++ > 200)
			break;

		auto& parent = trsBuffer[index];

		worldRot = parent.q * worldRot;

		index = parentIndicesBuffer[index];
	}

	cached_rotation = worldRot;
}