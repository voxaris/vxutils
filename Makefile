#
# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          := YES
# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO
# LIBRARY can be set to YES if the target is a shared object
LIBRARY        := NO
# QUIET can be set to YES if we don't want commands echo'd
QUIET          := NO
#

#
# Programs
#
CC      := gcc
LD      := $(CC)
INSTALL := /usr/bin/install -m 555
RM      := /bin/rm -f

ifeq (YES, ${QUIET})
   CC      := @$(CC)
   LD      := @$(LD)
   INSTALL := @$(INSTALL)
   RM      := @$(RM)
endif

#
# Platform
#
UNAME := ${shell uname}

#
# Directories
#
OBJDIR := .obj
LIBDIR := .
DEPDIR := .dep
BINDIR := .

#APRCONFIG=/opt/apr/bin/apr-1-config

#
# Includes
#
#INCLUDES  := -I. $(shell $(APRCONFIG) --cflags --cppflags --includes)
INCLUDES  := -I. 
INCLUDES  := $(INCLUDES) -I/usr/local/include

#
# Libraries
#
#LIBS      := $(shell $(APRCONFIG) --ldflags --libs --link-ld)
LIBS      := $(LIBS) -L/usr/local/lib -lfftw3 -lm -lrt

#
# Compiler and Linker flags plus preprocessor defs
#
DEBUG_CFLAGS     := -std=c99 -pedantic -Wall -Werror -Wdeclaration-after-statement

RELEASE_CFLAGS   := $(DEBUG_CFLAGS)

DEBUG_LDFLAGS    := -g -ggdb
RELEASE_LDFLAGS  :=

ifeq (YES, ${DEBUG})
   CFLAGS        := ${DEBUG_CFLAGS}
   LDFLAGS       := ${DEBUG_LDFLAGS}
   DEFS          := -D${UNAME} -D_REENTRANT -DDEBUG -D_GNU_SOURCE
else
   CFLAGS        := ${RELEASE_CFLAGS}
   LDFLAGS       := ${RELEASE_LDFLAGS}
   DEFS          := -D${UNAME} -D_REENTRANT
endif

ifeq (YES, ${PROFILE})
   CFLAGS        := ${CFLAGS} -pg -O2
   LDFLAGS       := ${LDFLAGS} -pg
endif

ifeq (YES, ${LIBRARY})
   CFLAGS        := ${CFLAGS} -fPIC
   LDFLAGS       := ${LDFLAGS} -shared -fPIC
endif

#
# Source, Header, Object and Depends files
#
SOURCES := $(shell echo *.c)
HEADERS := $(shell echo *.h)
OBJECTS := $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))
DEPENDS := $(addprefix $(DEPDIR)/, $(SOURCES:.c=.d))

#
# Auto-dependency file create rule
#
$(addprefix $(DEPDIR)/, %.d) : %.c
	@echo "[DEP] $@"
	$(CC) -MM $(INCLUDES) $^ -MT $(addprefix $(OBJDIR)/, $(^:.c=.o)) > $@

#
# Object-file creation rule
#
$(addprefix $(OBJDIR)/, %.o) : %.c
	@echo "[CC]  $<"
	$(CC) -g -ggdb -c $(DEFS) $(CFLAGS) $(INCLUDES) $< -o $@

#
# The final product
#
VX_HASH := vx_hash
VX_HASH_OBJS := vx_hash.o
VX_HASH_OBJS := $(addprefix $(OBJDIR)/, $(VX_HASH_OBJS))

VX_SOCKET := vx_socket
VX_SOCKET_OBJS := vx_socket.o
VX_SOCKET_OBJS := $(addprefix $(OBJDIR)/, $(VX_SOCKET_OBJS))

#
# The build rule
#
all: $(VX_HASH) $(VX_SOCKET)

$(VX_HASH): $(VX_HASH_OBJS)  
	@echo "[LD]  $@"
	$(LD) $(VX_HASH_OBJS) -o $@ $(LDFLAGS) $(LIBS)

$(VX_SOCKET): $(VX_SOCKET_OBJS)  
	@echo "[LD]  $@"
	$(LD) $(VX_SOCKET_OBJS) -o $@ $(LDFLAGS) $(LIBS)

#
# Include auto-generated dependencies
#
-include $(DEPENDS)

#
# Clean it up
#
clean:
	$(RM) $(OBJECTS) $(DEPENDS)
	$(RM) $(VX_HASH) $(VX_SOCKET)
	$(RM) -r docs/html docs/latex

#
# Make Doxygen docs
#
dox:
	doxygen docs/Doxyfile

cleandox:
	$(RM) -r docs/html docs/latex

#
# List out some of the defined stuff
#
list:
	@echo "HASH_USAGE   : $(HASH_USAGE)"
	@echo "UNAME  : $(UNAME)"
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"
	@echo "DEPENDS: $(DEPENDS)"
	@echo "HEADERS: $(HEADERS)"
	@echo "INCLUDE: $(INCLUDES)"
	@echo "CFLAGS : $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"

#
# Install
#
install: $(HASH_USAGE)
	$(INSTALL) -v -p $(HASH_USAGE) /opt/emad/bin/
