/**************************************************************************
* @ file    : demo.cgi.c
* @ author  : qinhj@lsec.cc.ac.cn
* @ date    : 2019.08.08
* @ brief   : std cgi demo to handle http(s) get/post request
* @Copyright (c) 2019 ***.
* -------------------------------------------------------------------------
* Note:
* 1. One can't read more then CONTENT_LENGTH bytes from stdin. Otherwise,
* cgi will block since there isn't enough input data from console.
* 2. In cgi, one can't use feof(stdin)! Otherwise, also result in endless
* runtime block, since stdin need EOF as CTRL+Z.
***************************************************************************/

#include <stdio.h>  // need for: NULL
#include <stdlib.h> // need for: calloc, getenv
#include <string.h> // need for: strlen
#include <assert.h> // need for: assert
/* user includes */
#include "my_config.h"
#include "my_log.h"

#define str_safe(str)   ((str) ? (str) : "NULL")

/* ----------------------------------- private interface ----------------------------------- */

#ifdef _DEBUG
// @brief:  return with display env value
static __inline const char* _env_show(const char *name) {
    const char *_env = NULL;
    if (name) {
        _env = getenv(name);
        log_debug("[%s] %s: %s\n", __FUNCTION__, name, str_safe(_env));
    }
    return _env;
}
#else
#define _env_show   getenv
#endif

/**************************************************************************
* Note: The following handlers should be moved to another file, either *.h
* as static __inline functions or *.c as common utils.
***************************************************************************/

#define DEFAULT_FILE_PATH   "/tmp/upload"
#define CONT_LINE_MAX       1024
#define CONT_TYPE_MULTIPART "multipart"
// @brie:   handle post request with content type "multipart"
// @param:  [ in]type   (env) content type
// @param:  [ in]len    (env) content length
// @param:  [ in]fp     (env) input file ptr (usually: stdin)
// @return: 0: ok; !0: error
static int post_handler_multipart(const char *type, int len, FILE *fp) {
    // always check input parameters
    if (NULL == type || len < 0 || NULL == fp) {
        fprintf(stdout, "handle post request erroe: invalid params\n");
        log_info("[%s] handle post request erroe: invalid params\n", __FUNCTION__);
        return -1;
    }

    // 1st: get content boundary
    const char *bdy = strstr(type, "boundary=");
    if (bdy) {
        bdy += strlen("boundary=");
    }
    else {
        fprintf(stdout, "boundary not found in content type\n");
        log_info("[%s] boundary not found in content type\n", __FUNCTION__);
        return -2;
    }

    // 2nd: find content body
    char cont[CONT_LINE_MAX], *name = NULL;
    if (NULL == cont) return -3;// err: alloc memory
    int rlen = 0, wlen, tlen = len;
    while (rlen < len) {
        memset(cont, 0x00, CONT_LINE_MAX);
        fgets(cont, len, fp);
        log_debug("[%s] fgets[%02d]: %s", __FUNCTION__, (int)strlen(cont), cont);
        rlen += strlen(cont);
        if (!strcmp(cont, "\n") || !strcmp(cont, "\r\n")) {
            break;
        }
        else if (!strncmp(cont, "Content-Disposition", strlen("Content-Disposition"))) {
            const char *_name = strstr(cont, "name=\"");
            if (_name) _name += strlen("name=\"");
            char *_delimiter = _name ? strstr(_name, "\"") : NULL;
            if (_name && _delimiter) {
                _delimiter[0] = '\0';
                name = strdup(_name);
            }
            else {
                name = strdup(DEFAULT_FILE_PATH);
            }
            assert(name);
            log_debug("[%s] save octet-stream to: %s\n", __FUNCTION__, name);
        }
    }
    log_debug("[%s] header len: %d, boundary len: %d\n", __FUNCTION__, rlen, (int)strlen(bdy));

    // 3rd: read and save octet-stream
    FILE *fout = fopen(name, "wb");
    if (NULL == fout) {
        fprintf(stdout, "create file %s fail\n", name);
        log_info("[%s] create file %s fail\n", __FUNCTION__, name);
        free(name);
        return -4;
    }
    // octet stream len
    tlen -= rlen + strlen(bdy) + 8; // \r\n--$(boundary)--\r\n
    log_info("[%s] octet-stream len: %d\n", __FUNCTION__, tlen);
    while (tlen) {
        memset(cont, 0x00, CONT_LINE_MAX);
        rlen = fread(cont, 1, tlen < CONT_LINE_MAX ? tlen : CONT_LINE_MAX, fp);
        wlen = 0;
        while (wlen != rlen) {
            wlen += fwrite(cont + wlen, 1, rlen - wlen, fout);
        }
        tlen -= rlen;
    }
    fclose(fout);

    free(name);
    return 0;
}

// @brief:  http(s) get/post request handler
// @return: 0: ok; !0: error code
static int _request_handler() {
    // required cgi env
    const char *env = _env_show("REQUEST_METHOD");
    if (env && !strcmp("GET", env)) {
        env = _env_show("QUERY_STRING");
        fprintf(stdout, "nothing to do with GET\n");
        log_info("[%s] nothing to do with GET\n", __FUNCTION__);
        return 0;
    }
    else if (env && !strcmp("POST", env)) {
        // read content
        env = _env_show("CONTENT_LENGTH");
        if (NULL == env) {
            fprintf(stdout, "missing CONTENT_LENGTH\n");
            log_err("[%s] missing CONTENT_LENGTH\n", __FUNCTION__);
            return -2;
        }
        int len = atoi(env);
        // handle post type
        size_t rlen = 0;
        env = _env_show("CONTENT_TYPE");
        if (env && !strncmp(env, CONT_TYPE_MULTIPART, strlen(CONT_TYPE_MULTIPART))) {
            // content type: multipart
            return post_handler_multipart(env, len, stdin);
        }
        else {
            // common post content type: json
            char *cont = (char *)calloc(1, len + 1);
            if (NULL == cont) {
                fprintf(stdout, "alloc memory error\n");
                log_err("[%s] alloc memory erro\n", __FUNCTION__);
                return -3;
            }
            rlen = fread(cont, 1, len, stdin);
            log_info("[%s] content len: %d, read size: %d\n", __FUNCTION__, len, (int)rlen);
            log_debug("[%s] content:\n%s\n", __FUNCTION__, cont);
            // release resource
            if (cont) free(cont);
        }
    }
    else {
        fprintf(stdout, "unsupported request method!\n");
        log_err("[%s] unsupported request method!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

/* ----------------------------------- public  interface ----------------------------------- */

int main(int argc, char **argv) {
    // fill in content type
    //fprintf(stdout, "100-continue" EOL EOL);
    fprintf(stdout, "Content-type: text/plain\n\n");
    log_info("[%s] Content-type: text/plain\n\n", __FUNCTION__);

    // read request content
    int rc = _request_handler();
    fprintf(stdout, "code: %d\n", rc);
    log_info("[%s] cgi fin, code: %d\n", __FUNCTION__, rc);
    return 0;
}
