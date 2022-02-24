#include "Raycasting.h"

bool Loki_Raycasting::DoRayCast(RE::Actor* a_actor, RE::hkVector4 a_from, RE::hkVector4 a_to) {
    RE::hkpWorldRayCastInput input = { a_from, a_to, false, 0 };
    RE::hkpWorldRayCastOutput output = {};

    uint32_t collisionLayerMark = 0xFFFFFFFF - 0x1F;
    auto collisionResult = 0 + (collisionLayerMark & input.filterInfo);
    input.filterInfo = collisionResult;

    auto bhkWorld = a_actor->parentCell->GetbhkWorld();
    auto hkpWorld = bhkWorld->GetWorld();
    hkpWorld->CastRay(input, output);
    return output.HasHit();
}

RE::NiPoint3 Loki_Raycasting::GetForwardVector(RE::NiPoint3 eulerIn) {
    float pitch = eulerIn.x;
    float yaw = eulerIn.z;

    return RE::NiPoint3(
        sin(yaw) * cos(pitch),
        cos(yaw) * cos(pitch),
        sin(pitch));
}

RE::NiPoint3 Loki_Raycasting::RotateVector(RE::NiQuaternion quatIn, RE::NiPoint3 vecIn) {
    float num = quatIn.x * 2.0f;
    float num2 = quatIn.y * 2.0f;
    float num3 = quatIn.z * 2.0f;
    float num4 = quatIn.x * num;
    float num5 = quatIn.y * num2;
    float num6 = quatIn.z * num3;
    float num7 = quatIn.x * num2;
    float num8 = quatIn.x * num3;
    float num9 = quatIn.y * num3;
    float num10 = quatIn.w * num;
    float num11 = quatIn.w * num2;
    float num12 = quatIn.w * num3;
    RE::NiPoint3 result;
    result.x = (1.0f - (num5 + num6)) * vecIn.x + (num7 - num12) * vecIn.y + (num8 + num11) * vecIn.z;
    result.y = (num7 + num12) * vecIn.x + (1.0f - (num4 + num6)) * vecIn.y + (num9 - num10) * vecIn.z;
    result.z = (num8 - num11) * vecIn.x + (num9 + num10) * vecIn.y + (1.0f - (num4 + num5)) * vecIn.z;
    return result;
}

RE::NiPoint3 Loki_Raycasting::GetForwardVector(RE::NiQuaternion quatIn) {
    return RotateVector(quatIn, RE::NiPoint3(0.0f, 1.0f, 0.0f));
}