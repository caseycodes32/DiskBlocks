#include "ui_helper.h"

RGBColor maximum_dissimilar_colors[64] = {
    {0,255,120},
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
    {64,64,64},
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

void UIDynamicFileVisualizer(DiskElement tree)
{
    if (tree.size != 0)
    {
        static std::vector<std::vector<VisualizerElement>> de_visualizer_list;

        static int max_width = ImGui::GetWindowContentRegionMax().x - 64;
        ImVec2 origin_coord = ImGui::GetCursorScreenPos();
        ImVec2 end_coord = origin_coord;
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();

        int col_idx = 0;
        
        std::vector<VisualizerElement> initial_path;
        initial_path.push_back(VisualizerElement{tree, col_idx++, 0});
        de_visualizer_list.push_back(initial_path);

        std::vector<VisualizerElement> initial_children;
        for (DiskElement child : tree.children)
            initial_children.push_back(VisualizerElement{child, col_idx++ % 64, 1});
        de_visualizer_list.push_back(initial_children);


        int column = 0;
        for (std::vector<VisualizerElement> de_row : de_visualizer_list)
        {
            unsigned int rect_offset = 0;
            for (VisualizerElement de_column : de_row)
            {
                if (de_column.de.parent == nullptr)
                {
                    DrawDiskElementRect(draw_list, ImVec2(origin_coord), ImVec2(origin_coord.x + max_width, origin_coord.y + 20), maximum_dissimilar_colors[de_column.color_idx], de_column.de);
                }
                else
                {
                    uint64_t parent_disk_size = de_column.de.parent->size;
                    uint64_t this_element_size = de_column.de.size;
                    unsigned int this_rect_width = std::round((static_cast<double>(this_element_size) / parent_disk_size) * max_width);
                    DrawDiskElementRect(draw_list, ImVec2(origin_coord.x + rect_offset, origin_coord.y + (de_column.level * 20)), ImVec2(origin_coord.x + this_rect_width + rect_offset, origin_coord.y + 20 + (de_column.level * 20)), maximum_dissimilar_colors[de_column.color_idx], de_column.de);

                    rect_offset += this_rect_width;
                }
            }
            column++;
        }

        //DrawDiskElementRect(draw_list, ImVec2(origin_coord), ImVec2(origin_coord.x + max_width, origin_coord.y + 20), maximum_dissimilar_colors[1], tree);
        /*
        int color_cur_idx = 3;
        for (DiskElement child : tree.children)
        {
            int element_width = (static_cast<double>(child.size) / tree.size) * max_width;
             if (DrawDiskElementRect(draw_list, ImVec2(origin_coord.x + offset, origin_coord.y + (level * 20)), ImVec2(origin_coord.x + element_width + offset, origin_coord.y + 20 + (level * 20)), maximum_dissimilar_colors[color_cur_idx], child))
             {
                ImGui::NewLine();
                ImGui::Text("path: %s", GetPathFromTreeNode(&child).c_str()); //debug
             }
            offset += element_width;
            color_cur_idx++;
            
        }
            */
    }
}

bool DrawDiskElementRect(ImDrawList* draw_list, ImVec2 start_pos, ImVec2 end_pos, RGBColor color, DiskElement element)
{
    bool pressed = false;
    draw_list->AddRectFilled(start_pos, end_pos, IM_COL32(color.r, color.g, color.b, 255));
    int rect_width = end_pos.x - start_pos.x;
    draw_list->AddText(ImVec2(start_pos.x+6, start_pos.y+3), IM_COL32(GetColorNegative(color).r, GetColorNegative(color).g, GetColorNegative(color).b, 255), element.name.c_str());
    //draw_list->AddText(ImVec2(start_pos.x+5, start_pos.y+2), IM_COL32(0, 0, 0, 255), element_name.c_str());
    if ((ImGui::GetMousePos() > start_pos) && (ImGui::GetMousePos() < end_pos))
    {
        draw_list->AddRect(start_pos, end_pos, IM_COL32(GetColorNegative(color).r, GetColorNegative(color).g, GetColorNegative(color).b, 255));
        pressed = ImGui::IsMouseDown(0);
    }
    return pressed;
}

RGBColor GetColorNegative(RGBColor color)
{
    return RGBColor{255 - color.r, 255 - color.g, 255 - color.b};
}

std::string BytesToStr(uint64_t bytes)
{
    if (bytes < 1024) return std::to_string(bytes) + " b";
    else if (bytes < 1024 * 1024) return std::to_string(bytes / 1024.f) + " KB";
    else if (bytes < 1024 * 1024 * 1024) return std::to_string(bytes / 1024.f / 1024.f) + " MB";
    else if (bytes < uint64_t(1024 * 1024 * 1024 * 1024)) return std::to_string(bytes / 1024.f / 1024.f / 1024.f) + " GB";
    else return std::to_string(bytes) + " TB";
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