#include "main.hpp"

void update_status(const std::string& message) {
    gtk_statusbar_push(GTK_STATUSBAR(status_bar), status_context_id, message.c_str());
    while (gtk_events_pending()) gtk_main_iteration();
}

void trigger_kindle_scan(const std::string& full_path) {
#ifdef KINDLE
    // tell the Kindle Content Manager to index the new PDF
    std::string cmd = "dbus-send --system /default/com/lab126/content_manager com.lab126.content_manager.externalScan string:\"" + full_path + "\"";
    system(cmd.c_str());
#endif
}

std::string get_url_from_id(const std::string& feed_id) {
    return "https://archiveofourown.org/tags/" + feed_id + "/feed.atom";
}

void save_pdf(const char* data, size_t size, const std::string& filename) {
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

void on_pdf_download_finished(SoupSession *session, SoupMessage *msg, gpointer user_data) {
    DownloadContext *ctx = static_cast<DownloadContext*>(user_data);

    if (msg && msg->status_code == 200) {
        std::string filename = ctx->work_id + ".pdf";
        save_pdf(msg->response_body->data, msg->response_body->length, filename);
        update_status("Saved: " + filename);
    } else {
        int code = msg ? msg->status_code : 0;
        update_status("Download failed (HTTP " + std::to_string(code) + ")");
    }

    delete ctx;
}

void on_work_selected(GtkWidget* widget, gpointer data) {
    char* work_url = (char*)data;
    if (!work_url) return;

    std::string url_str(work_url);
    size_t last_slash = url_str.find_last_of('/');
    std::string work_id = url_str.substr(last_slash + 1);
    std::string pdf_url = "https://archiveofourown.org/downloads/" + work_id + "/work.pdf";

    update_status("Starting download for " + work_id + "...");

    SoupMessage* msg = soup_message_new("GET", pdf_url.c_str());
    if (!msg) {
        g_free(work_url);
        return;
    }
    soup_message_headers_append(msg->request_headers, "User-Agent", USER_AGENT);

    DownloadContext *ctx = new DownloadContext();
    ctx->work_id = work_id;

    soup_session_queue_message(session, msg, on_pdf_download_finished, ctx);

    g_free(work_url);
}

char* format_summary(const char* raw_html) {
    if (!raw_html) return g_strdup("");

    htmlDocPtr doc = htmlReadMemory(raw_html, strlen(raw_html), NULL, NULL, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!doc) return g_strdup("");

    xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
    
    xmlXPathObjectPtr authObj = xmlXPathEvalExpression((const xmlChar*)"//p[1]", ctx);
    xmlXPathObjectPtr descObj = xmlXPathEvalExpression((const xmlChar*)"//p[2]", ctx);
    xmlXPathObjectPtr statObj = xmlXPathEvalExpression((const xmlChar*)"//p[3]", ctx);
    xmlXPathObjectPtr tagsObj = xmlXPathEvalExpression((const xmlChar*)"//li/a", ctx);

    xmlChar* auth = (authObj->nodesetval->nodeNr > 0) ? xmlNodeGetContent(authObj->nodesetval->nodeTab[0]) : (xmlChar*)xmlStrdup((const xmlChar*)"Unknown");
    xmlChar* desc = (descObj->nodesetval->nodeNr > 0) ? xmlNodeGetContent(descObj->nodesetval->nodeTab[0]) : (xmlChar*)xmlStrdup((const xmlChar*)"");
    xmlChar* stats = (statObj->nodesetval->nodeNr > 0) ? xmlNodeGetContent(statObj->nodesetval->nodeTab[0]) : (xmlChar*)xmlStrdup((const xmlChar*)"");

    GString* tag_list = g_string_new("");
    if (tagsObj && tagsObj->nodesetval) {
        for (int i = 0; i < tagsObj->nodesetval->nodeNr; i++) {
            xmlChar* tag_name = xmlNodeGetContent(tagsObj->nodesetval->nodeTab[i]);
            g_string_append_printf(tag_list, (i == 0) ? "%s" : ", %s", (char*)tag_name);
            xmlFree(tag_name);
        }
    }

    // 'stats' already contains: words chapters lang
    char* final_text = g_strdup_printf(
        "%s\n%s\n\n%s\nTags: %s",
        (char*)auth, (char*)desc, (char*)stats, tag_list->str
    );

    g_string_free(tag_list, TRUE);
    xmlFree(auth); xmlFree(desc); xmlFree(stats);
    xmlXPathFreeObject(authObj); xmlXPathFreeObject(descObj); 
    xmlXPathFreeObject(statObj); xmlXPathFreeObject(tagsObj);
    xmlXPathFreeContext(ctx);
    xmlFreeDoc(doc);

    return final_text;
}

void parse_and_display_works(const char* xml_data, int size, GtkWidget* target_label) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(work_list_vbox));
    for(GList *iter = children; iter != NULL; iter = g_list_next(iter))
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    xmlDocPtr doc = xmlReadMemory(xml_data, size, "noname.xml", NULL, XML_PARSE_NOENT);
    if (!doc) { update_status("XML Error"); return; }

    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    xmlXPathRegisterNs(xpathCtx, (const xmlChar*)"atom", (const xmlChar*)"http://www.w3.org/2005/Atom");

    xmlXPathObjectPtr feedTitleObj = xmlXPathEvalExpression((const xmlChar*)"/atom:feed/atom:title", xpathCtx);
    if (feedTitleObj && feedTitleObj->nodesetval && feedTitleObj->nodesetval->nodeNr > 0) {
        xmlChar* raw_content = xmlNodeGetContent(feedTitleObj->nodesetval->nodeTab[0]);
        gtk_label_set_text(GTK_LABEL(target_label), (const char*)(raw_content + 17));
        xmlFree(raw_content);
    }
    xmlXPathFreeObject(feedTitleObj);
    
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar*)"//atom:entry", xpathCtx);
    if (xpathObj && xpathObj->nodesetval) {
        for (int i = 0; i < xpathObj->nodesetval->nodeNr; i++) {
            xmlNodePtr entry = xpathObj->nodesetval->nodeTab[i];
            xmlXPathContextPtr entryCtx = xmlXPathNewContext(doc);
            entryCtx->node = entry;
            xmlXPathRegisterNs(entryCtx, (const xmlChar*)"atom", (const xmlChar*)"http://www.w3.org/2005/Atom");
            
            xmlXPathObjectPtr tObj = xmlXPathEvalExpression((const xmlChar*)"atom:title", entryCtx);
            xmlXPathObjectPtr sObj = xmlXPathEvalExpression((const xmlChar*)"atom:summary", entryCtx);
            xmlXPathObjectPtr lObj = xmlXPathEvalExpression((const xmlChar*)"atom:link[@rel='alternate']/@href", entryCtx);

            if (tObj->nodesetval->nodeNr > 0 && sObj->nodesetval->nodeNr > 0 && lObj->nodesetval->nodeNr > 0) {
                xmlChar* title = xmlNodeGetContent(tObj->nodesetval->nodeTab[0]);
                xmlChar* link = xmlNodeGetContent(lObj->nodesetval->nodeTab[0]);
                xmlChar* summary = xmlNodeGetContent(sObj->nodesetval->nodeTab[0]);

                char* formatted_summary = format_summary((const char*)summary);
                char* combined_text = g_strdup_printf("%s\n%s", (char*)title, formatted_summary);

                GtkWidget* btn = gtk_button_new_with_label(combined_text);

                GtkWidget* lbl = gtk_bin_get_child(GTK_BIN(btn));
                gtk_widget_set_size_request(lbl, 580, -1);
                gtk_label_set_line_wrap(GTK_LABEL(lbl), TRUE);
                gtk_label_set_line_wrap_mode(GTK_LABEL(lbl), PANGO_WRAP_WORD_CHAR);

                gtk_button_set_alignment(GTK_BUTTON(btn), 0, 0.5);
                g_signal_connect(btn, "clicked", G_CALLBACK(on_work_selected), g_strdup((const char*)link));
                
                gtk_box_pack_start(GTK_BOX(work_list_vbox), btn, TRUE, TRUE, 2);

                g_free(combined_text);
                g_free(formatted_summary);
                xmlFree(title);
                xmlFree(link);
                xmlFree(summary);
            }
            xmlXPathFreeObject(tObj);
            xmlXPathFreeObject(sObj);
            xmlXPathFreeObject(lObj);
            xmlXPathFreeContext(entryCtx);
        }
    }
    update_status("Feed loaded.");
    gtk_widget_show_all(work_list_vbox);
    xmlXPathFreeObject(xpathObj); xmlXPathFreeContext(xpathCtx); xmlFreeDoc(doc);

    save_feeds_to_disk();
}

void on_message_completed(SoupSession *session, SoupMessage *msg, gpointer user_data) {
    if (msg->status_code == 200) {
        parse_and_display_works(msg->response_body->data, msg->response_body->length, GTK_WIDGET(user_data));
    } else {
        std::string error_text = "Failed (" + std::to_string(msg->status_code) + "): " + soup_status_get_phrase(msg->status_code);
        update_status(error_text);
    }
}

void on_feed_selected(GtkWidget* widget, gpointer data) {
    char* tag = (char*)data;

    GtkWidget* hbox = gtk_widget_get_parent(widget);
    GList* children = gtk_container_get_children(GTK_CONTAINER(hbox));
    GtkWidget* target_label = GTK_WIDGET(children->data); // the label
    g_list_free(children);

    std::string url = "https://archiveofourown.org/tags/" + std::string(tag) + "/feed.atom";
    update_status("Fetching tag: " + std::string(tag));

    SoupMessage* msg = soup_message_new("GET", url.c_str());
    soup_message_headers_append(msg->request_headers, "User-Agent", USER_AGENT);

    soup_session_queue_message(session, msg, on_message_completed, target_label);
}

void on_delete_feed(GtkWidget* widget, gpointer row_container) {
    gtk_widget_destroy(GTK_WIDGET(row_container));
    update_status("Feed removed.");
    save_feeds_to_disk();
}

// takes text = ID string, title = RSS name string
void add_feed_row(const char* text, const char* title) {
    if (strlen(text) > 0) {
        GtkWidget* hbox = gtk_hbox_new(FALSE, 2);
        GtkWidget* lbl = gtk_label_new(title);
        GtkWidget* btn = gtk_button_new_with_label("Load");
        GtkWidget* del = gtk_button_new_with_label("Unsubscribe");

        g_object_set_data_full(G_OBJECT(btn), "tag_id", g_strdup(text), (GDestroyNotify)g_free);
        
        g_signal_connect(btn, "clicked", G_CALLBACK(on_feed_selected), g_strdup(text));
        g_signal_connect(del, "clicked", G_CALLBACK(on_delete_feed), hbox);
        
        gtk_box_pack_start(GTK_BOX(hbox), lbl, TRUE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(hbox), btn, FALSE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), del, FALSE, FALSE, 0);
        
        gtk_box_pack_start(GTK_BOX(global_feed_vbox), hbox, FALSE, FALSE, 2);
        gtk_widget_show_all(hbox);
        update_status("Added Feed ID: " + std::string(text));
    }
}

void on_add_feed_clicked(GtkWidget* widget, gpointer entry_ptr) {
    const char* text = gtk_entry_get_text(GTK_ENTRY(entry_ptr));
    
    // make sure numeric
    for (int i = 0; text[i] != '\0'; i++) {
        if (!isdigit(text[i])) {
            update_status("Error: Tag ID must be numeric (e.g. 207209)");
            return;
        }
    }

    add_feed_row(text, text);
    gtk_entry_set_text(GTK_ENTRY(entry_ptr), "");
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    session = soup_session_new();

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
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
