#include "feeds.hpp"

void on_feed_selected(GtkWidget* widget, gpointer data)
{
    GtkWidget* hbox = gtk_widget_get_parent(widget);
    GList* children = gtk_container_get_children(GTK_CONTAINER(hbox));
    GtkWidget* target_label = GTK_WIDGET(children->data); // the label
    g_list_free(children);

    char* tag = (char*)g_object_get_data(G_OBJECT(widget), "tag_id");

    std::string url = "https://archiveofourown.org/tags/" + std::string(tag) + "/feed.atom";
    update_status("Fetching tag: " + std::string(tag));

    SoupMessage* msg = soup_message_new("GET", url.c_str());
    soup_message_headers_append(msg->request_headers, "User-Agent", USER_AGENT);

    soup_session_queue_message(session, msg, on_download_works_completed, target_label);
}

void on_delete_feed(GtkWidget* widget, gpointer row_container)
{
    gtk_widget_destroy(GTK_WIDGET(row_container));
    update_status("Feed removed.");
    save_feeds_to_disk();
}

// takes text = ID string, title = RSS name string
void add_feed_row(const char* text, const char* title)
{
    if (strlen(text) <= 0) return;

    GtkWidget* hbox = gtk_hbox_new(FALSE, 2);
    GtkWidget* lbl = gtk_label_new(title);
    GtkWidget* btn = gtk_button_new_with_label("Load");
    GtkWidget* del = gtk_button_new_with_label("Unsubscribe");

    g_object_set_data_full(G_OBJECT(btn), "tag_id", g_strdup(text), (GDestroyNotify)g_free);
    
    g_signal_connect(btn, "clicked", G_CALLBACK(on_feed_selected), NULL);
    g_signal_connect(del, "clicked", G_CALLBACK(on_delete_feed), hbox);
    
    gtk_box_pack_start(GTK_BOX(hbox), lbl, TRUE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), btn, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), del, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(global_feed_vbox), hbox, FALSE, FALSE, 2);
    gtk_widget_show_all(hbox);
    update_status("Added Feed ID: " + std::string(text));
}
