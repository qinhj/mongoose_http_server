# Http(s) Server based on Mongoose #

## Quick Start ##
```
## linux
$ cp config.rc.sample config.rc
$ vi config.rc
$ make
## windows
> copy data msvc
> ...
```

## Quick Test ##
```
## http test
$ curl -v localhost:8888/say_hello 2>/dev/null
$ curl -v localhost:8888/say_hello -d "post request" 2>/dev/null
$ curl -v localhost:8888/run -d "ls -l" 2>/dev/null
$ curl -v localhost:8888/download -d "config.rc.sample" 2>/dev/null > cfg.rc
$ curl -v localhost:8888/upload -F "cfg.rc=@config.rc.sample" 2>/dev/null
## https test(-k: turn off curl's verification of the certificate)
$ curl -v -k https://localhost:8443/say_hello 2>/dev/null
$ curl -v -k https://localhost:8443/say_hello -d "post request" 2>/dev/null
$ curl -v -k https://localhost:8443/run -d "ls -l" 2>/dev/null
$ curl -v -k https://localhost:8443/download -d "config.rc.sample" 2>/dev/null > cfg.rc
## cgi test
$ curl -v localhost:8888/demo.cgi?name=qinhj\&age=31 2>/dev/null
$ curl -v localhost:8888/demo.cgi?name=qinhj\&age=31 -d "post request" 2>/dev/null
```

## CGI Test ##
```
0. GET with normal parameters
# windows
* Note: unknown how to support multi-parameters
> curl -v localhost:8888/demo.cgi?name=qinhj
# linux
> curl -v localhost:8888/demo.cgi?name=qinhj\&age=31 2>/dev/null
> curl -v localhost:8888/demo.cgi?name=qinhj\&age=31 -d "post request" 2>/dev/null

1. POST with normal data
# Default Content-Type: application/x-www-form-urlencoded
> curl -v localhost:8888/demo.cgi -d "title=comewords&content=articleContent" -X POST
* Note: Unnecessary use of -X or --request, POST is already inferred.
> curl -v localhost:8888/demo.cgi -d "title=comewords&content=articleContent"

2. POST with json data
* Note: Unnecessary use of -X or --request, POST is already inferred.
# linux
> curl -v localhost:8888/demo.cgi -d '"title":"words","content":"articleContent"' \
-H "Content-Type:application/json"
# windows
> curl -v localhost:8888/demo.cgi -d '"title":"words","content":"articleContent"' ^
-H "Content-Type:application/json"

3. POST with file
* Note: if MG_ENABLE_HTTP_STREAMING_MULTIPART not 0, one could be blocked with: Done waiting for 100-continue
> curl -v -k https://localhost:8443/demo.cgi -F "file=@./README.md"

4. POST raw data to target file
* Note: if MG_ENABLE_HTTP_STREAMING_MULTIPART not 0, one could be blocked with: Done waiting for 100-continue
> curl -v -k https://localhost:8443/demo.cgi -F "/tmp/file=hello"
> cat /tmp/file
hello
```

## Toolkit Test ##
```
## linux
$ ./scripts/toolkit.sh
## windows (todo)
$ ./scripts/toolkit.bat
```

## Appendix ##
```
0. mongoose
(1) If 0 < MG_ENABLE_HTTP_STREAMING_MULTIPART, then all "MULTIPART" request will be
captured by http server, which means cgi won't be called.

1. Line continuation characters
windows: ^
linux:   \

2. http get env for cgi
REQUEST_METHOD
QUERY_STRING

3. http post env for cgi
REQUEST_METHOD
CONTENT_TYPE
CONTENT_LENGTH

4. start http server by python
$ sudo python -m SimpleHTTPServer 80
$ sudo python3 -m http.server 80
```
