#
# Copyright (C) 2014-2015 OpenWrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mac-telnet
PKG_VERSION:=2016-10-11
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/haakonnessjoen/MAC-Telnet.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=1ea1361874ae1fa9a981db5c5c0f4d7153d18a54
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_HASH:=13429f1a870121cdbbfb687e0581967904798000eca1f8f9c1e9dc87dee5669b

PKG_LICENSE:=GPL-2.0+
PKG_MAINTAINER:=Håkon Nessjøen <http://lunatic.no/>

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

define Build/Configure
	sed -i '/Searching for MikroTik routers/{ s/MikroTik/compatible/ }' $(PKG_BUILD_DIR)/src/mndp.c
	sed -i -e 's/AM_GNU_GETTEXT/#AM_GNU_GETTEXT/g' $(PKG_BUILD_DIR)/configure.ac
	sed -i -e 's/SUBDIRS =/SUBDIRS = src\n#SUBDIRS =/g' $(PKG_BUILD_DIR)/Makefile.am
	sed -i -e 's/#if BYTE_ORDER == LITTLE_ENDIAN/#if 1/g' $(PKG_BUILD_DIR)/src/protocol.c
	(cd $(PKG_BUILD_DIR) && ( ./autogen.sh; \
                $(TARGET_CONFIGURE_OPTS) \
                CFLAGS="$(TARGET_CFLAGS)" \
                ./configure \
--host=$(REAL_GNU_TARGET_NAME) \
                        --prefix=/var \
                        --disable-nls \
        ));
endef


# 1: name
# 2: executable
define BuildPlugin
  define Package/mac-telnet-$(1)
    SECTION:=net
    CATEGORY:=Network
    DEPENDS:=+libubox +librt
    TITLE:=MAC-Telnet $(1)
    URL:=https://github.com/haakonnessjoen/MAC-Telnet
  endef

  define Package/mac-telnet-$(1)/description
    Open source MAC Telnet client and server utilities for connecting to
    Mikrotik RouterOS routers and Linux machines via MAC address.
  endef

  define Package/mac-telnet-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(2) $$(1)/usr/sbin/
  endef

  $$(eval $$(call BuildPackage,mac-telnet-$(1)))
endef

$(eval $(call BuildPlugin,server,mactelnetd))
$(eval $(call BuildPlugin,client,mactelnet))
$(eval $(call BuildPlugin,ping,macping))
$(eval $(call BuildPlugin,discover,mndp))
