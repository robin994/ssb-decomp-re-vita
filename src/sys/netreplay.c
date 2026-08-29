#include <sys/netreplay.h>

#include <ft/fighter.h>
#include <if/ifcommon.h>
#include <sc/scmanager.h>
#include <sys/netinput.h>
#include <sys/netsync.h>
#include <sys/taskman.h>
#include <sys/utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PORT
extern void port_log(const char *fmt, ...);
extern char *getenv(const char *name);
extern int atoi(const char *s);
#endif

#define SYNETREPLAY_DEFAULT_RECORD_FRAMES 1800
#define SYNETREPLAY_DET_MAGIC 0x53534454U /* SSDT */
#define SYNETREPLAY_DET_VERSION 5U
/* Determinism test captures every logical frame so the first divergent frame
 * is exact. Live netplay state hashes remain rate-limited to 60 frames. */
#define SYNETREPLAY_DET_INTERVAL 1U
#define SYNETREPLAY_LIVE_HASH_INTERVAL 60U
#define SYNETREPLAY_DET_PATH_MAX 1024
#define SYNETREPLAY_DET_TEST_FRAMES 10800U

#ifdef __vita__
#define SYNETREPLAY_DET_TEST_PATH "ux0:data/battleship/determinism.ssnr"
#else
#define SYNETREPLAY_DET_TEST_PATH "determinism.ssnr"
#endif

typedef struct SYNetReplayFileHeader
{
	u32 magic;
	u32 version;
	u32 metadata_size;
	u32 frame_size;
	u32 frame_count;
	u32 player_count;
	u32 input_checksum;

} SYNetReplayFileHeader;

const char *sSYNetReplayRecordPath;
const char *sSYNetReplayPlayPath;
u32 sSYNetReplayRecordFrameLimit = SYNETREPLAY_DEFAULT_RECORD_FRAMES;
u32 sSYNetReplayLoadedFrameCount;
u32 sSYNetReplayLoadedInputChecksum;
sb32 sSYNetReplayIsRecording;
sb32 sSYNetReplayIsRecordWritten;
sb32 sSYNetReplayRecordCaptureComplete;
sb32 sSYNetReplayIsPlaybackLoaded;
sb32 sSYNetReplayIsPlaybackActive;
sb32 sSYNetReplayIsPlaybackVerified;
SYNetInputReplayMetadata sSYNetReplayLoadedMetadata;
SYNetInputFrame sSYNetReplayLoadedFrames[MAXCONTROLLERS][SYNETINPUT_REPLAY_MAX_FRAMES];
SYNetSyncDigest *sSYNetReplayRecordedDigests;
SYNetSyncDigest *sSYNetReplayLoadedDigests;
u32 sSYNetReplayRecordedDigestCount;
u32 sSYNetReplayLoadedDigestCount;
u32 sSYNetReplayRecordedDigestCapacity;
u32 sSYNetReplayLoadedDigestCursor;
u32 sSYNetReplayLastDigestFrame = UINT32_MAX;
sb32 sSYNetReplayDeterminismVerified;
sb32 sSYNetReplayDeterminismFailed;
sb32 sSYNetReplayDeterminismTestRecordArmed;
sb32 sSYNetReplayDeterminismTestPlayback;
u32 sSYNetReplayDeterminismPerturbFrame = UINT32_MAX;
char sSYNetReplayDeterminismTestPath[SYNETREPLAY_DET_PATH_MAX];

static void syNetReplayFreeRecordedDigests(void)
{
	if (sSYNetReplayRecordedDigests != NULL)
	{
		free(sSYNetReplayRecordedDigests);
		sSYNetReplayRecordedDigests = NULL;
	}
	sSYNetReplayRecordedDigestCount = 0;
	sSYNetReplayRecordedDigestCapacity = 0;
}

static void syNetReplayFreeLoadedDigests(void)
{
	if (sSYNetReplayLoadedDigests != NULL)
	{
		free(sSYNetReplayLoadedDigests);
		sSYNetReplayLoadedDigests = NULL;
	}
	sSYNetReplayLoadedDigestCount = 0;
	sSYNetReplayLoadedDigestCursor = 0;
}

static void syNetReplayFreeDeterminismBuffers(void)
{
	syNetReplayFreeRecordedDigests();
	syNetReplayFreeLoadedDigests();
	sSYNetReplayLastDigestFrame = UINT32_MAX;
}

static sb32 syNetReplayMakeDeterminismPath(const char *replay_path, char *out_path, u32 out_size)
{
	u32 replay_len;
	static const char suffix[] = ".det";

	if ((replay_path == NULL) || (out_path == NULL) || (out_size == 0)) return FALSE;
	replay_len = (u32)strlen(replay_path);
	if ((replay_len + sizeof(suffix)) > out_size) return FALSE;
	memcpy(out_path, replay_path, replay_len);
	memcpy(out_path + replay_len, suffix, sizeof(suffix));
	return TRUE;
}

static sb32 syNetReplaySetDeterminismTestPath(void)
{
	const char *path = SYNETREPLAY_DET_TEST_PATH;
	u32 length = (u32)strlen(path);
	if ((length + 1U) > sizeof(sSYNetReplayDeterminismTestPath)) return FALSE;
	memcpy(sSYNetReplayDeterminismTestPath, path, length + 1U);
	return TRUE;
}

static sb32 syNetReplayWriteU32BE(FILE *fp, u32 value)
{
	u8 bytes[4];
	bytes[0] = (u8)(value >> 24);
	bytes[1] = (u8)(value >> 16);
	bytes[2] = (u8)(value >> 8);
	bytes[3] = (u8)value;
	return (fwrite(bytes, sizeof(bytes), 1, fp) == 1) ? TRUE : FALSE;
}

static sb32 syNetReplayWriteU64BE(FILE *fp, u64 value)
{
	return (syNetReplayWriteU32BE(fp, (u32)(value >> 32)) != FALSE) &&
	       (syNetReplayWriteU32BE(fp, (u32)value) != FALSE);
}

static sb32 syNetReplayReadU32BE(FILE *fp, u32 *out_value)
{
	u8 bytes[4];
	if ((out_value == NULL) || (fread(bytes, sizeof(bytes), 1, fp) != 1)) return FALSE;
	*out_value = ((u32)bytes[0] << 24) | ((u32)bytes[1] << 16) | ((u32)bytes[2] << 8) | bytes[3];
	return TRUE;
}

static sb32 syNetReplayReadU64BE(FILE *fp, u64 *out_value)
{
	u32 high;
	u32 low;
	if ((out_value == NULL) || (syNetReplayReadU32BE(fp, &high) == FALSE) ||
	    (syNetReplayReadU32BE(fp, &low) == FALSE)) return FALSE;
	*out_value = ((u64)high << 32) | low;
	return TRUE;
}

static sb32 syNetReplayWriteDigest(FILE *fp, const SYNetSyncDigest *digest)
{
	s32 player;
	if ((fp == NULL) || (digest == NULL)) return FALSE;
	if ((syNetReplayWriteU32BE(fp, digest->frame) == FALSE) ||
	    (syNetReplayWriteU32BE(fp, (u32)digest->rng_seed) == FALSE) ||
	    (syNetReplayWriteU32BE(fp, digest->time_remain) == FALSE) ||
	    (syNetReplayWriteU32BE(fp, digest->time_passed) == FALSE) ||
	    (syNetReplayWriteU32BE(fp, digest->item_count) == FALSE) ||
	    (syNetReplayWriteU32BE(fp, digest->weapon_count) == FALSE) ||
	    (syNetReplayWriteU64BE(fp, digest->rng_hash) == FALSE) ||
	    (syNetReplayWriteU64BE(fp, digest->battle_hash) == FALSE)) return FALSE;
	for (player = 0; player < MAXCONTROLLERS; player++)
		if (syNetReplayWriteU64BE(fp, digest->fighter_hash[player]) == FALSE) return FALSE;
	if ((syNetReplayWriteU64BE(fp, digest->stage_hash) == FALSE) ||
	    (syNetReplayWriteU64BE(fp, digest->map_hash) == FALSE) ||
	    (syNetReplayWriteU64BE(fp, digest->item_hash) == FALSE) ||
	    (syNetReplayWriteU64BE(fp, digest->weapon_hash) == FALSE) ||
	    (syNetReplayWriteU64BE(fp, digest->total_hash) == FALSE)) return FALSE;
	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		if ((syNetReplayWriteU32BE(fp, (u32)digest->fighter_status[player]) == FALSE) ||
		    (syNetReplayWriteU32BE(fp, (u32)digest->fighter_motion[player]) == FALSE) ||
		    (syNetReplayWriteU32BE(fp, (u32)digest->fighter_damage[player]) == FALSE) ||
		    (syNetReplayWriteU32BE(fp, (u32)digest->fighter_stock[player]) == FALSE) ||
		    (syNetReplayWriteU32BE(fp, digest->fighter_pos_x_bits[player]) == FALSE) ||
		    (syNetReplayWriteU32BE(fp, digest->fighter_pos_y_bits[player]) == FALSE) ||
		    (syNetReplayWriteU32BE(fp, digest->fighter_vel_x_bits[player]) == FALSE) ||
		    (syNetReplayWriteU32BE(fp, digest->fighter_vel_y_bits[player]) == FALSE)) return FALSE;
	}
	return TRUE;
}

static sb32 syNetReplayReadDigest(FILE *fp, SYNetSyncDigest *digest)
{
	u32 rng_seed;
	s32 player;
	if ((fp == NULL) || (digest == NULL)) return FALSE;
	memset(digest, 0, sizeof(*digest));
	if ((syNetReplayReadU32BE(fp, &digest->frame) == FALSE) ||
	    (syNetReplayReadU32BE(fp, &rng_seed) == FALSE) ||
	    (syNetReplayReadU32BE(fp, &digest->time_remain) == FALSE) ||
	    (syNetReplayReadU32BE(fp, &digest->time_passed) == FALSE) ||
	    (syNetReplayReadU32BE(fp, &digest->item_count) == FALSE) ||
	    (syNetReplayReadU32BE(fp, &digest->weapon_count) == FALSE) ||
	    (syNetReplayReadU64BE(fp, &digest->rng_hash) == FALSE) ||
	    (syNetReplayReadU64BE(fp, &digest->battle_hash) == FALSE)) return FALSE;
	digest->rng_seed = (s32)rng_seed;
	for (player = 0; player < MAXCONTROLLERS; player++)
		if (syNetReplayReadU64BE(fp, &digest->fighter_hash[player]) == FALSE) return FALSE;
	if ((syNetReplayReadU64BE(fp, &digest->stage_hash) == FALSE) ||
	    (syNetReplayReadU64BE(fp, &digest->map_hash) == FALSE) ||
	    (syNetReplayReadU64BE(fp, &digest->item_hash) == FALSE) ||
	    (syNetReplayReadU64BE(fp, &digest->weapon_hash) == FALSE) ||
	    (syNetReplayReadU64BE(fp, &digest->total_hash) == FALSE)) return FALSE;
	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		if (syNetReplayReadU32BE(fp, &rng_seed) == FALSE) return FALSE;
		digest->fighter_status[player] = (s32)rng_seed;
		if (syNetReplayReadU32BE(fp, &rng_seed) == FALSE) return FALSE;
		digest->fighter_motion[player] = (s32)rng_seed;
		if (syNetReplayReadU32BE(fp, &rng_seed) == FALSE) return FALSE;
		digest->fighter_damage[player] = (s32)rng_seed;
		if (syNetReplayReadU32BE(fp, &rng_seed) == FALSE) return FALSE;
		digest->fighter_stock[player] = (s32)rng_seed;
		if ((syNetReplayReadU32BE(fp, &digest->fighter_pos_x_bits[player]) == FALSE) ||
		    (syNetReplayReadU32BE(fp, &digest->fighter_pos_y_bits[player]) == FALSE) ||
		    (syNetReplayReadU32BE(fp, &digest->fighter_vel_x_bits[player]) == FALSE) ||
		    (syNetReplayReadU32BE(fp, &digest->fighter_vel_y_bits[player]) == FALSE)) return FALSE;
	}
	return TRUE;
}

static sb32 syNetReplayWriteDeterminismFile(const char *replay_path)
{
	char path[SYNETREPLAY_DET_PATH_MAX];
	FILE *fp;
	u32 frame;

	if ((sSYNetReplayRecordedDigests == NULL) ||
	    (syNetReplayMakeDeterminismPath(replay_path, path, sizeof(path)) == FALSE)) return FALSE;
	fp = fopen(path, "wb");
	if (fp == NULL) return FALSE;
	if ((syNetReplayWriteU32BE(fp, SYNETREPLAY_DET_MAGIC) == FALSE) ||
	    (syNetReplayWriteU32BE(fp, SYNETREPLAY_DET_VERSION) == FALSE) ||
	    (syNetReplayWriteU32BE(fp, sSYNetReplayRecordedDigestCount) == FALSE))
	{
		fclose(fp);
		return FALSE;
	}
	for (frame = 0; frame < sSYNetReplayRecordedDigestCount; frame++)
	{
		if (syNetReplayWriteDigest(fp, &sSYNetReplayRecordedDigests[frame]) == FALSE)
		{
			fclose(fp);
			return FALSE;
		}
	}
	fclose(fp);
#ifdef PORT
	port_log("[NETPLAY][DET] wrote trace=%s samples=%u capture_interval=%u live_hash_interval=%u\n",
	         path, sSYNetReplayRecordedDigestCount, SYNETREPLAY_DET_INTERVAL,
	         SYNETREPLAY_LIVE_HASH_INTERVAL);
#endif
	return TRUE;
}

static sb32 syNetReplayLoadDeterminismFile(const char *replay_path)
{
	char path[SYNETREPLAY_DET_PATH_MAX];
	FILE *fp;
	u32 magic;
	u32 version;
	u32 count;
	u32 frame;

	if (syNetReplayMakeDeterminismPath(replay_path, path, sizeof(path)) == FALSE) return FALSE;
	fp = fopen(path, "rb");
	if (fp == NULL)
	{
#ifdef PORT
		port_log("[NETPLAY][DET] no determinism trace for replay=%s\n", replay_path);
#endif
		return FALSE;
	}
	if ((syNetReplayReadU32BE(fp, &magic) == FALSE) ||
	    (syNetReplayReadU32BE(fp, &version) == FALSE) ||
	    (syNetReplayReadU32BE(fp, &count) == FALSE) ||
	    (magic != SYNETREPLAY_DET_MAGIC) || (version != SYNETREPLAY_DET_VERSION) ||
	    (count == 0) || (count > SYNETINPUT_REPLAY_MAX_FRAMES))
	{
		fclose(fp);
		return FALSE;
	}
	syNetReplayFreeLoadedDigests();
	sSYNetReplayLoadedDigests = malloc((size_t)count * sizeof(SYNetSyncDigest));
	if (sSYNetReplayLoadedDigests == NULL)
	{
		fclose(fp);
		return FALSE;
	}
	memset(sSYNetReplayLoadedDigests, 0, (size_t)count * sizeof(SYNetSyncDigest));
	for (frame = 0; frame < count; frame++)
	{
		if (syNetReplayReadDigest(fp, &sSYNetReplayLoadedDigests[frame]) == FALSE)
		{
			free(sSYNetReplayLoadedDigests);
			sSYNetReplayLoadedDigests = NULL;
			fclose(fp);
			return FALSE;
		}
		if (((sSYNetReplayLoadedDigests[frame].frame % SYNETREPLAY_DET_INTERVAL) != 0U) ||
		    ((frame != 0U) &&
		     (sSYNetReplayLoadedDigests[frame].frame <= sSYNetReplayLoadedDigests[frame - 1U].frame)))
		{
			free(sSYNetReplayLoadedDigests);
			sSYNetReplayLoadedDigests = NULL;
			fclose(fp);
			return FALSE;
		}
	}
	fclose(fp);
	sSYNetReplayLoadedDigestCount = count;
	sSYNetReplayLoadedDigestCursor = 0;
#ifdef PORT
	port_log("[NETPLAY][DET] loaded trace=%s samples=%u capture_interval=%u\n",
	         path, count, SYNETREPLAY_DET_INTERVAL);
#endif
	return TRUE;
}

static void syNetReplayCaptureOrCompareDeterminism(void)
{
	u32 tick = syNetInputGetTick();
	u32 frame;
	SYNetSyncDigest digest;

	if (tick == 0) return;
	frame = tick - 1;
	if (frame == sSYNetReplayLastDigestFrame) return;
	sSYNetReplayLastDigestFrame = frame;
	if ((frame % SYNETREPLAY_DET_INTERVAL) != 0) return;
	syNetSyncComputeDeterministicDigest(frame, &digest);
#ifdef PORT
	if ((sSYNetReplayIsPlaybackActive != FALSE) && (frame == sSYNetReplayDeterminismPerturbFrame))
	{
		/* Test-only fault injection verifies that the comparator reports the
		 * exact first bad logical frame without mutating normal gameplay. */
		digest.total_hash ^= 1ULL;
	}
#endif

	if ((sSYNetReplayIsRecording != FALSE) && (sSYNetReplayRecordedDigests != NULL) &&
	    (frame < sSYNetReplayRecordFrameLimit) &&
	    (sSYNetReplayRecordedDigestCount < sSYNetReplayRecordedDigestCapacity))
	{
		sSYNetReplayRecordedDigests[sSYNetReplayRecordedDigestCount++] = digest;
	}
	if ((sSYNetReplayIsPlaybackActive != FALSE) && (sSYNetReplayLoadedDigests != NULL) &&
	    (sSYNetReplayLoadedDigestCursor < sSYNetReplayLoadedDigestCount) &&
	    (sSYNetReplayDeterminismFailed == FALSE))
	{
		SYNetSyncDigest *expected = &sSYNetReplayLoadedDigests[sSYNetReplayLoadedDigestCursor];
		if (expected->frame < frame)
		{
			sSYNetReplayDeterminismFailed = TRUE;
#ifdef PORT
			port_log("[NETPLAY][DET] missing replay digest sample expected_frame=%u current_frame=%u\n",
			         expected->frame, frame);
#endif
		}
		else if (expected->frame == frame)
		{
			if (syNetSyncDigestEqual(expected, &digest) == FALSE)
			{
				sSYNetReplayDeterminismFailed = TRUE;
				syNetSyncLogDigestMismatch(expected, &digest);
			}
			sSYNetReplayLoadedDigestCursor++;
		}
	}
}

void syNetReplayClearLoadedFrames(void)
{
	s32 player;
	s32 tick;

	sSYNetReplayLoadedFrameCount = 0;
	sSYNetReplayLoadedInputChecksum = 0;

	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		for (tick = 0; tick < SYNETINPUT_REPLAY_MAX_FRAMES; tick++)
		{
			memset(&sSYNetReplayLoadedFrames[player][tick], 0, sizeof(SYNetInputFrame));
		}
	}
}

void syNetReplayCaptureBattleMetadata(SCBattleState *battle_state, SYNetInputReplayMetadata *metadata)
{
	s32 player;

	memset(metadata, 0, sizeof(SYNetInputReplayMetadata));

	metadata->magic = SYNETINPUT_REPLAY_MAGIC;
	metadata->version = SYNETINPUT_REPLAY_VERSION;
	metadata->scene_kind = nSCKindVSBattle;
	metadata->rng_seed = syUtilsRandSeed();

	if (battle_state == NULL)
	{
		return;
	}
	metadata->player_count = battle_state->pl_count + battle_state->cp_count;
	metadata->stage_kind = battle_state->gkind;
	metadata->stocks = battle_state->stocks;
	metadata->time_limit = battle_state->time_limit;
	metadata->item_switch = battle_state->item_appearance_rate;
	metadata->item_toggles = battle_state->item_toggles;
	metadata->game_type = battle_state->game_type;
	metadata->game_rules = battle_state->game_rules;
	metadata->is_team_battle = battle_state->is_team_battle;
	metadata->handicap = battle_state->handicap;
	metadata->is_team_attack = battle_state->is_team_attack;
	metadata->is_stage_select = battle_state->is_stage_select;
	metadata->damage_ratio = battle_state->damage_ratio;
	metadata->item_appearance_rate = battle_state->item_appearance_rate;
	metadata->is_not_teamshadows = battle_state->is_not_teamshadows;

	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		metadata->player_kinds[player] = battle_state->players[player].pkind;
		metadata->fighter_kinds[player] = battle_state->players[player].fkind;
		metadata->costumes[player] = battle_state->players[player].costume;
		metadata->teams[player] = battle_state->players[player].team;
		metadata->handicaps[player] = battle_state->players[player].handicap;
		metadata->levels[player] = battle_state->players[player].level;
		metadata->shades[player] = battle_state->players[player].shade;
	}
}

void syNetReplayApplyBattleMetadata(const SYNetInputReplayMetadata *metadata)
{
	SCBattleState *battle_state = &gSCManagerTransferBattleState;
	s32 player;

	*battle_state = dSCManagerDefaultBattleState;
	battle_state->game_type = metadata->game_type;
	battle_state->gkind = metadata->stage_kind;
	battle_state->is_team_battle = metadata->is_team_battle;
	battle_state->game_rules = metadata->game_rules;
	battle_state->time_limit = metadata->time_limit;
	battle_state->stocks = metadata->stocks;
	battle_state->handicap = metadata->handicap;
	battle_state->is_team_attack = metadata->is_team_attack;
	battle_state->is_stage_select = FALSE;
	battle_state->damage_ratio = metadata->damage_ratio;
	battle_state->item_toggles = metadata->item_toggles;
	battle_state->item_appearance_rate = metadata->item_appearance_rate;
	battle_state->is_not_teamshadows = metadata->is_not_teamshadows;
	battle_state->pl_count = 0;
	battle_state->cp_count = 0;

	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		battle_state->players[player].player = (metadata->is_team_battle != FALSE) ? metadata->teams[player] : player;
		battle_state->players[player].team = metadata->teams[player];
		battle_state->players[player].pkind = metadata->player_kinds[player];
		battle_state->players[player].fkind = metadata->fighter_kinds[player];
		battle_state->players[player].costume = metadata->costumes[player];
		battle_state->players[player].shade = metadata->shades[player];
		battle_state->players[player].handicap = metadata->handicaps[player];
		battle_state->players[player].level = metadata->levels[player];
		battle_state->players[player].tag = (metadata->player_kinds[player] == nFTPlayerKindMan) ? player : GMCOMMON_PLAYERS_MAX;
		battle_state->players[player].is_single_stockicon = (metadata->game_rules & SCBATTLE_GAMERULE_TIME) ? TRUE : FALSE;

		if (metadata->player_kinds[player] == nFTPlayerKindMan)
		{
			battle_state->players[player].color =
			(metadata->is_team_battle == FALSE) ? player : dIFCommonPlayerTeamColorIDs[metadata->teams[player]];
			battle_state->pl_count++;
		}
		else if (metadata->player_kinds[player] == nFTPlayerKindCom)
		{
			battle_state->players[player].color =
			(metadata->is_team_battle == FALSE) ? GMCOMMON_PLAYERS_MAX : dIFCommonPlayerTeamColorIDs[metadata->teams[player]];
			battle_state->cp_count++;
		}
	}
	gSCManagerSceneData.gkind = metadata->stage_kind;
}

void syNetReplayInitDebugEnv(void)
{
#ifdef PORT
	const char *frame_limit_env;
	const char *perturb_env;

	syNetReplayFreeDeterminismBuffers();
	sSYNetReplayRecordPath = getenv("SSB64_REPLAY_RECORD");
	sSYNetReplayPlayPath = getenv("SSB64_REPLAY_PLAY");
	frame_limit_env = getenv("SSB64_REPLAY_RECORD_FRAMES");
	perturb_env = getenv("SSB64_DETERMINISM_PERTURB_FRAME");
	sSYNetReplayDeterminismPerturbFrame = UINT32_MAX;
	if (perturb_env != NULL)
	{
		s32 perturb_frame = atoi(perturb_env);
		if (perturb_frame >= 0) sSYNetReplayDeterminismPerturbFrame = (u32)perturb_frame;
	}

	if (frame_limit_env != NULL)
	{
		s32 frame_limit = atoi(frame_limit_env);

		if ((frame_limit > 0) && (frame_limit < SYNETINPUT_REPLAY_MAX_FRAMES))
		{
			sSYNetReplayRecordFrameLimit = frame_limit;
		}
	}
	if (sSYNetReplayPlayPath != NULL)
	{
		if (syNetReplayLoadDebugFile(sSYNetReplayPlayPath) != FALSE)
		{
			syNetReplayLoadDeterminismFile(sSYNetReplayPlayPath);
			syNetReplayApplyBattleMetadata(&sSYNetReplayLoadedMetadata);
			syUtilsSetRandomSeed(sSYNetReplayLoadedMetadata.rng_seed);
			gSCManagerSceneData.scene_prev = nSCKindVSMode;
			gSCManagerSceneData.scene_curr = nSCKindVSBattle;
		}
	}
#endif
}

void syNetReplayStartVSSession(SCBattleState *battle_state)
{
	SYNetInputReplayMetadata metadata;
	u32 tick;
	s32 player;

	if (sSYNetReplayIsPlaybackLoaded != FALSE)
	{
		sSYNetReplayLastDigestFrame = UINT32_MAX;
		sSYNetReplayLoadedDigestCursor = 0;
		sSYNetReplayDeterminismVerified = FALSE;
		sSYNetReplayDeterminismFailed = FALSE;
		syNetInputClearReplayFrames();
		syNetInputSetReplayMetadata(&sSYNetReplayLoadedMetadata);
		syUtilsSetRandomSeed(sSYNetReplayLoadedMetadata.rng_seed);

		for (tick = 0; tick < sSYNetReplayLoadedFrameCount; tick++)
		{
			for (player = 0; player < MAXCONTROLLERS; player++)
			{
				syNetInputSetReplayFrame(player, tick, &sSYNetReplayLoadedFrames[player][tick]);
			}
		}
		for (player = 0; player < MAXCONTROLLERS; player++)
		{
			syNetInputSetSlotSource(player, nSYNetInputSourceSaved);
		}
		sSYNetReplayIsPlaybackActive = TRUE;
		sSYNetReplayIsPlaybackVerified = FALSE;

#ifdef PORT
		port_log("SSB64 Replay: playback start path=%s frames=%u checksum=0x%08X stage=%u seed=%u\n",
		         sSYNetReplayPlayPath, sSYNetReplayLoadedFrameCount, sSYNetReplayLoadedInputChecksum,
		         sSYNetReplayLoadedMetadata.stage_kind, sSYNetReplayLoadedMetadata.rng_seed);
#endif
		return;
	}
	if (sSYNetReplayRecordPath != NULL)
	{
		u32 digest_capacity = (sSYNetReplayRecordFrameLimit + SYNETREPLAY_DET_INTERVAL - 1U) /
		                      SYNETREPLAY_DET_INTERVAL;
		syNetReplayFreeRecordedDigests();
		sSYNetReplayRecordedDigests = malloc((size_t)digest_capacity * sizeof(SYNetSyncDigest));
		if (sSYNetReplayRecordedDigests != NULL)
		{
			memset(sSYNetReplayRecordedDigests, 0,
			       (size_t)digest_capacity * sizeof(SYNetSyncDigest));
			sSYNetReplayRecordedDigestCapacity = digest_capacity;
		}
		sSYNetReplayRecordedDigestCount = 0;
		sSYNetReplayLastDigestFrame = UINT32_MAX;
		sSYNetReplayDeterminismVerified = FALSE;
		sSYNetReplayDeterminismFailed = FALSE;
		syNetReplayCaptureBattleMetadata(battle_state, &metadata);
		syNetInputClearReplayFrames();
		syNetInputSetReplayMetadata(&metadata);
		syNetInputSetRecordingEnabled(TRUE);
		sSYNetReplayIsRecording = TRUE;
		sSYNetReplayIsRecordWritten = FALSE;
		sSYNetReplayRecordCaptureComplete = FALSE;
		sSYNetReplayDeterminismTestRecordArmed = FALSE;

#ifdef PORT
		port_log("SSB64 Replay: recording start path=%s limit=%u stage=%u seed=%u players=%u\n",
		         sSYNetReplayRecordPath, sSYNetReplayRecordFrameLimit, metadata.stage_kind,
		         metadata.rng_seed, metadata.player_count);
#endif
	}
}

void syNetReplayUpdate(void)
{
	if ((sSYNetReplayIsRecording != FALSE) || (sSYNetReplayIsPlaybackActive != FALSE))
	{
		syNetReplayCaptureOrCompareDeterminism();
	}
	if ((sSYNetReplayIsRecording != FALSE) && (sSYNetReplayIsRecordWritten == FALSE) &&
		(syNetInputGetRecordedFrameCount() >= sSYNetReplayRecordFrameLimit))
	{
		/* Stop capturing at the requested limit, but defer file I/O until the
		 * scene exits so a determinism trace can never stall a live frame. */
		syNetInputSetRecordingEnabled(FALSE);
		sSYNetReplayIsRecording = FALSE;
		sSYNetReplayRecordCaptureComplete = TRUE;
#ifdef PORT
		port_log("[NETPLAY][DET] capture complete frames=%u; write deferred until VS exit\n",
		         syNetInputGetRecordedFrameCount());
#endif
	}
	if ((sSYNetReplayIsPlaybackActive != FALSE) && (sSYNetReplayIsPlaybackVerified == FALSE) &&
		(syNetInputGetTick() >= sSYNetReplayLoadedFrameCount))
	{
		u32 checksum = syNetInputGetHistoryInputChecksum(sSYNetReplayLoadedFrameCount);
		sb32 input_ok = (checksum == sSYNetReplayLoadedInputChecksum) ? TRUE : FALSE;
		sb32 state_ok = FALSE;

		if ((sSYNetReplayLoadedDigests != NULL) &&
		    (sSYNetReplayLoadedDigestCursor == sSYNetReplayLoadedDigestCount) &&
		    (sSYNetReplayDeterminismFailed == FALSE))
		{
			state_ok = TRUE;
		}
		if (input_ok == FALSE) sSYNetReplayDeterminismFailed = TRUE;
		sSYNetReplayDeterminismVerified =
			((input_ok != FALSE) && (state_ok != FALSE) && (sSYNetReplayDeterminismFailed == FALSE)) ? TRUE : FALSE;

#ifdef PORT
		port_log("SSB64 Replay: playback verify frames=%u expected=0x%08X actual=0x%08X result=%s\n",
		         sSYNetReplayLoadedFrameCount, sSYNetReplayLoadedInputChecksum, checksum,
		         (input_ok != FALSE) ? "PASS" : "FAIL");
		if (sSYNetReplayLoadedDigests != NULL)
		{
				port_log("[NETPLAY][DET] replay state verification samples=%u capture_interval=%u result=%s\n",
				         sSYNetReplayLoadedDigestCount, SYNETREPLAY_DET_INTERVAL,
				         (state_ok != FALSE) ? "PASS" : "FAIL");
		}
#endif
		sSYNetReplayIsPlaybackVerified = TRUE;
		sSYNetReplayIsPlaybackActive = FALSE;
		syNetReplayFreeLoadedDigests();

		if (sSYNetReplayDeterminismTestPlayback != FALSE)
		{
			sSYNetReplayDeterminismTestPlayback = FALSE;
			sSYNetReplayIsPlaybackLoaded = FALSE;
			sSYNetReplayPlayPath = NULL;
			gSCManagerSceneData.scene_prev = nSCKindVSBattle;
			gSCManagerSceneData.scene_curr = nSCKindModeSelect;
#ifdef PORT
			port_log("[NETPLAY][DET] Vita determinism test complete result=%s; returning to mode select\n",
			         (sSYNetReplayDeterminismVerified != FALSE) ? "PASS" : "FAIL");
#endif
			syTaskmanSetLoadScene();
		}
	}
}

void syNetReplayFinishVSSession(void)
{
	if (((sSYNetReplayIsRecording != FALSE) || (sSYNetReplayRecordCaptureComplete != FALSE)) &&
	    (sSYNetReplayIsRecordWritten == FALSE))
	{
		if (sSYNetReplayIsRecording != FALSE)
		{
			syNetInputSetRecordingEnabled(FALSE);
			sSYNetReplayIsRecording = FALSE;
		}
		syNetReplayWriteDebugFile(sSYNetReplayRecordPath);
		syNetReplayWriteDeterminismFile(sSYNetReplayRecordPath);
		if (sSYNetReplayRecordPath == sSYNetReplayDeterminismTestPath)
		{
			sSYNetReplayRecordPath = NULL;
		}
		sSYNetReplayRecordCaptureComplete = FALSE;
		sSYNetReplayIsRecordWritten = TRUE;
		syNetReplayFreeRecordedDigests();
	}
}

sb32 syNetReplayGetDeterminismVerified(void)
{
	return sSYNetReplayDeterminismVerified;
}

sb32 syNetReplayGetDeterminismFailed(void)
{
	return sSYNetReplayDeterminismFailed;
}

sb32 syNetReplayDeterminismTestTraceAvailable(void)
{
	FILE *fp;
	char det_path[SYNETREPLAY_DET_PATH_MAX];
	if ((sSYNetReplayDeterminismTestPath[0] == '\0') &&
	    (syNetReplaySetDeterminismTestPath() == FALSE)) return FALSE;
	fp = fopen(sSYNetReplayDeterminismTestPath, "rb");
	if (fp == NULL) return FALSE;
	fclose(fp);
	if (syNetReplayMakeDeterminismPath(sSYNetReplayDeterminismTestPath, det_path, sizeof(det_path)) == FALSE)
	{
		return FALSE;
	}
	fp = fopen(det_path, "rb");
	if (fp == NULL) return FALSE;
	fclose(fp);
	return TRUE;
}

sb32 syNetReplayDeterminismTestRecordArmed(void)
{
	return sSYNetReplayDeterminismTestRecordArmed;
}

sb32 syNetReplayArmDeterminismTestRecord(void)
{
	if (syNetReplaySetDeterminismTestPath() == FALSE) return FALSE;
	sSYNetReplayIsPlaybackActive = FALSE;
	sSYNetReplayIsPlaybackLoaded = FALSE;
	syNetReplayFreeLoadedDigests();
	sSYNetReplayRecordPath = sSYNetReplayDeterminismTestPath;
	sSYNetReplayPlayPath = NULL;
	sSYNetReplayRecordFrameLimit = SYNETREPLAY_DET_TEST_FRAMES;
	sSYNetReplayDeterminismTestRecordArmed = TRUE;
	sSYNetReplayDeterminismVerified = FALSE;
	sSYNetReplayDeterminismFailed = FALSE;
#ifdef PORT
	port_log("[NETPLAY][DET] armed next VS recording path=%s frames=%u fixed_timestep_hz=60 exact_frame_hash=1\n",
	         sSYNetReplayRecordPath, sSYNetReplayRecordFrameLimit);
#endif
	return TRUE;
}

sb32 syNetReplayLaunchDeterminismTestPlayback(void)
{
	if (syNetReplaySetDeterminismTestPath() == FALSE) return FALSE;
	if (syNetReplayLoadDebugFile(sSYNetReplayDeterminismTestPath) == FALSE) return FALSE;
	if (syNetReplayLoadDeterminismFile(sSYNetReplayDeterminismTestPath) == FALSE)
	{
		sSYNetReplayIsPlaybackLoaded = FALSE;
		return FALSE;
	}

	sSYNetReplayRecordPath = NULL;
	sSYNetReplayPlayPath = sSYNetReplayDeterminismTestPath;
	sSYNetReplayDeterminismTestRecordArmed = FALSE;
	sSYNetReplayDeterminismTestPlayback = TRUE;
	sSYNetReplayDeterminismVerified = FALSE;
	sSYNetReplayDeterminismFailed = FALSE;
	syNetReplayApplyBattleMetadata(&sSYNetReplayLoadedMetadata);
	syUtilsSetRandomSeed(sSYNetReplayLoadedMetadata.rng_seed);
	gSCManagerSceneData.scene_prev = nSCKindModeSelect;
	gSCManagerSceneData.scene_curr = nSCKindVSBattle;
#ifdef PORT
	port_log("[NETPLAY][DET] launching Vita determinism playback path=%s frames=%u seed=%u\n",
	         sSYNetReplayPlayPath, sSYNetReplayLoadedFrameCount, sSYNetReplayLoadedMetadata.rng_seed);
#endif
	syTaskmanSetLoadScene();
	return TRUE;
}

sb32 syNetReplayWriteDebugFile(const char *path)
{
	SYNetReplayFileHeader header;
	SYNetInputReplayMetadata metadata;
	SYNetInputFrame frame;
	FILE *fp;
	u32 tick;
	s32 player;

	if ((path == NULL) || (syNetInputGetReplayMetadata(&metadata) == FALSE))
	{
		return FALSE;
	}
	fp = fopen(path, "wb");

	if (fp == NULL)
	{
#ifdef PORT
		port_log("SSB64 Replay: failed to open record path=%s\n", path);
#endif
		return FALSE;
	}
	header.magic = SYNETINPUT_REPLAY_MAGIC;
	header.version = SYNETINPUT_REPLAY_VERSION;
	header.metadata_size = sizeof(SYNetInputReplayMetadata);
	header.frame_size = sizeof(SYNetInputFrame);
	header.frame_count = syNetInputGetRecordedFrameCount();
	header.player_count = MAXCONTROLLERS;
	header.input_checksum = syNetInputGetReplayInputChecksum();

	fwrite(&header, sizeof(header), 1, fp);
	fwrite(&metadata, sizeof(metadata), 1, fp);

	for (tick = 0; tick < header.frame_count; tick++)
	{
		for (player = 0; player < MAXCONTROLLERS; player++)
		{
			if (syNetInputGetReplayFrame(player, tick, &frame) == FALSE)
			{
				memset(&frame, 0, sizeof(frame));
				frame.tick = tick;
			}
			fwrite(&frame, sizeof(frame), 1, fp);
		}
	}
	fclose(fp);

#ifdef PORT
	port_log("SSB64 Replay: wrote path=%s frames=%u checksum=0x%08X\n",
	         path, header.frame_count, header.input_checksum);
#endif
	return TRUE;
}

sb32 syNetReplayLoadDebugFile(const char *path)
{
	SYNetReplayFileHeader header;
	FILE *fp;
	u32 tick;
	s32 player;

	if (path == NULL)
	{
		return FALSE;
	}
	fp = fopen(path, "rb");

	if (fp == NULL)
	{
#ifdef PORT
		port_log("SSB64 Replay: failed to open playback path=%s\n", path);
#endif
		return FALSE;
	}
	if (fread(&header, sizeof(header), 1, fp) != 1)
	{
		fclose(fp);
		return FALSE;
	}
	if ((header.magic != SYNETINPUT_REPLAY_MAGIC) ||
		(header.version != SYNETINPUT_REPLAY_VERSION) ||
		(header.metadata_size != sizeof(SYNetInputReplayMetadata)) ||
		(header.frame_size != sizeof(SYNetInputFrame)) ||
		(header.frame_count > SYNETINPUT_REPLAY_MAX_FRAMES) ||
		(header.player_count != MAXCONTROLLERS))
	{
		fclose(fp);
		return FALSE;
	}
	if (fread(&sSYNetReplayLoadedMetadata, sizeof(sSYNetReplayLoadedMetadata), 1, fp) != 1)
	{
		fclose(fp);
		return FALSE;
	}
	syNetReplayClearLoadedFrames();
	sSYNetReplayLoadedFrameCount = header.frame_count;
	sSYNetReplayLoadedInputChecksum = header.input_checksum;

	for (tick = 0; tick < header.frame_count; tick++)
	{
		for (player = 0; player < MAXCONTROLLERS; player++)
		{
			if (fread(&sSYNetReplayLoadedFrames[player][tick], sizeof(SYNetInputFrame), 1, fp) != 1)
			{
				fclose(fp);
				return FALSE;
			}
		}
	}
	fclose(fp);
	sSYNetReplayIsPlaybackLoaded = TRUE;

#ifdef PORT
	port_log("SSB64 Replay: loaded path=%s frames=%u checksum=0x%08X stage=%u seed=%u\n",
	         path, sSYNetReplayLoadedFrameCount, sSYNetReplayLoadedInputChecksum,
	         sSYNetReplayLoadedMetadata.stage_kind, sSYNetReplayLoadedMetadata.rng_seed);
#endif
	return TRUE;
}
