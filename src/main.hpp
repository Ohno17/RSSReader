#pragma once

#include <gtk/gtk.h>
#include <libsoup/soup.h>
#include <string>

#define USER_AGENT "Kindle-RSS-AO3"
#define CONFIG_FILE "feeds.txt"

#ifdef KINDLE
    #define DATA_PATH "/mnt/us/documents/AO3/"
    #define DOWNLOAD_EXTENSION ".azw3"
#else
    #define DATA_PATH "./downloads/"
    #define DOWNLOAD_EXTENSION ".pdf"
#endif

inline SoupSession* session;
inline GtkWidget *work_list_vbox;
inline GtkWidget *global_feed_vbox;
inline GtkWidget *status_bar;
inline guint status_context_id;

void update_status(const std::string& message);

void on_download_works_completed(SoupSession *session, SoupMessage *msg, gpointer data);
void on_download_pdf_completed(SoupSession *session, SoupMessage *msg, gpointer data);
