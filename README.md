# cStuff
Set of ready C modules to be compiled into your Project.

* base64 - encode / decode functions (using libcrypt)
* buffer - class for bytes buffering
* config - lightweight parser for configuration text file
* crc32 - CRC32 calculating module
* datetime - class for datetime with nanoseconds
* dbx - module for asynchronous communication with PostgreSQL
* fs-utils - functions to work with filesystem
* list - class for list of pointers
* log - class and functions to have logging in your project
* md5 - functions to calculate MD5 checksum (using libcrypt)
* query-stream - stream parser for QUERY STRING (param=val&next_param=val2)
* retcodes.h - list of return codes for all cStuff modules
* sock-utils - set of functions for sockets
* str-builder - class to build null terminated strings by appending
* str-utils - set of functions to operate null terminated strings
* templight - class for generally HTML templating
* uri - function to decode URI
* version.h - common version structure
* vmpc - C implementation of VMPC encription/decription algorithm

## The Idea
* To do not link extra huge libraries for just couple of helper functions
* To do not make dependencies on unused code
* To have simple solutions for common tasks under your hand

## License
All code in this repository is being distributed under 
[Unlicense](http://unlicense.org/).
