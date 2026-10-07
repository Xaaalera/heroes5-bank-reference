#include "player_launch.hpp"
#include "../game-api/include/h5/hooks.hpp"
#include "bank_payload.hpp"
#include <cstring>
#include <iostream>
#include <vector>
#ifdef BANK_MANAGED_SELECTOR
#include "selector_runtime.hpp"
#endif

namespace {

constexpr uintptr_t entry = h5::hooks::BankLayout.address;
constexpr const auto& original = h5::hooks::BankLayout.expected;

void Require(bool success, const char* message) {
    if (!success) { throw std::runtime_error(message); }
}

void Write(HANDLE process, void* address, const void* data, SIZE_T size) {
    SIZE_T written{};
    Require(WriteProcessMemory(process, address, data, size, &written) && written == size,
            "Cannot install the reference into the new game process.");
}

void Rebase(std::vector<unsigned char>& bytes, unsigned offset, uint32_t delta) {
    Require(offset + sizeof(uint32_t) <= bytes.size(), "Invalid reference payload offset.");
    uint32_t value{};
    memcpy(&value, bytes.data() + offset, sizeof(value));
    value += delta;
    memcpy(bytes.data() + offset, &value, sizeof(value));
}

void InstallSelector(HANDLE process, uintptr_t hookAddress = entry) {
    unsigned char actual[sizeof(original)]{};
    SIZE_T received{};
    Require(ReadProcessMemory(process, reinterpret_cast<void*>(hookAddress), actual, sizeof(actual), &received)
            && received == sizeof(actual) && memcmp(actual, original, sizeof(actual)) == 0,
            "Unexpected game entry bytes. Reference not installed.");
    auto* allocation = static_cast<unsigned char*>(VirtualAllocEx(process, nullptr,
        4096 + sizeof(bank_payload::data), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    Require(allocation != nullptr, "Cannot allocate reference memory.");
    const auto base = reinterpret_cast<uintptr_t>(allocation);
    Require(base < 0x80000000u - 4096 - sizeof(bank_payload::data), "Reference memory outside the supported range.");
    std::vector<unsigned char> code(std::begin(bank_payload::code), std::end(bank_payload::code));
    std::vector<unsigned char> data(std::begin(bank_payload::data), std::end(bank_payload::data));
    const uint32_t delta = base - bank_payload::base;
    for (const auto& relocation : bank_payload::codeRelocations) {
        Rebase(code, relocation.offset, relocation.direction > 0 ? delta : 0u - delta);
    }
    for (const auto offset : bank_payload::dataRelocations) { Rebase(data, offset, delta); }
    Write(process, allocation, code.data(), code.size());
    Write(process, allocation + 4096, data.data(), data.size());
    DWORD previous{};
    Require(VirtualProtectEx(process, allocation, 4096, PAGE_EXECUTE_READ, &previous), "Cannot protect reference code.");
    unsigned char patch[sizeof(original)] = {0xe9, 0, 0, 0, 0, 0x90};
    const uint32_t displacement = base - hookAddress - 5;
    memcpy(patch + 1, &displacement, sizeof(displacement));
    Require(VirtualProtectEx(process, reinterpret_cast<void*>(hookAddress), sizeof(patch), PAGE_EXECUTE_READWRITE, &previous),
            "Cannot prepare the reference hook.");
    Write(process, reinterpret_cast<void*>(hookAddress), patch, sizeof(patch));
    DWORD ignored{};
    Require(VirtualProtectEx(process, reinterpret_cast<void*>(hookAddress), sizeof(patch), previous, &ignored)
            && FlushInstructionCache(process, nullptr, 0), "Cannot finalize the reference hook.");
}

void InstallPackage(const std::filesystem::path& source, const std::filesystem::path& target) {
    if (std::filesystem::exists(target)) {
        Require(universe_player::Sha256(target) == bank_payload::packageHash,
                "A different workshop-army-reference.h5u already exists. Nothing was overwritten.");
        return;
    }
    std::filesystem::create_directories(target.parent_path());
    // No overwrite: a concurrent installer cannot silently replace another file.
    Require(CopyFileW(source.c_str(), target.c_str(), TRUE), "Cannot install the reference H5U. Check folder permissions.");
    Require(universe_player::Sha256(target) == bank_payload::packageHash, "Installed H5U verification failed.");
}

int Run(int count, wchar_t* arguments[]) {
    std::filesystem::path game;
    bool checkOnly = false;
    for (int index = 1; index < count; ++index) {
        const std::wstring option = arguments[index];
        if (option == L"--game" && index + 1 < count) { game = std::filesystem::absolute(arguments[++index]); }
        else if (option == L"--check") { checkOnly = true; }
        else { throw std::runtime_error("Usage: workshop_bank_reference [--game H5_Game.exe] [--check]"); }
    }
    if (game.empty() && count != 1) { throw std::runtime_error("--game is required for command-line checks."); }
    if (game.empty()) { game = universe_player::SelectGame(); }
    if (game.empty()) { return 0; }
    universe_player::VerifyGame(game);
    const auto source = universe_player::LauncherDirectory() / L"workshop-army-reference.h5u";
    Require(universe_player::Sha256(source) == bank_payload::packageHash, "Reference H5U is missing or differs from this launcher.");
    if (checkOnly) {
        std::cout << "Supported game and matching H5U found; nothing installed or launched.\n";
        return 0;
    }
    universe_player::RequireGameClosed();
    const auto target = game.parent_path().parent_path() / L"UserMODs/workshop-army-reference.h5u";
    InstallPackage(source, target);
    STARTUPINFOW startup{sizeof(startup)};
    PROCESS_INFORMATION child{};
    std::wstring command = L"\"" + game.wstring() + L"\"";
    Require(CreateProcessW(game.c_str(), command.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED,
                          nullptr, game.parent_path().c_str(), &startup, &child), "Cannot start Universe.");
    try {
        InstallSelector(child.hProcess);
        Require(ResumeThread(child.hThread) != static_cast<DWORD>(-1), "Cannot resume Universe.");
    } catch (...) {
        // Only this launcher's newly created child is a cleanup target.
        TerminateProcess(child.hProcess, 1);
        WaitForSingleObject(child.hProcess, 5000);
        CloseHandle(child.hThread);
        CloseHandle(child.hProcess);
        throw;
    }
    std::cout << "Reference installed before game entry; PID " << child.dwProcessId << '\n';
    CloseHandle(child.hThread);
    CloseHandle(child.hProcess);
    return 0;
}

} // namespace

#ifdef BANK_MANAGED_SELECTOR
// Query only immutable buffers. The SDK copies them before unloading this DLL;
// no allocation, hook installation or game operation occurs during query.
extern "C" __declspec(dllexport) const heroes5_sdk::BankSelectorPayload* Heroes5BankSelectorQuery() {
    static constexpr heroes5_sdk::BankSelectorPayload payload{
        .code = bank_payload::callbackCode,
        .codeBytes = sizeof(bank_payload::callbackCode),
        .initialData = bank_payload::data,
        .dataBytes = sizeof(bank_payload::data),
        .dataOffsets = bank_payload::dataRelocations,
        .dataOffsetCount = std::size(bank_payload::dataRelocations),
        .codeFixups = bank_payload::callbackCodeRelocations,
        .codeFixupCount = std::size(bank_payload::callbackCodeRelocations),
        .dataFixups = bank_payload::callbackDataRelocations,
        .dataFixupCount = std::size(bank_payload::callbackDataRelocations),
        .sourceCode = bank_payload::base,
        .sourceData = bank_payload::base + 4096,
        .dataSchema = bank_payload::dataSchema,
        .packageSha256 = bank_payload::packageHash,
    };
    return &payload;
}
#endif

extern "C" __declspec(dllexport) DWORD WorkshopBankReferenceInstall() {
    static bool installed = false;
    if (installed) { return 1; }
    try {
        const auto executable = universe_player::LauncherDirectory() / L"H5_Game.exe";
#ifdef XKIT_GRAPHICS_SHA256
        universe_player::VerifyGame(executable, XKIT_GRAPHICS_SHA256);
#else
        universe_player::VerifyGame(executable);
#endif
        const auto directory = executable.parent_path().parent_path() / L"UserMODs";
        Require(!std::filesystem::exists(directory / L"workshop-object-reference.h5u"),
                "Remove the old workshop-object-reference.h5u text prototype before enabling bank reference.");
        Require(universe_player::Sha256(directory / L"workshop-army-reference.h5u") == bank_payload::packageHash,
                "Install the matching workshop-army-reference.h5u in UserMODs.");
#ifdef BANK_MANAGED_SELECTOR
        using Control = DWORD (WINAPI*)(void*);
        const auto loader = GetModuleHandleW((executable.parent_path() / L"dinput8.dll").c_str());
        const auto control = loader ? reinterpret_cast<Control>(GetProcAddress(loader, "Heroes5BankSelectorControl")) : nullptr;
        Require(control != nullptr, "This development bank DLL requires the matching xkit selector loader.");
        const auto& payload = *Heroes5BankSelectorQuery();
        heroes5_sdk::SelectorRequest request;
        const DWORD status = control(&request);
        Require(status == ERROR_SUCCESS || status == ERROR_NOT_READY, "Cannot inspect the resident bank selector.");
        request.expectedGeneration = request.generation;
        request.sourceData = payload.sourceData;
        request.dataSchema = payload.dataSchema;
        if (status == ERROR_NOT_READY) {
            request.action = heroes5_sdk::SelectorAction::Initialize;
            request.bytes = payload.initialData; request.byteCount = payload.dataBytes;
            request.stateOffsets = payload.dataOffsets;
            request.stateOffsetCount = payload.dataOffsetCount;
            Require(control(&request) == ERROR_SUCCESS, "Cannot initialize resident bank route state.");
            request.expectedGeneration = request.generation;
        }
        request.action = heroes5_sdk::SelectorAction::Replace;
        request.bytes = payload.code; request.byteCount = payload.codeBytes;
        request.sourceCode = payload.sourceCode;
        request.codeFixups = payload.codeFixups; request.codeFixupCount = payload.codeFixupCount;
        request.dataFixups = payload.dataFixups; request.dataFixupCount = payload.dataFixupCount;
        Require(control(&request) == ERROR_SUCCESS, "Resident bank selector rejected the code or retained-state layout.");
#else
        InstallSelector(GetCurrentProcess());
#endif
        installed = true;
        return 1;
    } catch (const std::exception& error) {
        OutputDebugStringA(error.what());
        return 0;
    }
}

int wmain(int count, wchar_t* arguments[]) {
    if (count == 1) { FreeConsole(); }
    try { return Run(count, arguments); }
    catch (const std::exception& error) {
        if (count == 1) { MessageBoxA(nullptr, error.what(), "Bank reference", MB_OK | MB_ICONERROR); }
        else { std::cerr << error.what() << '\n'; }
        return 1;
    }
}
