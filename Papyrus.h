#pragma once
#include <Global.h>

// Main Papyrus registration function
bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);

// Declare all functions here
void DisableCOBJ(RE::BGSConstructibleObject* cobj);
std::string GetDisplayName_Internal(RE::BGSInventoryItem* a_InvItem);
template<typename T>
RE::BSTArray<T> DuplicateArray_Internal(const RE::BSTArray<T>& sourceArray);
RE::BSTArray<RE::BGSConstructibleObject*> GetAllCOBJ_Internal();
RE::BSTArray<RE::TESObjectMISC*> GetAllLooseMiscItems_Internal();
std::uint32_t GetFormIDFromLooseMiscName_Internal(const std::string& a_name);
RE::TBO_InstanceData* GetInstanceData_Internal(RE::BGSInventoryItem* a_InvItem);
RE::TBO_InstanceData* GetInstanceData_Internal(RE::TESObjectREFR* a_Ref);
RE::BGSObjectInstanceExtra* GetInstanceDataExtra_Internal(RE::BGSInventoryItem* a_InvItem);
RE::BGSObjectInstanceExtra* GetInstanceDataExtra_Internal(RE::TESObjectREFR* a_Ref);
RE::BGSInventoryItem* GetInventoryItem_Internal(RE::Actor* akActor, RE::TESForm* akForm);
RE::BGSMod::Attachment::Mod* GetLegendaryOMOD_Internal(RE::BGSInventoryItem* a_InvItem);
uint32_t GetStackCount_Internal(RE::BGSInventoryItem* a_InvItem);
RE::BGSInventoryItem::Stack* GetStackData_Internal(RE::BGSInventoryItem* a_InvItem);
void InitializeData_Internal();
bool IsDummyOMOD_Internal(RE::BGSMod::Attachment::Mod* omod);
bool IsLegendary_Internal(RE::BGSInventoryItem* a_invItem);
bool IsLegendary_Internal(RE::BGSMod::Attachment::Mod* omod);
bool IsArmorOrWeapon_Internal(RE::TESForm* a_form);
bool IsArmorOrWeapon_Internal(RE::BGSInventoryItem* a_inventoryItem);
bool IsArmorOrWeapon_Internal(RE::TESObjectREFR* a_objectReference);
bool IsDummyOMOD_Internal(RE::BGSMod::Attachment::Mod* omod);
bool IsCOBJ_Internal(RE::BGSConstructibleObject* a_COBJ);
bool IsMiscItem_Internal(RE::BGSInventoryItem* a_Item);
bool IsMiscItem_Internal(RE::TESForm* a_Form);
RE::ObjectRefHandle ItemDrop_Internal(RE::Actor* aNPC, RE::BGSInventoryItem* aInvItem);
RE::ObjectRefHandle ItemDrop_Internal(RE::Actor* aNPC, RE::TESObjectMISC* aInvItem);
bool OMODClearDummy_Internal(RE::Actor* aNPC, RE::BGSInventoryItem* aInvItem);
void PatchLegendaryKeyword_Internal();
static bool ValidateInstanceKeywordData_Internal(RE::TBO_InstanceData* a_instance, RE::BGSKeyword* a_testKeyword) noexcept;

// Hooks
void MyCreateModdedInventoryItem(RE::ExamineMenu* menu);
bool MyTryCreate(RE::ExamineMenu* menu);
bool InstallExamineMenuHooks();

// NoLegendary Snapshot structure
struct NoLegendaryArrays {
    // Current snapshots
    std::unordered_map<RE::BGSMod::Attachment::Mod*,RE::TESForm*> omodToMisc;
    std::unordered_map<RE::TESForm*,RE::BGSMod::Attachment::Mod*> miscToOmod;
    std::unordered_map<RE::TESForm*,RE::BGSMod::Attachment::Mod*> miscToDummyARMO;
    std::unordered_map<RE::TESForm*,RE::BGSMod::Attachment::Mod*> miscToDummyWEAP;
    std::unordered_map<RE::BGSMod::Attachment::Mod*, std::string> omodToFullName;
    std::unordered_set<RE::BGSConstructibleObject*> COBJData;

    bool addOMODMapping(RE::BGSMod::Attachment::Mod* omod, RE::TESForm* misc) {
        const char* fullName = omod->GetFullName();
        if (omod && misc && fullName && strlen(fullName) != 0) {
            omodToMisc[omod] = misc;
            miscToOmod[misc] = omod;
            omodToFullName[omod] = fullName;
            return true;
        }
        return false;
    }
    void addDummyARMOMapping(RE::BGSMod::Attachment::Mod* omod, RE::TESForm* misc) {
        if (omod && misc) {
            miscToDummyARMO[misc] = omod;
        }
    }
    void addDummyWEAPMapping(RE::BGSMod::Attachment::Mod* omod, RE::TESForm* misc) {
        if (omod && misc) {
            miscToDummyWEAP[misc] = omod;
        }
    }
    void addCOBJ(RE::BGSConstructibleObject* cobj) {
        if (cobj) {
            COBJData.insert(cobj);
        }
    }
    void ClearAll() {
        omodToMisc.clear();
        miscToOmod.clear();
        miscToDummyARMO.clear();
        miscToDummyWEAP.clear();
        COBJData.clear();
    }
    // Helper: Get OMOD from MISC
    RE::BGSMod::Attachment::Mod* misc2OMOD(RE::TESForm* misc) const {
        auto it = miscToOmod.find(misc);
        return it != miscToOmod.end() ? it->second : nullptr;
    }
    // Helper: Get MISC from OMOD
    RE::TESForm* omod2MISC(RE::BGSMod::Attachment::Mod* omod) const {
        auto it = omodToMisc.find(omod);
        return it != omodToMisc.end() ? it->second : nullptr;
    }
    // Helper: Get FullName from OMOD
    std::string omod2FullName(RE::BGSMod::Attachment::Mod* omod) const {
        auto it = omodToFullName.find(omod);
        return it != omodToFullName.end() ? it->second : "";
    }
    // Helper: Get Dummy OMOD from MISC
    RE::BGSMod::Attachment::Mod* misc2DummyARMO(RE::TESForm* misc) const {
        auto it = miscToDummyARMO.find(misc);
        return it != miscToDummyARMO.end() ? it->second : nullptr;
    }
    RE::BGSMod::Attachment::Mod* misc2DummyWEAP(RE::TESForm* misc) const {
        auto it = miscToDummyWEAP.find(misc);
        return it != miscToDummyWEAP.end() ? it->second : nullptr;
    }
    bool containsCOBJ(RE::BGSConstructibleObject* cobj) const {
        return COBJData.find(cobj) != COBJData.end();
    }
};