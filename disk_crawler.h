#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <numeric>
#include <thread>
#include <filesystem>
#include <fileapi.h>
#include <handleapi.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdint.h>

struct DiskElement
{
    std::string name;
    bool is_directory;
    uint64_t size;
    std::vector<DiskElement> children;
    DiskElement *parent = nullptr;
};

enum ThreadStatus
{
  WAITING = 0,
  RUNNING = 1,
  COMPLETE = 2
};

std::vector<std::string> ListDrives();
std::vector<std::string> ListSubDirectories(std::string path);
std::vector<std::string> ListElementsInDirectory(std::string path);
bool PathIsDirectory(std::string path);
std::string FilenameFromPath(std::string path);
uint64_t GetFileSize(std::string path);
uint64_t GetFileTime(std::string path);
void PopulateTree(DiskElement &tree, std::string path, std::string &workingdir);
int PopulateTree2(DiskElement &tree, DiskElement *parent, std::string path, std::string &workingdir);
std::string GetPathFromTreeNode(DiskElement *tree_node);
void PopulateTreeThread(DiskElement &tree, std::string path, ThreadStatus &done, std::string &workingdir);
std::thread InitializePopulateTreeThread(DiskElement &tree, std::string path, ThreadStatus &done, std::string &workingdir);