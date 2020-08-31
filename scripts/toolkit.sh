#!/bin/bash

####################################################################
#                       Http(s) Server Toolkit
# ==================================================================
# @Author:  qinhj@lsec.cc.ac.cn
# ------------------------------------------------------------------
# @Note:    To use "getopts" "let", we need bash.
# ------------------------------------------------------------------
# @History:
# 2020/08/31 v1.0.0 init/create
####################################################################

#########################
#### Global Settings ####
#########################

## debug settings
PATH=.:$PATH
set -e # -ex
## script version
VERSION=1.0.0
## script options
COMMANDS="usage download upload debug config telnet"
COMMENTS="<command> [-c <config>] [-i <input>] [-o <output>] [-l <remote ip>] [-h <host ip>]"
OPTIONS=":c:i:o:l:h:"
EXAMPLE0="usage"
EXAMPLE1="download -l 127.0.0.1:8888 -i http_server -o xxx"
EXAMPLE2="upload -l 127.0.0.1:8888 -i list.txt"
EXAMPLE3="debug -l 127.0.0.1:8888 -i 'ls -l /tmp'"
EXAMPLE4="config -o toolkit.cfg"
EXAMPLE5="telnet -l 127.0.0.1:8888"
## gloval variable
PROTOCOL="http"     # https
CONFIG=   # "toolkit.cfg"
INFILE="list.txt"   # input file or commands
OUTFILE=  # output file path
CAMADDR=  # camera ip address
SVCADDR=  # host ip address

#########################
#### Function Region ####
#########################

### Private  Function ###

## hard code as Red
EchoError() {
  echo -e "\033[31m[Error ] $@\033[0m"
}

## hard code as LightBlue
EchoTitle() {
  echo -e "\033[36m$@\033[0m"
}

## hard code as Yellow
EchoSection() {
  echo -e "\033[33m$@\033[0m"
}

ShowTitle() {
  EchoTitle "Imi-Samsung Camera Toolkit [Version: $VERSION]"
}

ShowUsage() {
  EchoSection "Usage:"
  echo "  [bash] $0 $COMMENTS"
}

ShowCommand() {
  EchoSection "Command:"
  echo "  usage     show usage"
  echo "  download  download file from cam  (related options: -l -i -o)"
  echo "  upload    upload files to camera  (related options: -l -i)"
  echo "  debug     local debugging camera  (related options: -l)"
  echo "  config    create cfg sample file  (related options: -o)"
  echo "  telnet    run commands in camera  (related options: -l)"
}

ShowOptions() {
  EchoSection "Options:"
  echo "  -v    show version    (current: $VERSION)"
  echo "  -c    config file     (default: $CONFIG)"
  echo "  -i    input file/cmd  (default: $INFILE)"
  echo "  -o    output file     (default: $OUTFILE)"
  echo "  -l    remote camera ip(default: $CAMADDR)"
  echo "  -h    host server ip  (default: $SVCADDR)"
}

ShowExample() {
  EchoSection "Example:"
  echo -e "  $0 $EXAMPLE0\n  $0 $EXAMPLE1"
  echo -e "  $0 $EXAMPLE2\n  $0 $EXAMPLE3"
  echo -e "  $0 $EXAMPLE4\n  $0 $EXAMPLE5"
}

ShowNotes() {
  EchoSection "Note:"
  echo "1. To avoid overwrite, '-c' should be in front of all the other options."
}

## Error Code
## 101: miss/invalid parameter
## 102: open/close file fail
ErrorHandle() {
  #echo "[Debug ] Error Code: $1"
  case $1 in
    1)    ## command not defined
      EchoError "Unknown Command: $COMMAND" && usage;;
    127)  ## command not found
      usage;;
    *)    ## do nothing
      ;;
  esac
}

#### Public Function ####

## command 0
usage() {
  ShowTitle
  ShowUsage
  ShowCommand
  ShowOptions
  ShowExample
  ShowNotes
}

## command 1
download() {
  if [[ x"" == x"$CAMADDR" || x"" == x"$INFILE" ]]; then
    EchoError "[Download] Error: missing ip address(-l) or file path(-i)"
    return 101
  fi
  if [ x"" == x"$OUTFILE" ]; then
    ## /tmp/xxx.mp4 -> xxx.mp4
    OUTFILE=${INFILE##*/}
  fi
  curl -v -k ${PROTOCOL}://${CAMADDR}/download -d "$INFILE" > $OUTFILE
  echo "[Download] Finish."
}

## command 2
upload() {
  if [ x"" == x"$CAMADDR" ]; then
    EchoError "[Upload] Error: missing ip address, please add option -l"
    return 101
  fi
  if [ x"" == x"$INFILE" ]; then
    INFILE=list.txt
  fi
  if [ ! -f $INFILE ]; then
    EchoError "[Upload] Error: Open $INFILE failed!"
    return 102
  fi
  echo "[Upload] upload files to $CAMADDR ..."

  #set -x
  cnt=0
  ## read line by line: ^<from>:<to>:<mode>$
  while read line;
  do
    ## 0th check
    if [[ "#" == "${line:0:1}" || x"" == x"${line}" ]]; then
      echo "[Upload] skip line: $line"
      continue
    fi
    let cnt++
    from=${line%%:*}
    to=${line#*:}
    to=${to%%:*}
    mode=${line##*:}
    ## 1st backup
    echo "[Upload] ============================================"
    echo "[Upload] [$cnt] backup ${to} -> ${to}.bak"
    echo "[Upload] ============================================"
    curl -v -k ${PROTOCOL}://${CAMADDR}/run -d "mv -f ${to} ${to}.bak"
    ## 2nd upload
    echo "[Upload] ============================================"
    echo "[Upload] [$cnt] upload $from -> $to"
    echo "[Upload] ============================================"
    curl -v -k ${PROTOCOL}://${CAMADDR}/upload -F "${to}=@${from}"
    ## 3rd chmod
    echo "[Upload] ============================================"
    echo "[Upload] [$cnt] chmod $mode $to"
    echo "[Upload] ============================================"
    curl -v -k ${PROTOCOL}://${CAMADDR}/run -d "chmod ${mode} ${to}"
  done < $INFILE

  ## Todo: fix $cnt!
  echo "[Upload] Finish. Total uploaded files: $cnt."
  return 0
}

## command 3
debug() {
  if [ x"" == x"$CAMADDR" ]; then
    EchoError "[Debug ] Error: missing ip address, please add option -l"
    return 101
  fi
  if [ x"" == x"$INFILE" ]; then
    EchoError "[Debug ] Error: missing debug command, please add option -i"
    return 101
  fi
  #echo "[Debug ] $INFILE"
  if [ x"" == x"$OUTFILE" ]; then
    curl -v -k ${PROTOCOL}://${CAMADDR}/run -d "$INFILE" 2>/dev/null
  else
    curl -v -k ${PROTOCOL}://${CAMADDR}/run -d "$INFILE" > ${OUTFILE} 2>/dev/null
  fi
  #echo "[Debug ] Finish."
}

## command 4
config() {
  if [ x"" == x"$OUTFILE" ]; then
    OUTFILE="toolkit.cfg"
  fi
  cat <<EOF > $OUTFILE
## gloval variable
PROTOCOL="http"     # https
CONFIG="toolkit.cfg"
INFILE="list.txt"   # input file or commands
OUTFILE=  # output file path
CAMADDR=192.168.0.108 # remote ip address
SVCADDR=192.168.0.104 # host ip address
EOF
  echo "[Config] Finish."
}

## load config
ConfigLoad() {
  if [ -f $CONFIG ]; then
    source $CONFIG
    echo "[Config] update configuration from $CONFIG ..."
  else
    EchoError "[Config] Error: load config file fail!"
  fi
}

## command 5
telnet() {
  ## check has camera ip or not
  if [ x"" == x"$CAMADDR" ]; then
    EchoError "[Telnet] Error: missing ip address, please add option -l"
    return 101
  fi
  ## Todo: active backdoor, i.e. kill imi guardian
  ## reset input && output
  OUTPUT=
  while :; do
    read -p "# " INFILE
    if [ x"exit" == x"$INFILE" ]; then
      break
    else
      debug
    fi
  done
}

#########################
####  Main   Region  ####
#########################

## check inputs
if [ 0 -eq $# ]; then
  usage
  exit 0
fi

## get command
COMMAND="$1"
#echo "[Debug ] Command: $1"
shift

## parse option
while getopts $OPTIONS opt; do
  #echo "[Debug ] opt: $opt, arg: $OPTARG"
  case $opt in
    c)
      CONFIG=$OPTARG
      echo "[Debug ] reset config as: $CONFIG" && ConfigLoad;;
    i)
      INFILE=$OPTARG
      echo "[Debug ] reset input as: $INFILE";;
    o)
      OUTFILE=$OPTARG
      echo "[Debug ] reset output as: $OUTFILE";;
    l)
      CAMADDR=$OPTARG
      echo "[Debug ] reset camera ip: $CAMADDR";;
    h)
      SVCADDR=$OPTARG
      echo "[Debug ] reset server ip: $SVCADDR";;
    ?)
      EchoError "unknown option: $OPTARG"
      break
    esac
done

## shift to remain options
shift $(($OPTIND - 1))
#echo "[Debug ] remain opt: $@"

## run command
([[ $COMMANDS =~ $COMMAND ]] && $COMMAND) || (ErrorHandle $?)
