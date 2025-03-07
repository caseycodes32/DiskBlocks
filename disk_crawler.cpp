#include "disk_crawler.h"


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

std::vector<std::string> ListSubDirectories(std::string path)
{
    std::vector<std::string> vDirectoryStrings;
    
    DIR* dirCurrentDirectory = opendir(path.c_str());
    struct dirent* direntIterator;
    if (dirCurrentDirectory != nullptr)
    {
        while (true)
        {
            direntIterator = readdir(dirCurrentDirectory);
            if (direntIterator != nullptr)
            {
                vDirectoryStrings.push_back(direntIterator->d_name);
            }
            else break;
        }
        closedir (dirCurrentDirectory);
    }
    return vDirectoryStrings;
}