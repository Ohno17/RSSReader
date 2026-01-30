#include "works.hpp"

void on_work_selected(GtkWidget* widget, gpointer data)
{
    DownloadData *ctx = static_cast<DownloadData*>(data);
    if (!ctx) return;

    std::string pdf_url = "https://archiveofourown.org/downloads/" + ctx->work_id + "/work.pdf";

    update_status("Downloading " + ctx->work_id + "...");

    SoupMessage* msg = soup_message_new("GET", pdf_url.c_str());
    if (!msg) return;

    soup_message_headers_append(msg->request_headers, "User-Agent", USER_AGENT);

    DownloadData *download_task = new DownloadData(*ctx); // copy becauase original will be auto freed at callback end
    soup_session_queue_message(session, msg, on_download_pdf_completed, download_task);
}

char* format_summary(const char* raw_html)
{
    if (!raw_html) return g_strdup("");

    htmlDocPtr doc = htmlReadMemory(raw_html, strlen(raw_html), "summary.html", NULL, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
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
    if (tagsObj && tagsObj->nodesetval)
    {
        for (int i = 0; i < tagsObj->nodesetval->nodeNr; i++)
        {
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

void parse_and_display_works(const char* xml_data, int size, GtkWidget* target_label)
{
    GList *children = gtk_container_get_children(GTK_CONTAINER(work_list_vbox));
    for(GList *iter = children; iter != NULL; iter = g_list_next(iter))
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    xmlDocPtr doc = xmlReadMemory(xml_data, size, "atom.xml", NULL, XML_PARSE_NOENT | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (!doc) { update_status("XML Error: Entered ID is probably a non-cannonical tag."); return; }

    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    xmlXPathRegisterNs(xpathCtx, (const xmlChar*)"atom", (const xmlChar*)"http://www.w3.org/2005/Atom");

    xmlXPathObjectPtr feedTitleObj = xmlXPathEvalExpression((const xmlChar*)"/atom:feed/atom:title", xpathCtx);
    if (feedTitleObj && feedTitleObj->nodesetval && feedTitleObj->nodesetval->nodeNr > 0)
    {
        xmlChar* raw_content = xmlNodeGetContent(feedTitleObj->nodesetval->nodeTab[0]);
        gtk_label_set_text(GTK_LABEL(target_label), (const char*)(raw_content + 17));
        xmlFree(raw_content);
    }
    xmlXPathFreeObject(feedTitleObj);
    
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar*)"//atom:entry", xpathCtx);
    if (xpathObj && xpathObj->nodesetval)
    {
        for (int i = 0; i < xpathObj->nodesetval->nodeNr; i++)
        {
            xmlNodePtr entry = xpathObj->nodesetval->nodeTab[i];
            xmlXPathContextPtr entryCtx = xmlXPathNewContext(doc);
            entryCtx->node = entry;
            xmlXPathRegisterNs(entryCtx, (const xmlChar*)"atom", (const xmlChar*)"http://www.w3.org/2005/Atom");
            
            xmlXPathObjectPtr tObj = xmlXPathEvalExpression((const xmlChar*)"atom:title", entryCtx);
            xmlXPathObjectPtr sObj = xmlXPathEvalExpression((const xmlChar*)"atom:summary", entryCtx);
            xmlXPathObjectPtr lObj = xmlXPathEvalExpression((const xmlChar*)"atom:link[@rel='alternate']/@href", entryCtx);

            if (tObj->nodesetval->nodeNr > 0 && sObj->nodesetval->nodeNr > 0 && lObj->nodesetval->nodeNr > 0)
            {
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

                DownloadData* ctx = create_download_data((const char*)link, (const char*)title);

                g_signal_connect_data(btn, "clicked", G_CALLBACK(on_work_selected), ctx, (GClosureNotify)free_download_data, (GConnectFlags)0);
                
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
