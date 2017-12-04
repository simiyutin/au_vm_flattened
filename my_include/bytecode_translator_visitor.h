#pragma once

#include "../include/mathvm.h"
#include "../include/visitors.h"

#include <sstream>
#include <map>
#include "scope.h"

struct BytecodeTranslatorVisitor : mathvm::AstBaseVisitor {

#define VISITOR_FUNCTION(type, name) \
    virtual void visit##type(mathvm::type* node) override;

    FOR_NODES(VISITOR_FUNCTION)
#undef VISITOR_FUNCTION

    void printBytecode() const {
        bytecode.dump(std::cout);
    };

    mathvm::Bytecode getBytecode() const {
        return bytecode;
    };

    std::vector<std::string> getStringConstants() const {
        return stringConstants;
    };

    std::map<uint16_t, size_t> getFunctionOffsetsMap() const {
        return functionOffsetsMap;
    };

    std::map<std::string, int> getTopMostVars() {
        std::map<std::string, int> result;
        for (auto p : scopes[0].vars) {
            if (p.second < topMostVariablesNum) {
                result[p.first] = p.second;
            }
        }
        return result;
    };

    void handleBinaryLogic(mathvm::Instruction straight, mathvm::Instruction opposite) {
        if (expressionStartLabel) {
            bytecode.addBranch(inverse ? opposite : straight, *expressionStartLabel);
        } else {
            if (!expressionEndLabel) {
                expressionEndLabel = new mathvm::Label();
            }
            bytecode.addBranch(inverse? straight : opposite, *expressionEndLabel);
        }
    }

private:

    mathvm::NativeCallNode * checkNative(mathvm::FunctionNode *node);
    std::pair<uint16_t, uint16_t> findVar(std::string varName);
    void generateStoreVarBytecode(std::string name, mathvm::VarType type);
    void generateLoadVarBytecode(std::string name, mathvm::VarType type);
    void generateVarOperationBytecode(std::string name, mathvm::Instruction localInsn, mathvm::Instruction ctxInsn);
    void consumeTOS(mathvm::VarType type);


    std::vector<scope> scopes = {scope{}};

    std::map<std::string, int> functionMap;
    std::map<std::string, mathvm::VarType> functionTypesMap;
    std::map<uint16_t, size_t> functionOffsetsMap;
    int globalFunctionCounter = 0;

    mathvm::Bytecode bytecode;
    std::vector<mathvm::VarType> typeStack;
    int topMostVariablesNum = -1;
    std::vector<std::string> stringConstants = {""};
    mathvm::Label * expressionEndLabel = nullptr;
    mathvm::Label * expressionStartLabel = nullptr;
    bool inverse = false;
};