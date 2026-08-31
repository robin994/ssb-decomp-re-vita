#include <ft/fighter.h>
#include <ft/ftchar/ftmario/ftmario.h>
#include <ft/ftmain.h>
#include <gm/gmcollision.h>
#include <it/itcommon/itfflower.h>
#include <mp/mpcommon.h>
#include <sys/objanim.h>

#include "bridge/ftmodapi_bridge.h"

int port_mod_fighter_set_status(void *fighter_gobj, int status_id,
                                float frame_begin, float anim_speed,
                                uint32_t preserve_flags)
{
    if (fighter_gobj == NULL) return -1;
    ftMainSetStatus((GObj *)fighter_gobj, status_id, frame_begin, anim_speed, preserve_flags);
    return 0;
}

void port_mod_fighter_play_anim_events(void *fighter_gobj)
{
    if (fighter_gobj != NULL) ftMainPlayAnimEventsAll((GObj *)fighter_gobj);
}

void port_mod_fighter_set_anim_speed(void *fighter_gobj, float anim_speed)
{
    if (fighter_gobj != NULL) gcSetAnimSpeed((GObj *)fighter_gobj, anim_speed);
}

void port_mod_fighter_set_ground(void *fighter_gobj)
{
    if (fighter_gobj != NULL) mpCommonSetFighterGround(ftGetStruct((GObj *)fighter_gobj));
}

void port_mod_fighter_set_air(void *fighter_gobj)
{
    if (fighter_gobj != NULL) mpCommonSetFighterAir(ftGetStruct((GObj *)fighter_gobj));
}

void port_mod_fighter_set_wait_or_fall(void *fighter_gobj)
{
    if (fighter_gobj != NULL) mpCommonSetFighterWaitOrFall((GObj *)fighter_gobj);
}

int port_mod_fighter_check_landing(void *fighter_gobj)
{
    if (fighter_gobj == NULL) return 0;
    return mpCommonCheckFighterLanding((GObj *)fighter_gobj) != FALSE;
}

int port_mod_fighter_joint_world_position(void *fighter_gobj, int joint_id,
                                          float *out_x, float *out_y, float *out_z)
{
    FTStruct *fp;
    Vec3f pos;

    if (fighter_gobj == NULL || out_x == NULL || out_y == NULL || out_z == NULL) return -1;
    fp = ftGetStruct((GObj *)fighter_gobj);
    if (fp == NULL || joint_id < 0 || joint_id >= ARRAY_COUNT(fp->joints) || fp->joints[joint_id] == NULL)
    {
        return -2;
    }
    gmCollisionGetFighterPartsWorldPosition(fp->joints[joint_id], &pos);
    *out_x = pos.x;
    *out_y = pos.y;
    *out_z = pos.z;
    return 0;
}

static sb32 portModFighterSpecialHiProcPass(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!(fp->coll_data.floor_flags & MAP_VERTEX_COLL_PASS) ||
        fp->input.pl.stick_range.y >= FTMARIO_SUPERJUMP_STICK_Y_UNK)
    {
        return TRUE;
    }
    return FALSE;
}

void port_mod_fighter_special_hi_map(void *fighter_gobj, float landing_lag)
{
    GObj *gobj;
    FTStruct *fp;

    if (fighter_gobj == NULL) return;
    gobj = (GObj *)fighter_gobj;
    fp = ftGetStruct(gobj);

    if (fp->ga == nMPKineticsAir)
    {
        if (fp->motion_vars.flags.flag1 == 0 || fp->physics.vel_air.y >= 0.0F)
        {
            mpCommonCheckFighterProject(gobj);
        }
        else if (mpCommonCheckFighterPassCliff(gobj, portModFighterSpecialHiProcPass) != FALSE)
        {
            if (fp->coll_data.mask_stat & MAP_FLAG_CLIFF_MASK)
            {
                ftCommonCliffCatchSetStatus(gobj);
            }
            else ftCommonLandingFallSpecialSetStatus(gobj, FALSE, landing_lag);
        }
    }
    else mpCommonSetFighterFallOnEdgeBreak(gobj);
}

int port_mod_projectile_spawn_fflower(void *owner_gobj,
                                      float pos_x, float pos_y, float pos_z,
                                      float vel_x, float vel_y, float vel_z)
{
    Vec3f pos;
    Vec3f vel;

    if (owner_gobj == NULL) return -1;
    pos.x = pos_x;
    pos.y = pos_y;
    pos.z = pos_z;
    vel.x = vel_x;
    vel.y = vel_y;
    vel.z = vel_z;
    return itFFlowerWeaponFlameMakeWeapon((GObj *)owner_gobj, &pos, &vel) != NULL ? 0 : -2;
}
