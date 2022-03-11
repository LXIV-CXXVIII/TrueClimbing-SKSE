#pragma once
#include "TrueDirectionalMovementAPI.h"
#include "Raycasting/Raycasting.h"
#include "C:/dev/simpleini-master/SimpleIni.h"
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
        static void* CodeAllocation(Xbyak::CodeGenerator& a_code, SKSE::Trampoline* t_ptr);
        static TrueClimbing* GetSingleton();

        static void InstallUpdateHook();
        static void InstallSimulateClimbingHook();
        static void InstallClimbSimHook();

        float rayCastDist;
        float rayCastLowVaultDist, rayCastMediumVaultDist, rayCastHighVaultDist, rayCastClimbDist; // 00, 04, 08, 0C
        float rayCastLowVaultHeight, rayCastMediumVaultHeight, rayCastHighVaultHeight, rayCastClimbHeight; // 10, 14, 18, 1C

    private:
        static void ControllerSubroutine(RE::bhkCharacterController* a_controller);
        static void bhkCharacterStateClimbing_SimPhys(RE::bhkCharacterStateClimbing* a_climbing, RE::bhkCharacterController* a_controller);
        static void Update(RE::Actor* a_actor);

        static inline REL::Relocation<decltype(Update)> _Update;

    protected:

    };

};