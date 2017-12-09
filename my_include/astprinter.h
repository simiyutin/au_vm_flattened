#pragma once

#include "includes.h"

using namespace mathvm;

struct AstPrinter: Translator {

    AstPrinter();

    Status *translate(const string &program, Code **code) override;

    virtual ~AstPrinter() override;
};