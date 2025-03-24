#include "ui_helper.h"

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