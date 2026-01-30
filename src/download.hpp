#pragma once

#include "main.hpp"

struct DownloadData
{
    std::string work_id;
    std::string work_url;
    std::string work_filename;
};

DownloadData* create_download_data(std::string link, std::string title);
void free_download_data(gpointer data, GClosure *closure);
