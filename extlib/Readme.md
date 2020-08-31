## pre-build 3rd libraries for http/https server ##
OS: Ubuntu-64-14.04  

### openssl-1.0.2u ###
```
## build static library
$ ./config --prefix=./output
$ ./config -t
$ make && make install
```