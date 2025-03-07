#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <filesystem>
#include <fileapi.h>

std::vector<std::string> ListDrives()
{
    std::vector<std::string> vDriveStrings;
    DWORD SystemDrives = GetLogicalDrives();

    for (int i = 0; i < 26; i++)
    {
        if ((SystemDrives >> i) & 1)
        {
            std::string sDriveSymbol = "A";
            sDriveSymbol[0] += i;
            vDriveStrings.push_back(sDriveSymbol);
        }
    }
    return vDriveStrings;
}