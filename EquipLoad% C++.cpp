#include <chrono>
#include <thread>
#include <windows.h>
#include <string>

// Safely read addresses with error handling
float* GetAddr(HANDLE process, const char* ptr, bool readFloatVal) {
    try {
        std::string s(ptr);
        if (s[0] != '[') return nullptr; // Basic validation
        size_t pos = 0;
        uintptr_t addr = std::stoull(s.substr(1, s.find(']') - 1), nullptr, 16);
        s = s.substr(s.find(']') + 1);
        
        // Handle multi-level pointers
        while (!s.empty() && s[0] == '+') {
            size_t offset = std::stoull(s.substr(1), &pos, 16);
            if (!ReadProcessMemory(process, (LPCVOID)(addr + offset), &addr, sizeof(addr), nullptr))
                return nullptr;
            s = s.substr(pos + 1);
        }
        return readFloatVal ? (float*)addr : (float*)addr;
    } catch (...) {
        return nullptr;
    }
}

// Write string to memory
bool WriteString(HANDLE process, float* addr, const char* str) {
    return WriteProcessMemory(process, addr, str, strlen(str) + 1, nullptr);
}

// Main logic
int main() {
    HANDLE process = GetCurrentProcess(); // Simplified for current process
    while (true) {
        float* addr1 = GetAddr(process, "[DARKSOULS.exe+00F786D8]+290", true);
        float* addr2 = GetAddr(process, "[DARKSOULS.exe+00F786D8]+208", true);
        float* targetAddr = GetAddr(process, "[[[[[[DARKSOULS.exe+00F7F8F8]+1C]+24]+110]+34]+B0]+1A0", false);

        if (addr1 && addr2 && targetAddr) {
            float percent = std::floor(*addr1 / *addr2 * 1000 + 0.5) / 10;
            char buffer[10];
            snprintf(buffer, sizeof(buffer), "Mass %.1f%%", percent);
            WriteString(process, targetAddr, buffer);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}