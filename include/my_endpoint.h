/**************************************************************************
* @ file    : my_endpoint.h
* @ author  : qinhj@lsec.cc.ac.cn
* @ date    : 2019.12.14
* @Copyright (c) 2019 ***. All rights reserved.
***************************************************************************/

#ifndef MY_ENDPOINT_H
#define MY_ENDPOINT_H

#ifdef _MSC_VER
#include <io.h>     // need for: access
#define F_OK _A_NORMAL
#else // suppose linux
#include <unistd.h> // need for: access
#endif  /* _MSC_VER */

#include <stdio.h>  // need for: NULL
#include <stdlib.h> // need for: calloc
/* 3rd  includes */
#include "mongoose.h"
/* user includes */
#include "my_log.h"

#define BUF_SIZE    1024

#ifdef __cplusplus
extern "C" {
#endif

    // @brief:  event handler(endpoints:  /say_hello, /run, /download, etc.)
    static void ev_handler(struct mg_connection *nc, int ev, void *p) {
        if (ev == MG_EV_HTTP_REQUEST) {
            struct http_message *hm = (struct http_message *) p;
            // endpoint: /say_hello
            if (0 == mg_vcmp(&hm->uri, "/say_hello")) {
                // Check request method
                if (0 == mg_vcmp(&hm->method, "POST")) {
                    mg_printf(nc, "%s", "HTTP/1.1 200 OK (Post)" EOL);
                }
                else if (0 == mg_vcmp(&hm->method, "GET")) {
                    mg_printf(nc, "%s", "HTTP/1.1 200 OK (GET)" EOL);
                }
                else {
                    log_err("[%s] HTTP/1.1 400 (Unknown HTTP request type)\n", __FUNCTION__);
                }
                mg_printf(nc, "%s", "Transfer-Encoding: chunked" EOL EOL);
                // Output JSON object which holds reply data
                mg_printf_http_chunk(nc, "{ \"result\": \"hello world\" }" EOL);
                // Send empty chunk, the end of response
                mg_send_http_chunk(nc, "", 0);
            }
            // endpoint: /run
            else if (0 == mg_vcmp(&hm->uri, "/run")) {
                struct mg_str body = mg_strdup_nul(hm->body);
                log_debug("[%s] /run body[%d]: %s\n", __FUNCTION__, (int)body.len, body.p ? body.p : "NULL");
                // Use chunked encoding in order to avoid calculating Content-Length
                mg_printf(nc, "%s", "HTTP/1.1 200 OK" EOL "Transfer-Encoding: chunked" EOL EOL);
                if (body.p) {
                    FILE *pipe = popen(body.p, "r");
                    if (NULL == pipe) {
                        log_err("[%s] open pipe fail\n", __FUNCTION__);
                    }
                    else {
                        char buf[BUF_SIZE / 2] = { 0 };
                        while (NULL != fgets(buf, sizeof(buf), pipe)) {
                            // Output JSON object which holds reply data
                            mg_printf_http_chunk(nc, "%s", buf);
                        }
                        pclose(pipe);
                    }
                }
                // Send empty chunk, the end of response
                mg_send_http_chunk(nc, "", 0);
                mg_strfree(&body);
            }
            // endpoint: /download
            else if (0 == mg_vcmp(&hm->uri, "/download")) {
                struct mg_str body = mg_strdup_nul(hm->body);
                log_debug("[%s] /download body[%d]: %s\n", __FUNCTION__, (int)body.len, body.p ? body.p : "NULL");
                FILE *fp = NULL;
                // check existence
                if (body.p && 0 == access(body.p, F_OK) && (fp = fopen(body.p, "rb"))) {
                    log_info("[%s] open file '%s' OK\n", __FUNCTION__, body.p);
                    // Use chunked encoding in order to avoid calculating Content-Length
                    mg_printf(nc, "%s", "HTTP/1.1 200 OK" EOL
                        "Content-Type: application/octet-stream" EOL
                        "Transfer-Encoding: chunked" EOL EOL);
                    // Send http chunk
                    char buf[BUF_SIZE] = { 0 };
                    size_t rlen = 0;
                    fseek(fp, 0, SEEK_SET);
                    while (!feof(fp)) {
                        rlen = fread(buf, 1, BUF_SIZE, fp);
                        mg_send_http_chunk(nc, buf, rlen);
                    }
                    // Send empty chunk, the end of response
                    mg_send_http_chunk(nc, "", 0);
                    // Close file
                    if (fp) fclose(fp);
                }
                else {
                    log_err("[%s] open file '%s' failed\n", __FUNCTION__, body.p);
                    mg_printf(nc, "%s", "HTTP/1.1 500 (OPEN FAIL)" EOL "Transfer-Encoding: chunked" EOL EOL);
                    mg_send_http_chunk(nc, "", 0);
                }
                mg_strfree(&body);
            }
            // endpoint: others
            else {
                struct mg_str uri = mg_strdup_nul(hm->uri);
#ifndef ENABLE_INDEX
                // check and disable "/"
                int idx = uri.len - 1;
                while (-1 < idx && uri.p && ('/' == uri.p[idx] || '\\' == uri.p[idx])) {
                    idx--;
                }
                //log_debug("[%s] idx: %d\n", __FUNCTION__, idx);
                if (-1 == idx) {
                    log_info("[%s] %s not available\n", __FUNCTION__, (uri.p ? uri.p : "nil"));
                    mg_strfree(&uri);
                    mg_printf(nc, "%s", "HTTP/1.1 404 (Not Available)" EOL "Transfer-Encoding: chunked" EOL EOL);
                    mg_send_http_chunk(nc, "", 0);
                    return;
                }
#endif
                // output request uri info
                log_info("[%s] handle %s by mg_serve_http\n", __FUNCTION__, (uri.p ? uri.p : "nil"));
                mg_strfree(&uri);
                // serve static content
                static struct mg_serve_http_opts s_http_server_opts;
#ifdef MG_ENABLE_SSL
                s_http_server_opts.document_root = HTTPS_SVC_ROOT;
#else
                s_http_server_opts.document_root = HTTP_SVC_ROOT;
#endif //!MG_ENABLE_SSL
                s_http_server_opts.enable_directory_listing = "yes";
                mg_serve_http(nc, hm, s_http_server_opts);
            }
        }
    }

#ifdef __cplusplus
}
#endif

#endif  /* MY_ENDPOINT_H */
