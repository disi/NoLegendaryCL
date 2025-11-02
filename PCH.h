#pragma once

// Logging
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
// Version
using namespace std::literals;

// --- CommonLibF4 ---
// Main
#include "F4SE/Impl/PCH.h"
#include "REL/REL.h"
#include "F4SE/API.h"
#include "F4SE/F4SE.h"
#include "F4SE/Interfaces.h"
#include "F4SE/Logger.h"
#include "F4SE/Trampoline.h"
#include "F4SE/Version.h"
// General
#include "RE/Fallout.h"
#include "RE/NiRTTI_IDs.h"
#include "RE/RTTI.h"
#include "RE/RTTI_IDs.h"
#include "RE/VTABLE_IDs.h"
// Extra
#include "RE/Bethesda/Actor.h"
#include "RE/Bethesda/BGSInventoryItem.h"
#include "RE/Bethesda/BSContainer.h"
#include "RE/Bethesda/BSExtraData.h"
#include "RE/Bethesda/BSFixedString.h"
#include "RE/Bethesda/BSPointerHandle.h"
#include "RE/Bethesda/BSRandom.h"
#include "RE/Bethesda/BSScript.h" // IVirtuanMachine for Papyrus Interface
#include "RE/Bethesda/BSScriptUtil.h"
#include "RE/Bethesda/BSTEvent.h"
#include "RE/Bethesda/BSTList.h"
#include "RE/Bethesda/BSTSingleton.h"
#include "RE/Bethesda/BSUtilities.h"
#include "RE/Bethesda/FormComponents.h"
#include "RE/Bethesda/FormUtil.h"
#include "RE/Bethesda/GameScript.h"
#include "RE/Bethesda/Misc.h"
#include "RE/Bethesda/TESBoundObjects.h"
#include "RE/Bethesda/TESDataHandler.h"
#include "RE/Bethesda/TESFile.h"
#include "RE/Bethesda/TESForms.h"
#include "RE/Bethesda/TESObjectREFRs.h"
#include "RE/Bethesda/Utilities.h"
#include "RE/Bethesda/BSCoreTypes.h"
#include "RE/Bethesda/BSScript/Array.h"
#include "RE/Bethesda/BSScript/ArrayWrapper.h"
#include "RE/Bethesda/BSScript/TypeInfo.h"
#include "RE/Bethesda/BSScript/Variable.h"
#include "RE/Bethesda/BSTSmartPointer.h"
#include "RE/Bethesda/BSTEvent.h"
#include "RE/Bethesda/Events.h"
#include "RE/Bethesda/InputEvent.h"


// --- Windows ---
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ShlObj.h>
#include <cstdio>
#include <cstdint>
#include <string>
#include <new>
#include <array>
#include <functional>
#include <vector>
#include <map>
#include <algorithm>
#include <unordered_set>
#include <utility>
#include <atomic>
#include <exception>
#include <optional>
#include <filesystem>
#include <string_view>
#include <span>
#include <optional>

// --- Fixes ---

// FMT RE::BSFixedString
template <>
struct fmt::formatter<RE::BSFixedString>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& a_ctx) { return a_ctx.begin(); }

    template <typename FormatContext>
    auto format(const RE::BSFixedString& a_str, FormatContext& a_ctx) const
    {
        return fmt::format_to(a_ctx.out(), "{}", a_str.c_str());
    }
};