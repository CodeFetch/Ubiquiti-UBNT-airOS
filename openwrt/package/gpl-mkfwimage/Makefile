# 
# Copyright (C) 2007 Ubiquiti Networks, Inc
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mkfwimage
PKG_BUILD_DIR:=$(TOOL_BUILD_DIR)/mkfwimage

include $(INCLUDE_DIR)/host-build.mk

define Build/Compile
	$(CC) -O src/mkfwimage.c -o $(PKG_BUILD_DIR)/mkfwimage -lz
	$(CC) -O src/fwsplit.c -o $(PKG_BUILD_DIR)/fwsplit -lz
endef

define Build/Install
	mkdir -p $(STAGING_DIR)/bin
	$(CP) $(PKG_BUILD_DIR)/mkfwimage $(STAGING_DIR)/bin/
	mkdir -p $(STAGING_DIR)/usr/include/$(PKG_NAME)
	$(CP) src/*.h $(STAGING_DIR)/usr/include/$(PKG_NAME)/
endef

define Build/Clean
	rm -f $(STAGING_DIR)/bin/mkfwimage
endef

$(eval $(call HostBuild))
