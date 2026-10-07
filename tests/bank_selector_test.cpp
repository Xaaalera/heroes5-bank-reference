// Exercise the actual compiled installer only in this disposable test process.
#define wmain bank_launcher_entry
#include "../src/bank_launcher.cpp"
#undef wmain
#include <algorithm>
#include "selector_runtime.hpp"
#include <thread>

int wmain(int count, wchar_t* arguments[]) {
    try {
        if (count == 2) {
            const auto module = LoadLibraryExW(arguments[1], nullptr,
                LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
            Require(module != nullptr, "Cannot load the managed payload fixture.");
            using Query = const heroes5_sdk::BankSelectorPayload* (__cdecl*)();
            const auto query = reinterpret_cast<Query>(GetProcAddress(module, "Heroes5BankSelectorQuery"));
            Require(query != nullptr, "Managed query export is missing.");
            const auto* descriptor = query();
            Require(descriptor && descriptor == query() && descriptor->size == sizeof(*descriptor)
                    && descriptor->version == 1, "Managed descriptor shape or stability differs.");
            Require(heroes5_sdk::SelectorModuleBytes(module, descriptor, sizeof(*descriptor))
                    && heroes5_sdk::SelectorModuleBytes(module, descriptor->code, descriptor->codeBytes)
                    && heroes5_sdk::SelectorModuleBytes(module, descriptor->initialData, descriptor->dataBytes),
                    "Managed descriptor buffers are outside their module.");
            uint32_t foreignBuffer = 0;
            Require(!heroes5_sdk::SelectorModuleBytes(module, &foreignBuffer, sizeof(foreignBuffer))
                    && !heroes5_sdk::SelectorModuleBytes(module, descriptor->code, static_cast<size_t>(-1)),
                    "Module validation accepted a foreign or overflowing span.");
            Require(descriptor->codeBytes == sizeof(bank_payload::callbackCode)
                    && descriptor->dataBytes == sizeof(bank_payload::data)
                    && memcmp(descriptor->code, bank_payload::callbackCode, descriptor->codeBytes) == 0
                    && memcmp(descriptor->initialData, bank_payload::data, descriptor->dataBytes) == 0,
                    "Managed query bytes differ from the compiled fixture.");
            Require(descriptor->dataSchema == bank_payload::dataSchema
                    && strcmp(descriptor->packageSha256, bank_payload::packageHash) == 0,
                    "Managed query state/package identities differ.");
            std::vector<unsigned char> copiedCode(descriptor->code, descriptor->code + descriptor->codeBytes);
            Require(FreeLibrary(module), "Managed query fixture module did not unload.");
            Require(memcmp(copiedCode.data(), bank_payload::callbackCode, copiedCode.size()) == 0,
                    "Copied callback did not survive payload module unload.");
            return 0;
        }
        // Optional fixture-only resource installation. Never calls Run or starts a game.
        if (count == 3) {
            InstallPackage(arguments[1], arguments[2]);
            return 0;
        }
        Require(count == 1, "Expected either no arguments or fixture source/target paths.");
        void* page = VirtualAlloc(nullptr, 0x10000,
                                 MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        Require(page != nullptr, "Cannot reserve fake game memory.");
        auto* hook = static_cast<unsigned char*>(page) + 0x800;
        const auto hookAddress = reinterpret_cast<uintptr_t>(hook);
        bool refused = false;
        try { InstallSelector(GetCurrentProcess(), hookAddress); }
        catch (const std::runtime_error&) { refused = true; }
        Require(refused && hook[0] == 0, "Unexpected bytes were patched.");
        memcpy(hook, original, sizeof(original));
        DWORD ignored{};
        Require(VirtualProtect(page, 0x10000, PAGE_EXECUTE_READ, &ignored), "Test protection failed.");
        InstallSelector(GetCurrentProcess(), hookAddress);
        Require(hook[0] == 0xe9 && hook[5] == 0x90, "Hook was not installed.");
        int32_t displacement{};
        memcpy(&displacement, hook + 1, 4);
        auto* installed = reinterpret_cast<unsigned char*>(hookAddress + 5 + displacement);
        std::vector<unsigned char> expected(std::begin(bank_payload::code), std::end(bank_payload::code));
        const uint32_t delta = reinterpret_cast<uintptr_t>(installed) - bank_payload::base;
        for (const auto& relocation : bank_payload::codeRelocations) {
            Rebase(expected, relocation.offset, relocation.direction > 0 ? delta : 0u - delta);
        }
        Require(memcmp(installed, expected.data(), expected.size()) == 0, "Installed code differs.");
        Require(memcmp(installed + 4096, "H5UIcnt1", 8) == 0, "Reference table missing.");
        MEMORY_BASIC_INFORMATION memory{};
        Require(VirtualQuery(installed, &memory, sizeof(memory)) && memory.Protect == PAGE_EXECUTE_READ,
                "Reference code must not stay writable.");
        Require(VirtualQuery(installed + 4096, &memory, sizeof(memory)) && memory.Protect == PAGE_READWRITE,
                "Reference data must not be executable.");
        Require(VirtualQuery(hook, &memory, sizeof(memory)) && memory.Protect == PAGE_EXECUTE_READ,
                "Entry protection was not restored.");
        refused = false;
        try { InstallSelector(GetCurrentProcess(), hookAddress); }
        catch (const std::runtime_error&) { refused = true; }
        Require(refused, "Existing hook was silently overwritten.");

        // Exercise generated callback bytes as an ordinary C++ call, with a
        // separate retained route allocation. Substitute only the stock-root
        // global operand: this process contains no actual game globals.
        auto* callbackMemory = static_cast<unsigned char*>(VirtualAlloc(nullptr, 4096,
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
        auto* routeMemory = static_cast<unsigned char*>(VirtualAlloc(nullptr, sizeof(bank_payload::data),
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
        Require(callbackMemory && routeMemory, "Cannot allocate callback fixture.");
        std::vector<unsigned char> callback(std::begin(bank_payload::callbackCode), std::end(bank_payload::callbackCode));
        const uint32_t codeDelta = reinterpret_cast<uintptr_t>(callbackMemory) - bank_payload::base;
        const uint32_t dataDelta = reinterpret_cast<uintptr_t>(routeMemory) - bank_payload::base - 4096;
        for (const auto& relocation : bank_payload::callbackCodeRelocations) {
            Rebase(callback, relocation.offset, relocation.direction > 0 ? codeDelta : 0u - codeDelta);
        }
        for (const auto& relocation : bank_payload::callbackDataRelocations) {
            Rebase(callback, relocation.offset, relocation.direction > 0 ? dataDelta : 0u - dataDelta);
        }
        uintptr_t stockRoot = 0x13572468;
        auto globalOperand = std::search(callback.begin(), callback.end(), std::begin(original), std::end(original));
        Require(globalOperand != callback.end(), "Callback stock-root instruction is missing.");
        Require(std::search(globalOperand + sizeof(original), callback.end(), std::begin(original), std::end(original))
                == callback.end(), "Callback stock-root instruction is ambiguous.");
        const uintptr_t fixtureGlobal = reinterpret_cast<uintptr_t>(&stockRoot);
        memcpy(&*(globalOperand + 2), &fixtureGlobal, sizeof(fixtureGlobal));
        memcpy(callbackMemory, callback.data(), callback.size());
        std::vector<unsigned char> routeData(std::begin(bank_payload::data), std::end(bank_payload::data));
        for (const auto offset : bank_payload::dataRelocations) { Rebase(routeData, offset, dataDelta); }
        memcpy(routeMemory, routeData.data(), routeData.size());
        Require(VirtualProtect(callbackMemory, 4096, PAGE_EXECUTE_READ, &ignored)
                && FlushInstructionCache(GetCurrentProcess(), callbackMemory, callback.size()),
                "Cannot finalize callback fixture code.");
        static HANDLE callbackEntered = nullptr;
        static HANDLE callbackRelease = nullptr;
        const uintptr_t publicModelTable[] = {reinterpret_cast<uintptr_t>(+[]() -> uintptr_t {
            if (callbackEntered) {
                SetEvent(callbackEntered);
                WaitForSingleObject(callbackRelease, 5000);
            }
            return 0;
        })};
        const uintptr_t publicModel = reinterpret_cast<uintptr_t>(publicModelTable);
        using Selector = uintptr_t (__cdecl*)(uintptr_t);
        const auto selectRoot = reinterpret_cast<Selector>(callbackMemory);
        Require(selectRoot(reinterpret_cast<uintptr_t>(&publicModel)) == stockRoot,
                "Callback did not return the stock root for a missing public title.");
        stockRoot = 0x24681357;
        Require(selectRoot(reinterpret_cast<uintptr_t>(&publicModel)) == stockRoot,
                "Callback retained an obsolete stock root.");
        uint32_t callbackCalls = 0;
        memcpy(&callbackCalls, routeMemory + 8, sizeof(callbackCalls));
        Require(callbackCalls == 2, "Callback did not use the retained route allocation.");
        // Prepare/validate the next generation while the old one is retained,
        // then retire old executable bytes after its synchronous call returned.
        for (uint32_t generation = 0; generation < 8; ++generation) {
            auto* nextCode = static_cast<unsigned char*>(VirtualAlloc(nullptr, 4096,
                MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
            Require(nextCode && nextCode != callbackMemory, "Candidate overlaps active callback code.");
            auto nextCallback = callback;
            const uint32_t generationDelta = reinterpret_cast<uintptr_t>(nextCode)
                - bank_payload::base - codeDelta;
            for (const auto& relocation : bank_payload::callbackCodeRelocations) {
                Rebase(nextCallback, relocation.offset,
                    relocation.direction > 0 ? generationDelta : 0u - generationDelta);
            }
            memcpy(nextCode, nextCallback.data(), nextCallback.size());
            Require(VirtualProtect(nextCode, 4096, PAGE_EXECUTE_READ, &ignored)
                    && FlushInstructionCache(GetCurrentProcess(), nextCode, nextCallback.size()),
                    "Cannot finalize candidate callback code.");
            const auto candidate = reinterpret_cast<Selector>(nextCode);
            Require(candidate(reinterpret_cast<uintptr_t>(&publicModel)) == stockRoot,
                    "Candidate callback lost the current stock root.");
            Require(VirtualFree(callbackMemory, 0, MEM_RELEASE), "Old callback generation did not retire.");
            MEMORY_BASIC_INFORMATION retired{};
            Require(VirtualQuery(callbackMemory, &retired, sizeof(retired)) && retired.State == MEM_FREE,
                    "Retired callback allocation is still committed.");
            callbackMemory = nextCode;
            memcpy(&callbackCalls, routeMemory + 8, sizeof(callbackCalls));
            Require(callbackCalls == generation + 3, "Retained state reset across callback generations.");
        }
        Require(VirtualFree(callbackMemory, 0, MEM_RELEASE) && VirtualFree(routeMemory, 0, MEM_RELEASE),
                "Cannot retire callback fixture allocations.");
        heroes5_sdk::SelectorRuntime selector;
        Require(selector.Initialize(bank_payload::data, bank_payload::dataRelocations, bank_payload::base + 4096),
                "SDK selector state initialization failed.");
        uintptr_t selectedRoot = 0;
        for (uint32_t generation = 0; generation < 8; ++generation) {
            Require(selector.Replace(callback, bank_payload::callbackCodeRelocations,
                    bank_payload::callbackDataRelocations, bank_payload::base + codeDelta,
                    bank_payload::base + 4096 + dataDelta), "SDK selector replacement failed.");
            Require(selector.Invoke(reinterpret_cast<uintptr_t>(&publicModel), selectedRoot) && selectedRoot == stockRoot,
                    "SDK selector invocation lost the stock root.");
        }
        const bank_payload::Relocation invalidFixups[] = {{static_cast<unsigned>(callback.size()), 1}};
        Require(!selector.Replace(callback, invalidFixups, bank_payload::callbackDataRelocations,
                bank_payload::base + codeDelta, bank_payload::base + 4096 + dataDelta),
                "SDK selector accepted an out-of-range fixup.");
        Require(selector.Invoke(reinterpret_cast<uintptr_t>(&publicModel), selectedRoot) && selectedRoot == stockRoot,
                "Rejected SDK replacement discarded the working callback.");
        const bank_payload::Relocation invalidDirection[] = {{0, 0}};
        Require(!selector.Replace(callback, bank_payload::callbackCodeRelocations, invalidDirection,
                bank_payload::base + codeDelta, bank_payload::base + 4096 + dataDelta),
                "SDK selector accepted an invalid retained-data fixup direction.");
        Require(!selector.Replace(callback, bank_payload::callbackCodeRelocations, invalidFixups,
                bank_payload::base + codeDelta, bank_payload::base + 4096 + dataDelta),
                "SDK selector accepted an out-of-range retained-data fixup.");
        std::array<unsigned char, 4> unchangedState{};
        Require(selector.ReadState(8, unchangedState), "Cannot inspect state after rejected data fixups.");
        memcpy(&callbackCalls, unchangedState.data(), sizeof(callbackCalls));
        Require(callbackCalls == 9, "Rejected data fixups modified retained state.");
        Require(selector.Stop() && !selector.Invoke(reinterpret_cast<uintptr_t>(&publicModel), selectedRoot),
                "Stopped SDK selector accepted a callback.");
        Require(selector.Replace(callback, bank_payload::callbackCodeRelocations,
                bank_payload::callbackDataRelocations, bank_payload::base + codeDelta,
                bank_payload::base + 4096 + dataDelta), "SDK selector readd failed.");
        Require(selector.Invoke(reinterpret_cast<uintptr_t>(&publicModel), selectedRoot) && selectedRoot == stockRoot,
                "SDK selector readd lost the stock root.");
        std::array<unsigned char, 4> retainedCalls{};
        Require(selector.ReadState(8, retainedCalls), "Cannot read SDK retained state.");
        memcpy(&callbackCalls, retainedCalls.data(), sizeof(callbackCalls));
        Require(callbackCalls == 10, "SDK selector reset retained state across rejection/remove/readd.");
        callbackEntered = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        callbackRelease = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        HANDLE replacementStarted = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        HANDLE replacementFinished = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        Require(callbackEntered && callbackRelease && replacementStarted && replacementFinished,
                "Cannot create callback retirement synchronization fixtures.");
        bool invocationAccepted = false;
        bool replacementAccepted = false;
        std::thread invoking([&] {
            invocationAccepted = selector.Invoke(reinterpret_cast<uintptr_t>(&publicModel), selectedRoot);
        });
        const DWORD callbackWait = WaitForSingleObject(callbackEntered, 5000);
        std::thread replacing([&] {
            SetEvent(replacementStarted);
            replacementAccepted = selector.Replace(callback, bank_payload::callbackCodeRelocations,
                bank_payload::callbackDataRelocations, bank_payload::base + codeDelta,
                bank_payload::base + 4096 + dataDelta);
            SetEvent(replacementFinished);
        });
        const DWORD startedWait = WaitForSingleObject(replacementStarted, 5000);
        const DWORD prematureReplacement = WaitForSingleObject(replacementFinished, 100);
        SetEvent(callbackRelease); // Release before assertions so failure also joins safely.
        invoking.join(); replacing.join();
        CloseHandle(callbackEntered); CloseHandle(callbackRelease);
        CloseHandle(replacementStarted); CloseHandle(replacementFinished);
        callbackEntered = nullptr; callbackRelease = nullptr;
        Require(callbackWait == WAIT_OBJECT_0 && startedWait == WAIT_OBJECT_0,
                "Callback retirement synchronization did not reach the active call.");
        Require(prematureReplacement == WAIT_TIMEOUT && invocationAccepted && replacementAccepted,
                "SDK replacement did not wait for active callback completion.");
        Require(selectedRoot == stockRoot, "Active callback lost its root during replacement.");
        Require(selector.Invoke(reinterpret_cast<uintptr_t>(&publicModel), selectedRoot) && selectedRoot == stockRoot,
                "Callback after concurrent replacement failed.");
        Require(selector.Stop(), "SDK selector final stop failed.");
        std::cout << "Native selector checks passed in a disposable test process.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
