#pragma once
#include "TrueClimbing/APIs/TrueHUDAPI.h"

namespace Loki {

    class Raycasting {

    public:
        Raycasting() {
        }
        ~Raycasting(){
        }
        static Raycasting* GetSingleton() {
            static Raycasting singleton;
            return &singleton;
        }
        virtual RE::hkpWorldRayCastOutput* DoRayCast(RE::Actor* a_actor, RE::hkVector4 a_from, RE::hkVector4 a_to);
        virtual RE::NiPoint3 GetForwardVector(RE::NiPoint3 eulerIn);
        virtual RE::NiPoint3 RotateVector(RE::NiQuaternion quatIn, RE::NiPoint3 vecIn);
        virtual RE::NiPoint3 GetForwardVector(RE::NiQuaternion quatIn);

        static inline TRUEHUD_API::IVTrueHUD3* g_TrueHUD = NULL;

    private:

    protected:

    };

};