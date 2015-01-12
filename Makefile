## Haiku Generic Makefile v2.6 ## 

## Fill in this file to specify the project being created, and the referenced
## Makefile-Engine will do all of the hard work for you. This handles any
## architecture of Haiku.

# The name of the binary.
NAME = AVLDupTree

# The type of binary, must be one of:
#	APP:	Application
#	SHARED:	Shared library or add-on
#	STATIC:	Static library archive
#	DRIVER: Kernel driver
TYPE = SHARED

# 	If you plan to use localization, specify the application's MIME signature.
APP_MIME_SIG = 

#	The following lines tell Pe and Eddie where the SRCS, RDEFS, and RSRCS are
#	so that Pe and Eddie can fill them in for you.
#%{
# @src->@ 

#	Specify the source files to use. Full paths or paths relative to the 
#	Makefile can be included. All files, regardless of directory, will have
#	their object files created in the common object directory. Note that this
#	means this Makefile will not work correctly if two source files with the
#	same name (source.c or source.cpp) are included from different directories.
#	Also note that spaces in folder names do not work well with this Makefile.
SRCS = Source/AVLDupTree.c

#	Specify the resource definition files to use. Full or relative paths can be
#	used.
RDEFS = Source/AGMSAVLTest.rdef

#	Specify the resource files to use. Full or relative paths can be used.
#	Both RDEFS and RSRCS can be utilized in the same Makefile.
RSRCS = 

# End Pe/Eddie support.
# @<-src@ 
#%}

#	Specify libraries to link against.
#	There are two acceptable forms of library specifications:
#	-	if your library follows the naming pattern of libXXX.so or libXXX.a,
#		you can simply specify XXX for the library. (e.g. the entry for
#		"libtracker.so" would be "tracker")
#
#	-	for GCC-independent linking of standard C++ libraries, you can use
#		$(STDCPPLIBS) instead of the raw "stdc++[.r4] [supc++]" library names.
#
#	- 	if your library does not follow the standard library naming scheme,
#		you need to specify the path to the library and it's name.
#		(e.g. for mylib.a, specify "mylib.a" or "path/mylib.a")
LIBS = 

#	Specify additional paths to directories following the standard libXXX.so
#	or libXXX.a naming scheme. You can specify full paths or paths relative
#	to the Makefile. The paths included are not parsed recursively, so
#	include all of the paths where libraries must be found. Directories where
#	source files were specified are	automatically included.
LIBPATHS = 

#	Additional paths to look for system headers. These use the form
#	"#include <header>". Directories that contain the files in SRCS are
#	NOT auto-included here.
SYSTEM_INCLUDE_PATHS = 

#	Additional paths paths to look for local headers. These use the form
#	#include "header". Directories that contain the files in SRCS are
#	automatically included.
LOCAL_INCLUDE_PATHS = 

#	Specify the level of optimization that you want. Specify either NONE (O0),
#	SOME (O1), FULL (O2), or leave blank (for the default optimization level).
OPTIMIZE := FULL

# 	Specify the codes for languages you are going to support in this
# 	application. The default "en" one must be provided too. "make catkeys"
# 	will recreate only the "locales/en.catkeys" file. Use it as a template
# 	for creating catkeys for other languages. All localization files must be
# 	placed in the "locales" subdirectory.
LOCALES = en

#	Specify all the preprocessor symbols to be defined. The symbols will not
#	have their values set automatically; you must supply the value (if any) to
#	use. For example, setting DEFINES to "DEBUG=1" will cause the compiler
#	option "-DDEBUG=1" to be used. Setting DEFINES to "DEBUG" would pass
#	"-DDEBUG" on the compiler's command line.
DEFINES = 

#	Specify the warning level. Either NONE (suppress all warnings),
#	ALL (enable all warnings), or leave blank (enable default warnings).
WARNINGS = 

#	With image symbols, stack crawls in the debugger are meaningful.
#	If set to "TRUE", symbols will be created.
SYMBOLS := 

#	Includes debug information, which allows the binary to be debugged easily.
#	If set to "TRUE", debug info will be created.
DEBUGGER := 

#	Specify any additional compiler flags to be used.
COMPILER_FLAGS = 

#	Specify any additional linker flags to be used.
LINKER_FLAGS = 

#	Specify the version of this binary. Example:
#		-app 3 4 0 d 0 -short 340 -long "340 "`echo -n -e '\302\251'`"1999 GNU GPL"
#	This may also be specified in a resource.
APP_VERSION := 

#	(Only used when "TYPE" is "DRIVER"). Specify the desired driver install
#	location in the /dev hierarchy. Example:
#		DRIVER_PATH = video/usb
#	will instruct the "driverinstall" rule to place a symlink to your driver's
#	binary in ~/add-ons/kernel/drivers/dev/video/usb, so that your driver will
#	appear at /dev/video/usb when loaded. The default is "misc".
DRIVER_PATH = 

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
##	BeOS and Haiku Generic Makefile Engine v2.5.1
##	Does all the hard work for the Generic Makefile
##	which simply defines the project parameters

##	Supports Generic Makefile v2.0, 2.01, 2.1, 2.2, 2.3, 2.4, 2.5

#	determine wheather running on x86 or ppc
MACHINE=$(shell uname -m)
ifeq ($(MACHINE), BePC)
	CPU = x86
else
	CPU = $(MACHINE)
endif

#	create some default settings
ifeq ($(NAME), )
	NAME = NameThisApp
endif

ifeq ($(TYPE), )
	TYPE = APP
endif

ifeq ($(APP_MIME_SIG), )
	APP_MIME = x.vnd-Haiku-$(NAME)
endif

ifeq ($(DRIVER_PATH), )
	DRIVER_PATH = misc
endif

#	specify the mimeset tool
	MIMESET		:= mimeset

#	specify the tools for adding and removing resources
	XRES		:= xres

#	specify the tools for compiling resource definition files
	RESCOMP		:= rc

#	set the compiler and compiler flags
	CC			:=	gcc
	C++			:=	g++

#	SETTING: set the CFLAGS for each binary type
	ifeq ($(strip $(TYPE)), DRIVER)
		CFLAGS	+= -D_KERNEL_MODE=1 -fno-pic
	else
		CFLAGS +=
	endif

#	SETTING: set the proper optimization level
	ifeq ($(strip $(OPTIMIZE)), FULL)
		OPTIMIZER	= -O3
	else
	ifeq ($(strip $(OPTIMIZE)), SOME)
		OPTIMIZER	= -O1
	else
	ifeq ($(strip $(OPTIMIZE)), NONE)
		OPTIMIZER	= -O0
	else
#		OPTIMIZE not set so set to full
		OPTIMIZER	= -O3
	endif
	endif
	endif

#	SETTING: set proper debugger flags
	ifeq ($(strip $(DEBUGGER)), TRUE)
		DEBUG += -g
		OPTIMIZER = -O0
	endif

	CFLAGS += $(OPTIMIZER) $(DEBUG)

#	SETTING: set warning level
	ifeq ($(strip $(WARNINGS)), ALL)
		CFLAGS += -Wall -Wno-multichar -Wno-ctor-dtor-privacy
	else
	ifeq ($(strip $(WARNINGS)), NONE)
		CFLAGS += -w
	endif
	endif

#	set the linker and linker flags
ifeq ($(origin LD), default)
	LD			:= gcc
endif
	LDFLAGS		+= $(DEBUG)

#	SETTING: set linker flags for each binary type
	ifeq ($(strip $(TYPE)), APP)
		LDFLAGS += -Xlinker -soname=_APP_
	else
	ifeq ($(strip $(TYPE)), SHARED)
		LDFLAGS += -shared -Xlinker -soname=$(NAME)
	else
	ifeq ($(strip $(TYPE)), DRIVER)
		LDFLAGS += -nostdlib /boot/system/develop/lib/_KERNEL_ \
					/boot/system/develop/lib/haiku_version_glue.o
	endif
	endif
	endif

#	guess compiler version
	CC_VER = $(word 1, $(subst -, , $(subst ., , $(shell $(CC) -dumpversion))))

#	set the directory where object files and binaries will be created
	OBJ_DIR		:= objects.$(CPU)-$(CC)$(CC_VER)-$(if $(DEBUGGER),debug,release)

# 	specify the directory the binary should be created in by default
ifeq ($(TARGET_DIR), )
	TARGET_DIR	:= $(OBJ_DIR)
endif

# NOTE: make doesn't find the target if its name is enclosed in
#       quotation marks
ifeq ($(strip $(TYPE)), STATIC)
	TARGET		:= $(TARGET_DIR)/$(NAME).a
else
	TARGET		:= $(TARGET_DIR)/$(NAME)
endif


# psuedo-function for converting a list of source files in SRCS variable
# to a corresponding list of object files in $(OBJ_DIR)/xxx.o
# The "function" strips off the src file suffix (.ccp or .c or whatever)
# and then strips of the directory name, leaving just the root file name.
# It then appends the .o suffix and prepends the $(OBJ_DIR)/ path
define SRCS_LIST_TO_OBJS
	$(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(foreach file, $(SRCS), \
	$(basename $(notdir $(file))))))
endef

define SRCS_LIST_TO_DEPENDS
	$(addprefix $(OBJ_DIR)/, $(addsuffix .d, $(foreach file, $(SRCS), \
	$(basename $(notdir $(file))))))
endef

OBJS = $(SRCS_LIST_TO_OBJS)
DEPENDS = $(SRCS_LIST_TO_DEPENDS)

# create a unique list of paths to our sourcefiles and resources
SRC_PATHS += $(sort $(foreach file, $(SRCS) $(RSRCS) $(RDEFS), $(dir $(file))))

# add source paths to VPATH if not already present
VPATH :=
VPATH += $(addprefix :, $(subst  ,:, $(filter-out $($(subst, :, ,$(VPATH))), $(SRC_PATHS))))

#	SETTING: build the local and system include paths, compose C++ libs
ifneq (,$(filter $(CPU),x86 x86_64))
	LOC_INCLUDES = $(foreach path, $(SRC_PATHS) $(LOCAL_INCLUDE_PATHS), $(addprefix -I, $(path)))
 ifeq ($(CC_VER), 2)
	INCLUDES = $(LOC_INCLUDES)
	INCLUDES += -I-
	INCLUDES += $(foreach path, $(SYSTEM_INCLUDE_PATHS), $(addprefix -I, $(path)))

	STDCPPLIBS = stdc++.r4
 else
	INCLUDES = -iquote./
	INCLUDES += $(foreach path, $(SRC_PATHS) $(LOCAL_INCLUDE_PATHS), $(addprefix -iquote, $(path)))
	INCLUDES += $(foreach path, $(SYSTEM_INCLUDE_PATHS), $(addprefix -isystem, $(path)))

	STDCPPLIBS = stdc++ supc++
 endif
else
ifeq ($(CPU), ppc)
	LOC_INCLUDES = $(foreach path, $(SRC_PATHS) $(LOCAL_INCLUDE_PATHS), $(addprefix -I, $(path)))
	SYS_INCLUDES += -i-
	SYS_INCLUDES += $(foreach path, $(SYSTEM_INCLUDE_PATHS), $(addprefix -i , $(path)))

	INCLUDES = $(LOC_INCLUDES) $(SYS_INCLUDES)
endif
endif


# SETTING: add the -L prefix to all library paths to search
LINK_PATHS = $(foreach path, $(SRC_PATHS) $(LIBPATHS), \
	$(addprefix -L, $(path)))

#	SETTING: specify the additional libraries to link against
#	if the libraries have a .so or .a prefix, or if they are _APP_ or _KERNEL_
#	simply add them to the list
LINK_LIBS += $(filter %.so %.a _APP_ _KERNEL_, $(LIBS))
#	if the libraries do not have suffixes and are not _APP_ or _KERNEL_
#	prepend -l to each name: be becomes -lbe
LINK_LIBS += $(foreach lib, $(filter-out %.so %.a _APP_ _KERNEL_, $(LIBS)), $(addprefix -l, $(lib)))

# add to the linker flags
LDFLAGS += $(LINK_PATHS)  $(LINK_LIBS)

#	SETTING: add the defines to the compiler flags
CFLAGS += $(foreach define, $(DEFINES), $(addprefix -D, $(define)))

#	SETTING: add the additional compiler flags
CFLAGS += $(COMPILER_FLAGS)

#	SETTING: add the additional linker flags
LDFLAGS += $(LINKER_FLAGS)

#	SETTING: use the archive tools if building a static library
#	otherwise use the linker
ifeq ($(strip $(TYPE)), STATIC)
	BUILD_LINE = ar -cru "$(TARGET)" $(OBJS)
else
	BUILD_LINE = $(LD) -o "$@" $(OBJS) $(LDFLAGS)
endif

# pseudo-function for converting a list of resource definition files in RDEFS
# variable to a corresponding list of object files in $(OBJ_DIR)/xxx.rsrc
# The "function" strips off the rdef file suffix (.rdef) and then strips
# of the directory name, leaving just the root file name.
# It then appends the .rsrc suffix and prepends the $(OBJ_DIR)/ path
define RDEFS_LIST_TO_RSRCS
	$(addprefix $(OBJ_DIR)/, $(addsuffix .rsrc, $(foreach file, $(RDEFS), \
	$(basename $(notdir $(file))))))
endef

#	create the resource definitions instruction in case RDEFS is not empty
	ifeq ($(RDEFS), )
		RSRCS +=
	else
		RSRCS += $(RDEFS_LIST_TO_RSRCS)
	endif

#	create the resource instruction
	ifeq ($(RSRCS), )
		DO_RSRCS :=
	else
		DO_RSRCS := $(XRES) -o $(TARGET) $(RSRCS)
	endif

#	the directory for internationalization sources (catkeys)
	CATKEYS_DIR	:= locales

#	the directory for internationalization resource data (catalogs)
	CATALOGS_DIR := $(OBJ_DIR)/$(APP_MIME_SIG)

# pseudo-function for converting a list of language codes in CATALOGS variable
# to a corresponding list of catkeys files in $(CATALOGS_DIR)/xx.catalog
# The "function" appends the .catalog suffix and prepends the $(CATALOGS_DIR)/ path
define LOCALES_LIST_TO_CATALOGS
	$(addprefix $(CATALOGS_DIR)/, $(addsuffix .catalog, $(foreach lang, $(LOCALES), $(lang))))
endef

CATALOGS = $(LOCALES_LIST_TO_CATALOGS)

#	define the actual work to be done
default: $(TARGET)

$(TARGET):	$(OBJ_DIR) $(OBJS) $(RSRCS)
		$(BUILD_LINE)
		$(DO_RSRCS)
		$(MIMESET) -f "$@"


#	rule to create the object file directory if needed
$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#	rule to create the localization sources directory if needed
$(CATKEYS_DIR)::
	@[ -d $(CATKEYS_DIR) ] || mkdir $(CATKEYS_DIR) > /dev/null 2>&1

#	rule to create the localization data directory if needed
$(CATALOGS_DIR):: $(OBJ_DIR)
	@[ -d $(CATALOGS_DIR) ] || mkdir $(CATALOGS_DIR) > /dev/null 2>&1

# rules to make the dependency files
$(OBJ_DIR)/%.d : %.c
	[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1; \
	mkdepend $(LOC_INCLUDES) -p .c:$(OBJ_DIR)/%n.o -m -f "$@" $<
$(OBJ_DIR)/%.d : %.cpp
	[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1; \
	mkdepend $(LOC_INCLUDES) -p .cpp:$(OBJ_DIR)/%n.o -m -f "$@" $<
$(OBJ_DIR)/%.d : %.cp
	[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1; \
	mkdepend $(LOC_INCLUDES) -p .cp:$(OBJ_DIR)/%n.o -m -f "$@" $<
$(OBJ_DIR)/%.d : %.cc
	[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1; \
	mkdepend $(LOC_INCLUDES) -p .cc:$(OBJ_DIR)/%n.o -m -f "$@" $<
$(OBJ_DIR)/%.d : %.C
	[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1; \
	mkdepend $(LOC_INCLUDES) -p .C:$(OBJ_DIR)/%n.o -m -f "$@" $<
$(OBJ_DIR)/%.d : %.CC
	[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1; \
	mkdepend $(LOC_INCLUDES) -p .CC:$(OBJ_DIR)/%n.o -m -f "$@" $<
$(OBJ_DIR)/%.d : %.CPP
	[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1; \
	mkdepend $(LOC_INCLUDES) -p .CPP:$(OBJ_DIR)/%n.o -m -f "$@" $<

-include $(DEPENDS)

# rules to make the object files
$(OBJ_DIR)/%.o : %.c
	$(CC) -c $< $(INCLUDES) $(CFLAGS) -o "$@"
$(OBJ_DIR)/%.o : %.cpp
	$(C++) -c $< $(INCLUDES) $(CFLAGS) -o "$@"
$(OBJ_DIR)/%.o : %.cp
	$(CC) -c $< $(INCLUDES) $(CFLAGS) -o "$@"
$(OBJ_DIR)/%.o : %.cc
	$(C++) -c $< $(INCLUDES) $(CFLAGS) -o "$@"
$(OBJ_DIR)/%.o : %.C
	$(CC) -c $< $(INCLUDES) $(CFLAGS) -o "$@"
$(OBJ_DIR)/%.o : %.CC
	$(C++) -c $< $(INCLUDES) $(CFLAGS) -o "$@"
$(OBJ_DIR)/%.o : %.CPP
	$(C++) -c $< $(INCLUDES) $(CFLAGS) -o "$@"

# rules to compile resource definition files
$(OBJ_DIR)/%.rsrc : %.rdef
	cat $< | $(CC) -E $(INCLUDES) $(CFLAGS) - | grep -v '^#' | $(RESCOMP) -I $(dir $<) -o "$@" -
$(OBJ_DIR)/%.rsrc : %.RDEF
	cat $< | $(CC) -E $(INCLUDES) $(CFLAGS) - | grep -v '^#' | $(RESCOMP) -I $(dir $<) -o "$@" -

# rule to compile localization data catalogs
$(CATALOGS_DIR)/%.catalog : $(CATKEYS_DIR)/%.catkeys
	linkcatkeys -o "$@" -s $(APP_MIME_SIG) -l $(notdir $(basename $@)) $<

# rule to preprocess program sources into file ready for collecting catkeys
$(OBJ_DIR)/$(NAME).pre : $(SRCS)
	-cat $(SRCS) | $(CC) -E -x c++ $(INCLUDES) $(CFLAGS) -DB_COLLECTING_CATKEYS - > $(OBJ_DIR)/$(NAME).pre

# rules to collect localization catkeys
catkeys : $(CATKEYS_DIR)/en.catkeys

$(CATKEYS_DIR)/en.catkeys : $(CATKEYS_DIR) $(OBJ_DIR)/$(NAME).pre
	collectcatkeys -s $(APP_MIME_SIG) $(OBJ_DIR)/$(NAME).pre -o $(CATKEYS_DIR)/en.catkeys

# rule to create localization catalogs
catalogs : $(CATALOGS_DIR) $(CATALOGS)

#	rules to handle lex/flex and yacc/bison files

$(OBJ_DIR)/%.o: %.l
	flex $<
	$(CC) -c $(INCLUDES) $(CFLAGS) lex.yy.c -o "$@"
$(OBJ_DIR)/%.o: %.y
	bison -d -y $<
	$(CC) -c $(INCLUDES) $(CFLAGS) y.tab.c -o "$@"

#	empty rule. Things that depend on this rule will always get triggered
FORCE:

#	The generic clean command. Delete everything in the object folder.
clean :: FORCE
	-rm -rf "$(OBJ_DIR)"

#	remove just the application from the object folder
rmapp ::
	-rm -f $(TARGET)

# make it easy to install drivers for testing
USER_BIN_PATH := $(shell finddir B_USER_NONPACKAGED_ADDONS_DIRECTORY)/kernel/drivers/bin
USER_DEV_PATH := $(shell finddir B_USER_NONPACKAGED_ADDONS_DIRECTORY)/kernel/drivers/dev

driverinstall :: default
ifeq ($(strip $(TYPE)), DRIVER)
	copyattr --data $(TARGET) $(USER_BIN_PATH)/$(NAME)
	mkdir -p $(USER_DEV_PATH)/$(DRIVER_PATH)
	ln -sf $(USER_BIN_PATH)/$(NAME) $(USER_DEV_PATH)/$(DRIVER_PATH)/$(NAME)
endif

install :: default
ifeq ($(INSTALL_DIR), )
	@echo "No install directory specified for \"$(NAME)\" (INSTALL_DIR is empty)" >&2
else
	mkdir -p "$(INSTALL_DIR)"
	cp $(TARGET) $(INSTALL_DIR)/$(NAME)
endif

# catalog installation directory
CATALOG_INSTALL_DIR := $(shell finddir B_USER_NONPACKAGED_DATA_DIRECTORY)/locale/catalogs

#	rule to install localization resources catalogs
catalogsinstall :: catalogs
	mkdir -p "$(CATALOG_INSTALL_DIR)/$(APP_MIME_SIG)"
	-cp $(CATALOGS_DIR)/*.catalog "$(CATALOG_INSTALL_DIR)/$(APP_MIME_SIG)"

# alternative way of storing localization catalogs - bind into program executable's resources
bindcatalogs :
	for lc in $(LOCALES); do linkcatkeys -o $(TARGET) -s $(APP_MIME_SIG) -tr -l $$lc $(CATKEYS_DIR)/$$lc.catkeys; done

