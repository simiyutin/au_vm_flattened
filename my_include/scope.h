#pragma once

#include <map>
#include <string>

struct scope {
    scope(uint16_t function_id) : function_id(function_id) {}

    std::map<std::string, uint16_t> vars;
    const uint16_t function_id;
};