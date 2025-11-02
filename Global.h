#pragma once
#include <PCH.h>

#include <Papyrus.h>

// Global logger pointer
extern std::shared_ptr<spdlog::logger> gLog;

// Global debug flag
extern bool DEBUGGING;
// Global data handler
extern RE::TESDataHandler* g_dataHandle;
// Declare the F4SEMessagingInterface and F4SEScaleformInterface
extern const F4SE::MessagingInterface* g_messaging;
extern const F4SE::ScaleformInterface* g_scaleform;
// Declare the F4SEPapyrusInterface
extern const F4SE::PapyrusInterface* g_papyrus;
// Declare the F4SETaskInterface
extern const F4SE::TaskInterface* g_taskInterface;

// Default ini file
extern const char* defaultIni;

// The Snapshot structure
extern NoLegendaryArrays g_noLegendaryArrays;

// Keyword for ap_Legendary [KYWD:001E32C8]
extern std::string g_keywordLegendary;
// EditorID of the Dummy OMOD to use for misc items
extern std::unordered_set<std::string> g_editorIDDummyOMOD;
// EditorID prefixes for applied dummy OMODs
extern std::string g_editorIDDummyARMOOMODPrefix;
extern std::string g_editorIDDummyWEAPOMODPrefix;
// EditorID prefix for loose misc items
extern std::string g_editorIDLooseMiscPrefix;
// EditorID prefix for COBJ recipes
extern std::string g_editorIDCOBJPrefix;

// Helper function to convert string to lowercase
inline std::string ToLower(const std::string& str) {
    std::string out = str;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}