#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=miniupnpd
PKG_VERSION:=1.9.20151118
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://miniupnp.free.fr/files
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MD5SUM:=127050c71695e707853a0ccba0b7952b

include $(INCLUDE_DIR)/package.mk

SHORT_UBNT_FIRMWARE_VERSION=$(shell echo $(CONFIG_UBNT_FIRMWARE_VERSION) | sed "s/^.*\.v/v/g")

define Package/miniupnpd
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+iptables +libip4tc +IPV6:libip6tc +libnfnetlink +libcrypto +libssl
  TITLE:=Lightweight UPnP IGD, NAT-PMP & PCP daemon
  SUBMENU:=Firewall
  URL:=http://miniupnp.free.fr/
  PKG_MAINTAINER:=Markus Stenberg <fingon@iki.fi>
endef

define Package/miniupnpd/conffiles
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	echo "Ubiquiti/$(SHORT_UBNT_FIRMWARE_VERSION)" | tr \(\)\  _ >$(PKG_BUILD_DIR)/os.openwrt
endef

MAKE_FLAGS += \
	TEST=0 \
	LIBS="" \
	CC="$(TARGET_CC) -DIPTABLES_143 -DOS_VERSION=\\\"$(SHORT_UBNT_FIRMWARE_VERSION)\\\" -lnfnetlink -lip4tc $(if $(CONFIG_IPV6),-lip6tc)" \
	CONFIG_OPTIONS="$(if $(CONFIG_IPV6_disabled_by_UBNT_),--ipv6) --leasefile --vendorcfg" \
	-f Makefile.linux \
	miniupnpd

define Package/miniupnpd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/miniupnpd $(1)/usr/bin/miniupnpd
endef

$(eval $(call BuildPackage,miniupnpd))
