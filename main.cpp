#include <Global.h>

// Global debug flag
bool DEBUGGING = true;

// Global logger pointer
std::shared_ptr<spdlog::logger> gLog;

// --- Explicit F4SE_API Definition ---
// This macro is essential for exporting functions from the DLL.
// If the F4SE headers aren't providing it correctly for your setup,
// we define it directly.
#define F4SE_API __declspec(dllexport)

// This is used by commonLibF4
namespace Version
{
    inline constexpr std::size_t MAJOR = 0;
    inline constexpr std::size_t MINOR = 1;
    inline constexpr std::size_t PATCH = 0;
    inline constexpr auto NAME = "0.1.0"sv;
    inline constexpr auto AUTHORNAME = "disi"sv;
    inline constexpr auto PROJECT = "NoLegendaryCL"sv;
}

// Declare the F4SEMessagingInterface and F4SEScaleformInterface
const F4SE::MessagingInterface* g_messaging = nullptr;
// Papyrus interface
const F4SE::PapyrusInterface* g_papyrus = nullptr;
// Task interface for menus and threads
const F4SE::TaskInterface* g_taskInterface = nullptr;
// Scaleform interface
const F4SE::ScaleformInterface* g_scaleformInterface = nullptr;
// Plugin handle
F4SE::PluginHandle g_pluginHandle = 0;
// Datahandler
RE::TESDataHandler* g_dataHandle = 0;

// Global item arrays
NoLegendaryArrays g_noLegendaryArrays = NoLegendaryArrays();
// Keyword for ap_Legendary [KYWD:001E32C8]
std::string g_keywordLegendary = "ap_Legendary";
// EditorID of the Dummy OMOD to use for misc items
std::unordered_set<std::string> g_editorIDDummyOMOD = {
    "NoLegendaryOMODRemoveARMO",
    "NoLegendaryOMODRemoveWEAP"
};
// EditorID prefixes for applied dummy OMODs
std::string g_editorIDDummyARMOOMODPrefix = "NoLegendaryOMODDummyARMO";
std::string g_editorIDDummyWEAPOMODPrefix = "NoLegendaryOMODDummyWEAP";
// EditorID prefix for loose misc items
std::string g_editorIDLooseMiscPrefix = "miscmod_NoLegendaryMISC";
// EditorID prefix for COBJ recipes
std::string g_editorIDCOBJPrefix = "NoLegendaryCOBJ";

// Helper function to extract value from a line
inline std::string GetValueFromLine(const std::string& line) {
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos)
        return "";
    std::string value = line.substr(eqPos + 1);
    value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
    return value;
}
// Helper to get the directory of the plugin DLL
std::string GetPluginDirectory(HMODULE hModule) {
    char path[MAX_PATH];
    GetModuleFileNameA(hModule, path, MAX_PATH);
    std::string fullPath(path);
    size_t pos = fullPath.find_last_of("\\/");
    return (pos != std::string::npos) ? fullPath.substr(0, pos + 1) : "";
}
// Helper to parse hex string to uint32_t
uint32_t ParseHexFormID(const std::string& hexStr) {
    return static_cast<uint32_t>(std::stoul(hexStr, nullptr, 16));
}
void LoadConfig(HMODULE hModule) {
    std::string configPath = GetPluginDirectory(hModule) + "NoLegendary.ini";
    gLog->info("LoadConfig: Loading config from: {}", configPath);
    // First try to open the stream directly
    std::ifstream file(configPath);
    // Check if the file opened successfully
    if (!file.is_open()) {
        gLog->warn("LoadConfig: Could not open INI file: {}. Creating default.", configPath);
        // Create the file with defaultIni contents
        std::ofstream out(configPath);
        if (out.is_open()) {
            out << defaultIni;
            out.close();
            gLog->info("LoadConfig: Default INI created at: {}", configPath);
        } else {
            gLog->warn("LoadConfig: Failed to create default INI at: {}", configPath);
            return;
        }

        // Try to open again for reading
        file.open(configPath);
        if (!file.is_open()) {
            gLog->warn("LoadConfig: Still could not open INI file after creating default: {}", configPath);
            return;
        }
    }
    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        // Skip comments and empty lines
        if (line.empty() || line[0] == ';')
            continue;
        // Lower case for case-insensitive comparison
        std::string lowerLine = ToLower(line);
        // --- Debugging flag ---
        if (lowerLine.find("debugging") == 0) {
            std::string value = GetValueFromLine(line);
            if (ToLower(value) == "true" || value == "1") {
                DEBUGGING = true;
            } else {
                DEBUGGING = false;
            }
            continue;
        }
    }
    file.close();
    gLog->info("LoadConfig: Completed loading config.");
    gLog->info(" - Debugging: {}", DEBUGGING);
}

// Message handler definition
void F4SEMessageHandler(F4SE::MessagingInterface::Message* a_message) {
    switch (a_message->type) {
    case F4SE::MessagingInterface::kPostLoad:
        gLog->info("Received kMessage_PostLoad. Game data is now loaded!");
        break;
    case F4SE::MessagingInterface::kPostPostLoad:
        gLog->info("Received kMessage_PostPostLoad. Game data finished loading.");
        break;
    case F4SE::MessagingInterface::kGameDataReady:
        gLog->info("Received kMessage_GameDataReady. Game data is ready.");
        // Get the global data handle and interfaces
        g_dataHandle = RE::TESDataHandler::GetSingleton();
        if (g_dataHandle) {
            gLog->info("TESDataHandler singleton acquired successfully.");
        } else {
            gLog->warn("Failed to acquire TESDataHandler singleton.");
        }
        // Initialize the mods data
        InitializeData_Internal();
    }
}

// --- F4SE Entry Points - MUST have C linkage for F4SE to find them ---
extern "C"  { // This block ensures C-style (unmangled) names for the linker

    F4SE_API bool F4SEPlugin_Query(const F4SE::QueryInterface* f4se, F4SE::PluginInfo* info)
    {
        // Set the plugin information
        // This is crucial to load the plugin
        info->infoVersion = F4SE::PluginInfo::kVersion;
        info->name = Version::PROJECT.data();
        info->version = Version::MAJOR;

        // Set up the logger
        // F4SE::log::log_directory().value(); == Documents/My Games/F4SE/
        std::filesystem::path logPath = F4SE::log::log_directory().value();
        logPath = logPath.parent_path() / "Fallout4" / "F4SE" / fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
        // Create the file
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.string(), true);
        auto aLog = std::make_shared<spdlog::logger>("aLog"s, sink);
        // Configure the logger
        aLog->set_level(spdlog::level::info);
        aLog->flush_on(spdlog::level::info);
        // Set pattern
        aLog->set_pattern("[%T] [%^%l%$] %v"s);
        // Register to make it global accessable
        spdlog::register_logger(aLog);
        // Assign to global pointer
        gLog = spdlog::get("aLog");
        // First log
        gLog->info("{}: Plugin Query started.", Version::PROJECT);

        // Minimum version 1.10.163
        const auto ver = f4se->RuntimeVersion();
        if (ver < F4SE::RUNTIME_1_10_162) {
            gLog->critical("unsupported runtime v{}", ver.string());
            return false;
        }

        return true;
    }

    // This function is called after F4SE has loaded all plugins and the game is about to start.
    F4SE_API bool F4SEPlugin_Load(const F4SE::LoadInterface* f4se)
    {
        // Initialize the plugin with logger false to prevent F4SE to use its own logger
        F4SE::Init(f4se, false);

        // Log information
        gLog->info("{}: Plugin loaded!", Version::PROJECT);
        gLog->info("F4SE version: {}", F4SE::GetF4SEVersion().string());
        gLog->info("Game runtime version: {}", f4se->RuntimeVersion().string());

        // Get the global plugin handle and interfaces
        g_pluginHandle = f4se->GetPluginHandle();
        g_messaging = F4SE::GetMessagingInterface();
        g_papyrus = F4SE::GetPapyrusInterface();

        // Register Papyrus functions
        if (g_papyrus) {
            g_papyrus->Register(RegisterPapyrusFunctions);
            gLog->info("Papyrus functions registration callback successfully registered.");
        }
        else {
            gLog->warn("Failed to register Papyrus functions. This is critical for native functions.");
        }

        // Inject our ExamineMenu Hooks
        if (InstallExamineMenuHooks()) {
            gLog->info("Successfully installed ExamineMenu hooks.");
        } else {
            gLog->warn("Failed to install ExamineMenu hooks.");
        }

        // Set the messagehandler to listen to events
        if (g_messaging && g_messaging->RegisterListener(F4SEMessageHandler, "F4SE")) {
            gLog->info("Registered F4SE message handler.");
        }
        else {
            gLog->warn("Failed to register F4SE message handler.");
            return false;
        }

        // Get the task interface
        g_taskInterface = F4SE::GetTaskInterface();
        // Get the scaleform interface
        g_scaleformInterface = F4SE::GetScaleformInterface();

        // Get the DLL handle for this plugin
        HMODULE hModule = GetModuleHandleA("NoLegendaryCL.dll");
        // Load config
        LoadConfig(hModule);

        return true;
    }

    F4SE_API void F4SEPlugin_Release() {
        // This is a new function for cleanup. It is called when the plugin is unloaded.
        gLog->info("%s: Plugin released.", Version::PROJECT);
        gLog->flush();
        spdlog::drop_all();
    }
}