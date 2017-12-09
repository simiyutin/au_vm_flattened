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
    void call(uint16_t function_id, STACK & stack, const std::vector<std::string> & constantStrings, std::map<uint16_t, char *> & dynamicStrings) {
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
                if (string_id < constantStrings.size()) {
                    const std::string & constant = constantStrings[string_id];
                    const char * data = constant.c_str();
                    iRegValues[filledInt++] = (uint64_t) data;
                } else {
                    char * data = dynamicStrings[string_id];
                    iRegValues[filledInt++] = (uint64_t) data;
                }

            }
            if (type == mathvm::VT_INT) {
                int64_t value = stack.getInt64();
                iRegValues[filledInt++] = (uint64_t) value;
            }
            if (type == mathvm::VT_DOUBLE) {
                double value = stack.getDouble();
                dRegValues[filledDouble++] = value;
            }
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

        for (size_t i = 0; i < filledDouble; ++i) {
            if (i == 0) asm("movq %0, %%xmm0;":: "r" (dRegValues[i]));
            if (i == 1) asm("movq %0, %%xmm1;":: "r" (dRegValues[i]));
            if (i == 2) asm("movq %0, %%xmm2;":: "r" (dRegValues[i]));
            if (i == 3) asm("movq %0, %%xmm3;":: "r" (dRegValues[i]));
            if (i == 4) asm("movq %0, %%xmm4;":: "r" (dRegValues[i]));
            if (i == 5) asm("movq %0, %%xmm5;":: "r" (dRegValues[i]));
        }

        if (returnType == mathvm::VT_INT || returnType == mathvm::VT_STRING) {
            uint64_t result;
            asm(
                    "callq %1;"
                    "movq %%rax, %0;" : "=r" (result) : "r" (func)
            );
            if (returnType == mathvm::VT_INT) {
                stack.addTyped((int64_t) result);
            } else {
                char * strResult = (char *) result;
                uint16_t string_id = constantStrings.size() + dynamicStrings.size();
                dynamicStrings[string_id] = strResult;
                stack.addTyped(string_id);
            }
        } else if (returnType == mathvm::VT_DOUBLE) {
            double result;
            asm(
                    "callq %1;"
                    "movq %%xmm0, %0;" : "=r" (result) : "r" (func)
            );
            stack.addTyped(result);
        } else {
            asm("callq %0;" :: "r" (func));
        }
    }

private:
    using signature_t = std::vector<mathvm::VarType>;
    const std::map<uint16_t, std::pair<std::string, std::vector<mathvm::VarType>>> nativeFunctions;
    void * handle;
};