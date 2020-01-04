CC ?=gcc
AR ?=ar


WMSG = -Wall

TARGETBIN ?= ./bin
TARGETDIR ?= $(TARGETBIN)/lib
LDFLAGS ?= -lz -lpthread -lssl -L$(TARGETDIR) -lreflectjson
CXXFLAGS ?= -g -rdynamic -I ./ $(MACROS)

LIBDIRS = 
LIBSRC = $(patsubst %, %/*.c, $(LIBDIRS))

OBJECTS = $(patsubst %.c, %.o, $(wildcard ./*.c))


.PHONY: all clean cleanall

all: static
	@mkdir -p $(TARGETBIN)/include
	#cp -f ./llqvarymem.h    		$(TARGETBIN)/include/
	cp -f ./llqreflect.h          	$(TARGETBIN)/include/
	cp -f ./libreflectjson.h 		$(TARGETBIN)/include/

static:  $(OBJECTS) 
	@mkdir -p $(TARGETDIR)
	@echo "AR $(TARGETDIR)/libreflectjson.a"
	@$(AR) rc $(TARGETDIR)/libreflectjson.a $(OBJECTS)
	$(CC) $(WMSG) -o $(TARGETBIN)/testjson testjson.cc $(CXXFLAGS) $(LDFLAGS)
	@rm -f $(OBJECTS)
	@echo ""

$(OBJECTS): %.o : %.c
	@echo "CC $@"
	@$(CC) $(WMSG) -c $< -o $@  $(CXXFLAGS) 

clean:
	@rm -f  $(OBJECTS) 

cleanall:clean
	@rm -f $(OBJECTS)

