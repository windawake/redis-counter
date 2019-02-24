WARNINGS=-Wall -Wwrite-strings
DEBUG_FLAGS?= -g3

SERVER_SOURCE = server.c
CLIENT_SOURCE = client.c

CORE_INCLUDE = -I include

CORE_COMPILE = redis_assert.c \
	sds.c \
	dict.c \
	siphash.c \
	adlist.c \
	object.c \
	db.c \
	command.c \
	zmalloc.c

.PHONY: server client test sqlite db sds dict adlist command

server:
	gcc -o $@ $(SERVER_SOURCE)  $(CORE_COMPILE) $(DEBUG_FLAGS) $(WARNINGS) $(CORE_INCLUDE)

client:
	gcc -o $@ $(CLIENT_SOURCE)  $(CORE_COMPILE) $(DEBUG_FLAGS) $(WARNINGS) $(CORE_INCLUDE)

sds:
	gcc -o $@ $(CORE_COMPILE) $(DEBUG_FLAGS) $(WARNINGS) $(CORE_INCLUDE) -D SDS_TEST_MAIN

dict:
	gcc -o $@ $(CORE_COMPILE) $(DEBUG_FLAGS) $(WARNINGS) $(CORE_INCLUDE) -D DICT_TEST_MAIN

adlist:
	gcc -o $@ $(CORE_COMPILE) $(DEBUG_FLAGS) $(WARNINGS) $(CORE_INCLUDE) -D ADLIST_TEST_MAIN

db:
	gcc -o $@ $(CORE_COMPILE) $(DEBUG_FLAGS) $(WARNINGS) $(CORE_INCLUDE) -D DB_TEST_MAIN

command:
	gcc -o $@ $(CORE_COMPILE) $(DEBUG_FLAGS) $(WARNINGS) $(CORE_INCLUDE) -D COMMAND_TEST_MAIN

#sqlite:
#	gcc -o $@ sqlite.c  $(DEBUG_FLAGS) $(WARNINGS) -lsqlite3

clean:
	rm -rf server client adlist client db dict sds sqlite command