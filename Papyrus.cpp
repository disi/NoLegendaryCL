#include <PCH.h>
#include <Global.h>

// Helper function to add a never-true condition to a COBJ
void DisableCOBJ(RE::BGSConstructibleObject* cobj)
{
    if (!cobj) return;

    // The below just greys it out
    // Remove all existing components
    //cobj->requiredItems->clear();
    // Add a dummy component with a non-existent FormID
    //RE::TESForm* impossibleComponent = RE::TESForm::GetFormByEditorID<RE::TESObjectMISC>("MS11GuidanceChip");
    //RE::BGSTypedFormValuePair::SharedVal impossibleValue;
    //impossibleValue.f = 1.0f;
    // Construct the tuple and push it
    //cobj->requiredItems->push_back(RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>(impossibleComponent, impossibleValue));

    // The below have no effect
    //cobj->ClearData();
    //cobj->data.workshopPriority = -1; // Set to -1 to deprioritize
    //cobj->data.numConstructed = -1;    // Set to -1 to prevent crafting
    //cobj->benchKeyword = nullptr; // clear bench keyword
    //cobj->createdItem = nullptr; // Clear created item
    //cobj->SetDelete(true);

    // Free existing conditions (if any)
    /* auto* cond = cobj->conditions.head;
    while (cond) {
        auto* next = cond->next;
        delete cond;
        cond = next;
    }
    cobj->conditions.head = nullptr; */

    // Allocate a new condition item
    auto* condItem = new RE::TESConditionItem();
    memset(condItem, 0, sizeof(RE::TESConditionItem));

    // Set up the condition: GetItemCount [NoLegendary_DisableComponent] >= 1 (always false)
    condItem->data.functionData.function.set(static_cast<RE::SCRIPT_OUTPUT>(0x1E)); // GetItemCount
    condItem->data.condition = RE::ENUM_COMPARISON_CONDITION::kGreaterThanEqual;
    condItem->data.value = 1.0f;
    condItem->data.object.set(RE::CONDITIONITEMOBJECT::kRef);

    // Set the parameter to your dummy item
    auto* impossibleComponent = RE::TESForm::GetFormByEditorID<RE::TESObjectMISC>("MS11GuidanceChip");
    condItem->data.functionData.param[0] = impossibleComponent;

    // Set as the only condition
    condItem->next = nullptr;
    cobj->conditions.head = condItem;
    return;
}

// C++ implementation for DuplicateArray_Internal
template<typename T>
RE::BSTArray<T> DuplicateArray_Internal(const RE::BSTArray<T>& sourceArray)
{
    RE::BSTArray<T> newArray;
    newArray.resize(sourceArray.size());
    for (std::size_t i = 0; i < sourceArray.size(); ++i) {
        newArray[i] = sourceArray[i];
    }
    return newArray;
}

// Helper function to get all COBJ from the game
RE::BSTArray<RE::BGSConstructibleObject*> GetAllCOBJ_Internal()
{
    if (DEBUGGING) gLog->info("GetAllCOBJ_Internal: Collecting all COBJ items...");
    RE::BSTArray<RE::BGSConstructibleObject*> cobj;
    if (g_dataHandle) {
        auto& cobjForms = g_dataHandle->GetFormArray<RE::BGSConstructibleObject>();
        if (DEBUGGING) {
            gLog->info("GetAllCOBJ_Internal: Total COBJ forms in game: {}", cobjForms.size());
        }
        for (auto* cobjForm : cobjForms) {
            if (cobjForm) {
                if (IsCOBJ_Internal(cobjForm)) {
                    cobj.push_back(cobjForm);
                }
            }
        }
    }
    if (DEBUGGING) gLog->info("GetAllCOBJ_Internal: Found {} COBJ items.", cobj.size());
    return cobj;
}

// Helper function to get all loose misc items from the game
RE::BSTArray<RE::TESObjectMISC*> GetAllLooseMiscItems_Internal()
{
    if (DEBUGGING) gLog->info("GetAllLooseMiscItems_Internal: Collecting all loose misc items...");
    RE::BSTArray<RE::TESObjectMISC*> looseMiscItems;
    if (g_dataHandle) {
        auto& miscForms = g_dataHandle->GetFormArray<RE::TESObjectMISC>();
        if (DEBUGGING) {
            gLog->info("GetAllLooseMiscItems_Internal: Total misc forms in game: {}", miscForms.size());
        }
        for (auto* miscForm : miscForms) {
            if (miscForm) {
                if (IsMiscItem_Internal(miscForm)) {
                    looseMiscItems.push_back(miscForm);
                }
            }
        }
    }
    if (DEBUGGING) gLog->info("GetAllLooseMiscItems_Internal: Found {} loose misc items.", looseMiscItems.size());
    return looseMiscItems;
}

std::string GetDisplayName_Internal(RE::BGSInventoryItem* a_InvItem)
{
    if (!a_InvItem) {
        return std::string();
    }
    auto* stackData = GetStackData_Internal(a_InvItem);
    if (!stackData) {
        return std::string();
    }
    std::uint32_t idx = 0;
    while (stackData) {
        // check ExtraTextDisplayData override on this stack
        if (stackData->extra) {
            if (auto* text = stackData->extra->GetByType<RE::ExtraTextDisplayData>()) {
                if (!text->displayName.empty()) {
                    return std::string(text->displayName.c_str());
                }
            }
        }
        // try asking the inventory item for the display name for this stack index
        try {
            auto name = a_InvItem->GetDisplayFullName(idx);
            // compare the name to empty string
            if (name && name[0] != '\0') {
                return std::string(name);
            }
        } catch (...) {
            // skip this stack if the call is unsafe
            if (DEBUGGING) gLog->warn("GetDisplayName_Internal: GetDisplayFullName crashed for {:08X} stack {}", a_InvItem->object ? a_InvItem->object->GetFormID() : 0, idx);
        }
        stackData = stackData->nextStack.get();
        ++idx;
    }
    // final fallback: base form editor id
    if (a_InvItem->object) {
        if (a_InvItem->object->GetFormEditorID()) return std::string(a_InvItem->object->GetFormEditorID());
    }
    return {};
}

// Get FormID from loose name string
std::uint32_t GetFormIDFromLooseMiscName_Internal(const std::string& a_name)
{
    if (a_name.empty()) return 0;
    auto pos = a_name.rfind("[");
    if (pos == std::string::npos) return 0;
    pos = a_name.find_first_of("0123456789ABCDEFabcdef", pos);
    if (pos == std::string::npos || pos + 8 > a_name.size()) return 0;
    std::string hex = a_name.substr(pos, 8);
    for (char c : hex) if (!isxdigit(static_cast<unsigned char>(c))) return 0;
    try {
        return static_cast<std::uint32_t>(std::stoul(hex, nullptr, 16));
    } catch (...) {
        return 0;
    }
}

// Get the instanceData for an InventoryItem
RE::TBO_InstanceData* GetInstanceData_Internal(RE::BGSInventoryItem* a_InvItem)
{
    if (!a_InvItem) {
        return nullptr;
    }
    auto* testKeyword = RE::TESForm::GetFormByEditorID<RE::BGSKeyword>("ap_Legendary");
    if (!testKeyword) {
        if (DEBUGGING) gLog->warn("GetInstanceData_Internal: testKeyword not found.");
        return nullptr;
    }
    // Instantiate pointer
    RE::TBO_InstanceData* instanceData = nullptr;
    // Check all stacks for instance data
    const auto stackCount = GetStackCount_Internal(a_InvItem);
    for (int i = 0; i < stackCount; i++) {
        // get instance data for this stack
        try {
            instanceData = a_InvItem->GetInstanceData(i);
        } catch (...) {
            if (DEBUGGING) gLog->warn("GetInstanceData_Internal: Exception getting instance data for {:08X} stack {}.", a_InvItem->object ? a_InvItem->object->GetFormID() : 0, i);
            continue;
        }
        if (!instanceData) {
            continue;
        }
        // Validate instance keyword data (returns true -> valid)
        if (ValidateInstanceKeywordData_Internal(instanceData, testKeyword)) {
            return instanceData;
        } else {
            if (DEBUGGING) gLog->warn("GetInstanceData_Internal: Validation failed for {:08X} stack {}.", a_InvItem->object ? a_InvItem->object->GetFormID() : 0, i);
            continue;
        }
    }
    // If we reach here, the instanceData is valid
    return nullptr;
}
// Get the instanceData for an item object
RE::TBO_InstanceData* GetInstanceData_Internal(RE::TESObjectREFR* a_Ref)
{
    if (!a_Ref) {
        return nullptr;
    }
    auto* itemInstance = a_Ref->As<RE::BGSObjectInstance>();
    // get instance data for this stack
    // Instantiate pointer
    RE::TBO_InstanceData* instanceData = nullptr;
    try {
        instanceData = itemInstance->instanceData.get();
    } catch (...) {
        if (DEBUGGING) gLog->warn("GetInstanceData_Internal: Exception getting instance data for {:08X}.", a_Ref->GetFormID());
        return nullptr;
    }
    if (!instanceData) {
        return nullptr;
    }
    auto* testKeyword = RE::TESForm::GetFormByEditorID<RE::BGSKeyword>("ap_Legendary");
    if (!testKeyword) {
        if (DEBUGGING) gLog->warn("GetInstanceData_Internal: testKeyword not found.");
        return nullptr;
    }
    // Validate instance keyword data (returns true -> valid)
    if (ValidateInstanceKeywordData_Internal(instanceData, testKeyword)) {
        return instanceData;
    } else {
        if (DEBUGGING) gLog->warn("GetInstanceData_Internal: Validation failed for {:08X}.", a_Ref->GetFormID());
        return nullptr;
    }
    return nullptr;
}

// Get the instanceDataExtra for an InventoryItem
RE::BGSObjectInstanceExtra* GetInstanceDataExtra_Internal(RE::BGSInventoryItem* a_InvItem)
{
    if (!a_InvItem) {
        return nullptr;
    }
    // Check all stacks for instance data
    if (GetStackCount_Internal(a_InvItem) == 0) {
        return nullptr;
    }
    auto* stackData = GetStackData_Internal(a_InvItem);
    if (!stackData) {
        return nullptr;
    }
    std::uint32_t stackID = 0;
    while (stackData) {
        if (stackData->extra) {
            auto instanceExtra = stackData->extra->GetByType<RE::BGSObjectInstanceExtra>();
            if (instanceExtra) {
                // We need to validate the pointer to the indexdataextra by trying to access it to prevent crashes
                try {
                    if (!instanceExtra->values) {
                        return nullptr;
                    }
                    return instanceExtra;
                } catch (...) {
                    if (DEBUGGING) gLog->warn("GetInstanceDataExtra_Internal: Exception accessing index data for item.");
                }
            }
        }
        stackData = stackData->nextStack.get();
        stackID++;
    }
    return nullptr;
}
// Get the instanceDataExtra for an item object
RE::BGSObjectInstanceExtra* GetInstanceDataExtra_Internal(RE::TESObjectREFR* a_Ref)
{
    if (!a_Ref) {
        return nullptr;
    }
    auto currentExtraData = a_Ref->extraList.get();
    if (!currentExtraData) {
        return nullptr;
    }
    auto instanceExtra = currentExtraData->GetByType<RE::BGSObjectInstanceExtra>();
    try {
        // We need to validate the pointer to the indexdataextra by trying to access it to prevent crashes
        if (!instanceExtra->values) {
            return nullptr;
        }
        return instanceExtra;
    } catch (...) {
        if (DEBUGGING) gLog->warn("GetInstanceDataExtra_Internal: Exception accessing index data for item.");
    }
    return instanceExtra;
}

// Get InventoryItem from Actor
RE::BGSInventoryItem* GetInventoryItem_Internal(RE::Actor* akActor, RE::TESForm* akForm)
{
    if (!akActor || !akForm) {
        return nullptr;
    }
    auto inventoryList = akActor->inventoryList;
    if (!inventoryList) {
        return nullptr;
    }
    for (auto& item : inventoryList->data) {
        if (item.object == akForm) {
            return &item;
        }
    }
    return nullptr;
}

// Get the legendary OMOD from a BGSInventoryItem
RE::BGSMod::Attachment::Mod* GetLegendaryOMOD_Internal(RE::BGSInventoryItem* a_InvItem)
{
    if (!a_InvItem || !a_InvItem->object) {
        return nullptr;
    }
    // Get the instancedata for this inventory item
    auto* instanceExtra = GetInstanceDataExtra_Internal(a_InvItem);
    if (!instanceExtra) {
        return nullptr;
    }
    // Iterate through all attached mods and find the legendary one
    try {
        // Get the mod index data
        auto indexData = instanceExtra->GetIndexData();
        if (indexData.empty()) {
            return nullptr;
        }
        for (const auto& modIndex : indexData) {
            if (modIndex.disabled) {
                continue; // Skip disabled mods
            }
            // Convert objectID to actual mod form
            auto* omod = RE::TESForm::GetFormByID<RE::BGSMod::Attachment::Mod>(modIndex.objectID);
            if (omod && IsLegendary_Internal(omod)) {
                return omod;
            }
        }
    } catch (...) {
        if (DEBUGGING) gLog->warn("GetLegendaryOMOD_Internal: Exception accessing instance data for item.");
        return nullptr;
    }
    // no Legendary OMOD found
    return nullptr;
}

// Get the overridename from an InventoryItem
const char* GetOverrideName_Internal(RE::BGSInventoryItem* a_InvItem)
{
    if (!a_InvItem || !a_InvItem->object) {
        return "";
    }
    // Get the Overridename if possible
    const char* overrideName = nullptr;
    auto* stackData = GetStackData_Internal(a_InvItem);
    if (stackData && stackData->extra) {
        auto* textDisplayData = stackData->extra->GetByType<RE::ExtraTextDisplayData>();
        if (textDisplayData) {
            overrideName = textDisplayData->displayName.c_str();
        }
    }
    if (overrideName) {
        return overrideName;
    }
    return "";
}

// Get the stackcount by iteration
uint32_t GetStackCount_Internal(RE::BGSInventoryItem* a_InvItem)
{
    if (!a_InvItem) {
        return 0;
    }
    uint32_t count = 0;
    try {
        if (!a_InvItem->stackData) {
            return 0;
        }
        auto* currentStack = GetStackData_Internal(a_InvItem);
        while (currentStack) {
            count++;
            currentStack = currentStack->nextStack.get();
        }
    } catch (...) {
        if (DEBUGGING) gLog->warn("GetStackCount_Internal: Exception accessing stack data for item.");
    }
    return count;
}

// Get the Stackdata
RE::BGSInventoryItem::Stack* GetStackData_Internal(RE::BGSInventoryItem* a_InvItem)
{
    if (!a_InvItem) {
        return nullptr;
    }
    try {
        if (!a_InvItem->stackData) {
            return nullptr;
        }
        return a_InvItem->stackData.get();
    } catch (...) {
        if (DEBUGGING) gLog->warn("GetStackData_Internal: Exception accessing stack data for item.");
    }
    return nullptr;
}

// Initialize the loose mod associations for legendary OMODs and other data
// This is called on game load and new game start
void InitializeData_Internal()
{
    if (DEBUGGING) gLog->info("InitializeData_Internal: Starting live patching of legendary OMODs...");
    // Clear existing data
    g_noLegendaryArrays.ClearAll();
    // Check data handle
    if (!g_dataHandle) {
        if (DEBUGGING) gLog->warn("InitializeData_Internal: g_dataHandle is null.");
        return;
    }
    // Check data player
    RE::Actor* player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        if (DEBUGGING) gLog->warn("InitializeData_Internal: Player actor is null.");
        return;
    }
    // Get all COBJ of this mod
    auto cobjItems = GetAllCOBJ_Internal();
    if (cobjItems.empty()) {
        if (DEBUGGING) gLog->warn("InitializeData_Internal: No COBJ items found.");
        return;
    }
    for (auto* cobj : cobjItems) {
        if (cobj) {
            g_noLegendaryArrays.addCOBJ(cobj);
        }
    }
    // Get all loose misc items from the game
    auto looseMiscItems = GetAllLooseMiscItems_Internal();
    if (looseMiscItems.empty()) {
        if (DEBUGGING) gLog->warn("InitializeData_Internal: No loose misc items found.");
        return;
    }
    // Get all OMODs from the game data
    auto& omodForms = g_dataHandle->GetFormArray<RE::BGSMod::Attachment::Mod>();
    if (omodForms.empty()) {
        if (DEBUGGING) gLog->warn("InitializeData_Internal: No OMODs found in game data.");
        return;
    }
    // General count for mapped Legendary Mods
    int mapCount = 0;
    // First map existing loose mod misc items to their OMODs based on naming convention
    // This is important to preserve existing player inventory associations when a game is loaded
    auto inventoryList = player->inventoryList;
    if (inventoryList) {
        if (DEBUGGING) gLog->info("InitializeData_Internal: Actor inventory list size: {}", inventoryList->data.size());
        for (auto& invItem : inventoryList->data) {
            // process only loose mod misc items
            if (!IsMiscItem_Internal(&invItem)) continue;
            // prefer explicit override name on the stack, else display name
            const char* overrideName = GetOverrideName_Internal(&invItem);
            std::string name;
            if (overrideName && *overrideName) name = overrideName;
            else name = GetDisplayName_Internal(&invItem);
            if (name.empty()) continue;
            // extract the FormID from the name 'NL-ARMO-Poisoner's [001F3072]'
            std::string miscNameStr(name);  // Convert string_view to string
            // Acquire the Legendary OMOD from the name
            std::uint32_t parsedOMODFormID = GetFormIDFromLooseMiscName_Internal(name);
            if (parsedOMODFormID == 0) continue;
            //std::string formIDStr = miscNameStr.substr(miscNameStr.rfind(']') - 8, 8);
            auto* legendaryOMOD = RE::TESForm::GetFormByID<RE::BGSMod::Attachment::Mod>(parsedOMODFormID);
            if (!legendaryOMOD) continue;
            // Add our OMOD mapping
            if (g_noLegendaryArrays.addOMODMapping(legendaryOMOD, invItem.object)) {
                mapCount++;
            } else {
                continue;
            }
            // Get the omod name and description
            std::string omodName = legendaryOMOD->GetFormEditorID();
            // Get the corresponding dummy OMODs for armor and weapons
            // Format the MISC items EditorID last 3 characters to 3-digit string (001, 002, etc.)
            // This ensures the names match the misc item index
            std::string miscEditorID = invItem.object->GetFormEditorID();
            std::string formattedIndex = miscEditorID.substr(miscEditorID.length() - 3);
            // Build complete EditorIDs
            std::string dummyARMOEditorID = g_editorIDDummyARMOOMODPrefix + formattedIndex;
            std::string dummyWEAPEditorID = g_editorIDDummyWEAPOMODPrefix + formattedIndex;
            RE::BGSMod::Attachment::Mod* dummyOMODARMO = nullptr;
            RE::BGSMod::Attachment::Mod* dummyOMODWEAP = nullptr;
            dummyOMODARMO = RE::TESForm::GetFormByEditorID<RE::BGSMod::Attachment::Mod>(dummyARMOEditorID);
            dummyOMODWEAP = RE::TESForm::GetFormByEditorID<RE::BGSMod::Attachment::Mod>(dummyWEAPEditorID);
            // Add our Dummy OMOD mapping
            g_noLegendaryArrays.addDummyARMOMapping(dummyOMODARMO, invItem.object);
            g_noLegendaryArrays.addDummyWEAPMapping(dummyOMODWEAP, invItem.object);
            // Rename them to match the original OMOD plus our prefix for users to know the recipe is from this mod
            std::string prefixedName = "NL - " + omodName;
            std::string_view currentName = prefixedName;
            if (dummyOMODARMO) {
                dummyOMODARMO->SetFullName(*dummyOMODARMO, currentName);
            }
            if (dummyOMODWEAP) {
                //dummyOMODWEAP->fullName = currentName;
                dummyOMODWEAP->SetFullName(*dummyOMODWEAP, currentName);
            }
            if (DEBUGGING) gLog->info("InitializeData_Internal: Found loose MISC {:08X} mapped to OMOD {:08X} (name='{}')", invItem.object->GetFormID(), legendaryOMOD->GetFormID(), name);
        }
    }
    // Rebuild available loose misc items excluding already mapped ones
    RE::BSTArray<RE::TESObjectMISC*> unmappedLooseMiscItems;
    for (auto* miscItem : looseMiscItems) {
        if (!miscItem) continue;
        if (g_noLegendaryArrays.miscToOmod.contains(miscItem)) {
            continue; // Skip already mapped misc items
        }
        unmappedLooseMiscItems.push_back(miscItem);
    }
    // Now process all legendary OMODs to map loose misc items
    int skippedCount = 0;
    int miscIndex = 0;
    // Iterate through all OMODs
    for (auto* LegendaryOMOD : omodForms) {
        if (!LegendaryOMOD) continue;
        // Check if OMOD is legendary
        if (!IsLegendary_Internal(LegendaryOMOD)) {
            continue; // Skip non-legendary OMODs
        }
        // Check if OMOD is already mapped
        if (g_noLegendaryArrays.omodToMisc.contains(LegendaryOMOD)) {
            continue; // Skip already mapped OMODs
        }
        // Check if we have misc items available
        if (miscIndex < unmappedLooseMiscItems.size()) {
            auto* miscItem = unmappedLooseMiscItems[miscIndex];
            std::string miscEditorID = miscItem->GetFormEditorID();
            // Add our OMOD mapping
            if (g_noLegendaryArrays.addOMODMapping(LegendaryOMOD, miscItem)) {
                mapCount++;
                miscIndex++;
            } else {
                skippedCount++;
                continue;
            }
            // Get the omod name and description
            std::string omodName = g_noLegendaryArrays.omod2FullName(LegendaryOMOD);
            // Get the corresponding dummy OMODs for armor and weapons
            // Format the MISC items EditorID last 3 characters to 3-digit string (001, 002, etc.)
            // This ensures the names match the misc item index
            std::string formattedIndex = miscEditorID.substr(miscEditorID.length() - 3);
            // Build complete EditorIDs
            std::string dummyARMOEditorID = g_editorIDDummyARMOOMODPrefix + formattedIndex;
            std::string dummyWEAPEditorID = g_editorIDDummyWEAPOMODPrefix + formattedIndex;
            RE::BGSMod::Attachment::Mod* dummyOMODARMO = nullptr;
            RE::BGSMod::Attachment::Mod* dummyOMODWEAP = nullptr;
            dummyOMODARMO = RE::TESForm::GetFormByEditorID<RE::BGSMod::Attachment::Mod>(dummyARMOEditorID);
            dummyOMODWEAP = RE::TESForm::GetFormByEditorID<RE::BGSMod::Attachment::Mod>(dummyWEAPEditorID);
            // Add our Dummy OMOD mapping
            g_noLegendaryArrays.addDummyARMOMapping(dummyOMODARMO, miscItem);
            g_noLegendaryArrays.addDummyWEAPMapping(dummyOMODWEAP, miscItem);
            // Rename them to match the original OMOD plus our prefix for users to know the recipe is from this mod
            std::string prefixedName = "NL - " + omodName;
            std::string_view currentName = prefixedName;
            if (dummyOMODARMO) {
                dummyOMODARMO->SetFullName(*dummyOMODARMO, currentName);
            }
            if (dummyOMODWEAP) {
                dummyOMODWEAP->SetFullName(*dummyOMODWEAP, currentName);
            }
        } else {
            // No more misc items available
            break; // Stop processing since we're out of misc items
        }
    }
    // Iterate over the COBJ and disable any without omod mapped
    int disabledCount = 0;
    for (auto* cobj : g_noLegendaryArrays.COBJData) {
        if (!cobj || !cobj->requiredItems || cobj->requiredItems->empty())
            continue;
        bool hasValidMapping = false;
        if (g_editorIDDummyOMOD.find(cobj->createdItem->GetFormEditorID()) != g_editorIDDummyOMOD.end()) {
            continue; // Skip Removal dummy OMODs
        }
        for (const auto& req : *(cobj->requiredItems)) {
            auto* reqItem = req.first;
            if (reqItem && IsMiscItem_Internal(reqItem)) {
                if (g_noLegendaryArrays.misc2OMOD(reqItem)) {
                    hasValidMapping = true;
                    break;
                }
            }
        }
        if (!hasValidMapping) {
            DisableCOBJ(cobj);
            disabledCount++;
            if (DEBUGGING) gLog->info("Disabled COBJ {} due to missing OMOD mapping.", cobj->GetFormEditorID());
        }
    }
    if (DEBUGGING) {
        gLog->info("InitializeData_Internal: Processed {} COBJ items. Disabled {} COBJ due to missing OMOD mappings.", g_noLegendaryArrays.COBJData.size(), disabledCount);
    }
    if (DEBUGGING) {
        gLog->info("InitializeData_Internal: Live patching complete. {} Legendary OMODs mapped. Used {}/{} misc items.", mapCount, miscIndex, looseMiscItems.size());
    }
}

// For TESForm directly (what you actually need)
bool IsArmorOrWeapon_Internal(RE::TESForm* a_form)
{
    if (!a_form) {
        return false;
    }
    // First check the form type
    bool isArmorType = a_form->Is(RE::ENUM_FORM_ID::kARMO);
    bool isWeaponType = a_form->Is(RE::ENUM_FORM_ID::kWEAP);
    // If it's neither armor nor weapon, return false immediately
    if (!isArmorType && !isWeaponType) {
        return false;
    }
    // Now try to actually cast to verify the data is valid
    try {
        if (isArmorType) {
            auto* armor = a_form->As<RE::TESObjectARMO>();
            if (!armor) {
                // Form says it's armor but cast failed - corrupted data
                if (DEBUGGING) gLog->warn("IsArmorOrWeapon_Internal: Form {:08X} claims to be armor but cast failed.", a_form->GetFormID());
                return false;
            }
        }
        if (isWeaponType) {
            auto* weapon = a_form->As<RE::TESObjectWEAP>();
            if (!weapon) {
                // Form says it's weapon but cast failed - corrupted data
                if (DEBUGGING) gLog->warn("IsArmorOrWeapon_Internal: Form {:08X} claims to be weapon but cast failed.", a_form->GetFormID());
                return false;
            }
        }
    } catch (...) {
        // Cast operation crashed - definitely corrupted data
        if (DEBUGGING) gLog->warn("IsArmorOrWeapon_Internal: Exception during cast for form {:08X}.", a_form->GetFormID());
        return false;
    }
    return true;
}
// For BGSInventoryItem 
bool IsArmorOrWeapon_Internal(RE::BGSInventoryItem* a_inventoryItem)
{
    if (!a_inventoryItem) {
        return false;
    }
    return IsArmorOrWeapon_Internal(a_inventoryItem->object);
}
// For TESObjectREFR
bool IsArmorOrWeapon_Internal(RE::TESObjectREFR* a_objectReference)
{
    if (!a_objectReference) {
        return false;
    }
    auto baseForm = a_objectReference->GetObjectReference();
    return IsArmorOrWeapon_Internal(baseForm);
}

// Check if a given COBJ is valid by EditorID
bool IsCOBJ_Internal(RE::BGSConstructibleObject* a_COBJ)
{
    if (!a_COBJ) {
        return false;
    }
    // Check EditorID prefix instead of keyword
    auto editorID = a_COBJ->GetFormEditorID();
    if (!editorID) {
        return false;
    }
    // Check if EditorID starts with "NoLegendaryCOBJ"
    std::string editorIDStr(editorID);
    return editorIDStr.starts_with(g_editorIDCOBJPrefix);
}

// Check if the omod is a dummy OMOD by EditorID
bool IsDummyOMOD_Internal(RE::BGSMod::Attachment::Mod* omod)
{
    if (!omod) return false;
    auto editorID = omod->GetFormEditorID();
    if (!editorID) return false;
    // Check exact matches
    if (g_editorIDDummyOMOD.find(editorID) != g_editorIDDummyOMOD.end()) {
        return true;
    }
    // Check prefix matches
    if (std::string(editorID).starts_with(g_editorIDDummyARMOOMODPrefix) || 
        std::string(editorID).starts_with(g_editorIDDummyWEAPOMODPrefix)) {
        return true;
    }
    return false;
}

// Check if a given BGSInventoryItem is armor or weapon and has a legendary mod
bool IsLegendary_Internal(RE::BGSInventoryItem* a_invItem)
{
    if (!a_invItem) return false;
    // Only check armor and weapons
    if (!IsArmorOrWeapon_Internal(a_invItem->object)) return false;
    if (GetLegendaryOMOD_Internal(a_invItem)) return true;
    return false;
}
// Check if an OMOD is a legendary modification
bool IsLegendary_Internal(RE::BGSMod::Attachment::Mod* omod)
{
    if (!omod) return false;
    // Get the keyword from the attach point using the helper function
    auto attachPointKeyword = RE::detail::BGSKeywordGetTypedKeywordByIndex(
        RE::KeywordType::kAttachPoint, 
        omod->attachPoint.keywordIndex
    );
    if (attachPointKeyword) {
        // Check the EditorID
        auto editorID = attachPointKeyword->GetFormEditorID();
        if (editorID && strcmp(editorID, g_keywordLegendary.c_str()) == 0 && !IsDummyOMOD_Internal(omod)) {
            return true;
        }
    }
    return false;
}

// Check if a given TESForm is a Misc item with all required keywords
bool IsMiscItem_Internal(RE::BGSInventoryItem* a_Item)
{
    if (!a_Item) {
        return false;
    }
    // Get the base form from the inventory item
    auto form = a_Item->object;
    if (!form) {
        return false;
    }
    // First check if it's a MISC item
    if (!form->Is(RE::ENUM_FORM_ID::kMISC)) {
        return false;
    }
    // Check EditorID prefix instead of keyword
    auto editorID = form->GetFormEditorID();
    if (!editorID) {
        return false;
    }
    // Check if EditorID starts with "NoLegendaryMISC"
    std::string editorIDStr(editorID);
    return editorIDStr.starts_with(g_editorIDLooseMiscPrefix);
}
bool IsMiscItem_Internal(RE::TESForm* a_Form)
{
    if (!a_Form) {
        return false;
    }
    // First check if it's a MISC item
    if (!a_Form->Is(RE::ENUM_FORM_ID::kMISC)) {
        return false;
    }
    // Cast to TESObjectMISC to access keywords
    auto miscItem = a_Form->As<RE::TESObjectMISC>();
    if (!miscItem) {
        return false;
    }
    // Check EditorID prefix instead of keyword
    auto editorID = a_Form->GetFormEditorID();
    if (!editorID) {
        return false;
    }
    // Check if EditorID starts with "NoLegendaryMISC"
    std::string editorIDStr(editorID);
    return editorIDStr.starts_with(g_editorIDLooseMiscPrefix);
}

// Helper function to drop a single inventory item from an NPC
RE::ObjectRefHandle ItemDrop_Internal(RE::Actor* aNPC, RE::BGSInventoryItem* aInvItem)
{
    RE::ObjectRefHandle droppedHandle = RE::ObjectRefHandle();
    if (!aInvItem || !aInvItem->object || !aNPC) return droppedHandle;
    // Extract instance data
    auto* instanceData = GetInstanceData_Internal(aInvItem);
    // Get the ObjectInstance for the item
    RE::BGSObjectInstance objectInstance(aInvItem->object, instanceData);
    // Drop the item at the actor's position
    auto actorPosition = aNPC->GetPosition();
    droppedHandle = aNPC->DropObject(objectInstance, nullptr, 1, &actorPosition, nullptr);
    return droppedHandle;
}
RE::ObjectRefHandle ItemDrop_Internal(RE::Actor* aNPC, RE::TESObjectMISC* aMiscItem)
{
    RE::ObjectRefHandle droppedHandle = RE::ObjectRefHandle();
    if (!aMiscItem || !aNPC) return droppedHandle;
    // Extract instance data
    auto* instanceData = GetInstanceData_Internal(aMiscItem->As<RE::TESObjectREFR>());
    // Construct a BGSObjectInstance for a base MISC with no instance data (base misc, not a placed reference)
    RE::BGSObjectInstance objectInstance(aMiscItem, instanceData);
    // Drop the item at the actor's position
    auto actorPosition = aNPC->GetPosition();
    droppedHandle = aNPC->DropObject(objectInstance, nullptr, 1, &actorPosition, nullptr);
    return droppedHandle;
}

// Helper function to make an NPC pick up an inventory item
bool ItemPickup_Internal(RE::Actor* aNPC, RE::TESObjectREFR* aInvItem)
{
    if (!aNPC || !aInvItem) {
        return false;
    }
    aNPC->PickUpObject(aInvItem, 1, true);
    return true;
}

// minimal SEH helper — no C++ locals, only raw pointers/ints
static bool ValidateInstanceKeywordData_Internal(RE::TBO_InstanceData* a_instance, RE::BGSKeyword* a_testKeyword) noexcept
{
    if (!a_instance || !a_testKeyword) return false;
    #if defined(_MSC_VER)
        __try {
            auto* kd = a_instance->GetKeywordData();
            if (!kd) return false;
            // call that may access bad memory
            volatile bool has = kd->HasKeyword(a_testKeyword);
            (void)has;
            return true;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    #else
        try {
            auto* kd = a_instance->GetKeywordData();
            if (!kd) return false;
            volatile bool has = kd->HasKeyword(a_testKeyword);
            (void)has;
            return true;
        } catch (...) {
            return false;
        }
    #endif
}

bool OMODClearDummy_Internal(RE::BGSInventoryItem* aInvItem)
{
    // Get all OMODs attached to this item
    auto instanceExtra = GetInstanceDataExtra_Internal(aInvItem);
    if (!instanceExtra) {
        return false;
    }
    auto indexData = instanceExtra->GetIndexData();
    // Remove ALL OMODs that use the legendary attach point
    for (const auto& modIndex : indexData) {
        auto* omod = RE::TESForm::GetFormByID<RE::BGSMod::Attachment::Mod>(modIndex.objectID);
        if (omod && IsDummyOMOD_Internal(omod)) {
            // Remove this legendary OMOD (dummy or real)
            instanceExtra->RemoveMod(omod, omod->attachPoint.keywordIndex);
            if (DEBUGGING) gLog->info("OMODClearDummy_Internal: Removed dummy OMOD {:08X} from item {:08X}.", omod->GetFormID(), aInvItem->object->GetFormID());
        }
    }
    // Refresh the item instance data
    auto instanceData = GetInstanceData_Internal(aInvItem);
    if (instanceData) {
        instanceData->PostApplyMods(aInvItem->object);
    }
    return true;
}

bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    if (DEBUGGING) gLog->info("RegisterPapyrusFunctions: Attempting to register Papyrus functions. VM pointer: {}", static_cast<const void*>(vm));

    // vm->BindNativeMethod("<Name of the script binding the function>", "<Name of the function in Papyrus>", <Name of the function in F4SE>, <can run parallel to Papyrus>);

    if (DEBUGGING) gLog->info("RegisterPapyrusFunctions: All Papyrus functions registration attempts completed.");

    return true;
}

// Hook into the ExamineMenu CreateModdedInventoryItem() function
// This function is called to preview the outcome of the modding process in the Workbench menu
using CreateModdedInventoryItem_t = RE::BGSInventoryItem*(RE::ExamineMenu*);
CreateModdedInventoryItem_t* _originalCreateModdedInventoryItem = nullptr;
void MyCreateModdedInventoryItem(RE::ExamineMenu* menu)
{
    if (DEBUGGING) gLog->info("MyCreateModdedInventoryItem: ExamineMenu CreateModdedInventoryItem hook called.");
    // Call the original function first
    _originalCreateModdedInventoryItem(menu);
    // Now process our logic
    auto* modChoice = const_cast<RE::WorkbenchMenuBase::ModChoiceData*>(menu->QCurrentModChoiceData());
    auto* invItem = &menu->moddedInventoryItem;
    auto* recipe = modChoice ? modChoice->recipe : nullptr;
    auto* mod = modChoice ? modChoice->mod : nullptr;
    if (invItem && recipe && mod) {
        if (DEBUGGING) gLog->info("MyCreateModdedInventoryItem: Current mod choice data is valid. Item: {}", menu->moddedInventoryItem.GetDisplayFullName(0));
        if (DEBUGGING) gLog->info("MyCreateModdedInventoryItem: Current mod choice data is valid. COBJ: {}", recipe->GetFormEditorID());
        if (DEBUGGING) gLog->info("MyCreateModdedInventoryItem: Current mod choice data is valid. Mod: {}", mod->fullName.c_str());
        // Check if the COBJ is one of ours
        auto* cobj = const_cast<RE::BGSConstructibleObject*>(recipe);
        if (g_noLegendaryArrays.containsCOBJ(cobj)) {
            if (DEBUGGING) gLog->info("MyCreateModdedInventoryItem: The selected COBJ is one of our NoLegendaryCOBJ recipes!");
            if (g_editorIDDummyOMOD.find(mod->GetFormEditorID()) != g_editorIDDummyOMOD.end()) {
                if (DEBUGGING) gLog->info("MyCreateModdedInventoryItem: The selected OMOD is the Remove Legendary effect Dummy!");
            } else {
                if (DEBUGGING) gLog->info("MyCreateModdedInventoryItem: The selected OMOD is an Add Legendary effect Dummy!");
                // Check if any required items are our misc items
                // If it is, then this is a legendary Adding process, so we need to swap the dummy mod to the correct legendary OMOD
                auto& requiredItems = *(recipe->requiredItems);
                for (std::size_t i = 0; i < requiredItems.size(); ++i) {
                    auto* reqItem = requiredItems[i].first;
                    if (!reqItem || !reqItem->formID) continue;
                    if (!IsMiscItem_Internal(reqItem)) continue;
                    const char* miscEditorID = reqItem->GetFormEditorID();
                    std::string name = miscEditorID ? miscEditorID : "<no editorID>";
                    if (DEBUGGING) gLog->info("MyCreateModdedInventoryItem: Required Misc item: {} (FormID: {:08X})", miscEditorID, reqItem->GetFormID());
                    // Lookup the mapped OMOD (miscFormID -> omodFormID)
                    RE::BGSMod::Attachment::Mod* legendaryOMOD = nullptr;
                    legendaryOMOD = g_noLegendaryArrays.misc2OMOD(reqItem);
                    if (!legendaryOMOD) {
                        // Silent continue, no menu update without mapped legendary OMOD
                        continue;
                    }
                    if (DEBUGGING && legendaryOMOD) gLog->info("MyCreateModdedInventoryItem: Mapped misc {:08X} -> OMOD {:08X}", reqItem->GetFormID(), legendaryOMOD->GetFormID());
                    // Swap DUMMY omod to legendary OMOD
                    modChoice->mod = legendaryOMOD;
                    if (DEBUGGING) gLog->info("MyCreateModdedInventoryItem: Swapped DUMMY OMOD to {}!", legendaryOMOD->GetFormEditorID());
                    // Change the name on dummy OMOD for the menu display
                    RE::BGSMod::Attachment::Mod* dummyOMODARMO = g_noLegendaryArrays.misc2DummyARMO(reqItem);
                    RE::BGSMod::Attachment::Mod* dummyOMODWEAP = g_noLegendaryArrays.misc2DummyWEAP(reqItem);
                    if (dummyOMODARMO && dummyOMODWEAP) {
                        std::string prefixedName = "NL - ";
                        std::string omodName = g_noLegendaryArrays.omod2FullName(legendaryOMOD);
                        std::string_view newName = prefixedName + omodName;
                        // Set the name for preview
                        dummyOMODARMO->SetFullName(*dummyOMODARMO, newName);
                        dummyOMODWEAP->SetFullName(*dummyOMODWEAP, newName);
                        if (DEBUGGING) gLog->info("MyCreateModdedInventoryItem: Changed dummy OMOD name to {}", newName);
                    }
                }
            }
        }
    }
    // This below breaks the menu for this mod
    // menu->UpdateModChoiceList();
    // No effect
    //menu->UpdateItemCard(false);
    //menu->UpdateModSlotList();
}

// Hook into the ExamineMenu TryCreate() function
// This is called when the player mods an item at the Workbench
using TryCreate_t = bool(RE::ExamineMenu*);
TryCreate_t* _originalTryCreate = nullptr;
bool MyTryCreate(RE::ExamineMenu* menu)
{
    if (DEBUGGING) gLog->info("MyTryCreate: ExamineMenu TryCreate hook called.");
    //const auto* modChoice = menu->QCurrentModChoiceData();
    // Get the current mod choice data as non-const to allow modification
    auto* modChoice = const_cast<RE::WorkbenchMenuBase::ModChoiceData*>(menu->QCurrentModChoiceData());
    auto* invItem = &menu->moddedInventoryItem;
    auto* recipe = modChoice ? modChoice->recipe : nullptr;
    auto* mod = modChoice ? modChoice->mod : nullptr;
    if (invItem && recipe && mod) {
        // Check if the COBJ is one of ours
        auto* cobj = const_cast<RE::BGSConstructibleObject*>(recipe);
        if (g_noLegendaryArrays.containsCOBJ(cobj)) {
            if (DEBUGGING) gLog->info("MyTryCreate: The selected COBJ is one of our NoLegendaryCOBJ recipes!");
            if (DEBUGGING) gLog->info("MyTryCreate: Current mod choice data is valid. Item: {}", menu->moddedInventoryItem.GetDisplayFullName(0));
            if (DEBUGGING) gLog->info("MyTryCreate: Current mod choice data is valid. COBJ: {}", recipe->GetFormEditorID());
            if (DEBUGGING) gLog->info("MyTryCreate: Current mod choice data is valid. Mod: {}", mod->fullName.c_str());
            if (g_editorIDDummyOMOD.find(mod->GetFormEditorID()) != g_editorIDDummyOMOD.end()) {
                if (DEBUGGING) gLog->info("MyTryCreate: The selected OMOD is the Remove Legendary effect Dummy!");
                auto* instanceDataExtraList = menu->GetItemExtraDataList();
                auto* legendaryMod = const_cast<RE::ExtraDataList*>(instanceDataExtraList)->GetLegendaryMod();
                if (legendaryMod) {
                    if (DEBUGGING) gLog->info("MyTryCreate: Found legendary mod to remove: {} (FormID: {:08X})", legendaryMod->fullName.c_str(), legendaryMod->GetFormID());
                    RE::TESForm* miscForm = nullptr;
                    auto omodIt = g_noLegendaryArrays.omodToMisc.find(legendaryMod);
                    if (omodIt != g_noLegendaryArrays.omodToMisc.end()) {
                        RE::TESForm* miscForm = omodIt->second;
                        RE::TESObjectMISC* looseMisc = nullptr;
                        looseMisc = miscForm->As<RE::TESObjectMISC>();
                        // Get the OMOD data
                        auto omodFullName = g_noLegendaryArrays.omod2FullName(legendaryMod);
                        auto omodFormID = legendaryMod->GetFormID();
                        if (DEBUGGING) gLog->info("MyTryCreate: OMOD FullName: {}, FormID: {:08X}", omodFullName, omodFormID);
                        // Determine if it's weapon or armor
                        bool isWeapon = invItem->object->Is(RE::ENUM_FORM_ID::kWEAP);
                        std::string prefix = isWeapon ? "NL-WEAP-" : "NL-ARMO-";
                        // Build the full name with all components
                        std::string newName = prefix;
                        if (!omodFullName.empty()) {
                            newName += omodFullName;
                        } else {
                            newName += "Legendary";
                        }
                        // Add FormID for persistence
                        newName += " [" + std::format("{:08X}", omodFormID) + "]";
                        if (DEBUGGING) gLog->info("MyTryCreate: Constructed new misc name: {}", newName);
                        // Set the name on the baseform as well for future reference
                        looseMisc->SetFullName(*looseMisc, newName);
                        RE::Actor* player = RE::PlayerCharacter::GetSingleton();
                        if (!player) {
                            if (DEBUGGING) gLog->warn("MyTryCreate: player is null.");
                            return false;
                        }
                        // Add the misc to the player's inventory, drop it, and pick it up to ensure proper instance data handling
                        // If something goes wrong here, the item will appear as "Legendary" and not persist across saves
                        player->AddObjectToContainer(looseMisc, nullptr, 1, nullptr, RE::ITEM_REMOVE_REASON::kStoreContainer);
                        auto droppedHandle = ItemDrop_Internal(player, looseMisc);
                        auto droppedRef = droppedHandle.get();
                        ItemPickup_Internal(player, droppedRef.get());
                        // For savegame persistence
                        auto invMiscItem = GetInventoryItem_Internal(player, looseMisc);
                        auto* stackData = GetStackData_Internal(invMiscItem);
                        if (invMiscItem && stackData && stackData->extra) {
                            stackData->extra.get()->SetOverrideName(newName.c_str());
                            if (DEBUGGING) gLog->info("MyTryCreate: Set override name on misc extra data for persistence.");
                        }
                    }
                }
            } else {
                if (DEBUGGING) gLog->info("MyTryCreate: The selected OMOD is an Add Legendary effect Dummy!");
                // Check if any required items are our misc items
                // If it is, then this is a legendary Adding process, so we need to swap the dummy mod to the correct legendary OMOD
                auto& requiredItems = *(recipe->requiredItems);
                for (std::size_t i = 0; i < requiredItems.size(); ++i) {
                    auto* reqItem = requiredItems[i].first;
                    if (!reqItem || !reqItem->formID) continue;
                    if (IsMiscItem_Internal(reqItem)) {
                        if (DEBUGGING) gLog->info("MyTryCreate: The Required item {} is one of our MISC items! FormID: {:08X}", reqItem->GetFormEditorID(), reqItem->GetFormID());
                        // Try map lookup (miscFormID -> omodFormID)
                        RE::BGSMod::Attachment::Mod* legendaryOMOD = g_noLegendaryArrays.misc2OMOD(reqItem);
                        if (!legendaryOMOD) {
                            // Silent continue, no menu update without mapped legendary OMOD
                            continue;
                        }
                        if (DEBUGGING && legendaryOMOD) gLog->info("MyTryCreate: Mapped misc {:08X} -> OMOD {:08X}", reqItem->GetFormID(), legendaryOMOD->GetFormID());
                        // Swap DUMMY omod to legendary OMOD
                        modChoice->mod = legendaryOMOD;
                        if (DEBUGGING) gLog->info("MyTryCreate: Swapped DUMMY OMOD to {}!", legendaryOMOD->GetFormEditorID());
                    }
                }
            }
        }
        // We need to call the original function at the end or the game crashes
        if (DEBUGGING) gLog->info("MyTryCreate: Calling original TryCreate function.");
        bool originalResult = _originalTryCreate(menu);
        if (DEBUGGING && originalResult) gLog->info("MyTryCreate: Original TryCreate function returned true.");
        // clear dummy OMODs from the item if any
        bool clearResult = OMODClearDummy_Internal(invItem);
        if (DEBUGGING && clearResult) gLog->info("MyTryCreate: Cleared dummy OMODs from the modded inventory item.");
        // call update menu to refresh display
        menu->UpdateMenu();
        // return the original result
        return originalResult;
    } else {
        if (DEBUGGING) gLog->warn("MyTryCreate: Calling original TryCreate function.");
        return _originalTryCreate(menu);
    }
}

bool InstallExamineMenuHooks()
{
    // Get the vtable for ExamineMenu
    auto vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE::ExamineMenu[0]);
    // Overwrite vfunc at index 0x1B (27 decimal)
    _originalTryCreate = reinterpret_cast<TryCreate_t*>(
        vtbl.write_vfunc(0x1B, &MyTryCreate)
    );
    // Overwrite vfunc at index 0x2A (42 decimal)
    _originalCreateModdedInventoryItem = reinterpret_cast<CreateModdedInventoryItem_t*>(
        vtbl.write_vfunc(0x2A, &MyCreateModdedInventoryItem)
    );
    return true;
}