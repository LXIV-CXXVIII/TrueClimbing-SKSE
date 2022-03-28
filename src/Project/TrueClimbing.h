#pragma once
#include "APIs/TrueDirectionalMovementAPI.h"
#include "APIs/TrueHUDAPI.h"
#include "Hooks/Hooks.h"
#include "C:/dev/simpleini-master/SimpleIni.h"
#include "Raycasting/Raycasting.h"
//#include <toml++/toml.h>

namespace Loki {

    class TrueClimbing :
        public Raycasting {

    public:
        enum ClimbStartType : int32_t {

            kSmallVault = 0,
            kMediumVault = 1,
            kLargeVault = 2,
            kClimbFromGround = 3,
            kClimbFromAir = 4,

        };

        enum ClimbEndType : int32_t {

            kStepOut = 0,
            kFallOut = 1,
            kJumpForwards = 2,
            kJumpBackwards = 3,

        };

        TrueClimbing();
        virtual ~TrueClimbing();
        static TrueClimbing* GetSingleton();

        float rayCastDist;
        float rayCastLowVaultDist, rayCastMediumVaultDist, rayCastHighVaultDist, rayCastClimbDist; // 00, 04, 08, 0C
        float rayCastLowVaultHeight, rayCastMediumVaultHeight, rayCastHighVaultHeight, rayCastClimbHeight; // 10, 14, 18, 1C

        static inline TDM_API::IVTDM2* g_TDM = NULL;
        static inline TRUEHUD_API::IVTrueHUD3* g_TrueHUD = NULL;
        bool tControl;

        static void ControllerSubroutine(RE::bhkCharacterController* a_controller);
        static void bhkCharacterStateClimbing_SimPhys(RE::bhkCharacterStateClimbing* a_climbing, RE::bhkCharacterController* a_controller);
        static void Update(RE::Actor* a_actor);

        static inline REL::Relocation<decltype(Update)> _Update;

    private:

    protected:

    };

};