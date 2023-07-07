#include "pch.h"
#include "BPS.hpp"


std::map<std::string, std::any> BPS::parse(std::string data)
{
    auto parsedData = bps_core::parser::parse(data);
    return parsedData;
}

std::string BPS::plain(std::map<std::string, std::any> data)
{
    return "";// Parser.parse(data);
}
