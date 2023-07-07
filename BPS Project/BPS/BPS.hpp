#pragma once

#include "framework.h"
#include "bps_core.hpp"


class BPS
{
public:
    BPS() = default;

    /// <summary>
    /// Parse a string BPS data return a BPSFile.
    /// </summary>
    /// <param name="data">BPS data in string format.</param>
    /// <returns>BPS file representation from data.</returns>
    static std::map<std::string, std::any> parse(std::string data);

    /// <summary>
    /// Convert a BPS structured data to plain text.
    /// </summary>
    /// <param name="data">BPS structured data to convert.</param>
    /// <returns>A String representation from data.</returns>
    static std::string plain(std::map<std::string, std::any> data);
};
