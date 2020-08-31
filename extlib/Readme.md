## pre-build 3rd libraries for http/https server ##

### openssl-1.0.2u ###
```
* linux(e.g. Ubuntu-64-14.04)
## build static library
$ ./config --prefix=./output
$ ./config -t
$ make && make install

* msvc(https://www.npcglib.org/~stathis/blog/precompiled-openssl/)
```