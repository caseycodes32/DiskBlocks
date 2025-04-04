#pragma once

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "disk_crawler.h"
#include <chrono>

static inline bool operator>(const ImVec2& lhs, const ImVec2& rhs)   { return ((lhs.x > rhs.x) && (lhs.y > rhs.y)); }
static inline bool operator<(const ImVec2& lhs, const ImVec2& rhs)   { return ((lhs.x < rhs.x) && (lhs.y < rhs.y)); }

struct VisualizerNode
{
    std::string name;
    int width;
    int color_idx;
    DiskElement* de_ptr;
    std::vector<VisualizerNode> children;
};

void UIDirectoryTree(DiskElement &tree, std::string &selected_path);
void UIDynamicFileVisualizer(DiskElement tree, int level = 0, int x_pos = 0, int width = 0);
bool DrawDiskElementRect(ImDrawList* draw_list, ImVec2 start_pos, ImVec2 end_pos, int color[3], std::string element_name);
std::string BytesToStr(uint64_t bytes);
void PopulateSubDirectories(DiskElement &tree, std::string path);
std::string GetDirectoryNameFromPath(std::string path);
std::string GetCurrentLoadingSymbol();
long long curtime();