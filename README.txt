CONCURRENT TCP SERVER - TECHNICAL TEST
=======================================

DESCRIPTION:
-----------
Simple TCP key-value store server with TTL support.
Handles multiple clients concurrently using pthreads.

COMPILE:
--------
make              # compile using Makefile
gcc -pthread -o server teste.c   # manual compile

RUN:
----
./server          # run server on port 6379

CLEAN:
------
make clean        # remove executable

CONNECT:
--------
telnet localhost 6379
nc localhost 6379

COMMANDS:
---------
PING                    -> PONG
SET key value           -> OK
GET key                 -> VALUE value | NOT_FOUND
DEL key                 -> DELETED | NOT_FOUND
SETEX key seconds value -> OK (expires after seconds)

EXAMPLE SESSION:
---------------
$ telnet localhost 6379
PING
PONG
SET name John
OK
GET name
VALUE John
SETEX session 5 abc123
OK
GET session
VALUE abc123
( wait 5 seconds )
GET session
NOT_FOUND
DEL name
DELETED
GET name
NOT_FOUND

LIMITATIONS:
------------
- Maximum 100 keys
- Expired keys remain in memory until accessed
- Fixed-size buffers (for simplicity in this technical assessment)
- No persistence
- No authentication
