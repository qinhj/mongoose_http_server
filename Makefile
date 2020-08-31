# ==============================================
#           HTTP(S) SERVER && ENDPOINTS
# ----------------------------------------------
# @brief:   makefile for http(s) server and cgi
# @author:  qinhj@lsec.cc.ac.cn
# @version: v1.0.0 2020/08/31
# ==============================================

-include config.rc

# compiler
CC  := $(CROSS_TOOL)gcc
CXX := $(CROSS_TOOL)g++
AR  := $(CROSS_TOOL)ar
LD  := $(CROSS_TOOL)ld
STRIP := $(CROSS_TOOL)strip

# default config:
CFLAG   += --sysroot=$(SYSROOT) -fmessage-length=0
CPPFLAG += --sysroot=$(SYSROOT) -fmessage-length=0
# custom config:
CFLAG   += -O0 -g3 -Wall -Wno-unused-function #-D _GNU_SOURCE -fvisibility=hidden
CPPFLAG += -O0 -g3 -Wall -Wno-unused-function #-D _GNU_SOURCE -fvisibility=hidden
ifeq ($(MODE), debug)
CFLAG   += -D_DEBUG
CPPFLAG += -D_DEBUG
endif
ifneq ($(ENABLE_INDEX),)
CFLAG   += -DENABLE_INDEX
CPPFLAG += -DENABLE_INDEX
endif

# server config:
MG_HTTP := -DMG_ENABLE_MQTT=0
MG_HTTPS:= $(MG_HTTP) -DMG_ENABLE_SSL
# ------------------------------------------------------------------------------
# Pay attention please! With following macro, all request with type "MULTIPART"
# will be captured by server itself, which means that cgi won't be invoked.
# ------------------------------------------------------------------------------
MG_HTTP += -DMG_ENABLE_HTTP_STREAMING_MULTIPART=1
#MG_HTTPS+= -DMG_ENABLE_HTTP_STREAMING_MULTIPART=1

# include settings
inc     := -I./include
extinc  += -I./extinc
INCFLAG = $(inc) $(extinc)

# library settings
lib     := -L./lib
extlib  += -L./extlib
LIBPATH := $(lib) $(extlib)
# dependency for http/https server
# ------------------------------------------------------------------------------
# Pay attention please! ssl must be ahead of crypto in the library reference !!!
# Actually, the following libs are required only for https server.
# ------------------------------------------------------------------------------
LIBLINK := -lssl -lcrypto -ldl

# ========================================================================================

# compile flag
LIBFLAG := $(CFLAG) -shared -fPIC
LDFLAG  := $(LIBPATH) -rdynamic -Wl,--as-needed $(LIBLINK)

# runtime rpath dir
RPATH_DIR ?= #.

# targets: lib/bin/sample
BIN_SVC := http_server https_server
BIN_CGI := demo.cgi
TARGETS := $(BIN_SVC) $(BIN_CGI)

# source code files
SRC_DIRS := ./src
SRC_FILE := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
SRC_FILE += $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))
SRC_OBJS := $(patsubst %.c, %.o, $(SRC_FILE))
SRC_OBJS := $(patsubst %.cpp, %.o, $(SRC_OBJS))

# ========================================================================================

default: release

info:
	@echo "============================================================================="
	@echo "ROOT_DIR: $(ROOT_DIR)"
	@echo "COMPILER: $(CC)/$(CXX)"
	@echo "CFLAG:    $(CFLAG)"
	@echo "INCFLAG:  $(INCFLAG)"
	@echo "LIBLINK:  $(LIBLINK)"
	@echo "SYSROOT:  $(SYSROOT)"
	@echo "STAGING:  $(STAGING)"
	@echo "DESTDIR:  $(DESTDIR)"
	@echo "============================================================================="

_clean:
	for subdir in $(SRC_DIRS); do \
		(cd $$subdir && rm -rf *.o *~ *.d) \
	done;
clean:
	make _clean
	rm -f $(TARGETS)

release: clean info
	make svc
	make cgi

# ----------------
svc: $(BIN_SVC)
# ----------------
http_server: ./src/mongoose.c ./src/server.c
	$(CC) -o $@ $^ $(INCFLAG) $(CFLAG) $(MG_HTTP) $(LDFLAG) -Wl,-rpath,$(RPATH_DIR)
https_server: ./src/mongoose.c ./src/server.c
	$(CC) -o $@ $^ $(INCFLAG) $(CFLAG) $(MG_HTTPS) $(LDFLAG) -Wl,-rpath,$(RPATH_DIR)

# ----------------
cgi: $(BIN_CGI)
# ----------------
demo.cgi: ./src/demo.cgi.o
	$(CC) -o $@ $^ $(CFLAG) $(LDFLAG) -Wl,-rpath,$(RPATH_DIR)

_install_svc:
	@mkdir -p $(DESTDIR)/usr/bin/
	@cp -v $(BIN_SVC) $(DESTDIR)/usr/bin/
_install_cgi:
	@mkdir -p $(DESTDIR)/usr/bin/cgi/
	@cp -v $(BIN_CGI) $(DESTDIR)/usr/bin/cgi/
install: release
	@make _install_svc
	@make _install_cgi

#.NOTPARALLEL: $(TARGETS)
.PHONY: release clean info install

# ========================================================================================

%.o: %.c
	$(CC) $(INCFLAG) -fPIC -O2 $(CFLAG) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(INCFLAG) -fPIC -O2 $(CPPFLAG) -o $@ -c $<
