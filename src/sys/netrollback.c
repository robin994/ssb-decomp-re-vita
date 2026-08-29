#include <sys/netrollback.h>

#include <ft/fighter.h>
#include <gm/gmcamera.h>
#include <gr/ground.h>
#include <if/ifcommon.h>
#include <it/item.h>
#include <mp/map.h>
#include <netplay/netplay_bridge.h>
#include <sc/scene.h>
#include <sys/netinput.h>
#include <sys/netsync.h>
#include <sys/objdef.h>
#include <sys/objman.h>
#include <sys/utils.h>
#include <wp/weapon.h>

#include <string.h>

#ifdef PORT
extern void port_log(const char *fmt, ...);
#endif

#define SYNETROLLBACK_RING (SYNETROLLBACK_WINDOW + 3)
#define SYNETROLLBACK_DOBJ_MAX 384
#define SYNETROLLBACK_AOBJ_MAX 12
#define SYNETROLLBACK_ITEM_MAPOBJ_MAX 32
#define SYNETROLLBACK_ITEM_WEIGHT_MAX 64
#define SYNETROLLBACK_HASH_INTERVAL 60U

typedef struct SYNetRollbackAObjState
{
    AObj *ptr;
    u8 track;
    u8 kind;
    f32 length_invert;
    f32 length;
    f32 value_base;
    f32 value_target;
    f32 rate_base;
    f32 rate_target;
} SYNetRollbackAObjState;

typedef struct SYNetRollbackDObjState
{
    DObj *ptr;
    GCTranslate translate;
    GCRotate rotate;
    GCScale scale;
    u8 flags;
    f32 anim_wait;
    f32 anim_speed;
    f32 anim_frame;
    GCUserData user_data;
    u8 aobj_count;
    SYNetRollbackAObjState aobjs[SYNETROLLBACK_AOBJ_MAX];
} SYNetRollbackDObjState;

typedef struct SYNetRollbackFighterState
{
    GObj *gobj;
    FTStruct state;
    f32 gobj_anim_frame;
} SYNetRollbackFighterState;

typedef struct SYNetRollbackItemState
{
    GObj *gobj;
    ITStruct state;
    f32 gobj_anim_frame;
} SYNetRollbackItemState;

typedef struct SYNetRollbackWeaponState
{
    GObj *gobj;
    WPStruct state;
    f32 gobj_anim_frame;
} SYNetRollbackWeaponState;

typedef struct SYNetRollbackCameraState
{
    sb32 valid;
    CObj *ptr;
    CObjVec vec;
    f32 anim_wait;
    f32 anim_speed;
    f32 anim_frame;
} SYNetRollbackCameraState;

typedef struct SYNetRollbackItemManagerState
{
    u8 appear_mapobjs_num;
    u32 appear_spawn_wait;
    u8 appear_valids_num;
    u16 appear_weights_sum;
    u8 random_valids_num;
    u16 random_weights_sum;
    u8 appear_mapobjs[SYNETROLLBACK_ITEM_MAPOBJ_MAX];
    u8 appear_kinds[SYNETROLLBACK_ITEM_WEIGHT_MAX];
    u16 appear_blocks[SYNETROLLBACK_ITEM_WEIGHT_MAX];
    u8 random_kinds[SYNETROLLBACK_ITEM_WEIGHT_MAX];
    u16 random_blocks[SYNETROLLBACK_ITEM_WEIGHT_MAX];
} SYNetRollbackItemManagerState;

typedef struct SYNetRollbackSnapshot
{
    sb32 valid;
    sb32 restorable;
    u32 frame;
    s32 rng_seed;
    SCBattleState battle;
    GRStruct ground;
    GMCamera camera;
    MPAllBounds map_bounds;
    u16 map_update_tic;
    SYNetRollbackCameraState camera_cobj;
    SYNetRollbackItemManagerState item_manager;
    u16 fighter_count;
    u16 item_count;
    u16 weapon_count;
    u16 dobj_count;
    u16 yakumono_count;
    Vec3f yakumono_speeds[SYNETROLLBACK_DOBJ_MAX];
    SYNetRollbackFighterState fighters[GMCOMMON_PLAYERS_MAX];
    SYNetRollbackItemState items[ITEM_ALLOC_MAX];
    SYNetRollbackWeaponState weapons[WEAPON_ALLOC_MAX];
    SYNetRollbackDObjState dobjs[SYNETROLLBACK_DOBJ_MAX];
    sb32 hash_valid;
    u64 post_hash;
    SYNetSyncDigest post_digest;
} SYNetRollbackSnapshot;

static SYNetRollbackSnapshot sSYNetRollbackSnapshots[SYNETROLLBACK_RING];
static sb32 sSYNetRollbackResimulating;
static u32 sSYNetRollbackCount;
static u32 sSYNetRollbackMaxFrames;
static u32 sSYNetRollbackLateInputCount;
static u32 sSYNetRollbackSnapshotBytes;
static u32 sSYNetRollbackSnapshotEncodeUs;
static u32 sSYNetRollbackSnapshotRestoreUs;
static u32 sSYNetRollbackResimUs;
static u32 sSYNetRollbackLastHashSubmitted = UINT32_MAX;

static u32 syNetRollbackElapsedUs(u64 begin)
{
    u64 now = port_netplay_monotonic_us();
    u64 elapsed = (now >= begin) ? (now - begin) : 0;
    return (elapsed > UINT32_MAX) ? UINT32_MAX : (u32)elapsed;
}

static void syNetRollbackSaveAObjs(SYNetRollbackSnapshot *snapshot, DObj *dobj, SYNetRollbackDObjState *state)
{
    AObj *aobj;
    u32 count = 0;

    for (aobj = dobj->aobj; (aobj != NULL) && (count < SYNETROLLBACK_AOBJ_MAX); aobj = aobj->next)
    {
        SYNetRollbackAObjState *dst = &state->aobjs[count++];
        dst->ptr = aobj;
        dst->track = aobj->track;
        dst->kind = aobj->kind;
        dst->length_invert = aobj->length_invert;
        dst->length = aobj->length;
        dst->value_base = aobj->value_base;
        dst->value_target = aobj->value_target;
        dst->rate_base = aobj->rate_base;
        dst->rate_target = aobj->rate_target;
    }
    state->aobj_count = (u8)count;
    if (aobj != NULL) snapshot->restorable = FALSE;
}

static void syNetRollbackSaveDObjTree(SYNetRollbackSnapshot *snapshot, DObj *dobj)
{
    for (; dobj != NULL; dobj = dobj->sib_next)
    {
        SYNetRollbackDObjState *state;
        u32 i;

        for (i = 0; i < snapshot->dobj_count; i++)
        {
            if (snapshot->dobjs[i].ptr == dobj) break;
        }
        if (i == snapshot->dobj_count)
        {
            if (snapshot->dobj_count >= SYNETROLLBACK_DOBJ_MAX)
            {
                snapshot->restorable = FALSE;
                return;
            }
            state = &snapshot->dobjs[snapshot->dobj_count++];
            memset(state, 0, sizeof(*state));
            state->ptr = dobj;
            state->translate = dobj->translate;
            state->rotate = dobj->rotate;
            state->scale = dobj->scale;
            state->flags = dobj->flags;
            state->anim_wait = dobj->anim_wait;
            state->anim_speed = dobj->anim_speed;
            state->anim_frame = dobj->anim_frame;
            state->user_data = dobj->user_data;
            syNetRollbackSaveAObjs(snapshot, dobj, state);
        }
        if (dobj->child != NULL) syNetRollbackSaveDObjTree(snapshot, dobj->child);
    }
}

static void syNetRollbackSaveGObjDObjs(SYNetRollbackSnapshot *snapshot, GObj *gobj)
{
    if ((gobj != NULL) && (gobj->obj_kind == nGCCommonAppendDObj) && (gobj->obj != NULL))
    {
        syNetRollbackSaveDObjTree(snapshot, DObjGetStruct(gobj));
    }
}

static void syNetRollbackSaveItemManager(SYNetRollbackSnapshot *snapshot)
{
    SYNetRollbackItemManagerState *state = &snapshot->item_manager;
    u32 count;

    state->appear_mapobjs_num = gITManagerAppearActor.mapobjs_num;
    state->appear_spawn_wait = gITManagerAppearActor.spawn_wait;
    state->appear_valids_num = gITManagerAppearActor.weights.valids_num;
    state->appear_weights_sum = gITManagerAppearActor.weights.weights_sum;
    state->random_valids_num = gITManagerRandomWeights.valids_num;
    state->random_weights_sum = gITManagerRandomWeights.weights_sum;

    if ((state->appear_mapobjs_num > SYNETROLLBACK_ITEM_MAPOBJ_MAX) ||
        (state->appear_valids_num > SYNETROLLBACK_ITEM_WEIGHT_MAX) ||
        (state->random_valids_num > SYNETROLLBACK_ITEM_WEIGHT_MAX))
    {
        snapshot->restorable = FALSE;
        return;
    }
    count = state->appear_mapobjs_num;
    if ((count != 0) && (gITManagerAppearActor.mapobjs != NULL))
        memcpy(state->appear_mapobjs, gITManagerAppearActor.mapobjs, count);
    count = state->appear_valids_num;
    if ((count != 0) && (gITManagerAppearActor.weights.kinds != NULL) &&
        (gITManagerAppearActor.weights.blocks != NULL))
    {
        memcpy(state->appear_kinds, gITManagerAppearActor.weights.kinds, count);
        memcpy(state->appear_blocks, gITManagerAppearActor.weights.blocks, count * sizeof(u16));
    }
    count = state->random_valids_num;
    if ((count != 0) && (gITManagerRandomWeights.kinds != NULL) && (gITManagerRandomWeights.blocks != NULL))
    {
        memcpy(state->random_kinds, gITManagerRandomWeights.kinds, count);
        memcpy(state->random_blocks, gITManagerRandomWeights.blocks, count * sizeof(u16));
    }
}

static void syNetRollbackSaveObjects(SYNetRollbackSnapshot *snapshot)
{
    GObj *gobj;

    for (gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; gobj != NULL; gobj = gobj->link_next)
    {
        FTStruct *fp;
        SYNetRollbackFighterState *dst;
        if (snapshot->fighter_count >= GMCOMMON_PLAYERS_MAX)
        {
            snapshot->restorable = FALSE;
            break;
        }
        fp = ftGetStruct(gobj);
        if (fp == NULL) continue;
        dst = &snapshot->fighters[snapshot->fighter_count++];
        dst->gobj = gobj;
        dst->state = *fp;
        dst->gobj_anim_frame = gobj->anim_frame;
        syNetRollbackSaveGObjDObjs(snapshot, gobj);
    }
    for (gobj = gGCCommonLinks[nGCCommonLinkIDItem]; gobj != NULL; gobj = gobj->link_next)
    {
        ITStruct *ip;
        SYNetRollbackItemState *dst;
        if (snapshot->item_count >= ITEM_ALLOC_MAX)
        {
            snapshot->restorable = FALSE;
            break;
        }
        ip = itGetStruct(gobj);
        if (ip == NULL) continue;
        dst = &snapshot->items[snapshot->item_count++];
        dst->gobj = gobj;
        dst->state = *ip;
        dst->gobj_anim_frame = gobj->anim_frame;
        syNetRollbackSaveGObjDObjs(snapshot, gobj);
    }
    for (gobj = gGCCommonLinks[nGCCommonLinkIDWeapon]; gobj != NULL; gobj = gobj->link_next)
    {
        WPStruct *wp;
        SYNetRollbackWeaponState *dst;
        if (snapshot->weapon_count >= WEAPON_ALLOC_MAX)
        {
            snapshot->restorable = FALSE;
            break;
        }
        wp = wpGetStruct(gobj);
        if (wp == NULL) continue;
        dst = &snapshot->weapons[snapshot->weapon_count++];
        dst->gobj = gobj;
        dst->state = *wp;
        dst->gobj_anim_frame = gobj->anim_frame;
        syNetRollbackSaveGObjDObjs(snapshot, gobj);
    }
}

static void syNetRollbackSaveMap(SYNetRollbackSnapshot *snapshot)
{
    s32 i;
    snapshot->map_bounds = gMPCollisionBounds;
    snapshot->map_update_tic = gMPCollisionUpdateTic;
    if ((gMPCollisionYakumonosNum < 0) || (gMPCollisionYakumonosNum > SYNETROLLBACK_DOBJ_MAX))
    {
        snapshot->restorable = FALSE;
        return;
    }
    snapshot->yakumono_count = (u16)gMPCollisionYakumonosNum;
    for (i = 0; i < gMPCollisionYakumonosNum; i++)
    {
        if (gMPCollisionSpeeds != NULL) snapshot->yakumono_speeds[i] = gMPCollisionSpeeds[i];
        if ((gMPCollisionYakumonoDObjs != NULL) && (gMPCollisionYakumonoDObjs->dobjs[i] != NULL))
            syNetRollbackSaveDObjTree(snapshot, gMPCollisionYakumonoDObjs->dobjs[i]);
    }
}

void syNetRollbackReset(void)
{
    memset(sSYNetRollbackSnapshots, 0, sizeof(sSYNetRollbackSnapshots));
    sSYNetRollbackResimulating = FALSE;
    sSYNetRollbackCount = 0;
    sSYNetRollbackMaxFrames = 0;
    sSYNetRollbackLateInputCount = 0;
    sSYNetRollbackSnapshotBytes = sizeof(SYNetRollbackSnapshot);
    sSYNetRollbackSnapshotEncodeUs = 0;
    sSYNetRollbackSnapshotRestoreUs = 0;
    sSYNetRollbackResimUs = 0;
    sSYNetRollbackLastHashSubmitted = UINT32_MAX;
}

sb32 syNetRollbackIsResimulating(void)
{
    return sSYNetRollbackResimulating;
}

void syNetRollbackCapturePreFrame(u32 frame)
{
    SYNetRollbackSnapshot *snapshot = &sSYNetRollbackSnapshots[frame % SYNETROLLBACK_RING];
    u64 begin = port_netplay_monotonic_us();
    CObj *camera_cobj;

    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->frame = frame;
    snapshot->valid = TRUE;
    snapshot->restorable = TRUE;
    snapshot->rng_seed = syUtilsRandSeed();
    if (gSCManagerBattleState != NULL) snapshot->battle = *gSCManagerBattleState;
    snapshot->ground = gGRCommonStruct;
    snapshot->camera = gGMCameraStruct;

    camera_cobj = (gGMCameraGObj != NULL) ? CObjGetStruct(gGMCameraGObj) : NULL;
    if (camera_cobj != NULL)
    {
        snapshot->camera_cobj.valid = TRUE;
        snapshot->camera_cobj.ptr = camera_cobj;
        snapshot->camera_cobj.vec = camera_cobj->vec;
        snapshot->camera_cobj.anim_wait = camera_cobj->anim_wait;
        snapshot->camera_cobj.anim_speed = camera_cobj->anim_speed;
        snapshot->camera_cobj.anim_frame = camera_cobj->anim_frame;
    }
    syNetRollbackSaveItemManager(snapshot);
    syNetRollbackSaveMap(snapshot);
    syNetRollbackSaveObjects(snapshot);
    sSYNetRollbackSnapshotEncodeUs = syNetRollbackElapsedUs(begin);
}

static sb32 syNetRollbackTopologyMatches(const SYNetRollbackSnapshot *snapshot)
{
    GObj *gobj;
    u32 i = 0;

    for (gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; gobj != NULL; gobj = gobj->link_next)
    {
        if ((i >= snapshot->fighter_count) || (snapshot->fighters[i].gobj != gobj)) return FALSE;
        i++;
    }
    if (i != snapshot->fighter_count) return FALSE;
    i = 0;
    for (gobj = gGCCommonLinks[nGCCommonLinkIDItem]; gobj != NULL; gobj = gobj->link_next)
    {
        if ((i >= snapshot->item_count) || (snapshot->items[i].gobj != gobj)) return FALSE;
        i++;
    }
    if (i != snapshot->item_count) return FALSE;
    i = 0;
    for (gobj = gGCCommonLinks[nGCCommonLinkIDWeapon]; gobj != NULL; gobj = gobj->link_next)
    {
        if ((i >= snapshot->weapon_count) || (snapshot->weapons[i].gobj != gobj)) return FALSE;
        i++;
    }
    return (i == snapshot->weapon_count) ? TRUE : FALSE;
}

static void syNetRollbackRestoreDObjs(const SYNetRollbackSnapshot *snapshot)
{
    u32 i;
    for (i = 0; i < snapshot->dobj_count; i++)
    {
        const SYNetRollbackDObjState *state = &snapshot->dobjs[i];
        DObj *dobj = state->ptr;
        u32 a;
        if (dobj == NULL) continue;
        dobj->translate = state->translate;
        dobj->rotate = state->rotate;
        dobj->scale = state->scale;
        dobj->flags = state->flags;
        dobj->anim_wait = state->anim_wait;
        dobj->anim_speed = state->anim_speed;
        dobj->anim_frame = state->anim_frame;
        dobj->user_data = state->user_data;
        for (a = 0; a < state->aobj_count; a++)
        {
            const SYNetRollbackAObjState *src = &state->aobjs[a];
            AObj *dst = src->ptr;
            if (dst == NULL) continue;
            dst->track = src->track;
            dst->kind = src->kind;
            dst->length_invert = src->length_invert;
            dst->length = src->length;
            dst->value_base = src->value_base;
            dst->value_target = src->value_target;
            dst->rate_base = src->rate_base;
            dst->rate_target = src->rate_target;
        }
    }
}

static void syNetRollbackRestoreItemManager(const SYNetRollbackSnapshot *snapshot)
{
    const SYNetRollbackItemManagerState *state = &snapshot->item_manager;
    gITManagerAppearActor.mapobjs_num = state->appear_mapobjs_num;
    gITManagerAppearActor.spawn_wait = state->appear_spawn_wait;
    gITManagerAppearActor.weights.valids_num = state->appear_valids_num;
    gITManagerAppearActor.weights.weights_sum = state->appear_weights_sum;
    gITManagerRandomWeights.valids_num = state->random_valids_num;
    gITManagerRandomWeights.weights_sum = state->random_weights_sum;
    if ((state->appear_mapobjs_num != 0) && (gITManagerAppearActor.mapobjs != NULL))
        memcpy(gITManagerAppearActor.mapobjs, state->appear_mapobjs, state->appear_mapobjs_num);
    if ((state->appear_valids_num != 0) && (gITManagerAppearActor.weights.kinds != NULL) &&
        (gITManagerAppearActor.weights.blocks != NULL))
    {
        memcpy(gITManagerAppearActor.weights.kinds, state->appear_kinds, state->appear_valids_num);
        memcpy(gITManagerAppearActor.weights.blocks, state->appear_blocks,
               state->appear_valids_num * sizeof(u16));
    }
    if ((state->random_valids_num != 0) && (gITManagerRandomWeights.kinds != NULL) &&
        (gITManagerRandomWeights.blocks != NULL))
    {
        memcpy(gITManagerRandomWeights.kinds, state->random_kinds, state->random_valids_num);
        memcpy(gITManagerRandomWeights.blocks, state->random_blocks,
               state->random_valids_num * sizeof(u16));
    }
}

static sb32 syNetRollbackRestoreSnapshot(u32 frame)
{
    SYNetRollbackSnapshot *snapshot = &sSYNetRollbackSnapshots[frame % SYNETROLLBACK_RING];
    u64 begin = port_netplay_monotonic_us();
    u32 i;

    if ((snapshot->valid == FALSE) || (snapshot->frame != frame) ||
        (snapshot->restorable == FALSE) || (syNetRollbackTopologyMatches(snapshot) == FALSE))
    {
        return FALSE;
    }
    syUtilsSetRandomSeed(snapshot->rng_seed);
    if (gSCManagerBattleState != NULL) *gSCManagerBattleState = snapshot->battle;
    gGRCommonStruct = snapshot->ground;
    gGMCameraStruct = snapshot->camera;
    gMPCollisionBounds = snapshot->map_bounds;
    gMPCollisionUpdateTic = snapshot->map_update_tic;
    for (i = 0; (i < snapshot->yakumono_count) && (gMPCollisionSpeeds != NULL); i++)
        gMPCollisionSpeeds[i] = snapshot->yakumono_speeds[i];
    syNetRollbackRestoreItemManager(snapshot);

    for (i = 0; i < snapshot->fighter_count; i++)
    {
        FTStruct *fp = ftGetStruct(snapshot->fighters[i].gobj);
        if (fp != NULL) *fp = snapshot->fighters[i].state;
        snapshot->fighters[i].gobj->anim_frame = snapshot->fighters[i].gobj_anim_frame;
    }
    for (i = 0; i < snapshot->item_count; i++)
    {
        ITStruct *ip = itGetStruct(snapshot->items[i].gobj);
        if (ip != NULL) *ip = snapshot->items[i].state;
        snapshot->items[i].gobj->anim_frame = snapshot->items[i].gobj_anim_frame;
    }
    for (i = 0; i < snapshot->weapon_count; i++)
    {
        WPStruct *wp = wpGetStruct(snapshot->weapons[i].gobj);
        if (wp != NULL) *wp = snapshot->weapons[i].state;
        snapshot->weapons[i].gobj->anim_frame = snapshot->weapons[i].gobj_anim_frame;
    }
    syNetRollbackRestoreDObjs(snapshot);
    if ((snapshot->camera_cobj.valid != FALSE) && (snapshot->camera_cobj.ptr != NULL))
    {
        CObj *camera = snapshot->camera_cobj.ptr;
        camera->vec = snapshot->camera_cobj.vec;
        camera->anim_wait = snapshot->camera_cobj.anim_wait;
        camera->anim_speed = snapshot->camera_cobj.anim_speed;
        camera->anim_frame = snapshot->camera_cobj.anim_frame;
    }
    sSYNetRollbackSnapshotRestoreUs = syNetRollbackElapsedUs(begin);
    return TRUE;
}

s32 syNetRollbackHandlePredictionMismatch(u32 current_frame)
{
    u32 mismatch_frame;
	u32 rollback_frames;
	u32 frame;
	u64 begin;
	sb32 terminal_state;
	s32 decision;

	if (syNetInputConsumePredictionMismatch(&mismatch_frame) == FALSE) return FALSE;
	terminal_state = ((gSCManagerBattleState != NULL) &&
	                  ((gSCManagerBattleState->game_status == nSCBattleGameStatusEnd) ||
	                   (gSCManagerBattleState->game_status == nSCBattleGameStatusBossDefeat) ||
	                   (gSCManagerBattleState->game_status == nSCBattleGameStatusSet))) ? TRUE : FALSE;
	decision = syNetRollbackClassifyMismatch(mismatch_frame, current_frame, terminal_state, TRUE);
	if ((decision == nSYNetRollbackDecisionNone) && (terminal_state == FALSE)) return FALSE;

	/* A normal VS KO can destroy the defeated fighter GObj before the
     * GAME SET sequence has finished. Once battle state is terminal there
     * is nothing useful to resimulate: trying to restore the pre-KO object
     * topology turns a legitimate match end into a rollback desync. */
	if (terminal_state != FALSE)
    {
#ifdef PORT
        port_log("[NETPLAY] rollback skipped terminal battle frame=%u current=%u status=%u\n",
                 mismatch_frame, current_frame, gSCManagerBattleState->game_status);
#endif
        return FALSE;
    }
	rollback_frames = current_frame - mismatch_frame + 1U;
	if (decision == nSYNetRollbackDecisionAbort)
    {
        sSYNetRollbackLateInputCount++;
#ifdef PORT
        port_log("[NETPLAY] late input outside rollback window frame=%u current=%u window=%u; controlled stall/desync risk\n",
                 mismatch_frame, current_frame, SYNETROLLBACK_WINDOW);
#endif
        port_netplay_gameplay_abort_desync(mismatch_frame, current_frame, 1);
        return -1;
    }
	if ((syNetRollbackRestoreSnapshot(mismatch_frame) == FALSE) &&
	    (syNetRollbackClassifyMismatch(mismatch_frame, current_frame, FALSE, FALSE) ==
	     nSYNetRollbackDecisionAbort))
    {
        sSYNetRollbackLateInputCount++;
#ifdef PORT
        port_log("[NETPLAY] rollback snapshot unavailable frame=%u current=%u topology_changed=1\n",
                 mismatch_frame, current_frame);
#endif
        port_netplay_gameplay_abort_desync(mismatch_frame, current_frame, 2);
        return -1;
    }

    begin = port_netplay_monotonic_us();
    sSYNetRollbackResimulating = TRUE;
    syNetInputPrepareResimulation(mismatch_frame);
    for (frame = mismatch_frame; frame <= current_frame; frame++)
    {
        syNetRollbackCapturePreFrame(frame);
        syNetInputPublishResimulationFrame(frame);
        ifCommonBattleUpdateInterfaceAll();
        if ((frame % SYNETROLLBACK_HASH_INTERVAL) == 0U)
        {
            SYNetRollbackSnapshot *snapshot = &sSYNetRollbackSnapshots[frame % SYNETROLLBACK_RING];
            syNetSyncComputeDeterministicDigest(frame, &snapshot->post_digest);
            snapshot->post_hash = snapshot->post_digest.total_hash;
            snapshot->hash_valid = TRUE;
        }
    }
    sSYNetRollbackResimulating = FALSE;
    sSYNetRollbackResimUs = syNetRollbackElapsedUs(begin);
    sSYNetRollbackCount++;
    if (rollback_frames > sSYNetRollbackMaxFrames) sSYNetRollbackMaxFrames = rollback_frames;
#ifdef PORT
    port_log("[NETPLAY] rollback frame=%u current=%u frames=%u restore_us=%u resim_us=%u snapshot_bytes=%u\n",
             mismatch_frame, current_frame, rollback_frames, sSYNetRollbackSnapshotRestoreUs,
             sSYNetRollbackResimUs, sSYNetRollbackSnapshotBytes);
#endif
    return TRUE;
}

void syNetRollbackPostFrame(u32 frame)
{
    SYNetRollbackSnapshot *snapshot = &sSYNetRollbackSnapshots[frame % SYNETROLLBACK_RING];
    if ((frame % SYNETROLLBACK_HASH_INTERVAL) == 0U && snapshot->valid != FALSE && snapshot->frame == frame)
    {
        syNetSyncComputeDeterministicDigest(frame, &snapshot->post_digest);
        snapshot->post_hash = snapshot->post_digest.total_hash;
        snapshot->hash_valid = TRUE;
    }

    if (frame >= SYNETROLLBACK_WINDOW)
    {
        u32 confirmed_frame = frame - SYNETROLLBACK_WINDOW;
        SYNetRollbackSnapshot *confirmed = &sSYNetRollbackSnapshots[confirmed_frame % SYNETROLLBACK_RING];
        if ((confirmed_frame % SYNETROLLBACK_HASH_INTERVAL) == 0U &&
            confirmed->valid != FALSE && confirmed->frame == confirmed_frame && confirmed->hash_valid != FALSE &&
            sSYNetRollbackLastHashSubmitted != confirmed_frame)
        {
#ifdef PORT
            port_log("[NETPLAY][DET] HASH frame=%u total=%08X%08X rng=%08X%08X battle=%08X%08X "
                     "p1=%08X%08X p2=%08X%08X p3=%08X%08X p4=%08X%08X "
                     "stage=%08X%08X map=%08X%08X items=%08X%08X weapons=%08X%08X counts=%u/%u\n",
                     confirmed_frame,
                     (u32)(confirmed->post_digest.total_hash >> 32), (u32)confirmed->post_digest.total_hash,
                     (u32)(confirmed->post_digest.rng_hash >> 32), (u32)confirmed->post_digest.rng_hash,
                     (u32)(confirmed->post_digest.battle_hash >> 32), (u32)confirmed->post_digest.battle_hash,
                     (u32)(confirmed->post_digest.fighter_hash[0] >> 32), (u32)confirmed->post_digest.fighter_hash[0],
                     (u32)(confirmed->post_digest.fighter_hash[1] >> 32), (u32)confirmed->post_digest.fighter_hash[1],
                     (u32)(confirmed->post_digest.fighter_hash[2] >> 32), (u32)confirmed->post_digest.fighter_hash[2],
                     (u32)(confirmed->post_digest.fighter_hash[3] >> 32), (u32)confirmed->post_digest.fighter_hash[3],
                     (u32)(confirmed->post_digest.stage_hash >> 32), (u32)confirmed->post_digest.stage_hash,
                     (u32)(confirmed->post_digest.map_hash >> 32), (u32)confirmed->post_digest.map_hash,
                     (u32)(confirmed->post_digest.item_hash >> 32), (u32)confirmed->post_digest.item_hash,
                     (u32)(confirmed->post_digest.weapon_hash >> 32), (u32)confirmed->post_digest.weapon_hash,
                     confirmed->post_digest.item_count, confirmed->post_digest.weapon_count);
#endif
            port_netplay_submit_state_hash(confirmed_frame, (u32)(confirmed->post_hash >> 32),
                                           (u32)confirmed->post_hash);
            sSYNetRollbackLastHashSubmitted = confirmed_frame;
        }
    }
}

u32 syNetRollbackGetCount(void) { return sSYNetRollbackCount; }
u32 syNetRollbackGetMaxFrames(void) { return sSYNetRollbackMaxFrames; }
u32 syNetRollbackGetLateInputCount(void) { return sSYNetRollbackLateInputCount; }
u32 syNetRollbackGetSnapshotBytes(void) { return sSYNetRollbackSnapshotBytes; }
u32 syNetRollbackGetSnapshotEncodeUs(void) { return sSYNetRollbackSnapshotEncodeUs; }
u32 syNetRollbackGetSnapshotRestoreUs(void) { return sSYNetRollbackSnapshotRestoreUs; }
u32 syNetRollbackGetResimUs(void) { return sSYNetRollbackResimUs; }
