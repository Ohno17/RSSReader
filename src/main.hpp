#pragma once

#include <gtk/gtk.h>
#include <libsoup/soup.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/HTMLparser.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "download.hpp"
#include "feeds.hpp"
#include "works.hpp"
#include "save.hpp"

#define USER_AGENT "Kindle-RSS-AO3"

#ifdef KINDLE
    #define DOWNLOAD_PATH "/mnt/us/documents/AO3/"
    #define CONFIG_PATH "/mnt/us/documents/AO3/feeds.txt"
    #define IS_KINDLE true
#else
    #define DOWNLOAD_PATH "./downloads/"
    #define CONFIG_PATH "./downloads/feeds.txt"
    #define IS_KINDLE false
#endif

inline SoupSession* session;
inline GtkWidget *work_list_vbox;
inline GtkWidget *global_feed_vbox;
inline GtkWidget *status_bar;
inline guint status_context_id;

void update_status(const std::string& message);

void on_download_works_completed(SoupSession *session, SoupMessage *msg, gpointer data);
void on_download_pdf_completed(SoupSession *session, SoupMessage *msg, gpointer data);
