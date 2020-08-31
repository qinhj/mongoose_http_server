/*
 * Copyright (c) 2017-2018 ***. All rights reserved.
 */

#ifndef MY_CONFIG_H
#define MY_CONFIG_H

#ifdef _DEBUG

/* debug region */

#define HTTP_SVC_PORT   "8888"
#define HTTP_SVC_ROOT   "."

#define HTTPS_SVC_PORT  "8443"
#define HTTPS_SVC_ROOT  "."
#define HTTPS_SSL_CERT  "data/certificate"
#define HTTPS_SSL_KEY   "data/client-pkcs8.pem"

#else

/* release region */

#define HTTP_SVC_PORT   "80"
#define HTTP_SVC_ROOT   "/usr/bin"

#define HTTPS_SVC_PORT  "443"
#define HTTPS_SVC_ROOT  "/usr/bin"
#define HTTPS_SSL_CERT  "/tmp/certificate"
#define HTTPS_SSL_KEY   "/tmp/client-pkcs8.pem"

#endif

#ifdef _MSC_VER
#define EOL "\r\n"
#else
#define EOL "\n"
#endif

#endif /* MY_CONFIG_H */
