#pragma once

namespace Loki {

	class Hooks {
	
	public:
		static void* CodeAllocation(Xbyak::CodeGenerator& a_code, SKSE::Trampoline* t_ptr);
		static void InstallUpdateHook();
		static void InstallSimulateClimbingHook();
		static void InstallClimbSimHook();

	private:

	protected:
	
	};

};