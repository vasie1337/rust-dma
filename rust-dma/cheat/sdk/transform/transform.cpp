#include "../../../include.hpp"

void Transform::UpdateTrsXBuffer(HANDLE scatter_handle) const 
{
	trsBuffer.updateBuffer(scatter_handle, transformArrays.localTransforms, transformAccess.index + 1);
}

void Transform::UpdateParentIndicesBuffer(HANDLE scatter_handle) const
{
	parentIndicesBuffer.updateBuffer(scatter_handle, transformArrays.parentIndices, transformAccess.index + 1);
}

Vector3 Transform::CalculatePosition()
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
	return worldPos;
}

Vector4 Transform::CalculateRotation()
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
	return worldRot;
}