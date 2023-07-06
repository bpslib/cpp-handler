#include "pch.h"
#include "framework.h"


class BPS
{
public:
    BPS() = default;

    /// <summary>
    /// Parse a string BPS data return a BPSFile.
    /// </summary>
    /// <param name="data">BPS data in string format.</param>
    /// <returns>BPS file representation from data.</returns>
    static std::map<std::string, void> parse(std::string data)
    {
        auto parsedData = Parser.parse(data);
        return parsedData;
    }

    /// <summary>
    /// Convert a BPS structured data to plain text.
    /// </summary>
    /// <param name="data">BPS structured data to convert.</param>
    /// <returns>A String representation from data.</returns>
    static std::string plain(std::map<std::string, void> data)
    {
        return Parser.parse(data);
    }
};
