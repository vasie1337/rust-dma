#pragma once
#include "../../../include.hpp"

enum BoneList
{
    l_knee = 2,
    l_foot = 3,
    r_knee = 14,
    r_foot = 15,
    spine1 = 18,
    l_forearm = 25,
    l_hand = 26,
    neck = 46,
    head = 47,
    r_forearm = 56,
    r_hand = 57
};

class Bone
{
public:
    Bone() = default;

    std::uintptr_t transform = 0;
    std::uintptr_t transform_internal = 0;
    std::uintptr_t transform_access_readonly = 0;
    std::uintptr_t transform_array = 0;
    std::uintptr_t transform_indices = 0;

    std::uint32_t index = 0;
    std::int32_t transform_index = 0;

    __m128 result = { 0, 0, 0, 0 };

    std::vector<uintptr_t> transform_array_reads;
    std::vector<Matrix3x4> matrix_reads;

private:
    int MAX_ITERATIONS = 32;

    __m128 mulVec0 = { -2.000f, 2.000f, -2.000f, 0.000f };
    __m128 mulVec1 = { 2.000f, -2.000f, -2.000f, 0.000f };
    __m128 mulVec2 = { -2.000f, -2.000f, 2.000f, 0.000f };

public:
    int GetIterationCount(int initial_transform_index) const
    {
        int iteration_count = 0;
        int current_transform_index = initial_transform_index;

        while (iteration_count < MAX_ITERATIONS && current_transform_index >= 0) {
            current_transform_index = dma.Read<int>(transform_indices + 0x4 * current_transform_index);
            ++iteration_count;
        }

        return iteration_count;
    }

    void Transform()
    {
        result = _mm_setzero_ps();

        for (const auto& matrix34 : matrix_reads)
        {
            __m128 xxxx = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x00));
            __m128 yyyy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x55));
            __m128 zwxy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x8E));
            __m128 wzyw = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xDB));
            __m128 zzzz = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xAA));
            __m128 yxwy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x71));
            __m128 tmp7 = _mm_mul_ps(*(__m128*)(&matrix34.vec2), result);

            result = _mm_add_ps(
                _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(
                            _mm_sub_ps(
                                _mm_mul_ps(_mm_mul_ps(xxxx, mulVec1), zwxy),
                                _mm_mul_ps(_mm_mul_ps(yyyy, mulVec2), wzyw)),
                            _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0xAA))),
                        _mm_mul_ps(
                            _mm_sub_ps(
                                _mm_mul_ps(_mm_mul_ps(zzzz, mulVec2), wzyw),
                                _mm_mul_ps(_mm_mul_ps(xxxx, mulVec0), yxwy)),
                            _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x55)))),
                    _mm_add_ps(
                        _mm_mul_ps(
                            _mm_sub_ps(
                                _mm_mul_ps(_mm_mul_ps(yyyy, mulVec0), yxwy),
                                _mm_mul_ps(_mm_mul_ps(zzzz, mulVec1), zwxy)),
                            _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x00))),
                        tmp7)), *(__m128*)(&matrix34.vec0));
        }
    }

    Vector3 GetPosition()
    {
        return Vector3(result.m128_f32[0], result.m128_f32[1], result.m128_f32[2]);
    }
};

#define max_bones 60

struct BoneConnection {
    Vector3 start;
    Vector3 end;
};