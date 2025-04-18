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

void UIDynamicFileVisualizer(DiskElement &tree)
{
    if (tree.size != 0)
    {
        static std::vector<std::vector<VisualizerElement>> de_visualizer_list;
        static DiskElement *last_selected_element = nullptr;
        static int max_width = ImGui::GetWindowContentRegionMax().x - 64;
        static int color_idx = 0;
        static bool btn_down = false;
        const int rect_height = 22;
        const int rect_spacing = 7;
        ImVec2 origin_coord = ImGui::GetCursorScreenPos();
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();

        if (btn_down && !ImGui::IsMouseDown(0)) btn_down = false;
        
        if (de_visualizer_list.size() == 0)
        {
            std::vector<VisualizerElement> initial_path;
            initial_path.push_back(VisualizerElement{&tree, color_idx++, 0});
            de_visualizer_list.push_back(initial_path);

            std::vector<VisualizerElement> initial_children;
            for (DiskElement &child : tree.children)
                initial_children.push_back(VisualizerElement{&child, color_idx++ % 64, 1});
            de_visualizer_list.push_back(initial_children);
        }
        else
        {
            if (de_visualizer_list.at(0).at(0).de != &tree)
            {
                de_visualizer_list.clear();
                return;
            }
        }
        unsigned int row_idx = 0;
        for (std::vector<VisualizerElement> de_row : de_visualizer_list)
        {
            unsigned int rect_offset = 0;
            unsigned int col_idx = 0;
            for (VisualizerElement de_column : de_row)
            {
                if (de_column.de->parent == nullptr)
                {
                    DrawDiskElementRect(draw_list, ImVec2(origin_coord), ImVec2(origin_coord.x + max_width, origin_coord.y + rect_height), maximum_dissimilar_colors[de_column.color_idx], *de_column.de);
                }
                else
                {
                    uint64_t parent_disk_size = de_column.de->parent->size;
                    uint64_t this_element_size = de_column.de->size;
                    unsigned int this_rect_width;
                    if (parent_disk_size && this_element_size) this_rect_width = std::round((static_cast<double>(this_element_size) / parent_disk_size) * max_width);
                    else this_rect_width = 1;
                    ImVec2 rect_lower_right = {origin_coord.x + this_rect_width + rect_offset, origin_coord.y + rect_height + (de_column.level * (rect_height + rect_spacing))};
                    if (col_idx + 1 == de_row.size())
                        rect_lower_right.x = origin_coord.x + max_width;

                    if (DrawDiskElementRect(draw_list, ImVec2(origin_coord.x + rect_offset, origin_coord.y + (de_column.level * (rect_height + rect_spacing))), rect_lower_right, maximum_dissimilar_colors[de_column.color_idx], *de_column.de))
                    {
                        last_selected_element = de_column.de;
                        if (!btn_down && ImGui::IsMouseDown(0))
                        {
                            btn_down = true;
                            if (de_visualizer_list.size() == row_idx + 1)
                            {
                                std::vector<VisualizerElement> current_children;
                                for (DiskElement &child : de_column.de->children)
                                {
                                    child.parent = de_column.de;
                                    current_children.push_back(VisualizerElement{&child, ((color_idx++) % 64), de_column.level + 1});
                                }
                                if (current_children.size()) de_visualizer_list.push_back(current_children);
                            }
                            else
                            {
                                de_visualizer_list.erase(de_visualizer_list.begin() + de_column.level + 1, de_visualizer_list.end());
                                return;
                            }
                        }
                    }
                    if (row_idx != de_visualizer_list.size() - 1)
                        if (de_column.de == de_visualizer_list.at(row_idx + 1).at(0).de->parent)
                            if (this_rect_width > 20) DrawAnimatedDownArrow(draw_list, ImVec2(rect_lower_right.x - 12, rect_lower_right.y - 14));
                            else DrawAnimatedDownArrow(draw_list, ImVec2(rect_lower_right.x - 6, rect_lower_right.y - 14));
                    rect_offset += this_rect_width;
                }
                if (row_idx)
                {
                    draw_list->AddLine(ImVec2(origin_coord.x, origin_coord.y + (de_column.level * (rect_height + rect_spacing)) - 4), ImVec2(origin_coord.x + max_width + 56, origin_coord.y + (de_column.level * (rect_height + rect_spacing)) - 4), IM_COL32(200, 200, 200, 255));
                    draw_list->AddText(ImVec2(origin_coord.x + max_width + 10, origin_coord.y + (de_column.level * (rect_height + rect_spacing)) + 3), IM_COL32(255, 255, 255, 255), BytesToStr(de_row.at(0).de->parent->size).c_str());
                }

                col_idx++;
            }
            row_idx++;
        }

        draw_list->AddLine(ImVec2(origin_coord.x + max_width + 4, origin_coord.y - 1), ImVec2(origin_coord.x + max_width + 4, origin_coord.y + (row_idx * (rect_height + rect_spacing)) - 8), IM_COL32(200, 200, 200, 255));
        
        ImVec2 ig_cursor_pos = ImGui::GetCursorPos();
        ig_cursor_pos.y += row_idx * (rect_height + rect_spacing);
        ImGui::SetCursorPos(ig_cursor_pos);
        if (last_selected_element != nullptr)
            PrintSelectedDEInformation(last_selected_element);
    }
    return;
}

bool DrawDiskElementRect(ImDrawList* draw_list, ImVec2 start_pos, ImVec2 end_pos, RGBColor color, DiskElement element)
{
    bool pressed = false;
    bool hovered = false;
    draw_list->AddRectFilled(start_pos, end_pos, IM_COL32(color.r, color.g, color.b, 255));
    int rect_width = end_pos.x - start_pos.x;
    
    if ((ImGui::GetMousePos() > start_pos) && (ImGui::GetMousePos() < end_pos))
    {
        draw_list->AddRect(start_pos, end_pos, IM_COL32(GetColorNegative(color).r, GetColorNegative(color).g, GetColorNegative(color).b, 255));
        pressed = ImGui::IsMouseDown(0);
        hovered = true;
    }

    DrawDiskElementText(draw_list, start_pos, color, element.name, (end_pos.x - start_pos.x), hovered);

    return pressed;
}

void DrawDiskElementText(ImDrawList* draw_list, ImVec2 pos, RGBColor color, std::string text, int constraint, bool cycle)
{
    int max_chars = 0;
    max_chars = (constraint - 9) / 7;
    std::string shortened_text = text.substr(0, max_chars);

    if (shortened_text.length() == text.length()) cycle = false;
    if (constraint > 16 && !cycle)
        draw_list->AddText(ImVec2(pos.x+6, pos.y+3), IM_COL32(GetColorNegative(color).r, GetColorNegative(color).g, GetColorNegative(color).b, 255), shortened_text.c_str());
    else if (constraint > 16)
    {
        static unsigned long long start_time = curtime();
        unsigned long long delta_time = curtime();
        static int index = 0;
        if (delta_time - start_time > 200)
        {
            start_time = curtime();
            index += 1;
        }
        if (index >= text.length()) index = 0;

        std::string wrap_text = WraparoundText(text, max_chars, index);
        draw_list->AddText(ImVec2(pos.x+6, pos.y+3), IM_COL32(GetColorNegative(color).r, GetColorNegative(color).g, GetColorNegative(color).b, 255), wrap_text.c_str());
    }
}

void DrawAnimatedDownArrow(ImDrawList* draw_list, ImVec2 pos)
{
    static bool invert_colors = false;
    static RGBColor white{255, 255, 255};
    static RGBColor black{0, 0, 0};
    static unsigned long long start_time = curtime();
    unsigned long long delta_time = curtime();
    static int index = 0;
    if (delta_time - start_time > 200)
    {
        start_time = curtime();
        invert_colors = !invert_colors;
    }
    draw_list->AddTriangleFilled(pos, ImVec2(pos.x + 6, pos.y), ImVec2(pos.x + 3, pos.y + 6), invert_colors ? IM_COL32(white.r, white.g, white.b, 255) : IM_COL32(black.r, black.g, black.b, 255));
    draw_list->AddTriangle(pos, ImVec2(pos.x + 6, pos.y), ImVec2(pos.x + 3, pos.y + 6), invert_colors ? IM_COL32(black.r, black.g, black.b, 255) : IM_COL32(white.r, white.g, white.b, 255));
}

std::string WraparoundText(std::string text, int char_length, int index)
{
    std::string new_text;
    const int space_chars = 1;
    std::string start_text = std::string(text + " ");

    new_text = start_text.substr(index, start_text.length() - index);
    if (new_text.length() > char_length) new_text = new_text.substr(0, char_length);
    else
    {
        int remaining_chars = char_length - new_text.length();
        new_text += start_text.substr(0, remaining_chars);
    }

    return new_text;
}

void PrintSelectedDEInformation(DiskElement *element)
{
    std::string file_type;
    element->is_directory ? file_type = "directory" : file_type = "file";

    ImGui::Separator();
    ImGui::Text("Name: %s", element->name.c_str());
    ImGui::Text("Type: %s", file_type.c_str());
    ImGui::Text("Size: %s", BytesToStr(element->size).c_str());
    ImGui::Text("Path: %s", GetPathFromTreeNode(element).c_str());
}

RGBColor GetColorNegative(RGBColor color)
{
    return RGBColor{255 - color.r, 255 - color.g, 255 - color.b};
}

std::string BytesToStr(uint64_t bytes)
{
    if (bytes < 1024) return std::to_string(bytes) + " b";
    else if (bytes < 1048576) return std::to_string(bytes / 1024) + " KB";
    else if (bytes < 1073741824) return std::to_string(bytes / 1024 / 1024) + " MB";
    else if (bytes < uint64_t(1099511627776)) return std::to_string(bytes / 1024 / 1024 / 1024) + " GB";
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
    static unsigned long long start_time = curtime();
    unsigned long long delta_time = curtime();

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