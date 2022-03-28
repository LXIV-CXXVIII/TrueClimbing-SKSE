#include "Hooks.h"
#include "Project/TrueClimbing.h"

void* Loki::Hooks::CodeAllocation(Xbyak::CodeGenerator& a_code, SKSE::Trampoline* t_ptr) {
    auto result = t_ptr->allocate(a_code.getSize());
    std::memcpy(result, a_code.getCode(), a_code.getSize());
    return result;
}

void Loki::Hooks::InstallUpdateHook() {
    REL::Relocation<std::uintptr_t> ActorUpdate{ REL::ID(39375) };

    auto& trampoline = SKSE::GetTrampoline();
    Loki::TrueClimbing::_Update = trampoline.write_call<5>(ActorUpdate.address() + 0x8AC, Loki::TrueClimbing::Update);

    logger::info("Actor Update hook injected");
}

void Loki::Hooks::InstallSimulateClimbingHook() {
    REL::Relocation<std::uintptr_t> ClimbSim{ REL::ID(78195) };

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.write_branch<5>(ClimbSim.address(), &Loki::TrueClimbing::bhkCharacterStateClimbing_SimPhys);

    logger::info("Climbing Simulation hook injected");
}

void Loki::Hooks::InstallClimbSimHook() {
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