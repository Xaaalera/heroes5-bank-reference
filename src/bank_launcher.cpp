#include "player_launch.hpp"
#include "bank_payload.hpp"
#include <cstring>
#include <iostream>
#include <vector>

namespace {

constexpr uintptr_t entry = 0x5f8800;
constexpr unsigned char original[] = {0x8b, 0x2d, 0x68, 0x96, 0xfd, 0x00};

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

void InstallSelector(HANDLE process) {
    unsigned char actual[sizeof(original)]{};
    SIZE_T received{};
    Require(ReadProcessMemory(process, reinterpret_cast<void*>(entry), actual, sizeof(actual), &received)
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
    const uint32_t displacement = base - entry - 5;
    memcpy(patch + 1, &displacement, sizeof(displacement));
    Require(VirtualProtectEx(process, reinterpret_cast<void*>(entry), sizeof(patch), PAGE_EXECUTE_READWRITE, &previous),
            "Cannot prepare the reference hook.");
    Write(process, reinterpret_cast<void*>(entry), patch, sizeof(patch));
    DWORD ignored{};
    Require(VirtualProtectEx(process, reinterpret_cast<void*>(entry), sizeof(patch), previous, &ignored)
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

int wmain(int count, wchar_t* arguments[]) {
    if (count == 1) { FreeConsole(); }
    try { return Run(count, arguments); }
    catch (const std::exception& error) {
        if (count == 1) { MessageBoxA(nullptr, error.what(), "Bank reference", MB_OK | MB_ICONERROR); }
        else { std::cerr << error.what() << '\n'; }
        return 1;
    }
}
