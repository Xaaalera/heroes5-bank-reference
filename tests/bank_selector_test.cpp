// Exercise the actual compiled installer only in this disposable test process.
#define wmain bank_launcher_entry
#include "../src/bank_launcher.cpp"
#undef wmain

int main() {
    try {
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
        std::cout << "Native selector checks passed in a disposable test process.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
