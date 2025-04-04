#pragma once

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "disk_crawler.h"
#include <chrono>
#include <tuple>
#include <cmath>

static inline bool operator>(const ImVec2& lhs, const ImVec2& rhs)   { return ((lhs.x > rhs.x) && (lhs.y > rhs.y)); }
static inline bool operator<(const ImVec2& lhs, const ImVec2& rhs)   { return ((lhs.x < rhs.x) && (lhs.y < rhs.y)); }

struct VisualizerElement
{
    DiskElement *de;
    int color_idx;
    int level;
};

struct RGBColor
{
    int r;
    int g;
    int b;
};

void UIDirectoryTree(DiskElement &tree, std::string &selected_path);
void UIDynamicFileVisualizer(DiskElement &tree);
bool DrawDiskElementRect(ImDrawList* draw_list, ImVec2 start_pos, ImVec2 end_pos, RGBColor color, DiskElement element);
RGBColor GetColorNegative(RGBColor color);
std::string BytesToStr(uint64_t bytes);
void PopulateSubDirectories(DiskElement &tree, std::string path);
std::string GetDirectoryNameFromPath(std::string path);
std::string GetCurrentLoadingSymbol();
long long curtime();