#include <sys/netinput.h>

#include <sys/netpeer.h>
#include <sys/taskman.h>
#ifdef PORT
#include <netplay/netplay_bridge.h>
extern void port_log(const char *fmt, ...);
#endif

#include <limits.h>

typedef struct SYNetInputSlot
{
	SYNetInputSource source;
	SYNetInputFrame last_confirmed;
	SYNetInputFrame last_published;

} SYNetInputSlot;

SYNetInputSlot sSYNetInputSlots[MAXCONTROLLERS];
SYNetInputFrame sSYNetInputHistory[MAXCONTROLLERS][SYNETINPUT_HISTORY_LENGTH];
SYNetInputFrame sSYNetInputRemoteHistory[MAXCONTROLLERS][SYNETINPUT_HISTORY_LENGTH];
SYNetInputFrame sSYNetInputLocalDelayedHistory[MAXCONTROLLERS][SYNETINPUT_HISTORY_LENGTH];
SYNetInputFrame sSYNetInputSavedHistory[MAXCONTROLLERS][SYNETINPUT_HISTORY_LENGTH];
SYNetInputFrame sSYNetInputReplayFrames[MAXCONTROLLERS][SYNETINPUT_REPLAY_MAX_FRAMES];
SYNetInputReplayMetadata sSYNetInputReplayMetadata;
u32 sSYNetInputTick;
u32 sSYNetInputRecordedFrameCount;
sb32 sSYNetInputIsRecording;
sb32 sSYNetInputIsReplayMetadataValid;
sb32 sSYNetInputModernNetplayIsActive;
s32 sSYNetInputModernLocalPlayer = -1;
u32 sSYNetInputPredictionMismatchTick = UINT_MAX;
sb32 sSYNetInputModernIsFrameReady;
u32 sSYNetInputModernStallCount;
u32 sSYNetInputModernStallFrame = UINT_MAX;
u32 sSYNetInputModernStallPolls;

u32 syNetInputGetTick(void)
{
	return sSYNetInputTick;
}

void syNetInputSetTick(u32 tick)
{
	sSYNetInputTick = tick;
}

sb32 syNetInputCheckPlayer(s32 player)
{
	return ((player >= 0) && (player < MAXCONTROLLERS)) ? TRUE : FALSE;
}

void syNetInputClearFrame(SYNetInputFrame *frame)
{
	frame->tick = 0;
	frame->buttons = 0;
	frame->stick_x = 0;
	frame->stick_y = 0;
	frame->source = nSYNetInputSourceLocal;
	frame->is_predicted = FALSE;
	frame->is_valid = FALSE;
}

void syNetInputMakeFrame(SYNetInputFrame *frame, u32 tick, u16 buttons, s8 stick_x, s8 stick_y, SYNetInputSource source, sb32 is_predicted)
{
	frame->tick = tick;
	frame->buttons = buttons;
	frame->stick_x = stick_x;
	frame->stick_y = stick_y;
	frame->source = source;
	frame->is_predicted = is_predicted;
	frame->is_valid = TRUE;
}

void syNetInputStoreFrame(SYNetInputFrame history[][SYNETINPUT_HISTORY_LENGTH], s32 player, SYNetInputFrame *frame)
{
	history[player][frame->tick % SYNETINPUT_HISTORY_LENGTH] = *frame;
}

sb32 syNetInputGetStoredFrame(SYNetInputFrame history[][SYNETINPUT_HISTORY_LENGTH], s32 player, u32 tick, SYNetInputFrame *out_frame)
{
	SYNetInputFrame *frame;

	if (syNetInputCheckPlayer(player) == FALSE)
	{
		return FALSE;
	}
	frame = &history[player][tick % SYNETINPUT_HISTORY_LENGTH];

	if ((frame->is_valid == FALSE) || (frame->tick != tick))
	{
		return FALSE;
	}
	if (out_frame != NULL)
	{
		*out_frame = *frame;
	}
	return TRUE;
}

void syNetInputReset(void)
{
	s32 player;
	s32 i;

	sSYNetInputTick = 0;
	sSYNetInputRecordedFrameCount = 0;
	sSYNetInputIsRecording = FALSE;
	sSYNetInputIsReplayMetadataValid = FALSE;
	sSYNetInputModernNetplayIsActive = FALSE;
	sSYNetInputModernLocalPlayer = -1;
	sSYNetInputPredictionMismatchTick = UINT_MAX;
	sSYNetInputModernIsFrameReady = FALSE;
	sSYNetInputModernStallCount = 0;
	sSYNetInputModernStallFrame = UINT_MAX;
	sSYNetInputModernStallPolls = 0;

	sSYNetInputReplayMetadata.magic = SYNETINPUT_REPLAY_MAGIC;
	sSYNetInputReplayMetadata.version = SYNETINPUT_REPLAY_VERSION;
	sSYNetInputReplayMetadata.scene_kind = 0;
	sSYNetInputReplayMetadata.player_count = 0;
	sSYNetInputReplayMetadata.stage_kind = 0;
	sSYNetInputReplayMetadata.stocks = 0;
	sSYNetInputReplayMetadata.time_limit = 0;
	sSYNetInputReplayMetadata.item_switch = 0;
	sSYNetInputReplayMetadata.item_toggles = 0;
	sSYNetInputReplayMetadata.rng_seed = 0;
	sSYNetInputReplayMetadata.game_type = 0;
	sSYNetInputReplayMetadata.game_rules = 0;
	sSYNetInputReplayMetadata.is_team_battle = FALSE;
	sSYNetInputReplayMetadata.handicap = 0;
	sSYNetInputReplayMetadata.is_team_attack = FALSE;
	sSYNetInputReplayMetadata.is_stage_select = FALSE;
	sSYNetInputReplayMetadata.damage_ratio = 0;
	sSYNetInputReplayMetadata.item_appearance_rate = 0;
	sSYNetInputReplayMetadata.is_not_teamshadows = FALSE;

	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		sSYNetInputSlots[player].source = nSYNetInputSourceLocal;
		syNetInputClearFrame(&sSYNetInputSlots[player].last_confirmed);
		syNetInputClearFrame(&sSYNetInputSlots[player].last_published);
		sSYNetInputReplayMetadata.player_kinds[player] = 0;
		sSYNetInputReplayMetadata.fighter_kinds[player] = 0;
		sSYNetInputReplayMetadata.costumes[player] = 0;
		sSYNetInputReplayMetadata.teams[player] = 0;
		sSYNetInputReplayMetadata.handicaps[player] = 0;
		sSYNetInputReplayMetadata.levels[player] = 0;
		sSYNetInputReplayMetadata.shades[player] = 0;

		for (i = 0; i < SYNETINPUT_HISTORY_LENGTH; i++)
		{
			syNetInputClearFrame(&sSYNetInputHistory[player][i]);
			syNetInputClearFrame(&sSYNetInputRemoteHistory[player][i]);
			syNetInputClearFrame(&sSYNetInputLocalDelayedHistory[player][i]);
			syNetInputClearFrame(&sSYNetInputSavedHistory[player][i]);
		}
		for (i = 0; i < SYNETINPUT_REPLAY_MAX_FRAMES; i++)
		{
			syNetInputClearFrame(&sSYNetInputReplayFrames[player][i]);
		}
	}
}

void syNetInputStartVSSession(void)
{
	syNetInputReset();
}

void syNetInputSetSlotSource(s32 player, SYNetInputSource source)
{
	if (syNetInputCheckPlayer(player) != FALSE)
	{
		sSYNetInputSlots[player].source = source;
	}
}

SYNetInputSource syNetInputGetSlotSource(s32 player)
{
	if (syNetInputCheckPlayer(player) == FALSE)
	{
		return nSYNetInputSourceLocal;
	}
	return sSYNetInputSlots[player].source;
}

void syNetInputSetRemoteInput(s32 player, u32 tick, u16 buttons, s8 stick_x, s8 stick_y)
{
	SYNetInputFrame frame;
	SYNetInputFrame published;

	if (syNetInputCheckPlayer(player) != FALSE)
	{
		if ((syNetInputGetStoredFrame(sSYNetInputHistory, player, tick, &published) != FALSE) &&
		    (published.is_predicted != FALSE) &&
		    ((published.buttons != buttons) || (published.stick_x != stick_x) || (published.stick_y != stick_y)))
		{
			if ((sSYNetInputPredictionMismatchTick == UINT_MAX) || (tick < sSYNetInputPredictionMismatchTick))
			{
				sSYNetInputPredictionMismatchTick = tick;
			}
		}
		syNetInputMakeFrame(&frame, tick, buttons, stick_x, stick_y, nSYNetInputSourceRemoteConfirmed, FALSE);
		syNetInputStoreFrame(sSYNetInputRemoteHistory, player, &frame);
	}
}

sb32 syNetInputConsumePredictionMismatch(u32 *out_tick)
{
	if (sSYNetInputPredictionMismatchTick == UINT_MAX) return FALSE;
	if (out_tick != NULL) *out_tick = sSYNetInputPredictionMismatchTick;
	sSYNetInputPredictionMismatchTick = UINT_MAX;
	return TRUE;
}

void syNetInputSetSavedInput(s32 player, u32 tick, u16 buttons, s8 stick_x, s8 stick_y)
{
	SYNetInputFrame frame;

	if (syNetInputCheckPlayer(player) != FALSE)
	{
		syNetInputMakeFrame(&frame, tick, buttons, stick_x, stick_y, nSYNetInputSourceSaved, FALSE);
		syNetInputStoreFrame(sSYNetInputSavedHistory, player, &frame);
	}
}

void syNetInputMakeLocalFrame(s32 player, u32 tick, SYNetInputFrame *out_frame)
{
	SYController *controller = &gSYControllerDevices[player];

	if ((sSYNetInputModernNetplayIsActive != FALSE) && (player == sSYNetInputModernLocalPlayer))
	{
		if (syNetInputGetStoredFrame(sSYNetInputLocalDelayedHistory, player, tick, out_frame) == FALSE)
		{
			syNetInputMakeFrame(out_frame, tick, 0, 0, 0, nSYNetInputSourceLocal, FALSE);
		}
		return;
	}

	syNetInputMakeFrame
	(
		out_frame,
		tick,
		controller->button_hold,
		controller->stick_range.x,
		controller->stick_range.y,
		nSYNetInputSourceLocal,
		FALSE
	);
}

void syNetInputMakePredictedFrame(s32 player, u32 tick, SYNetInputFrame *out_frame)
{
	SYNetInputFrame *last_confirmed = &sSYNetInputSlots[player].last_confirmed;

	if (last_confirmed->is_valid != FALSE)
	{
		syNetInputMakeFrame
		(
			out_frame,
			tick,
			last_confirmed->buttons,
			last_confirmed->stick_x,
			last_confirmed->stick_y,
			nSYNetInputSourceRemotePredicted,
			TRUE
		);
	}
	else syNetInputMakeFrame(out_frame, tick, 0, 0, 0, nSYNetInputSourceRemotePredicted, TRUE);
}

void syNetInputResolveFrame(s32 player, u32 tick, SYNetInputFrame *out_frame)
{
	switch (sSYNetInputSlots[player].source)
	{
	case nSYNetInputSourceRemoteConfirmed:
	case nSYNetInputSourceRemotePredicted:
		if (syNetInputGetStoredFrame(sSYNetInputRemoteHistory, player, tick, out_frame) != FALSE)
		{
			sSYNetInputSlots[player].last_confirmed = *out_frame;
		}
		else syNetInputMakePredictedFrame(player, tick, out_frame);
		break;

	case nSYNetInputSourceSaved:
		if (syNetInputGetReplayFrame(player, tick, out_frame) != FALSE)
		{
			out_frame->source = nSYNetInputSourceSaved;
			out_frame->is_predicted = FALSE;
		}
		else if (syNetInputGetStoredFrame(sSYNetInputSavedHistory, player, tick, out_frame) == FALSE)
		{
			syNetInputMakeFrame(out_frame, tick, 0, 0, 0, nSYNetInputSourceSaved, FALSE);
		}
		break;

	case nSYNetInputSourceLocal:
	default:
		syNetInputMakeLocalFrame(player, tick, out_frame);
		break;
	}
}

void syNetInputPublishFrame(s32 player, SYNetInputFrame *frame)
{
	SYNetInputFrame *last_published = &sSYNetInputSlots[player].last_published;
	u16 prev_buttons = (last_published->is_valid != FALSE) ? last_published->buttons : 0;
	u16 pressed = (frame->buttons ^ prev_buttons) & frame->buttons;
	u16 released = (frame->buttons ^ prev_buttons) & prev_buttons;

	gSYControllerDevices[player].button_hold = frame->buttons;
	gSYControllerDevices[player].button_tap = pressed;
	gSYControllerDevices[player].button_release = released;
	gSYControllerDevices[player].button_update = pressed;
	gSYControllerDevices[player].stick_range.x = frame->stick_x;
	gSYControllerDevices[player].stick_range.y = frame->stick_y;

	sSYNetInputSlots[player].last_published = *frame;
	syNetInputStoreFrame(sSYNetInputHistory, player, frame);
}

void syNetInputPublishMainController(void)
{
	s32 player = ((sSYNetInputModernNetplayIsActive != FALSE) &&
	              (sSYNetInputModernLocalPlayer >= 0) && (sSYNetInputModernLocalPlayer < MAXCONTROLLERS))
	             ? sSYNetInputModernLocalPlayer : 0;
	gSYControllerMain.button_hold = gSYControllerDevices[player].button_hold;
	gSYControllerMain.button_tap = gSYControllerDevices[player].button_tap;
	gSYControllerMain.button_update = gSYControllerDevices[player].button_update;
	gSYControllerMain.button_release = gSYControllerDevices[player].button_release;
	gSYControllerMain.stick_range.x = gSYControllerDevices[player].stick_range.x;
	gSYControllerMain.stick_range.y = gSYControllerDevices[player].stick_range.y;
}

sb32 syNetInputGetHistoryFrame(s32 player, u32 tick, SYNetInputFrame *out_frame)
{
	return syNetInputGetStoredFrame(sSYNetInputHistory, player, tick, out_frame);
}

sb32 syNetInputGetPublishedFrame(s32 player, SYNetInputFrame *out_frame)
{
	if (syNetInputCheckPlayer(player) == FALSE)
	{
		return FALSE;
	}
	if (sSYNetInputSlots[player].last_published.is_valid == FALSE)
	{
		return FALSE;
	}
	if (out_frame != NULL)
	{
		*out_frame = sSYNetInputSlots[player].last_published;
	}
	return TRUE;
}

u32 syNetInputGetHistoryChecksum(s32 player, u32 tick_begin, u32 frame_count)
{
	SYNetInputFrame frame;
	u32 checksum = 2166136261U;
	u32 i;

	for (i = 0; i < frame_count; i++)
	{
		if (syNetInputGetHistoryFrame(player, tick_begin + i, &frame) != FALSE)
		{
			checksum ^= frame.tick;
			checksum *= 16777619U;
			checksum ^= frame.buttons;
			checksum *= 16777619U;
			checksum ^= (u8)frame.stick_x;
			checksum *= 16777619U;
			checksum ^= (u8)frame.stick_y;
			checksum *= 16777619U;
			checksum ^= frame.source;
			checksum *= 16777619U;
			checksum ^= frame.is_predicted;
			checksum *= 16777619U;
		}
	}
	return checksum;
}

u32 syNetInputAccumulateInputChecksum(u32 checksum, s32 player, SYNetInputFrame *frame)
{
	checksum ^= (u32)player;
	checksum *= 16777619U;
	checksum ^= frame->tick;
	checksum *= 16777619U;
	checksum ^= frame->buttons;
	checksum *= 16777619U;
	checksum ^= (u8)frame->stick_x;
	checksum *= 16777619U;
	checksum ^= (u8)frame->stick_y;
	checksum *= 16777619U;

	return checksum;
}

u32 syNetInputGetHistoryInputChecksum(u32 frame_count)
{
	SYNetInputFrame frame;
	u32 checksum = 2166136261U;
	u32 tick;
	s32 player;

	for (tick = 0; tick < frame_count; tick++)
	{
		for (player = 0; player < MAXCONTROLLERS; player++)
		{
			if (syNetInputGetHistoryFrame(player, tick, &frame) != FALSE)
			{
				checksum = syNetInputAccumulateInputChecksum(checksum, player, &frame);
			}
		}
	}
	return checksum;
}

u32 syNetInputGetHistoryInputValueChecksumForPlayer(s32 player, u32 tick_begin, u32 frame_count)
{
	SYNetInputFrame frame;
	u32 checksum = 2166136261U;
	u32 i;

	for (i = 0; i < frame_count; i++)
	{
		if (syNetInputGetHistoryFrame(player, tick_begin + i, &frame) != FALSE)
		{
			checksum = syNetInputAccumulateInputChecksum(checksum, player, &frame);
		}
	}
	return checksum;
}

void syNetInputGetHistoryInputValueChecksumWindow(u32 tick_begin, u32 frame_count, u32 *out_checksums,
                                                  u32 *out_combined_checksum)
{
	SYNetInputFrame frame;
	u32 checksum = 2166136261U;
	u32 tick_limit;
	u32 tick;
	s32 player;

	tick_limit = tick_begin + frame_count;

	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		u32 player_checksum = 2166136261U;

		for (tick = tick_begin; tick < tick_limit; tick++)
		{
			if (syNetInputGetHistoryFrame(player, tick, &frame) != FALSE)
			{
				player_checksum = syNetInputAccumulateInputChecksum(player_checksum, player, &frame);
			}
		}
		checksum ^= player_checksum;
		checksum *= 16777619U;

		if (out_checksums != NULL)
		{
			out_checksums[player] = player_checksum;
		}
	}
	if (out_combined_checksum != NULL)
	{
		*out_combined_checksum = checksum;
	}
}

void syNetInputSetRecordingEnabled(sb32 is_enabled)
{
	sSYNetInputIsRecording = is_enabled;

	if (is_enabled != FALSE)
	{
		sSYNetInputRecordedFrameCount = 0;
	}
}

sb32 syNetInputGetRecordingEnabled(void)
{
	return sSYNetInputIsRecording;
}

u32 syNetInputGetRecordedFrameCount(void)
{
	return sSYNetInputRecordedFrameCount;
}

void syNetInputClearReplayFrames(void)
{
	s32 player;
	s32 i;

	sSYNetInputRecordedFrameCount = 0;

	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		for (i = 0; i < SYNETINPUT_REPLAY_MAX_FRAMES; i++)
		{
			syNetInputClearFrame(&sSYNetInputReplayFrames[player][i]);
		}
	}
}

sb32 syNetInputSetReplayFrame(s32 player, u32 tick, const SYNetInputFrame *frame)
{
	if ((syNetInputCheckPlayer(player) == FALSE) || (frame == NULL) || (tick >= SYNETINPUT_REPLAY_MAX_FRAMES))
	{
		return FALSE;
	}
	sSYNetInputReplayFrames[player][tick] = *frame;
	sSYNetInputReplayFrames[player][tick].tick = tick;
	sSYNetInputReplayFrames[player][tick].is_valid = TRUE;

	if (sSYNetInputRecordedFrameCount < (tick + 1))
	{
		sSYNetInputRecordedFrameCount = tick + 1;
	}
	return TRUE;
}

sb32 syNetInputGetReplayFrame(s32 player, u32 tick, SYNetInputFrame *out_frame)
{
	SYNetInputFrame *frame;

	if ((syNetInputCheckPlayer(player) == FALSE) || (tick >= SYNETINPUT_REPLAY_MAX_FRAMES))
	{
		return FALSE;
	}
	frame = &sSYNetInputReplayFrames[player][tick];

	if ((frame->is_valid == FALSE) || (frame->tick != tick))
	{
		return FALSE;
	}
	if (out_frame != NULL)
	{
		*out_frame = *frame;
	}
	return TRUE;
}

u32 syNetInputGetReplayInputChecksum(void)
{
	SYNetInputFrame frame;
	u32 checksum = 2166136261U;
	u32 tick;
	s32 player;

	for (tick = 0; tick < sSYNetInputRecordedFrameCount; tick++)
	{
		for (player = 0; player < MAXCONTROLLERS; player++)
		{
			if (syNetInputGetReplayFrame(player, tick, &frame) != FALSE)
			{
				checksum = syNetInputAccumulateInputChecksum(checksum, player, &frame);
			}
		}
	}
	return checksum;
}

void syNetInputSetReplayMetadata(const SYNetInputReplayMetadata *metadata)
{
	if (metadata != NULL)
	{
		sSYNetInputReplayMetadata = *metadata;
		sSYNetInputReplayMetadata.magic = SYNETINPUT_REPLAY_MAGIC;
		sSYNetInputReplayMetadata.version = SYNETINPUT_REPLAY_VERSION;
		sSYNetInputIsReplayMetadataValid = TRUE;
	}
}

sb32 syNetInputGetReplayMetadata(SYNetInputReplayMetadata *out_metadata)
{
	if (sSYNetInputIsReplayMetadataValid == FALSE)
	{
		return FALSE;
	}
	if (out_metadata != NULL)
	{
		*out_metadata = sSYNetInputReplayMetadata;
	}
	return TRUE;
}

static void syNetInputProcessModernFrame(void)
{
#ifdef PORT
	SYNetInputFrame frame;
	SYNetInputFrame local_frame;
	u32 tick = syNetInputGetTick();
	u32 participant_mask = 0;
	u32 confirmed_mask = 0;
	u32 remote_tick;
	u16 remote_buttons;
	u16 physical_buttons = gSYControllerDevices[0].button_hold;
	s8 remote_x;
	s8 remote_y;
	s8 physical_x = gSYControllerDevices[0].stick_range.x;
	s8 physical_y = gSYControllerDevices[0].stick_range.y;
	s32 remote_player;
	s32 player;
	s32 delay = port_netplay_gameplay_get_input_delay();

	while (port_netplay_gameplay_consume_input(&remote_player, &remote_tick, &remote_buttons, &remote_x, &remote_y) != 0)
	{
		syNetInputSetRemoteInput(remote_player, remote_tick, remote_buttons, remote_x, remote_y);
	}
	if (delay < 0) delay = 0;
	if (delay > 4) delay = 4;
	if ((sSYNetInputModernLocalPlayer >= 0) && (sSYNetInputModernLocalPlayer < MAXCONTROLLERS))
	{
		u32 target_tick = tick + (u32)delay;
		if (syNetInputGetStoredFrame(sSYNetInputLocalDelayedHistory, sSYNetInputModernLocalPlayer,
		                             target_tick, NULL) == FALSE)
		{
			syNetInputMakeFrame(&local_frame, target_tick, physical_buttons, physical_x, physical_y,
			                    nSYNetInputSourceLocal, FALSE);
			syNetInputStoreFrame(sSYNetInputLocalDelayedHistory, sSYNetInputModernLocalPlayer, &local_frame);
			port_netplay_gameplay_submit_input(target_tick, physical_buttons, physical_x, physical_y);
		}
	}
	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		SYNetInputSource source = sSYNetInputSlots[player].source;
		if ((source != nSYNetInputSourceRemoteConfirmed) &&
		    (source != nSYNetInputSourceRemotePredicted) &&
		    (player != sSYNetInputModernLocalPlayer))
		{
			continue;
		}
		participant_mask |= (1U << player);
		if ((player == sSYNetInputModernLocalPlayer) ||
		    (syNetInputGetStoredFrame(sSYNetInputRemoteHistory, player, tick, NULL) != FALSE))
		{
			confirmed_mask |= (1U << player);
		}
	}
	if (syNetInputConfirmedBarrierReady(tick, (u32)delay, participant_mask, confirmed_mask,
	                                   (u32)sSYNetInputModernLocalPlayer) == FALSE)
	{
		sSYNetInputModernIsFrameReady = FALSE;
		sSYNetInputModernStallCount++;
		if (sSYNetInputModernStallFrame != tick)
		{
			sSYNetInputModernStallFrame = tick;
			sSYNetInputModernStallPolls = 1;
		}
		else sSYNetInputModernStallPolls++;
		return;
	}
	if ((sSYNetInputModernStallFrame == tick) && (sSYNetInputModernStallPolls > 1U) &&
	    ((sSYNetInputModernStallPolls >= 4U) || ((tick % 60U) == 0U)))
	{
		port_log("[NETPLAY] confirmed input barrier released frame=%u waits=%u required=0x%02X\n",
		         tick, sSYNetInputModernStallPolls, participant_mask);
	}
	sSYNetInputModernStallFrame = UINT_MAX;
	sSYNetInputModernStallPolls = 0;
	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		syNetInputResolveFrame(player, tick, &frame);
		syNetInputPublishFrame(player, &frame);
	}
	syNetInputPublishMainController();
	sSYNetInputTick++;
	sSYNetInputModernIsFrameReady = TRUE;
#else
	(void)0;
#endif
}

void syNetInputActivateModernSession(void)
{
#ifdef PORT
	s32 player;
	s32 i;

	if (sSYNetInputModernNetplayIsActive != FALSE) return;
	if (port_netplay_gameplay_active() == 0) return;
	sSYNetInputModernLocalPlayer = port_netplay_lobby_get_local_player();
	if ((sSYNetInputModernLocalPlayer < 0) || (sSYNetInputModernLocalPlayer >= MAXCONTROLLERS)) return;
	sSYNetInputTick = 0;
	sSYNetInputPredictionMismatchTick = UINT_MAX;
	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		syNetInputClearFrame(&sSYNetInputSlots[player].last_confirmed);
		syNetInputClearFrame(&sSYNetInputSlots[player].last_published);
		sSYNetInputSlots[player].source = (player == sSYNetInputModernLocalPlayer)
		    ? nSYNetInputSourceLocal
		    : (port_netplay_gameplay_slot_connected(player) != 0
		       ? nSYNetInputSourceRemotePredicted : nSYNetInputSourceLocal);
		for (i = 0; i < SYNETINPUT_HISTORY_LENGTH; i++)
		{
			syNetInputClearFrame(&sSYNetInputHistory[player][i]);
			syNetInputClearFrame(&sSYNetInputRemoteHistory[player][i]);
			syNetInputClearFrame(&sSYNetInputLocalDelayedHistory[player][i]);
		}
	}
	{
		s32 delay = port_netplay_gameplay_get_input_delay();
		if (delay < 0) delay = 0;
		if (delay > 4) delay = 4;
		for (player = 0; player < MAXCONTROLLERS; player++)
		{
			if (sSYNetInputSlots[player].source == nSYNetInputSourceRemotePredicted)
			{
				for (i = 0; i < delay; i++)
				{
					SYNetInputFrame neutral;
					syNetInputMakeFrame(&neutral, (u32)i, 0, 0, 0,
					                    nSYNetInputSourceRemoteConfirmed, FALSE);
					syNetInputStoreFrame(sSYNetInputRemoteHistory, player, &neutral);
				}
			}
			else if (player == sSYNetInputModernLocalPlayer)
			{
				for (i = 0; i < delay; i++)
				{
					SYNetInputFrame neutral;
					syNetInputMakeFrame(&neutral, (u32)i, 0, 0, 0, nSYNetInputSourceLocal, FALSE);
					syNetInputStoreFrame(sSYNetInputLocalDelayedHistory, player, &neutral);
				}
			}
		}
	}
	sSYNetInputModernNetplayIsActive = TRUE;
	sSYNetInputModernIsFrameReady = FALSE;
	sSYNetInputModernStallFrame = UINT_MAX;
	sSYNetInputModernStallPolls = 0;
	syNetInputProcessModernFrame();
#endif
}

void syNetInputDeactivateModernSession(void)
{
	sSYNetInputModernNetplayIsActive = FALSE;
	sSYNetInputModernLocalPlayer = -1;
	sSYNetInputModernIsFrameReady = FALSE;
	sSYNetInputPredictionMismatchTick = UINT_MAX;
	sSYNetInputModernStallFrame = UINT_MAX;
	sSYNetInputModernStallPolls = 0;
}

sb32 syNetInputModernNetplayActive(void)
{
	return sSYNetInputModernNetplayIsActive;
}

sb32 syNetInputModernFrameReady(void)
{
	return sSYNetInputModernIsFrameReady;
}

u32 syNetInputGetModernStallCount(void)
{
	return sSYNetInputModernStallCount;
}

void syNetInputPrepareResimulation(u32 start_tick)
{
	s32 player;
	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		u32 search_tick;
		syNetInputClearFrame(&sSYNetInputSlots[player].last_published);
		syNetInputClearFrame(&sSYNetInputSlots[player].last_confirmed);
		if (start_tick != 0)
		{
			SYNetInputFrame frame;
			if (syNetInputGetHistoryFrame(player, start_tick - 1U, &frame) != FALSE)
				sSYNetInputSlots[player].last_published = frame;
			if ((sSYNetInputSlots[player].source == nSYNetInputSourceRemoteConfirmed) ||
			    (sSYNetInputSlots[player].source == nSYNetInputSourceRemotePredicted))
			{
				search_tick = start_tick - 1U;
				for (;;)
				{
					if (syNetInputGetStoredFrame(sSYNetInputRemoteHistory, player, search_tick, &frame) != FALSE)
					{
						sSYNetInputSlots[player].last_confirmed = frame;
						break;
					}
					if ((search_tick == 0) || ((start_tick - search_tick) >= SYNETINPUT_HISTORY_LENGTH)) break;
					search_tick--;
				}
			}
		}
	}
	sSYNetInputTick = start_tick;
}

void syNetInputPublishResimulationFrame(u32 tick)
{
	SYNetInputFrame frame;
	s32 player;
	sSYNetInputTick = tick;
	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		syNetInputResolveFrame(player, tick, &frame);
		syNetInputPublishFrame(player, &frame);
	}
	syNetInputPublishMainController();
	sSYNetInputTick = tick + 1U;
}

void syNetInputFuncRead(void)
{
	SYNetInputFrame frame;
	u32 tick;
	s32 player;

	syControllerFuncRead();
	if (sSYNetInputModernNetplayIsActive != FALSE)
	{
		syNetInputProcessModernFrame();
		return;
	}
	tick = syNetInputGetTick();

	for (player = 0; player < MAXCONTROLLERS; player++)
	{
		syNetInputResolveFrame(player, tick, &frame);
		syNetInputPublishFrame(player, &frame);

		if (sSYNetInputIsRecording != FALSE)
		{
			syNetInputSetReplayFrame(player, tick, &frame);
		}
	}
	syNetInputPublishMainController();

	if (syNetPeerCheckStartBarrierReleased() == FALSE)
	{
		return;
	}
	sSYNetInputTick++;
}
