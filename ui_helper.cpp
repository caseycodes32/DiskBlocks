#include "ui_helper.h"

int maximum_dissimilar_colors[64][3] = {
    {64,64,64},
    {1,0,103},
    {213,255,0},
    {255,0,86},
    {158,0,142},
    {14,76,161},
    {255,229,2},
    {0,95,57},
    {0,255,0},
    {149,0,58},
    {255,147,126},
    {164,36,0},
    {0,21,68},
    {145,208,203},
    {98,14,0},
    {107,104,130},
    {0,0,255},
    {0,125,181},
    {106,130,108},
    {0,174,126},
    {194,140,159},
    {190,153,112},
    {0,143,156},
    {95,173,78},
    {255,0,0},
    {255,0,246},
    {255,2,157},
    {104,61,59},
    {255,116,163},
    {150,138,232},
    {152,255,82},
    {167,87,64},
    {1,255,254},
    {255,238,232},
    {254,137,0},
    {189,198,255},
    {1,208,255},
    {187,136,0},
    {117,68,177},
    {165,255,210},
    {255,166,254},
    {119,77,0},
    {122,71,130},
    {38,52,0},
    {0,71,84},
    {67,0,44},
    {181,0,255},
    {255,177,103},
    {255,219,102},
    {144,251,146},
    {126,45,210},
    {189,211,147},
    {229,111,254},
    {222,255,116},
    {0,255,120},
    {0,155,255},
    {0,100,1},
    {0,118,255},
    {133,169,0},
    {0,185,23},
    {120,130,49},
    {0,255,198},
    {255,110,65},
    {232,94,190}};

void UIDirectoryTree(DiskElement &tree, std::string &selected_path)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    bool node_open = ImGui::TreeNodeEx((GetDirectoryNameFromPath(tree.name)).c_str(), flags);
    if (node_open) 
    {
        selected_path = tree.name;
        if (tree.children.size() == 0) PopulateSubDirectories(tree, tree.name);
        for (DiskElement &child : tree.children)
        {
            UIDirectoryTree(child, selected_path);
        }
        ImGui::TreePop();
    }
}

void UIDynamicFileVisualizer(DiskElement tree, int level, int x_pos, int width)
{
    if (true)
    {
        static int max_width = ImGui::GetWindowContentRegionMax().x - 8;
        ImVec2 origin_coord = ImGui::GetCursorScreenPos();
        origin_coord.x += x_pos;
        origin_coord.y += (level * 24);
        ImVec2 end_coord = origin_coord;
        end_coord.x += (width ? width : max_width);
        end_coord.y += 20;
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        DrawDiskElementRect(draw_list, origin_coord, end_coord, maximum_dissimilar_colors[13], tree.name);
    }
}

bool DrawDiskElementRect(ImDrawList* draw_list, ImVec2 start_pos, ImVec2 end_pos, int color[3], std::string element_name)
{
    draw_list->AddRectFilled(start_pos, end_pos, IM_COL32(color[0], color[1], color[2], 255));
    int rect_width = end_pos.x - start_pos.x;
    if ((ImGui::GetMousePos() > start_pos) && (ImGui::GetMousePos() < end_pos))
    {
        draw_list->AddRect(start_pos, end_pos, IM_COL32(255, 0, 255, 255));
    }
}

void PopulateSubDirectories(DiskElement &tree, std::string path)
{
    for (std::string DirElement : ListSubDirectories(path))
    {
        DiskElement child;
        child.name = DirElement;
        tree.children.push_back(child);
    }
}

std::string GetDirectoryNameFromPath(std::string path)
{
    return path.substr(path.find_last_of("/\\") + 1);
}

std::string GetCurrentLoadingSymbol()
{
    static long long start_time = curtime();
    long long delta_time = curtime();

    const char display_symbols[4] = {'|', '/', '-', '\\'};
    static int current_symbol = 0;

    if (delta_time - start_time > 200)
    {
        start_time = curtime();
        if (current_symbol < 3) current_symbol++;
        else current_symbol = 0;
    }

    return std::string(1, display_symbols[current_symbol]);
}

long long curtime()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
      ).count();
}