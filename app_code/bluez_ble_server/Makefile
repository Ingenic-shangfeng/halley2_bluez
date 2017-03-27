app = ble_server

srcExt = c
objDir = obj
binDir = .
inc = . src src/common src/lib src/mainloop

CFlags = -Wall -O2 -fdata-sections -ffunction-sections
LDFlags = -Wl,--gc-sections
libs = rt pthread
libDir =

sources=src/main.c \
	 src/gatt.c \
	 src/att.c \
	 src/att_io.c \
	 src/bt_util.c \
	 src/common/log.c \
	 src/common/mem_manage.c \
	 src/common/queue.c \
	 src/common/bt_mutex.c \
	 src/common/bt_semaphore.c \
	 src/mainloop/io-mainloop.c \
	 src/mainloop/timeout-mainloop.c \
	 src/mainloop/mainloop.c \
	 src/mainloop/util.c \
	 src/lib/mgmt.c \
	 src/lib/hci.c \
	 src/lib/uuid.c \
	 src/lib/bluetooth.c \

sources += $(shell find ./src/services/ -name '*.$(srcExt)')

#************************ DO NOT EDIT BELOW THIS LINE! ************************

inc := $(addprefix -I,$(inc))
libs := $(addprefix -l,$(libs))
libDir := $(addprefix -L,$(libDir))
CFlags += -c $(inc) $(libDir) $(libs)
LDFlags += $(libs)
srcDirs := $(shell find . -name '*.$(srcExt)' -exec dirname {} \; | uniq)
objects := $(patsubst %.$(srcExt),$(objDir)/%.o,$(sources))

.phony: all clean distclean

all: $(binDir)/$(app)

$(binDir)/$(app): buildrepo $(objects)
	@mkdir -p `dirname $@`
	@echo "Linking $@..."
	@$(CC) $(objects) $(LDFlags) -o $@

sinclude $(sources:%.$(srcExt)=$(objDir)/%.d)
$(objDir)/%.o: %.$(srcExt)
#	@echo "Generating dependencies for $<..."
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	@echo "Compiling $<..."
	@$(CC) $(CFlags) $< -o $@

clean:
	@$(RM) -r $(objDir)

distclean: clean
	@$(RM) -r $(binDir)/$(app)

buildrepo:
	@$(call make-repo)

define make-repo
   for dir in $(srcDirs); \
   do \
	mkdir -p $(objDir)/$$dir; \
   done
endef


# usage: $(call make-depend,source-file,object-file,depend-file)
define make-depend
  $(CC) -MM       \
        -MF $3    \
        -MP       \
        -MT $2    \
        $(CFlags) \
        $1
endef
