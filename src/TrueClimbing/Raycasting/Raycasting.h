#pragma once

namespace Loki {

    class Raycasting {

    public:
        Raycasting();
        ~Raycasting();
        virtual bool DoRayCast(RE::Actor* a_actor, RE::hkVector4 a_from, RE::hkVector4 a_to);
        virtual RE::NiPoint3 GetForwardVector(RE::NiPoint3 eulerIn);
        virtual RE::NiPoint3 RotateVector(RE::NiQuaternion quatIn, RE::NiPoint3 vecIn);
        virtual RE::NiPoint3 GetForwardVector(RE::NiQuaternion quatIn);

    private:

    protected:

    };

};