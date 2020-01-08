AUTOMAKE_OPTIONS=foreign 
include_HEADERS = libreflectjson.h

#SUBDIRS     = example

# noinst_LIBRARIES            = libreflectjson.a
#lib_LIBRARIES            = libreflectjson.a
#libreflectjson_a_SOURCES    = json2str.c json4str.c jsonchilds.c jsonentity.c jsonobject.c jsonreflect.c
#libreflectjson_a_LDFLAGS    = -lz -lpthread -lssl

# noinst_LTLIBRARIES         = libreflectjson.la
lib_LTLIBRARIES              = libreflectjson.la
libreflectjson_la_SOURCES    = json2str.c json4str.c jsonchilds.c jsonentity.c jsonobject.c jsonreflect.c
libreflectjson_la_LDFLAGS    = -lz -lpthread -lssl

noinst_PROGRAMS    = testjson
testjson_SOURCES    = example/testjson.c
testjson_LDFLAGS    = -lz -lpthread -lssl -lreflectjson

noinst_PROGRAMS    = testentity
testentity_SOURCES    = example/testentity.c
testentity_LDFLAGS    = -lz -lpthread -lssl -lreflectjson
