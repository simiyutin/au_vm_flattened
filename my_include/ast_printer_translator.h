#pragma once

#include "../include/mathvm.h"

using namespace mathvm;

struct AstPrinterTranslatorImpl: Translator {

    AstPrinterTranslatorImpl();

    Status *translate(const string &program, Code **code) override;

    virtual ~AstPrinterTranslatorImpl() override;
};