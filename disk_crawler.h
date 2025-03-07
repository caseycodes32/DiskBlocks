#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <filesystem>
#include <fileapi.h>
#include <handleapi.h>
#include <dirent.h>
#include <sys/stat.h>

std::vector<std::string> ListDrives();
std::vector<std::string> ListSubDirectories(std::string path);