#pragma once

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "disk_crawler.h"
#include <chrono>

void UIDirectoryTree(DiskElement &tree, std::string &selected_path);
void PopulateSubDirectories(DiskElement &tree, std::string path);
std::string GetDirectoryNameFromPath(std::string path);
std::string GetCurrentLoadingSymbol();
long long curtime();