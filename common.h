#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#define internal static
#define global_variable static
#define local_persist static

#define SQRT2	1.41421356237f
#define PI		3.14159265359f
#define DEG2RAD_CONSTANT	PI / 180.f
#define RAD2DEG_CONSTANT	180.f / PI

#define CLIP(X, A, B) ((X < A) ? A : ((X > B) ? B : X))
#define ABS(X) (X < 0 ? -X : X)