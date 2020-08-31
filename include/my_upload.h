// @author: qinhj@lsec.cc.ac.cn
// @file:   mp_upload.h
// @brief:  handler for /upload endpoint
//
// Copyright (c) 2019 ***. All rights reserved.
// ------------------------------------------------------------------------------
// Note:
// 1. mg_http_multipart_part ptr "mp" can only be used under event MG_EV_HTTP_PART_BEGIN/DATA/END;
// 2. handle event step by step:
//  MG_EV_HTTP_MULTIPART_REQUEST -> MG_EV_HTTP_PART_BEGIN -> MG_EV_HTTP_PART_DATA -> ... ->
//  MG_EV_HTTP_PART_END -> MG_EV_HTTP_MULTIPART_REQUEST_END -> MG_EV_CLOSE
// 3. If 0 < MG_ENABLE_HTTP_STREAMING_MULTIPART, then all "MULTIPART" request will be captured by
//  server, which means cgi won't be called.

#ifndef MP_UPLOAD_H
#define MP_UPLOAD_H

#include <stdio.h>  // need for: NULL
#include <stdlib.h> // need for: calloc
/* 3rd  includes */
#include "mongoose.h"
/* user includes */
#include "my_log.h"

#ifndef str_safe
#define str_safe(str)   ((str) ? (str) : "NULL")
#endif

#ifdef __cplusplus
extern "C" {
#endif

    struct file_writer_data {
        FILE *fp;
        size_t bytes_written;
    };

    static void _mg_http_multipart_display(const struct mg_http_multipart_part *mp) {
        log_verbose("[%s] file name: %s\n", __FUNCTION__, str_safe(mp->file_name));
        log_verbose("[%s] var  name: %s\n", __FUNCTION__, str_safe(mp->var_name));
        log_verbose("[%s] data  len: %d\n", __FUNCTION__, (int)mp->data.len);
        return;
    }

    // @brief:  handle upload (multi-call)
    // @Note:
    // MG_F_SEND_AND_CLOSE:     Push remaining data and close
    // MG_F_CLOSE_IMMEDIATELY:  Disconnect(=>MG_EV_HTTP_PART_END)
    static void handle_upload(struct mg_connection *nc, int ev, void *p) {
        struct file_writer_data *data = (struct file_writer_data *) nc->user_data;
        struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) p;

        switch (ev) {
            case MG_EV_HTTP_MULTIPART_REQUEST: {
                log_debug("[%s] MG_EV_HTTP_MULTIPART_REQUEST\n", __FUNCTION__);
                break;
            }
            case MG_EV_HTTP_PART_BEGIN: {
                log_debug("[%s] MG_EV_HTTP_PART_BEGIN\n", __FUNCTION__);
                _mg_http_multipart_display(mp);
                if (data == NULL) {
                    // new file_writer_data obj
                    data = (struct file_writer_data *)calloc(1, sizeof(struct file_writer_data));
                    data->fp = fopen(mp->var_name, "wb");
                    data->bytes_written = 0;
                    nc->user_data = (void *)data;
                    log_info("[%s] Create new user data[%p]\n", __FUNCTION__, nc->user_data);

                    if (data->fp == NULL) {
                        mg_printf(nc, "%s",
                            "HTTP/1.1 500 Fail to open file\r\n"
                            "Content-Length: 0\r\n\r\n");
                        nc->flags |= MG_F_SEND_AND_CLOSE;
                        // close request immediately
                        nc->flags |= MG_F_CLOSE_IMMEDIATELY;
                        log_err("[%s] Fail to open file: %s\n", __FUNCTION__, str_safe(mp->var_name));
                        return;
                    }
                }
                break;
            }
            case MG_EV_HTTP_PART_DATA: {
                log_verbose("[%s] MG_EV_HTTP_PART_DATA\n", __FUNCTION__);
                _mg_http_multipart_display(mp);
                // check input params
                if (!data || !data->fp) {
                    // close request immediately
                    nc->flags |= MG_F_CLOSE_IMMEDIATELY;
                    log_err("[%s] Fail to write file: %s, invalid params\n", __FUNCTION__, str_safe(mp->var_name));
                    return;
                }
                size_t wlen = fwrite(mp->data.p, 1, mp->data.len, data->fp);
                data->bytes_written += wlen;
                if (wlen != mp->data.len) {
                    mg_printf(nc, "%s",
                        "HTTP/1.1 500 Fail to write file\r\n"
                        "Content-Length: 0\r\n\r\n");
                    nc->flags |= MG_F_SEND_AND_CLOSE;
                    // close request immediately
                    nc->flags |= MG_F_CLOSE_IMMEDIATELY;
                    log_err("[%s] Fail to write file: %s, fwrite error\n", __FUNCTION__, str_safe(mp->var_name));
                }
                break;
            }
            case MG_EV_HTTP_PART_END: {
                log_debug("[%s] MG_EV_HTTP_PART_END\n", __FUNCTION__);
                _mg_http_multipart_display(mp);
                mg_printf(nc,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Connection: close\r\n\r\n"
                    "Written %ld of POST data to a temp file\n\n",
                    (data && data->fp) ? (long)data->bytes_written/*ftell(data->fp)*/ : 0);
                nc->flags |= MG_F_SEND_AND_CLOSE;
                if (data && data->fp) fclose(data->fp);
                if (data) free(data);
                nc->user_data = NULL;
                log_info("[%s] Release user data[%p]\n", __FUNCTION__, data);
                break;
            }
            case MG_EV_HTTP_MULTIPART_REQUEST_END: {
                log_debug("[%s] MG_EV_HTTP_MULTIPART_REQUEST_END\n", __FUNCTION__);
                break;
            }
            case MG_EV_CLOSE: {
                log_debug("[%s] MG_EV_CLOSE\n", __FUNCTION__);
                break;
            }
            case MG_EV_HTTP_REQUEST: {
                // serve static content
                static struct mg_serve_http_opts s_http_server_opts;
#ifdef MG_ENABLE_SSL
                s_http_server_opts.document_root = HTTPS_SVC_ROOT;
#else
                s_http_server_opts.document_root = HTTP_SVC_ROOT;
#endif //!MG_ENABLE_SSL
                s_http_server_opts.enable_directory_listing = "yes";
                mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
                break;
            }
            default: {
                log_debug("[%s] unknown event: %d\n", __FUNCTION__, ev);
                break;
            }
        }
    }

#ifdef __cplusplus
}
#endif

#endif  /* MP_UPLOAD_H */
