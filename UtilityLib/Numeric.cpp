#include "Numeric.h"

bool isPerp(const Vector3& v0, const Vector3& v1)
{
    return fabs(dot(v0, v1)) < ZERO_TOLERANCE_LOW;
}
