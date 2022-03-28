#include "Project/TrueClimbing.h"

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface * a_skse, SKSE::PluginInfo * a_info)
{
#ifndef NDEBUG
    auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
    auto path = logger::log_directory();
    if (!path) {
        return false;
    }

    *path /= "loki_TrueClimbingSKSE.log"sv;
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

    auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
    log->set_level(spdlog::level::trace);
#else
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);
#endif

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

    logger::info("loki_TrueClimbingSKSE v1.0.0");

    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "loki_TrueClimbingSKSE";
    a_info->version = 1;

    if (a_skse->IsEditor()) {
        logger::critical("Loaded in editor, marking as incompatible"sv);
        return false;
    }

    const auto ver = a_skse->RuntimeVersion();
    if (ver < SKSE::RUNTIME_1_5_39) {
        logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
        return false;
    }

    return true;
}

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {

    switch (message->type) {
    case SKSE::MessagingInterface::kNewGame:
    case SKSE::MessagingInterface::kPostLoadGame: {
        break;
    }
    case SKSE::MessagingInterface::kPostLoad: {
        break;
    }
    default:
        break;
    }

}

static void MessageHandler(SKSE::MessagingInterface::Message* message) {

    switch (message->type) {
        case SKSE::MessagingInterface::kDataLoaded: {
            auto ptr = Loki::TrueClimbing::GetSingleton();
            if (ptr->tControl) {
                if (ptr->g_TDM) {
                }
            };
            break;
        }
        case SKSE::MessagingInterface::kNewGame:
        case SKSE::MessagingInterface::kPostLoadGame: {
            break;
        }
        case SKSE::MessagingInterface::kPostLoad: {

            Loki::TrueClimbing::GetSingleton()->g_TDM = reinterpret_cast<TDM_API::IVTDM2*>(TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V2));
            if (Loki::TrueClimbing::GetSingleton()->g_TDM) {
                logger::info("Obtained TDM API -> {0:x}", (uintptr_t)Loki::TrueClimbing::GetSingleton()->g_TDM);
            } 
            else {
                logger::warn("Failed to obtain TDM API");
            };

            Loki::Raycasting::GetSingleton()->g_TrueHUD = reinterpret_cast<TRUEHUD_API::IVTrueHUD3*>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V3));
            if (Loki::Raycasting::GetSingleton()->g_TrueHUD) {
                logger::info("Obtained TrueHUD API -> {0:x}", (uintptr_t)Loki::Raycasting::GetSingleton()->g_TrueHUD);
            }
            else {
                logger::warn("Failed to obtain TrueHUD API");
            };

            break;
        }
        case SKSE::MessagingInterface::kPostPostLoad: {
        }
        default:
            break;
    }

}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface * a_skse)
{
    logger::info("Climbing loaded");
    SKSE::Init(a_skse);
    SKSE::AllocTrampoline(128);

    Loki::Hooks::InstallUpdateHook();
    Loki::Hooks::InstallClimbSimHook();

    //Loki_Climbing::InstallSimulateClimbingHook();

    return true;
}

// TODO: Climbing in Holds is illegal
// TODO: Particles for material
// TODO: Difficulty climbing in harsh weather