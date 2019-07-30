# 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=php
PKG_VERSION:=2.0.1
PKG_RELEASE:=11

PKG_SOURCE_MIRROR:=""
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://lt.corp.ubnt.com/stash/scm/ap/php2.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)
PKG_SOURCE_VERSION:=$(PKG_VERSION)-$(PKG_RELEASE)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE).tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/php2
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=PHP2 Web Scripting Language
  URL:=http://museum.php.net/php2/
endef

define Package/php2/description
	PHP is a widely-used general-purpose scripting language that is especially suited for Web development and can be embedded into HTML.
endef

define Build/Configure
        (cd $(PKG_BUILD_DIR); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		LDFLAGS=-lm \
		CFLAGS=-O3 \
		CC="$(TARGET_CC)" \
                ./install \
                        --prefix=/usr \
			--sysconfdir=/etc \
			--datadir=/usr/share \
			--host=$(GNU_TARGET_NAME) \
			--build=$(GNU_HOST_NAME) \
        )
endef

define Build/Compile
	-$(MAKE) -C $(PKG_BUILD_DIR)/src clean
	$(MAKE) -C $(PKG_BUILD_DIR)/src
endef


define Package/php2/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/php.cgi $(1)/sbin/cgi
endef	

$(eval $(call BuildPackage,php2))
