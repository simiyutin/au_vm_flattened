#pragma once

#include <cstdint>
#include <string>
#include <map>
#include "../include/mathvm.h"
#include <dlfcn.h>
#include <cstring>
#include <sstream>

struct dynamic_loader {
    dynamic_loader(const std::map<uint16_t, std::pair<std::string, std::vector<mathvm::VarType>>> & nativeFunctions) :
            nativeFunctions(nativeFunctions),
            handle(dlopen("libc", RTLD_LAZY))
    {}

    template <typename STACK>
    void call(uint16_t function_id, STACK & stack, const std::vector<std::string> & constants) {
        auto p = nativeFunctions.find(function_id);
        const std::string & function_name = p->second.first;
        const auto & signature = p->second.second;
        void * func = dlsym(handle, function_name.c_str());

        size_t usedIRegisters = 0;
        size_t usedFloatRegisters = 0;
        uint64_t iRegValues[6]; size_t filledInt = 0;
        double dRegValues[6]; size_t filledDouble = 0;
        for (size_t i = 1; i < signature.size(); ++i) {
            auto type = signature[i];
            if (type == mathvm::VT_STRING) {
                uint16_t string_id = stack.getUInt16();
                const std::string & constant = constants[string_id];
                const char * data = constant.c_str();
                iRegValues[filledInt++] = (uint64_t) data;
            }
            //todo double and int
        }

        auto returnType = signature[0];

        for (size_t i = 0; i < filledInt; ++i) {
            if (i == 0) asm("movq %0, %%rdi;":: "r" (iRegValues[i]));
            if (i == 1) asm("movq %0, %%rsi;":: "r" (iRegValues[i]));
            if (i == 2) asm("movq %0, %%rdx;":: "r" (iRegValues[i]));
            if (i == 3) asm("movq %0, %%rcx;":: "r" (iRegValues[i]));
            if (i == 4) asm("movq %0, %%r8;":: "r" (iRegValues[i]));
            if (i == 5) asm("movq %0, %%r9;":: "r" (iRegValues[i]));
        }

        //todo double

        if (returnType == mathvm::VT_INT || returnType == mathvm::VT_STRING) {
            uint64_t result = 0;
            asm(
                    "callq %1;"
                    "movq %%rax, %0;" : "=r" (result) : "r" (func)
            );
            if (returnType == mathvm::VT_INT) {
                std::cout << "function called, int result = " << result << std::endl;
            } else {
                std::cout << "function called, str result = " << (const char *) result << std::endl;
            }
        } else {
            double result = 0;
            asm(
                    "callq %1;"
                    "movq %%xmm0, %0;" : "=r" (result) : "r" (func)
            );
            std::cout << "function called, double result = " << result << std::endl;
        }

        exit(42);
    }

private:
    using signature_t = std::vector<mathvm::VarType>;
    size_t getSizeOfParameters(const signature_t & signature) {
        size_t result = 0;
        for (size_t i = 1; i < signature.size(); ++i) {
            result += typeSizes.find(signature[i])->second;
        }
        return result;
    }

    const std::map<mathvm::VarType, size_t> typeSizes = {
            {mathvm::VT_INT, sizeof(int)},
            {mathvm::VT_DOUBLE, sizeof(double)},
            {mathvm::VT_STRING, sizeof(char *)},
            {mathvm::VT_VOID, 0}
    };

    const std::vector<std::string> intOrPointerRegisters = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    const std::vector<std::string> floatingRegisters = {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"};

    const std::map<uint16_t, std::pair<std::string, std::vector<mathvm::VarType>>> nativeFunctions;
    void * handle;
};