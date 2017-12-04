#pragma once
#include "../include/mathvm.h"
#include <vector>
#include <map>
#include "stack_frame.h"
#include "identity.h"

namespace detail {
    template <typename T>
    struct image;

    template <>
    struct image<int64_t> {
        using type = int64_t;
    };

    template <>
    struct image<double> {
        using type = double;
    };

    template <>
    struct image<std::string> {
        using type = uint16_t;
    };
}

struct Interpreter : mathvm::Code {

    Interpreter(const mathvm::Bytecode & bytecode,
             const std::map<std::string, int> & topMostVars,
             const std::vector<std::string> & stringConstants,
             const std::map<uint16_t, size_t> & functionOffsets) :
            bytecode(bytecode),
            topMostVars(topMostVars),
            stringConstants(stringConstants),
            functionOffsets(functionOffsets),
            call_stack({stack_frame(0, -1)})
    {}

    mathvm::Status* execute(std::vector<mathvm::Var*>& vars) override;

    void disassemble(std::ostream& out = std::cout, mathvm::FunctionFilter* filter = 0) override;

private:
    struct Stack : mathvm::Bytecode {
        int64_t getInt64() {
            return getTyped<int64_t>();
        }

        template <typename T>
        T getTyped() {
            if (_data.size() < sizeof(T)) {
                std::cout << "stack has less bytes than needed for requested type" << std::endl;
                std::cout << "cur stack size: " << _data.size() << std::endl;
                std::cout << "needed: " << sizeof(T) << std::endl;
                exit(42);
            }

            T result = mathvm::Bytecode::getTyped<T>(_data.size() - sizeof(T));
            for (size_t i = 0; i < sizeof(T); ++i) {
                _data.pop_back();
            }
            return result;
        }
    };

    template<typename T>
    void handleLoad() {
        handleLoad(identity<T>());
    }

    template <typename T>
    void handleLoad(identity<T>) {
        T val = bytecode.getTyped<T>(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(T);
        stack.addTyped(val);
    }

    void handleLoad(identity<std::string>) {
        uint16_t stringTableIndex = bytecode.getInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        stack.addUInt16(stringTableIndex);
    }

    template <typename T>
    void handleLoad1() {
        T val = 1;
        stack.addTyped(val);
    }

    template <typename T>
    void handleLoad0() {
        T val = 0;
        stack.addTyped(val);
    }


    template <typename T>
    void handleAdd() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst + snd);
    }

    template <typename T>
    void handleSub() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst - snd);
    }

    template <typename T>
    void handleMul() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst * snd);
    }

    template <typename T>
    void handleDiv() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst / snd);
    }

    template <typename T>
    void handleMod() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst % snd);
    }

    template <typename T>
    void handleAnd() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst & snd);
    }

    template <typename T>
    void handleOr() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst | snd);
    }

    template <typename T>
    void handleXor() {
        T fst = stack.getTyped<T>();
        T snd = stack.getTyped<T>();
        stack.addTyped(fst ^ snd);
    }

    template <typename T>
    void handleNeg() {
        T fst = stack.getTyped<T>();
        stack.addTyped(-fst);
    }

    template <typename T>
    void handleStoreVar() {
        handleStoreVar(identity<T>());
    }

    template <typename T>
    void handleStoreVar(identity<T>) {
        uint16_t varId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        T val = stack.getTyped<T>();
//        std::cout << "--store ctx var @"
//                  << varId
//                  << "val="
//                  << val
//                  << std::endl;
        getVarMap<T>()[varId] = val;
    }

    void handleStoreVar(identity<std::string>) {
        uint16_t varId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        uint16_t stringId = stack.getTyped<uint16_t>();
        getVarMap<std::string>()[varId] = stringId;
    }

    template <typename T>
    void handleStoreCtxVar() {
        handleStoreCtxVar(identity<T>());
    }

    template <typename T>
    void handleStoreCtxVar(identity<T>) {
        uint16_t contextId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        uint16_t varId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        T val = stack.getTyped<T>();
//        std::cout << "--store ctx var @"
//                  << contextId
//                  << ":"
//                  << varId
//                  << ", val="
//                  << val
//                  << std::endl;
        getVarMap<T>(contextId)[varId] = val;
    }

    void handleStoreCtxVar(identity<std::string>) {
        uint16_t contextId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        uint16_t varId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        uint16_t stringId = stack.getTyped<uint16_t>();
        getVarMap<std::string>(contextId)[varId] = stringId;
    }


    template <typename T>
    void handleLoadVar() {
        handleLoadVar(identity<T>());
    }

    template <typename T>
    void handleLoadVar(identity<T>) {
        uint16_t varId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        auto& varMap = getVarMap<T>();
        auto it = varMap.find(varId);
        if (it == varMap.end()) {
            std::cout << "ERROR: no such variable found: id = " << varId << std::endl;
            exit(300);
        }
        T val = it->second;
        stack.addTyped(val);
    }

    void handleLoadVar(identity<std::string>) {
        uint16_t varId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        auto & varMap = getVarMap<std::string>();
        auto it = varMap.find(varId);
        if (it == varMap.end()) {
            std::cout << "ERROR: no such variable found: id = " << varId << std::endl;
            exit(300);
        }
        uint16_t stringId = it->second;
        stack.addUInt16(stringId);
    }

    template <typename T>
    void handleLoadCtxVar() {
        handleLoadCtxVar(identity<T>());
    }

    template <typename T>
    void handleLoadCtxVar(identity<T>) {
        uint16_t contextId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        uint16_t varId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);

        auto& varMap = getVarMap<T>(contextId);
        auto it = varMap.find(varId);
        if (it == varMap.end()) {
            std::cout << "ERROR: no such variable found: id = " << varId << ", context = " << contextId << std::endl;
            exit(300);
        }
        T val = it->second;
        stack.addTyped(val);
    }

    void handleLoadCtxVar(identity<std::string>) {
        uint16_t contextId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        uint16_t varId = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);

        auto & varMap = getVarMap<std::string>(contextId);
        auto it = varMap.find(varId);
        if (it == varMap.end()) {
            std::cout << "ERROR: no such variable found: id = " << varId << ", context = " << contextId << std::endl;
            exit(300);
        }
        uint16_t stringId = it->second;

        stack.addUInt16(stringId);
    }

    template <typename T>
    void handlePrint() {
        handlePrint(identity<T>());
    }

    template <typename T>
    void handlePrint(identity<T>) {
        T el = stack.getTyped<T>();
        std::cout << el;
    }

    void handlePrint(identity<std::string>) {
        uint16_t id = stack.getTyped<uint16_t>();
        std::cout << stringConstants[id];
    }

    void handleCmpge() {
        handleConditionalJump([](int upper, int lower){ return upper >= lower;});
    }

    void handleCmple() {
        handleConditionalJump([](int upper, int lower){ return upper <= lower;});
    }

    void handleCmpg() {
        handleConditionalJump([](int upper, int lower){ return upper > lower;});
    }

    void handleCmpl() {
        handleConditionalJump([](int upper, int lower){ return upper < lower;});
    }

    void handleCmpe() {
        handleConditionalJump([](int upper, int lower){ return upper == lower;});
    }

    void handleCmpne() {
        handleConditionalJump([](int upper, int lower){ return upper != lower;});
    }

    template <typename FUNCTOR>
    void handleConditionalJump(const FUNCTOR & f) {
        int16_t shift = bytecode.getInt16(call_stack.back().executionPoint);

        int64_t upper = stack.getTyped<int64_t>();
        int64_t lower = stack.getTyped<int64_t>();
        if (f(upper, lower)) {
            call_stack.back().executionPoint += shift;
        } else {
            call_stack.back().executionPoint += sizeof(int16_t);
        }
    }

    void handleJa() {
        int16_t shift = bytecode.getInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += shift;
    }

    //assume swapping integers
    void handleSwap() {
        int64_t upper = stack.getTyped<int64_t>();
        int64_t lower = stack.getTyped<int64_t>();
        stack.addInt64(upper);
        stack.addInt64(lower);
    }

    //assume popping integers
    void handlePop() {
        int64_t upper = stack.getTyped<int64_t>();
        (void) upper;
    }
    
    void handleCall() {
//        std::cout << "--call" << std::endl;
        uint16_t function_id = bytecode.getUInt16(call_stack.back().executionPoint);
        call_stack.back().executionPoint += sizeof(uint16_t);
        call_stack.back().stack_size = stack.length();
        if (call_stack.back().function_id != int64_t(function_id)) {
//            std::cout << "--push context" << std::endl;
            int64_t cur_stack_frame_ind = call_stack.size() - 1;
            context_frames_indices.push_back(cur_stack_frame_ind);
        }
        size_t new_execution_point = functionOffsets.find(function_id)->second;
        call_stack.push_back(stack_frame(new_execution_point, int64_t(function_id)));
    }

    void handleReturn() {
//        std::cout << "--return" << std::endl;
        int64_t top_function_id = call_stack.back().function_id;
        call_stack.pop_back();
        int64_t bottom_function_id = call_stack.back().function_id;
        if (top_function_id != bottom_function_id) {
//            std::cout << "--pop context" << std::endl;
            context_frames_indices.pop_back();
        }

        int64_t new_stack_size = stack.length();
        int64_t old_stack_size = call_stack.back().stack_size;
        if (new_stack_size > old_stack_size + sizeof(int64_t)) {
            std::cout << "STACK SANITIZER ERROR: BEFORE: " << old_stack_size << " AFTER: " << new_stack_size
                      << std::endl;
            exit(300);
        }
    }

    template <typename T>
    std::map<int, typename detail::image<T>::type> & getVarMap(uint16_t context = 0) {
        if (context_frames_indices.size() < context) {
            throw std::runtime_error("context id is out of range");
        }

        size_t stackframe_id = call_stack.size() - 1;
        if (context > 0) {
            // that's correct, last element in context_frames_indices is previous context
            size_t context_index = context_frames_indices.size() - context;
            stackframe_id = context_frames_indices[context_index];
        }
        return call_stack[stackframe_id].getVarMap(identity<T>());
    };

    mathvm::Bytecode bytecode;
    Stack stack;
    std::map<std::string, int> topMostVars;
    std::vector<std::string> stringConstants;
    const std::map<uint16_t , size_t> functionOffsets;
    std::vector<stack_frame> call_stack;
    std::vector<int> context_frames_indices;
};