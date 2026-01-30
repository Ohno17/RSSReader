#include "main.hpp"

void trigger_kindle_scan(const std::string& full_path)
{
#ifdef KINDLE
    // tell the Kindle Content Manager to index the new PDF
    std::string cmd = "dbus-send --system /default/com/lab126/content_manager com.lab126.content_manager.externalScan string:\"" + full_path + "\"";
    system(cmd.c_str());
#endif
}

std::string get_url_from_id(const std::string& feed_id)
{
    return "https://archiveofourown.org/tags/" + feed_id + "/feed.atom";
}

void save_pdf(const char* data, size_t size, const std::string& filename)
{
    std::string folder = DOWNLOAD_PATH;
    std::string full_path = folder + filename;

    std::string mkdir_cmd = "mkdir -p " + folder;
    system(mkdir_cmd.c_str());

    FILE *fp = fopen(full_path.c_str(), "wb");
    if (fp) {
        fwrite(data, 1, size, fp);
        fclose(fp);
        update_status("Saved: " + filename);
        trigger_kindle_scan(full_path);
    } else {
        update_status("Error: Write failed.");
    }
}

void on_download_pdf_completed(SoupSession *session, SoupMessage *msg, gpointer data)
{
    DownloadData *ctx = static_cast<DownloadData*>(data);

    if (msg && msg->status_code == 200) {
        std::string filename = ctx->work_filename + ".pdf";
        save_pdf(msg->response_body->data, msg->response_body->length, ctx->work_filename);
        update_status("Saved as: " + ctx->work_filename);
    } else {
        int code = msg ? msg->status_code : 0;
        std::string error_text = "Download Failed (" + std::to_string(code) + "): " + soup_status_get_phrase(code);
        update_status(error_text);
    }

    delete ctx;
}

void on_download_works_completed(SoupSession *session, SoupMessage *msg, gpointer data)
{
    if (msg->status_code == 200)
    {
        parse_and_display_works(msg->response_body->data, msg->response_body->length, GTK_WIDGET(data));
    }
    else
    {
        int code = msg ? msg->status_code : 0;
        std::string error_text = "Failed (" + std::to_string(code) + "): " + soup_status_get_phrase(code);
        update_status(error_text);
    }
}

void on_add_feed_clicked(GtkWidget* widget, gpointer entry_ptr)
{
    const char* text = gtk_entry_get_text(GTK_ENTRY(entry_ptr));
    
    // make sure numeric
    for (int i = 0; text[i] != '\0'; i++)
    {
        if (!isdigit(text[i]))
        {
            update_status("Error: Feed ID must be numeric (e.g. 207209)");
            return;
        }
    }

    add_feed_row(text, text);
    gtk_entry_set_text(GTK_ENTRY(entry_ptr), "");
}

void update_status(const std::string& message)
{
    gtk_statusbar_push(GTK_STATUSBAR(status_bar), status_context_id, message.c_str());
    while (gtk_events_pending()) gtk_main_iteration();
}

int main(int argc, char* argv[])
{
    gtk_init(&argc, &argv);

    session = soup_session_new();

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "L:A_N:application_PC:TB_ID:com.ohno.rssreader");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 800);
    
    GtkWidget* main_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    GtkWidget* entry = gtk_entry_new();
    GtkWidget* add_btn = gtk_button_new_with_label("Add Feed ID");
    global_feed_vbox = gtk_vbox_new(FALSE, 2);

    gtk_box_pack_start(GTK_BOX(main_vbox), entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_vbox), add_btn, FALSE, FALSE, 2);
    
    // Feed scroll area
    GtkWidget* f_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(f_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(f_scroll), global_feed_vbox);
    gtk_widget_set_size_request(f_scroll, -1, 150);
    gtk_box_pack_start(GTK_BOX(main_vbox), f_scroll, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(main_vbox), gtk_hseparator_new(), FALSE, FALSE, 5);

    // Works scroll area
    GtkWidget* w_scroll = gtk_scrolled_window_new(NULL, NULL);
    work_list_vbox = gtk_vbox_new(FALSE, 2);
    gtk_widget_set_size_request(work_list_vbox, -1, -1);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(w_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(w_scroll), work_list_vbox);
    gtk_box_pack_start(GTK_BOX(main_vbox), w_scroll, TRUE, TRUE, 5);

    status_bar = gtk_statusbar_new();
    status_context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_bar), "status");
    gtk_box_pack_start(GTK_BOX(main_vbox), status_bar, FALSE, FALSE, 0);

    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_add_feed_clicked), entry);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    load_feeds_from_disk();
    
    #ifdef KINDLE
        update_status("Ready (Kindle Mode)");
    #else
        update_status("Ready (Desktop Mode)");
    #endif

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
