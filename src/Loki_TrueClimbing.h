#pragma once
#include "Raycasting/Raycasting.h"
#include "C:/dev/ExamplePlugin-CommonLibSSE/build/simpleini-master/SimpleIni.h"

class Loki_TrueClimbing : 
    public Loki_Raycasting {

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

    Loki_TrueClimbing();
    virtual ~Loki_TrueClimbing();
    static void* CodeAllocation(Xbyak::CodeGenerator& a_code, SKSE::Trampoline* t_ptr);
    static Loki_TrueClimbing* GetSingleton();

    static void InstallUpdateHook();
    static void InstallSimulateClimbingHook();
    static void InstallClimbSimHook();

    float rayCastDist;
    float rayCastLowVaultDist, rayCastMediumVaultDist, rayCastLargeVaultDist, rayCastClimbDist; // 00, 04, 08, 0C
    float rayCastLowVaultHeight, rayCastMediumVaultHeight, rayCastLargeVaultHeight, rayCastClimbHeight; // 10, 14, 18, 1C

private:
    static void ControllerSubroutine(RE::bhkCharacterController* a_controller);
    static void bhkCharacterStateClimbing_SimPhys(RE::bhkCharacterStateClimbing* a_climbing, RE::bhkCharacterController* a_controller);
    static void Update(RE::Actor* a_actor);

    static inline REL::Relocation<decltype(Update)> _Update;

protected:

};