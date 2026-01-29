#include "save.hpp"

void save_feeds_to_disk() {
    std::ofstream outfile(CONFIG_PATH);
    if (!outfile.is_open()) return;

    GList *children = gtk_container_get_children(GTK_CONTAINER(global_feed_vbox));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        GList *hbox_children = gtk_container_get_children(GTK_CONTAINER(iter->data));
        
        GtkWidget *label = GTK_WIDGET(g_list_nth_data(hbox_children, 0));
        GtkWidget *btn = GTK_WIDGET(g_list_nth_data(hbox_children, 1));
        
        outfile << (char*)g_object_get_data(G_OBJECT(btn), "tag_id") << "|" 
                << gtk_label_get_text(GTK_LABEL(label)) << std::endl;
        
        g_list_free(hbox_children);
    }
    g_list_free(children);
    outfile.close();
}

void load_feeds_from_disk() {
    std::ifstream infile(CONFIG_PATH);
    if (!infile.is_open()) return;

    std::string line;
    while (std::getline(infile, line)) {
        if (!line.empty()) {
            add_feed_row(line.c_str());
        }
    }
    infile.close();
    update_status("Feeds loaded from disk.");
}
