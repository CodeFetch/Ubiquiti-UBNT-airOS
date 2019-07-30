#ifndef _AG7240_PHY_H
#define _AG7240_PHY_H

extern u32 athrs16_phy_get_xmii_config(int unit, u32 speed);
extern u32 athrs8021_phy_get_xmii_config(int unit, u32 speed);
extern u32 athrs8035_phy_get_xmii_config(int unit, u32 speed);
extern u32 athrs26_phy_get_xmii_config(int unit, u32 speed);

static inline void ag7240_phy_setup(int unit)
{
#ifdef CONFIG_AR7242_S16_PHY
    if (is_ar7242() && (unit==0)) {
        athrs16_phy_setup(unit);
    } else
#endif
#ifdef CONFIG_AR7242_8035_PHY
    if (is_ar7242() && (unit==0) && athr8035_phy_probe(unit)) {
        athr8035_phy_setup(unit);
    } else
#endif
#ifdef CONFIG_AR7242_8021_PHY
    if (is_ar7242() && (unit==0) && athr8021_phy_probe(unit)) {
        athr8021_phy_setup(unit);
    } else
#endif
    {
        athrs26_phy_setup(unit);
    }
}

static inline void ag7240_phy_link(int unit, int *link)
{
#ifdef CONFIG_AR7242_S16_PHY
    if (is_ar7242() && (unit==0)) {
         *link = athrs16_phy_is_up(unit);
    } else
#endif
#ifdef CONFIG_AR7242_8035_PHY
    if (is_ar7242() && (unit==0) && athr8035_phy_probe(unit)) {
        *link = athr8035_phy_is_up(unit);
    } else
#endif
#ifdef CONFIG_AR7242_8021_PHY
    if (is_ar7242() && (unit==0) && athr8021_phy_probe(unit)) {
        *link = athr8021_phy_is_up(unit);
    } else
#endif
    {
         *link = athrs26_phy_is_up(unit);
    }
}

static inline void ag7240_phy_duplex(int unit, int *duplex)
{
#ifdef CONFIG_AR7242_S16_PHY
    if (is_ar7242() && (unit==0)) {
        *duplex = athrs16_phy_is_fdx(unit);
    } else
#endif
#ifdef CONFIG_AR7242_8035_PHY
    if (is_ar7242() && (unit==0) && athr8035_phy_probe(unit)) {
        *duplex = athr8035_phy_is_fdx(unit);
    } else
#endif
#ifdef CONFIG_AR7242_8021_PHY
    if (is_ar7242() && (unit==0) && athr8021_phy_probe(unit)) {
        *duplex = athr8021_phy_is_fdx(unit);
    } else
#endif
    {
        *duplex = athrs26_phy_is_fdx(unit);
    }
}

static inline void ag7240_phy_speed(int unit, int *speed)
{
#ifdef CONFIG_AR7242_S16_PHY
    if (is_ar7242() && (unit==0)) {
        *speed = athrs16_phy_speed(unit);
    } else
#endif
#ifdef CONFIG_AR7242_8035_PHY
    if (is_ar7242() && (unit==0) && athr8035_phy_probe(unit)) {
        *speed = athr8035_phy_speed(unit);
    } else
#endif
#ifdef CONFIG_AR7242_8021_PHY
    if (is_ar7242() && (unit==0) && athr8021_phy_probe(unit)) {
        *speed = athr8021_phy_speed(unit);
    } else
#endif
    {
        *speed = athrs26_phy_speed(unit);
    }
}

static inline void ag7240_phy_get_xmii_config(int unit, u32 speed, u32 *xmii_config)
{
#ifdef CONFIG_AR7242_S16_PHY
    if (is_ar7242() && (unit==0)) {
        *xmii_config = athrs16_phy_get_xmii_config(unit, speed);
    } else
#endif
#ifdef CONFIG_AR7242_8035_PHY
    if (is_ar7242() && (unit==0) && athr8035_phy_probe(unit)) {
        *xmii_config = athr8035_phy_get_xmii_config(unit, speed);
    } else
#endif
#ifdef CONFIG_AR7242_8021_PHY
    if (is_ar7242() && (unit==0) && athr8021_phy_probe(unit)) {
        *xmii_config = athr8021_phy_get_xmii_config(unit, speed);
    } else
#endif
    {
        *xmii_config = athrs26_phy_get_xmii_config(unit, speed);
    }
}

#endif /*_AG7240_PHY_H*/
