/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

/*
 * For https server, this example starts an SSL web server on https://localhost:(8)443/
 *
 * Please note that the certificate used is a self-signed one and will not be
 * recognised as valid. You should expect an SSL error and will need to
 * explicitly allow the browser to proceed.
 *
 * Attention:
 * 1. If 0 < MG_ENABLE_HTTP_STREAMING_MULTIPART, then all "MULTIPART" request will be
 * captured by https server, which means cgi won't be called.
 */

/* user includes */
#include "my_config.h"
#include "my_endpoint.h"
#include "my_log.h"
#if defined(MG_ENABLE_HTTP_STREAMING_MULTIPART) && MG_ENABLE_HTTP_STREAMING_MULTIPART
#include "my_upload.h"
#endif

#if MG_ENABLE_SSL
static const char *s_http_port = HTTPS_SVC_PORT;
static const char *s_ssl_cert = HTTPS_SSL_CERT;
static const char *s_ssl_key = HTTPS_SSL_KEY;
#else
static const char *s_http_port = HTTP_SVC_PORT;
#endif /* MG_ENABLE_SSL */

/* ------------------------- public interface ------------------------- */

int main(int argc, char *argv[]) {
    if (1 < argc && argv[1]) {
        s_http_port = argv[1];
    }

    struct mg_mgr mgr;
    struct mg_connection *nc;

    mg_mgr_init(&mgr, NULL);
#if MG_ENABLE_SSL
    struct mg_bind_opts bind_opts;
    const char *err;

    memset(&bind_opts, 0, sizeof(bind_opts));
    bind_opts.ssl_cert = s_ssl_cert;
    bind_opts.ssl_key = s_ssl_key;
    bind_opts.error_string = &err;
    log_verbose("Starting SSL server on port %s, cert from %s, key from %s\n",
        s_http_port, bind_opts.ssl_cert, bind_opts.ssl_key);
    nc = mg_bind_opt(&mgr, s_http_port, ev_handler, bind_opts);
    if (nc == NULL) {
        log_err("Failed to create listener: %s\n", err);
        return 1;
    }
#else
    log_verbose("Starting web server on port %s\n", s_http_port);
    // create a connection to input address
    nc = mg_bind(&mgr, s_http_port, ev_handler);
    if (nc == NULL) {
        log_err("Failed to create listener\n");
        return 1;
    }
#endif /* MG_ENABLE_SSL */

#if defined(MG_ENABLE_HTTP_STREAMING_MULTIPART) && MG_ENABLE_HTTP_STREAMING_MULTIPART
    // register new endpoint: /upload
    mg_register_http_endpoint(nc, "/upload", handle_upload MG_UD_ARG(NULL));
#endif // HTTP MULTIPART

    // Set up HTTP server parameters
    mg_set_protocol_http_websocket(nc);

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);

    return 0;
}
