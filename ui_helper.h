#pragma once

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "disk_crawler.h"

void UIDirectoryTree(DiskElement &tree);
void PopulateSubDirectories(DiskElement &tree, std::string path);
std::string GetDirectoryNameFromPath(std::string path);