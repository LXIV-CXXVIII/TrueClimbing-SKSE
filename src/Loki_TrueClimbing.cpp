#include "Loki_TrueClimbing.h"

void* Loki_TrueClimbing::CodeAllocation(Xbyak::CodeGenerator& a_code, SKSE::Trampoline* t_ptr) {
    auto result = t_ptr->allocate(a_code.getSize());
    std::memcpy(result, a_code.getCode(), a_code.getSize());
    return result;
}

Loki_TrueClimbing::Loki_TrueClimbing() {
    CSimpleIniA ini;
    ini.SetUnicode();
    auto filename = L"Data/SKSE/Plugins/loki_Climbing.ini";
    SI_Error rc = ini.LoadFile(filename);

    this->rayCastDist = ini.GetDoubleValue("SETTINGS", "fRayCastDistance", -1.00f);
    return;
}

Loki_TrueClimbing::~Loki_TrueClimbing() {
}

Loki_TrueClimbing* Loki_TrueClimbing::GetSingleton() {
    static Loki_TrueClimbing* singleton = new Loki_TrueClimbing();
    return singleton;
}

void Loki_TrueClimbing::InstallUpdateHook() {
    REL::Relocation<std::uintptr_t> ActorUpdate{ REL::ID(39375) };

    auto& trampoline = SKSE::GetTrampoline();
    _Update = trampoline.write_call<5>(ActorUpdate.address() + 0x8AC, Update);

    logger::info("Actor Update hook injected");
}

void Loki_TrueClimbing::InstallSimulateClimbingHook() {
    REL::Relocation<std::uintptr_t> ClimbSim{ REL::ID(78195) };

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.write_branch<5>(ClimbSim.address(), &bhkCharacterStateClimbing_SimPhys);

    logger::info("Climbing Simulation hook injected");
}

void Loki_TrueClimbing::InstallClimbSimHook() {
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

void Loki_TrueClimbing::ControllerSubroutine(RE::bhkCharacterController* a_controller) {
    using func_t = decltype(ControllerSubroutine);
    REL::Relocation<func_t> func{ REL::ID(76440) }; /*dc08e0*/
    return func(a_controller);
}

void Loki_TrueClimbing::bhkCharacterStateClimbing_SimPhys(RE::bhkCharacterStateClimbing* a_climbing, RE::bhkCharacterController* a_controller) {
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

void Loki_TrueClimbing::Update(RE::Actor* a_actor) {
    auto ptr = Loki_TrueClimbing::GetSingleton();

    if (a_actor->IsPlayerRef()) {
        auto controller = a_actor->GetCharController();
        controller->context.currentState = RE::hkpCharacterStateType::kClimbing;
        static bool isClimbing = false;


        RE::PlayerCharacter::GetSingleton()->questLog;


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


        auto center = a_actor->GetCurrent3D()->worldBound.center;
        center.z += ptr->rayCastLowVaultHeight;
        RE::hkVector4 start = { center.x, center.y, center.z, 0.00f };
        auto forwardvec = ptr->GetForwardVector(center);
        forwardvec.x *= ptr->rayCastLowVaultDist;
        forwardvec.y *= ptr->rayCastLowVaultDist;
        forwardvec.z = 0.00f;
        auto normalized = forwardvec / forwardvec.Length();
        RE::hkVector4 end = { normalized.x, normalized.y, normalized.z, 0.00f };


        if (ptr->DoRayCast(a_actor, start, end)) {
            bool jmp;
            a_actor->GetGraphVariableBool("CanJump", jmp);
            if (jmp) { // if jump input
                a_actor->SetGraphVariableInt("climb_ClimbStartType", ClimbStartType::kSmallVault);
                a_actor->NotifyAnimationGraph("climb_ClimbStart");  // start climb
                //isClimbing = true;
            }
        }

        if (ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {
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
        else if (ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {
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
        else if (ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {
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
        else if (ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {
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

        /**
        if (ptr->DoRayCast(a_actor, start, end)) {
            bool jmp;
            a_actor->GetGraphVariableBool("CanJump", jmp);
            if (jmp) { // if jump input
                if (isClimbing) { // if we're already climbing
                    if (a_actor->actorState1.sprinting) {
                        a_actor->SetGraphVariableInt("climb_ClimbEndType", EndType::kJumpBackwards);
                        a_actor->NotifyAnimationGraph("climb_ClimbEnd");
                        a_actor->GetCharController()->context.currentState = RE::hkpCharacterStateType::kJumping;
                        bool isClimbing = false;
                    }
                    a_actor->SetGraphVariableInt("climb_ClimbEndType", EndType::kJumpForwards);
                    a_actor->NotifyAnimationGraph("climb_ClimbEnd");  // end climb
                    a_actor->GetCharController()->context.currentState = RE::hkpCharacterStateType::kJumping;
                    bool isClimbing = false;
                } else { // if we're not climbing
                    if (a_actor->GetCharController()->context.currentState == RE::hkpCharacterStateType::kInAir) {
                        a_actor->SetGraphVariableInt("climb_ClimbStartType", StartType::kClimbFromAir);
                        if (a_actor->NotifyAnimationGraph("climb_ClimbStart")) {  // start climb
                            bool isClimbing = true;
                        }
                    } else { // if in air above, if on ground below
                        a_actor->SetGraphVariableInt("climb_ClimbStartType", StartType::kClimbFromGround);
                        a_actor->NotifyAnimationGraph("climb_ClimbStart");  // start climb
                        bool isClimbing = true;
                    }
                }
            }
        }
        */



        /**
        RE::hkVector4 start;
        controller->GetPositionImpl(start, false);
        start.quad.m128_f32[2] += 5.00f;
        auto from = start;

        RE::NiPoint3 pos = a_actor->data.location;
        RE::NiPoint3 angl = a_actor->data.angle;
        auto forward = controller->forwardVec;
        for (int i = 0; i < 3; i++) {
            forward.quad.m128_f32[i] += 10.00f;
        }


        RE::hkVector4 end;
        float dist = ptr->rayCastDist;
        end.quad.m128_f32[0] = from.quad.m128_f32[0] + (cos(from.quad.m128_f32[3]) * dist); // x
        end.quad.m128_f32[1] = from.quad.m128_f32[1] + (sin(from.quad.m128_f32[3]) * dist); // y
        end.quad.m128_f32[2] = from.quad.m128_f32[2]; // z
        end.quad.m128_f32[3] = from.quad.m128_f32[3]; // a
        auto to = end;
        */

        /*
        if (Loki_Climbing::DoRayCast(a_actor, from, forward)) {
            RE::ConsoleLog::GetSingleton()->Print("Raycast has hit collision object");
            a_actor->SetGraphVariableBool("IsClimbing", true);
            a_actor->SetGraphVariableInt("IsClimb", 1);
            controller->context.currentState = RE::hkpCharacterStateType::kClimbing;
            controller->pitchAngle = -1.50f;

        } else {
            bool isClimbing = FALSE;
            a_actor->GetGraphVariableBool("IsClimbing", isClimbing);
            if (isClimbing) {
                a_actor->SetGraphVariableBool("IsClimbing", false);
                controller->context.currentState = controller->context.previousState;
            }

            //a_actor->SetGraphVariableBool("IsClimbing", false);
            //a_actor->SetGraphVariableInt("IsClimb", 0);
            //a_actor->GetCharController()->context.currentState = RE::hkpCharacterStateType::kJumping;
            //RE::hkpCharacterStateType::kInAir    could be good for shield surfing?
        }
        */

    }
    return _Update(a_actor);

    RE::hkpCharacterContext;
}