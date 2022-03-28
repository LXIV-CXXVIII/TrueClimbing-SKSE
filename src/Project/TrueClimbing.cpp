#include "Project/TrueClimbing.h"

Loki::TrueClimbing::TrueClimbing() {
    CSimpleIniA ini;
    ini.SetUnicode();
    auto filename = L"Data/SKSE/Plugins/loki_TrueClimbing.ini";
    SI_Error rc = ini.LoadFile(filename);

    rayCastDist = ini.GetDoubleValue("SETTINGS", "fRayCastDistance", -1.00f);
    rayCastLowVaultDist = ini.GetDoubleValue("SETTINGS", "fRayCastLowVaultDistance", -1.00f);
    rayCastMediumVaultDist = ini.GetDoubleValue("SETTINGS", "fRayCastMediumVaultDistance", -1.00f);
    rayCastHighVaultDist = ini.GetDoubleValue("SETTINGS", "fRayCastHighVaultDistance", -1.00f);
    rayCastClimbDist = ini.GetDoubleValue("SETTINGS", "fRayCastClimbDistance", -1.00f);
    rayCastLowVaultHeight = ini.GetDoubleValue("SETTINGS", "fRayCastLowVaultHeight", -1.00f);
    rayCastMediumVaultHeight = ini.GetDoubleValue("SETTINGS", "fRayCastMediumVaultHeight", -1.00f);
    rayCastHighVaultHeight = ini.GetDoubleValue("SETTINGS", "fRayCastHighVaultHeight", -1.00f);
    rayCastClimbHeight = ini.GetDoubleValue("SETTINGS", "fRayCastClimbHeight", -1.00f);
    return;
}

Loki::TrueClimbing::~TrueClimbing() {
}

Loki::TrueClimbing* Loki::TrueClimbing::GetSingleton() {
    static Loki::TrueClimbing* singleton = new Loki::TrueClimbing();
    return singleton;
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

    //auto bhkWorld = a_actor->parentCell->GetbhkWorld();
    //auto hkpWorld = bhkWorld->GetWorld();
    
    //hkpWorld->simulationType.set(RE::hkpWorldCinfo::SimulationType::kContinuous);
    //auto c = a_actor->GetCharController();
    //RE::bhkCharacterController;

    if (a_actor->IsPlayerRef()) {
        //auto controller = a_actor->GetCharController();
        //controller->context.currentState = RE::hkpCharacterStateType::kClimbing;
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

        if (auto output = ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {  // do raycast
            auto center = a_actor->GetCurrent3D()->worldBound.center;
            center.z += ptr->rayCastLowVaultHeight;
            RE::hkVector4 start = { center.x, center.y, center.z, 0.00f };
            return start;
        }(), CalculateForwardRaycast(ptr->rayCastLowVaultDist, ptr->rayCastLowVaultHeight))) {

            if (output->HasHit()) {
                bool jmp;
                a_actor->GetGraphVariableBool("CanJump", jmp);
                if (jmp) {
                    a_actor->SetGraphVariableInt("climb_ClimbStartType", ClimbStartType::kSmallVault);
                    a_actor->NotifyAnimationGraph("climb_ClimbStart");  // start vault
                }
            }

        } 
        else if (auto output = ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {  // do raycast
            auto center = a_actor->GetCurrent3D()->worldBound.center;
            center.z += ptr->rayCastMediumVaultHeight;
            RE::hkVector4 start = { center.x, center.y, center.z, 0.00f };
            return start;
        }(), CalculateForwardRaycast(ptr->rayCastMediumVaultDist, ptr->rayCastMediumVaultHeight))) {

            if (output->HasHit()) {
                bool jmp;
                a_actor->GetGraphVariableBool("CanJump", jmp);
                if (jmp) {
                    a_actor->SetGraphVariableInt("climb_ClimbStartType", ClimbStartType::kMediumVault);
                    a_actor->NotifyAnimationGraph("climb_ClimbStart");  // start vault
                }
            }

        } 
        else if (auto output = ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {  // do raycast
            auto center = a_actor->GetCurrent3D()->worldBound.center;
            center.z += ptr->rayCastHighVaultHeight;
            RE::hkVector4 start = { center.x, center.y, center.z, 0.00f };
            return start;
        }(), CalculateForwardRaycast(ptr->rayCastHighVaultDist, ptr->rayCastHighVaultHeight))) {

            if (output->HasHit()) {
                bool jmp;
                a_actor->GetGraphVariableBool("CanJump", jmp);
                if (jmp) {
                    a_actor->SetGraphVariableInt("climb_ClimbStartType", ClimbStartType::kLargeVault);
                    a_actor->NotifyAnimationGraph("climb_ClimbStart");  // start vault
                }
            }

        } 
        else if (auto output = ptr->DoRayCast(a_actor, [a_actor, ptr]() -> RE::hkVector4 {  // do raycast
            auto center = a_actor->GetCurrent3D()->worldBound.center;
            center.z += ptr->rayCastClimbHeight;
            RE::hkVector4 start = { center.x, center.y, center.z, 0.00f };
            return start;
        }(), CalculateForwardRaycast(ptr->rayCastClimbDist, ptr->rayCastClimbHeight))) {

            if (output->HasHit()) {
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

                    } else if (isClimbing) {

                        a_actor->SetGraphVariableInt("climb_ClimbEndType", ClimbEndType::kJumpForwards);
                        if (a_actor->NotifyAnimationGraph("climb_ClimbEnd")) {
                            context.currentState = RE::hkpCharacterStateType::kJumping;
                            isClimbing = false;
                        }// end climb

                    } else if (context.currentState == RE::hkpCharacterStateType::kInAir) {

                        a_actor->SetGraphVariableInt("climb_ClimbStartType", ClimbStartType::kClimbFromAir);
                        if (a_actor->NotifyAnimationGraph("climb_ClimbStart")) {

                            context.currentState = RE::hkpCharacterStateType::kClimbing;
                            if (g_TDM->RequestYawControl(SKSE::GetPluginHandle(), 0.50f) == TDM_API::APIResult::OK || 
                                g_TDM->RequestYawControl(SKSE::GetPluginHandle(), 0.50f) == TDM_API::APIResult::AlreadyGiven) {
                            
                                g_TDM->SetPlayerYaw(SKSE::GetPluginHandle(), output->normal.quad.m128_f32[1]);
                                g_TDM->ReleaseYawControl(SKSE::GetPluginHandle());
                            
                            }
                            isClimbing = true;

                        }// start climb

                    } else {

                        a_actor->SetGraphVariableInt("climb_ClimbStartType", ClimbStartType::kClimbFromGround);
                        if (a_actor->NotifyAnimationGraph("climb_ClimbStart")) {

                            context.currentState = RE::hkpCharacterStateType::kClimbing;
                            if (g_TDM->RequestYawControl(SKSE::GetPluginHandle(), 0.50f) == TDM_API::APIResult::OK ||
                                g_TDM->RequestYawControl(SKSE::GetPluginHandle(), 0.50f) == TDM_API::APIResult::AlreadyGiven) {

                                g_TDM->SetPlayerYaw(SKSE::GetPluginHandle(), output->normal.quad.m128_f32[1]);
                                g_TDM->ReleaseYawControl(SKSE::GetPluginHandle());

                            }
                            isClimbing = true;

                        }// start climb

                    }
                }
            }

        }
    }
    return _Update(a_actor);
}