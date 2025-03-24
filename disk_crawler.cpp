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

void PopulateTree2(DiskElement &tree, std::string path, DiskElement *parent)
{
    tree.name = path;
    if (parent != nullptr) tree.parent = parent;
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
            PopulateTree2(child, DirElement, &tree);
            tree.children.push_back(child);
        }
    }
}

void PopulateTreeByLevel(DiskElement &tree, std::string path, DiskElement *parent)
{
    tree.name = path;
    if (parent != nullptr) tree.parent = parent;
    tree.is_directory = true;
    tree.size = 0;
    for (std::string DirElement : ListSubDirectories(path))
    {
        DiskElement child;
        child.name = DirElement;
        child.is_directory = true;
        child.size = 0;
        tree.children.push_back(child);
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

void PopulateTreeThread(DiskElement &tree, std::string path, bool &done, std::string &workingdir)
{
    std::thread tPopulateTree(PopulateTree, std::ref(tree), path, std::ref(workingdir));
    tPopulateTree.join();
    done = true;
}

std::thread InitializePopulateTreeThread(DiskElement &tree, std::string path, bool &done, std::string &workingdir)
{
    std::thread tPopulateTree(PopulateTreeThread, std::ref(tree), path, std::ref(done), std::ref(workingdir));
    return tPopulateTree;
}

