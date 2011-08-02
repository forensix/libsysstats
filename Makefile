include theos/makefiles/common.mk

LOCAL_INSTALL_PATH = /usr/lib
LIBRARY_NAME = libsysstats
libsysstats_FILES = sysstats.c

include $(THEOS_MAKE_PATH)/library.mk
