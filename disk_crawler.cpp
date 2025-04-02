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
    }
    closedir (dirCurrentDirectory);
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

    return ((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (dwFileAttributes != INVALID_FILE_ATTRIBUTES));
}

std::string FilenameFromPath(std::string path)
{
    size_t idxLastSlash = path.find_last_of("/\\");
    if (idxLastSlash != std::string::npos)
    {
        return path.substr(idxLastSlash + 1);
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

uint64_t GetFileTime(std::string path)
{
    WIN32_FIND_DATAA FileData;
    FILETIME FileTime;
    HANDLE FileHandle = FindFirstFileA(path.c_str(), &FileData);
    if (FileHandle == INVALID_HANDLE_VALUE)
        return -1;

    FileTime = FileData.ftLastAccessTime;
    FindClose(FileHandle);
    return (FileTime.dwLowDateTime | (__int64)FileTime.dwLowDateTime << 32);
}

void PopulateTree(DiskElement &tree, std::string path, std::string &workingdir)
{
    tree.name = path;
    workingdir = path;
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
            PopulateTree(child, DirElement, workingdir);
            tree.children.push_back(child);
        }
    }
}

int PopulateTree2(DiskElement &tree, DiskElement *parent, std::string path, std::string &workingdir)
{
    tree.name = path;
    workingdir = path;
    if (parent != nullptr) tree.parent = parent;
    if (!PathIsDirectory(GetPathFromTreeNode(&tree)))
    {
        tree.is_directory = false;
        tree.size = GetFileSize(path);
        return tree.size;
    }
    else
    {
        tree.is_directory = true;

        uint64_t directory_size = 0;
        for (std::string DirElement : ListElementsInDirectory(GetPathFromTreeNode(&tree)))
        {
            DiskElement child;
            directory_size += PopulateTree2(child, &tree, DirElement.substr(DirElement.find_last_of("/\\") + 1), workingdir);
            tree.children.push_back(child);
        }
        tree.size = directory_size;
        return tree.size;
    }
}

std::string GetPathFromTreeNode(DiskElement *tree_node)
{
    std::string path = tree_node->name;
    
    while (tree_node->parent != nullptr)
    {
        tree_node = tree_node->parent;
        path = tree_node->name + "/" + path;
    }
    return path;
}

void PopulateTreeThread(DiskElement &tree, std::string path, ThreadStatus &done, std::string &workingdir)
{
    std::thread tPopulateTree(PopulateTree2, std::ref(tree), nullptr, path, std::ref(workingdir));
    tPopulateTree.join();
    done = ThreadStatus::COMPLETE;
}

std::thread InitializePopulateTreeThread(DiskElement &tree, std::string path, ThreadStatus &done, std::string &workingdir)
{
    done = ThreadStatus::RUNNING;
    std::thread tPopulateTree(PopulateTreeThread, std::ref(tree), path, std::ref(done), std::ref(workingdir));
    return tPopulateTree;
}

