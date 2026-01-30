#include "download.hpp"

DownloadData *create_download_data(std::string link, std::string title)
{
    size_t last_slash = link.find_last_of('/');
    std::string work_id = link.substr(last_slash + 1);

    std::string forbidden = "/\\?%*:|\"<>!@#$^&()[]{};', \t\n\r";
    std::string safe_filename = title;

    std::replace_if(
        safe_filename.begin(),
        safe_filename.end(),
        [&](char c)
        {
            return forbidden.find(c) != std::string::npos;
        }
    , '_');

    if (!safe_filename.empty() && safe_filename[0] == '.') safe_filename[0] = '_';
    if (safe_filename.empty()) safe_filename = "download_" + work_id;
    safe_filename += ".pdf";

    return new DownloadData {work_id, link, safe_filename};
}

void free_download_data(gpointer data, GClosure *closure)
{
    DownloadData *d = static_cast<DownloadData*>(data);
    delete d;
}
