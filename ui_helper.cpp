#include "ui_helper.h"

void UIDirectoryTree(DiskElement &tree)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    bool node_open = ImGui::TreeNodeEx((GetDirectoryNameFromPath(tree.name)).c_str(), flags);
    if (node_open) 
    {
        if (tree.children.size() == 0) PopulateSubDirectories(tree, tree.name);
        for (DiskElement &child : tree.children)
        {
            UIDirectoryTree(child);
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