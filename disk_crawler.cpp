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
                if (strcmp(direntIterator->d_name, ".") == 0) continue;
                if (strcmp(direntIterator->d_name, "..") == 0) continue;
                if (PathIsDirectory(path + "/" + direntIterator->d_name))
                    vDirectoryStrings.push_back(path + "/" + direntIterator->d_name);
            }
            else break;
        }
        closedir (dirCurrentDirectory);
    }
    return vDirectoryStrings;
}

std::vector<std::string> ListElementsInDirectory(std::string path)
{
    std::vector<std::string> vElementStrings;
    
    DIR* dirCurrentDirectory = opendir(path.c_str());
    struct dirent* direntIterator;
    if (dirCurrentDirectory != nullptr)
    {
        while (true)
        {
            direntIterator = readdir(dirCurrentDirectory);
            if (direntIterator != nullptr)
            {
                if (strcmp(direntIterator->d_name, ".") == 0) continue;
                if (strcmp(direntIterator->d_name, "..") == 0) continue;
                vElementStrings.push_back(path + "/" + direntIterator->d_name);
            }
            else break;
        }
        closedir (dirCurrentDirectory);
    }
    return vElementStrings;
}

bool PathIsDirectory(std::string path)
{
    DWORD dwFileAttributes = GetFileAttributesA(path.c_str());

    if ((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        return true;

    else return false;
}

std::string FilenameFromPath(std::string path)
{
    size_t IndexLastSlash = path.find_last_of("/\\");
    if (IndexLastSlash != std::string::npos)
    {
        return path.substr(IndexLastSlash + 1);
    }
    return path;
}

uint64_t GetFileSize(std::string path)
{
    WIN32_FIND_DATAA FileData;
    HANDLE FileHandle = FindFirstFileA(path.c_str(), &FileData);
    if (FileHandle == INVALID_HANDLE_VALUE)
        return -1;

    FindClose(FileHandle);
    return (FileData.nFileSizeLow | (__int64)FileData.nFileSizeHigh << 32);
}

void PopulateTree(DiskElement &tree, std::string path)
{
    tree.name = path;
    if (!PathIsDirectory(path))
    {
        tree.is_directory = false;
        tree.size = GetFileSize(path);
    }
    else
    {
        tree.is_directory = true;
        tree.size = 0;
        for (std::string DirElement : ListElementsInDirectory(path))
        {
            DiskElement child;
            PopulateTree(child, DirElement);
            tree.children.push_back(child);
        }
    }
}

void PopulateTreeThread(DiskElement &tree, std::string path, bool &done)
{
    std::thread tPopulateTree(PopulateTree, std::ref(tree), path);
    tPopulateTree.join();
    done = true;
}

std::thread InitializePopulateTreeThread(DiskElement &tree, std::string path, bool &done)
{
    std::thread tPopulateTree(PopulateTreeThread, std::ref(tree), path, std::ref(done));
    return tPopulateTree;
}