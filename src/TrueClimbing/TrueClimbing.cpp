#include "TrueClimbing/TrueClimbing.h"

void* Loki::TrueClimbing::CodeAllocation(Xbyak::CodeGenerator& a_code, SKSE::Trampoline* t_ptr) {
    auto result = t_ptr->allocate(a_code.getSize());
    std::memcpy(result, a_code.getCode(), a_code.getSize());
    return result;
}

Loki::TrueClimbing::TrueClimbing() {
    CSimpleIniA ini;
    ini.SetUnicode();
    auto filename = L"Data/SKSE/Plugins/loki_Climbing.ini";
    SI_Error rc = ini.LoadFile(filename);

    rayCastDist = ini.GetDoubleValue("SETTINGS", "fRayCastDistance", -1.00f);
    return;
}

Loki::TrueClimbing::~TrueClimbing() {
}

Loki::TrueClimbing* Loki::TrueClimbing::GetSingleton() {
    static Loki::TrueClimbing* singleton = new Loki::TrueClimbing();
    return singleton;
}

void Loki::TrueClimbing::InstallUpdateHook() {
    REL::Relocation<std::uintptr_t> ActorUpdate{ REL::ID(39375) };

    auto& trampoline = SKSE::GetTrampoline();
    _Update = trampoline.write_call<5>(ActorUpdate.address() + 0x8AC, Update);

    logger::info("Actor Update hook injected");
}

void Loki::TrueClimbing::InstallSimulateClimbingHook() {
    REL::Relocation<std::uintptr_t> ClimbSim{ REL::ID(78195) };

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.write_branch<5>(ClimbSim.address(), &bhkCharacterStateClimbing_SimPhys);

    logger::info("Climbing Simulation hook injected");
}

void Loki::TrueClimbing::InstallClimbSimHook() {
    REL::Relocation<std::uintptr_t> ClimbSim{ REL::ID(78195/*e1d520*/) };
    REL::Relocation<std::uintptr_t> subroutine{ REL::ID(76440/*dc08e0*/) };

    struct Patch : Xbyak::CodeGenerator {
        Patch(std::uintptr_t a_sub) {
            Xbyak::Label l1;
            Xbyak::Label l2;

            cmp(dword[rdx + 0x21C], 0x0B); // wantState
            jz(l1);
            mov(rcx, rdx);
            jmp(ptr[rip + l2]);

            L(l1);
            or_(dword[rdx + 0x218], 0x400); // set kCanJump
            xorps(xmm2, xmm2);
            //xorps  (xmm1, xmm1); //og code
            xorps(xmm1, xmm1); // custom code
            movss(xmm0, dword[rdx + 0xB4]); // velocityMod[1] ogcode Y velocity
            movaps(xmm2, ptr[rdx + 0xB0]); //->
              // xmm2[0] = (velModX)
              // xmm2[1] = (velModY)
              // xmm2[2] = (velModZ)
              // xmm2[3] = (?)
            unpcklps(xmm2, xmm0); //->
              // xmm2[0] = xmm2[0] (velModX)
              // xmm2[1] = xmm0[0] (velModY)
              // xmm2[2] = xmm2[1] (velModY)
              // xmm2[3] = xmm0[1] 0
            xorps(xmm0, xmm0);
            unpcklps(xmm2, xmm0); //->
              // xmm2[0] = xmm2[0] (velModX)
              // xmm2[1] = xmm0[0] 0
              // xmm2[2] = xmm2[1] (velModY)
              // xmm2[3] = xmm0[1] 0
            mulss(xmm2, ptr[rdx + 0xDC]); // mul outVelocity X by rotCenter X
            movaps(ptr[rdx + 0x90], xmm2); // outVelocity
            ret();

            L(l2);
            dq(a_sub);
        }
    };

    Patch patch(subroutine.address());
    patch.ready();

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.write_branch<5>(ClimbSim.address(), CodeAllocation(patch, &trampoline));
}

void Loki::TrueClimbing::ControllerSubroutine(RE::bhkCharacterController* a_controller) {
    using func_t = decltype(ControllerSubroutine);
    REL::Relocation<func_t> func{ REL::ID(76440) }; /*dc08e0*/
    return func(a_controller);
}

void Loki::TrueClimbing::bhkCharacterStateClimbing_SimPhys(RE::bhkCharacterStateClimbing* a_climbing, RE::bhkCharacterController* a_controller) {
    __m128 z128 = { 0.0f,0.0f,0.0f,0.0f };

    __m128 vMod = a_controller->velocityMod.quad;
    __m128 vModY = { a_controller->velocityMod.quad.m128_f32[1], 0.0f, 0.0f, 0.0f };

    if (a_controller->wantState == RE::hkpCharacterStateType::kClimbing) {
        a_controller->flags.set(RE::CHARACTER_FLAGS::kCanJump);
        RE::hkVector4 hkVelocityMod = {};
        hkVelocityMod.quad = _mm_unpacklo_ps(_mm_unpacklo_ps(vMod, vModY), z128);
        hkVelocityMod.quad.m128_f32[0] *= a_controller->rotCenter.quad.m128_f32[0];
        hkVelocityMod.quad.m128_f32[1] *= a_controller->rotCenter.quad.m128_f32[1];
        hkVelocityMod.quad.m128_f32[2] *= a_controller->rotCenter.quad.m128_f32[2];
        a_controller->outVelocity = hkVelocityMod;
    } 
    else {
        ControllerSubroutine(a_controller);
    }
    return;

    /**
    if (a_controller->wantState == 😈::kClimbing) {
        a_controller->flags.set(RE::CHARACTER_FLAGS::kCanJump);
        __m128 velocityMod = { a_controller->velocityMod.quad.m128_f32[1], 0.0f, 0.0f, 0.0f };
        a_controller->outVelocity.quad = _mm_unpacklo_ps(_mm_unpacklo_ps(z128, velocityMod), z128);
    } else {
        ControllerSubroutine(a_controller);
    }
    return;
    */
}

void Loki::TrueClimbing::Update(RE::Actor* a_actor) {
    auto ptr = Loki::TrueClimbing::GetSingleton();

    auto bhkWorld = a_actor->parentCell->GetbhkWorld();
    auto hkpWorld = bhkWorld->GetWorld();
    
    hkpWorld->simulationType.set(RE::hkpWorldCinfo::SimulationType::kContinuous);
    auto c = a_actor->GetCharController();
    RE::bhkCharacterController;

    if (a_actor->IsPlayerRef()) {
        auto controller = a_actor->GetCharController();
        controller->context.currentState = RE::hkpCharacterStateType::kClimbing;
        static bool isClimbing = false;

        auto CalculateForwardRaycast = [a_actor, ptr](float _dist, float _height) -> RE::hkVector4 {
            auto center = a_actor->GetCurrent3D()->worldBound.center;
            auto forwardvec = ptr->GetForwardVector(center);
            forwardvec.x *= _dist;
            forwardvec.y *= _dist;
            forwardvec.z += _height;
            auto normalized = forwardvec / forwardvec.Length();
            RE::hkVector4 hkv = { normalized.x, normalized.y, normalized.z, 0.00f };
            return hkv;
        };

        if (ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {  // do raycast
            auto center = a_actor->GetCurrent3D()->worldBound.center;
            center.z += ptr->rayCastLowVaultHeight;
            RE::hkVector4 start = { center.x, center.y, center.z, 0.00f };
            return start;
        }(), CalculateForwardRaycast(ptr->rayCastLowVaultDist, ptr->rayCastLowVaultHeight))) {
            bool jmp;
            a_actor->GetGraphVariableBool("CanJump", jmp);
            if (jmp) {
                a_actor->SetGraphVariableInt("climb_ClimbStartType", ClimbStartType::kSmallVault);
                a_actor->NotifyAnimationGraph("climb_ClimbStart");  // start vault
            }
        } 
        else if (ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {  // do raycast
            auto center = a_actor->GetCurrent3D()->worldBound.center;
            center.z += ptr->rayCastMediumVaultHeight;
            RE::hkVector4 start = { center.x, center.y, center.z, 0.00f };
            return start;
        }(), CalculateForwardRaycast(ptr->rayCastMediumVaultDist, ptr->rayCastMediumVaultHeight))) {
            bool jmp;
            a_actor->GetGraphVariableBool("CanJump", jmp);
            if (jmp) {
                a_actor->SetGraphVariableInt("climb_ClimbStartType", ClimbStartType::kMediumVault);
                a_actor->NotifyAnimationGraph("climb_ClimbStart");  // start vault
            }
        } 
        else if (ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {  // do raycast
            auto center = a_actor->GetCurrent3D()->worldBound.center;
            center.z += ptr->rayCastLargeVaultHeight;
            RE::hkVector4 start = { center.x, center.y, center.z, 0.00f };
            return start;
        }(), CalculateForwardRaycast(ptr->rayCastLargeVaultDist, ptr->rayCastLargeVaultHeight))) {
            bool jmp;
            a_actor->GetGraphVariableBool("CanJump", jmp);
            if (jmp) {
                a_actor->SetGraphVariableInt("climb_ClimbStartType", ClimbStartType::kLargeVault);
                a_actor->NotifyAnimationGraph("climb_ClimbStart");  // start vault
            }
        } 
        else if (ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {  // do raycast
            auto center = a_actor->GetCurrent3D()->worldBound.center;
            center.z += ptr->rayCastClimbHeight;
            RE::hkVector4 start = { center.x, center.y, center.z, 0.00f };
            return start;
        }(), CalculateForwardRaycast(ptr->rayCastClimbDist, ptr->rayCastClimbHeight))) {
            bool jmp;
            a_actor->GetGraphVariableBool("CanJump", jmp);
            if (jmp) {
                auto& context = a_actor->GetCharController()->context;
                if (isClimbing && a_actor->actorState1.sprinting) {
                    a_actor->SetGraphVariableInt("climb_ClimbEndType", ClimbEndType::kJumpBackwards);
                    if (a_actor->NotifyAnimationGraph("climb_ClimbEnd")) {
                        context.currentState = RE::hkpCharacterStateType::kJumping;
                        isClimbing = false;
                    }// end climb
                } 
                else if (isClimbing) {
                    a_actor->SetGraphVariableInt("climb_ClimbEndType", ClimbEndType::kJumpForwards);
                    if (a_actor->NotifyAnimationGraph("climb_ClimbEnd")) {
                        context.currentState = RE::hkpCharacterStateType::kJumping;
                        isClimbing = false;
                    }// end climb
                } 
                else if (context.currentState == RE::hkpCharacterStateType::kInAir) {
                    a_actor->SetGraphVariableInt("climb_ClimbStartType", ClimbStartType::kClimbFromAir);
                    if (a_actor->NotifyAnimationGraph("climb_ClimbStart")) {
                        context.currentState = RE::hkpCharacterStateType::kClimbing;
                        isClimbing = true;
                    }// start climb
                } 
                else {
                    a_actor->SetGraphVariableInt("climb_ClimbStartType", ClimbStartType::kClimbFromGround);
                    if (a_actor->NotifyAnimationGraph("climb_ClimbStart")) {
                        context.currentState = RE::hkpCharacterStateType::kClimbing;
                        isClimbing = true;
                    }// start climb
                }
            }
        }
    }
    return _Update(a_actor);
}