#include "../my_include/bytecode_translator_visitor.h"
#include "../my_include/insn_factory.h"
#include <fstream>

using namespace mathvm;

void BytecodeTranslatorVisitor::visitBinaryOpNode(BinaryOpNode *node) {
    //std::cout << "start BinaryOpNode" << std::endl;
    switch (node->kind()) {
        case tADD: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = typeStack.back();
            bytecode.addInsn(getAddInsn(type));
            typeStack.pop_back();
            typeStack.pop_back();
            typeStack.push_back(type);
            break;
        }
        case tSUB: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = typeStack.back();
            bytecode.addInsn(getSubInsn(type));
            typeStack.pop_back();
            typeStack.pop_back();
            typeStack.push_back(type);
            break;
        }
        case tMUL: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = typeStack.back();
            bytecode.addInsn(getMulInsn(type));
            typeStack.pop_back();
            typeStack.pop_back();
            typeStack.push_back(type);
            break;
        }
        case tDIV: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = typeStack.back();
            bytecode.addInsn(getDivInsn(type));
            typeStack.pop_back();
            typeStack.pop_back();
            typeStack.push_back(type);
            break;
        }
        case tMOD: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = typeStack.back();
            bytecode.addInsn(getModInsn(type));
            typeStack.pop_back();
            typeStack.pop_back();
            typeStack.push_back(type);
            break;
        }
        case tAAND: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = typeStack.back();
            bytecode.addInsn(getAndInsn(type));
            typeStack.pop_back();
            typeStack.pop_back();
            typeStack.push_back(type);
            break;
        }
        case tAOR: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = typeStack.back();
            bytecode.addInsn(getOrInsn(type));
            typeStack.pop_back();
            typeStack.pop_back();
            typeStack.push_back(type);
            break;
        }
        case tAXOR: {
            node->right()->visit(this);
            node->left()->visit(this);
            VarType type = typeStack.back();
            bytecode.addInsn(getXorInsn(type));
            typeStack.pop_back();
            typeStack.pop_back();
            typeStack.push_back(type);
            break;
        }
        case tEQ: {
            node->right()->visit(this);
            node->left()->visit(this);
            handleBinaryLogic(BC_IFICMPE, BC_IFICMPNE);
            break;
        }
        case tLT: {
            node->right()->visit(this);
            node->left()->visit(this);
            handleBinaryLogic(BC_IFICMPL, BC_IFICMPGE);
            break;
        }
        case tGT: {
            node->right()->visit(this);
            node->left()->visit(this);
            handleBinaryLogic(BC_IFICMPG, BC_IFICMPLE);
            break;
        }
        case tGE: {
            node->right()->visit(this);
            node->left()->visit(this);
            handleBinaryLogic(BC_IFICMPGE, BC_IFICMPL);
            break;
        }
        case tLE: {
            node->right()->visit(this);
            node->left()->visit(this);
            handleBinaryLogic(BC_IFICMPLE, BC_IFICMPG);
            break;
        }
        case tAND: {
            if (!expressionEndLabel) {
                assert(!expressionStartLabel);
                expressionEndLabel = new Label();
            }
            node->right()->visit(this);
            node->left()->visit(this);
            break;
        }
        case tOR: {
            if (!expressionStartLabel) {
                assert(!expressionEndLabel);
                expressionStartLabel = new Label();
            }
            node->left()->visit(this);
            node->right()->visit(this);
            break;
        }
        case tRANGE: {
            node->left()->visit(this);
            node->right()->visit(this);
            break;
        }
        default:
            std::cout << "unhandled binary token:" << tokenStr(node->kind()) << std::endl;
            exit(42);
            break;
    }
    //std::cout << "end BinaryOpNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitUnaryOpNode(UnaryOpNode *node) {
    //std::cout << "start UnaryOpNode" << std::endl;
    switch (node->kind()) {
        case tSUB: {
            node->visitChildren(this);
            VarType type = typeStack.back();
            bytecode.addInsn(getNegInsn(type));
            break;
        }
        case tNOT: {
            bool prevInverse = inverse;
            inverse = !inverse;
            node->visitChildren(this);
            inverse = prevInverse;
            break;
        }
        default:
            std::cout << "unhandled unary token:" << tokenStr(node->kind()) << std::endl;
            exit(42);
            break;
    }
    //std::cout << "end BinaryOpNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitStringLiteralNode(StringLiteralNode *node) {
    //std::cout << "string literal:" << node->literal() << std::endl;
    bytecode.addInsn(getLoadInsn(VT_STRING));
    stringConstants.push_back(node->literal());
    bytecode.addUInt16(stringConstants.size() - 1);
    typeStack.push_back(VT_STRING);
}

void BytecodeTranslatorVisitor::visitDoubleLiteralNode(DoubleLiteralNode *node) {
    //std::cout << "double literal:" << node->literal() << std::endl;
    bytecode.addInsn(getLoadInsn(VT_DOUBLE));
    bytecode.addDouble(node->literal());
    typeStack.push_back(VT_DOUBLE);
}

void BytecodeTranslatorVisitor::visitIntLiteralNode(IntLiteralNode *node) {
    //std::cout << "int literal:" << node->literal() << std::endl;
    bytecode.addInsn(getLoadInsn(VT_INT));
    bytecode.addInt64(node->literal());
    typeStack.push_back(VT_INT);
}

void BytecodeTranslatorVisitor::visitLoadNode(LoadNode *node) {
    VarType type = node->var()->type();
    std::pair <uint16_t, uint16_t> p = findVar(node->var()->name());
    uint16_t context = p.first;
    uint16_t var_id = p.second;
    bytecode.addInsn(getLoadCtxVarInsn(type));
    bytecode.addUInt16(context);
    bytecode.addUInt16(var_id);
    typeStack.push_back(type);
}

void BytecodeTranslatorVisitor::visitStoreNode(StoreNode *node) {

    //calculated value is now on TOS
    node->value()->visit(this);

    VarType type = typeStack.back();
    switch (node->op()) {
        case tASSIGN:
            generateStoreVarBytecode(node->var()->name(), type);
            break;
        case tINCRSET:
            generateLoadVarBytecode(node->var()->name(), type);
            bytecode.addInsn(getAddInsn(type));
            generateStoreVarBytecode(node->var()->name(), type);
            break;
        case tDECRSET:
            generateLoadVarBytecode(node->var()->name(), type);
            bytecode.addInsn(getSubInsn(type));
            generateStoreVarBytecode(node->var()->name(), type);
            break;
        default:
            break;
    }

    typeStack.pop_back();
}

void BytecodeTranslatorVisitor::generateStoreVarBytecode(const std::string & name, mathvm::VarType type) {
    generateVarOperationBytecode(name, getStoreCtxVarInsn(type));
}

void BytecodeTranslatorVisitor::generateLoadVarBytecode(const std::string & name, mathvm::VarType type) {
    generateVarOperationBytecode(name, getLoadCtxVarInsn(type));
}

void BytecodeTranslatorVisitor::generateVarOperationBytecode(const std::string & name, mathvm::Instruction ctxInsn) {
    std::pair<uint16_t, uint16_t> p = findVar(name);
    uint16_t context = p.first;
    uint16_t var_id = p.second;
    bytecode.addInsn(ctxInsn);
    bytecode.addUInt16(context);
    bytecode.addUInt16(var_id);
}

void BytecodeTranslatorVisitor::visitForNode(ForNode *node) {

    node->inExpr()->visit(this); // на стеке сейчас находится сверху конечное значение, снизу начальное
    bytecode.addInsn(BC_SWAP);

    // присваиваем начальное значение переменной
    bytecode.addInsn(getStoreCtxVarInsn(VT_INT));
    bytecode.addUInt16(scopes.back().function_id);
    bytecode.addUInt16(scopes.back().vars[node->var()->name()]);
    bytecode.addInsn(BC_POP);

    Label startLabel;
    uint32_t startPosition = bytecode.length();
    Label endLabel;

    //проверяем, не нужно ли прекратить цикл.
    node->inExpr()->visit(this); // на стеке сейчас находится сверху конечное значение, снизу начальное
    bytecode.addInsn(BC_SWAP);
    bytecode.addInsn(BC_POP);
    bytecode.addInsn(getLoadCtxVarInsn(VT_INT));
    bytecode.addUInt16(scopes.back().function_id);
    bytecode.addUInt16(scopes.back().vars[node->var()->name()]);
    bytecode.addBranch(BC_IFICMPG, endLabel);

    //выполняем вычисления
    node->body()->visit(this);

    //инкрементируем переменную
    bytecode.addInsn(getLoadCtxVarInsn(VT_INT));
    bytecode.addUInt16(scopes.back().function_id);
    bytecode.addUInt16(scopes.back().vars[node->var()->name()]);
    bytecode.addInsn(BC_ILOAD1);
    bytecode.addInsn(BC_IADD);
    bytecode.addInsn(getStoreCtxVarInsn(VT_INT));
    bytecode.addUInt16(scopes.back().function_id);
    bytecode.addUInt16(scopes.back().vars[node->var()->name()]);

    //делаем джамп к проверке
    bytecode.addBranch(BC_JA, startLabel);
    startLabel.bind(startPosition, &bytecode);

    //конец цикла
    endLabel.bind(bytecode.current(), &bytecode);
    
}

void BytecodeTranslatorVisitor::visitWhileNode(WhileNode *node) {
    Label startLabel;
    uint32_t startPosition = bytecode.length();
    node->whileExpr()->visit(this);
    Label * localExprStartLabel = expressionStartLabel;
    expressionStartLabel = nullptr;
    Label * localExprEndLabel = expressionEndLabel;
    expressionEndLabel = nullptr;
    //after expression visit, only one of (expressionStartLabel, expressionEndLabel) is initialized,
    //depending on type of logical operators within expression (||, &&)
    if (localExprStartLabel) { // || operators
        localExprEndLabel = new Label();
        bytecode.addBranch(BC_JA, *localExprEndLabel);
        localExprStartLabel->bind(bytecode.length(), &bytecode);
    }
    node->loopBlock()->visit(this);
    bytecode.addBranch(BC_JA, startLabel);
    startLabel.bind(startPosition, &bytecode);
    localExprEndLabel->bind(bytecode.length(), &bytecode);
    delete localExprStartLabel;
    delete localExprEndLabel;
}

void BytecodeTranslatorVisitor::visitIfNode(IfNode *node) {
    node->ifExpr()->visit(this);
    Label * localExprStartLabel = expressionStartLabel;
    expressionStartLabel = nullptr;
    Label * localExprEndLabel = expressionEndLabel;
    expressionEndLabel = nullptr;
    //after expression visit, only one of (expressionStartLabel, expressionEndLabel) is initialized,
    //depending on type of logical operators within expression (||, &&)
    if (localExprStartLabel) { // || operators
        localExprEndLabel = new Label();
        bytecode.addBranch(BC_JA, *localExprEndLabel);
        localExprStartLabel->bind(bytecode.length(), &bytecode);
    }
    node->thenBlock()->visit(this);
    Label endOfElseBlockLabel;
    bytecode.addBranch(BC_JA, endOfElseBlockLabel);
    localExprEndLabel->bind(bytecode.length(), &bytecode);
    if (node->elseBlock()) {
        node->elseBlock()->visit(this);
    }
    endOfElseBlockLabel.bind(bytecode.current(), &bytecode);
    delete localExprStartLabel;
    delete localExprEndLabel;
}

void BytecodeTranslatorVisitor::visitBlockNode(BlockNode *node) {
    //std::cout << "start blockNode" << std::endl;

    Scope::VarIterator it(node->scope());

    while (it.hasNext()) {
        const AstVar * var = it.next();
        size_t var_id = scopes.back().vars.size();
        scopes.back().vars[var->name()] = var_id;
    }
    if (topMostVariablesNum == -1) {
        topMostVariablesNum = scopes.back().vars.size();
    }

    // первый проход: получение имен и возвращаемых типов,
    // чтобы потом можно было вызывать функции вне зависимости от порядка из объявления
    Scope::FunctionIterator fit0(node->scope());
    while (fit0.hasNext()) {
        const AstFunction * func = fit0.next();
        functionMap[func->name()] = ++globalFunctionCounter;
        functionTypesMap[func->name()] = func->returnType();
    }


    // второй проход: генерация тел функций.
    Scope::FunctionIterator fit(node->scope());
    while (fit.hasNext()) {
        const AstFunction * func = fit.next();

        Label funcEndLabel;
        bytecode.addBranch(BC_JA, funcEndLabel);
        functionOffsetsMap[functionMap[func->name()]] = bytecode.length();
        uint16_t function_id = functionMap[func->name()];
        scopes.emplace_back(function_id);
        for (uint32_t i = 0; i < func->parametersNumber(); ++i) {
            auto newVarId = uint16_t(scopes.back().vars.size());
            scopes.back().vars[func->parameterName(i)] = newVarId;
            bytecode.addInsn(getStoreCtxVarInsn(func->parameterType(i)));
            bytecode.addUInt16(function_id);
            bytecode.addUInt16(newVarId);
        }
        func->node()->visit(this);
        scopes.pop_back();
        funcEndLabel.bind(bytecode.length(), &bytecode);
    }

    for (uint32_t i = 0; i < node->nodes(); i++) {
        AstNode * child = node->nodeAt(i);
        child->visit(this);
        if (child->isCallNode()) {
            VarType returnType = functionTypesMap[child->asCallNode()->name()];
            if (returnType != VT_VOID) {
                consumeTOS(returnType);
            }
        }
    }

    //std::cout << "end blockNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitFunctionNode(FunctionNode *node) {
//    std::cout << "start functionNode" << std::endl;
    NativeCallNode * native = checkNative(node);
    if (native) {
        native->visit(this);
    } else {
        node->visitChildren(this);
    }
//    std::cout << "end functionNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitReturnNode(ReturnNode *node) {
//    std::cout << "start returnNode" << std::endl;
    if (node->returnExpr()) {
        node->returnExpr()->visit(this);
    }
    bytecode.addInsn(BC_RETURN);
//    std::cout << "end returnNode" << std::endl;
}

void BytecodeTranslatorVisitor::visitCallNode(CallNode *node) {
//    std::cout << "start callNode" << std::endl;
    for (int i = (int) node->parametersNumber() - 1; i >= 0; --i) {
        node->parameterAt(i)->visit(this);
    }
    bytecode.addInsn(BC_CALL);
    bytecode.addUInt16(functionMap[node->name()]);
    typeStack.push_back(functionTypesMap[node->name()]);
//    std::cout << "end callNode" << std::endl;

}

void BytecodeTranslatorVisitor::visitNativeCallNode(NativeCallNode *node) {
    //std::cout << "start nativeCallNode" << std::endl;
    //std::cout << "end nativeCallNode" << std::endl;
//    ss_ << "native '" << node->nativeName()<< "';";
//    newline();
}

void BytecodeTranslatorVisitor::visitPrintNode(PrintNode *node) {
    //std::cout << "start printNode" << std::endl;
    for (int i = 0; i < (int) node->operands(); ++i) {
        node->operandAt(i)->visit(this); //now operand is on TOS
        switch (typeStack.back()) {
            case VT_INT:
                bytecode.addInsn(BC_IPRINT);
                break;
            case VT_DOUBLE:
                bytecode.addInsn(BC_DPRINT);
                break;
            case VT_STRING:
                bytecode.addInsn(BC_SPRINT);
                break;
            default:
                break;
        }
    }
    //std::cout << "end printNode" << std::endl;
}

NativeCallNode *BytecodeTranslatorVisitor::checkNative(FunctionNode *node) {
    BlockNode * block = node->body();
    AstNode * first_child = block->nodeAt(0);
    NativeCallNode * native = dynamic_cast<NativeCallNode*>(first_child);
    return native;
}

std::pair<uint16_t, uint16_t> BytecodeTranslatorVisitor::findVar(const std::string & varName) {
    uint16_t context = 0;
    uint16_t id = 0;
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->vars.find(varName) != it->vars.end()) {
            id = (it->vars)[varName];
            context = it->function_id;
            break;
        }
    }
    return {context, id};
}

void BytecodeTranslatorVisitor::consumeTOS(mathvm::VarType type) {
    bytecode.addInsn(getStoreVar0(type));
}
