#include <mn/menu.h>
#include <sc/scene.h>
#include <gm/gmsound.h>
#include <sys/controller.h>
#include <sys/video.h>
#include <sys/rdp.h>
#include <reloc_data.h>
#ifdef PORT
#include <sys/audio.h>
extern void *func_800269C0_275C0(u16 id);
#endif
#if defined(PORT) && defined(__vita__)
#include <stdio.h>
#include <string.h>
#include <netplay/netplay_bridge.h>
#endif


// // // // // // // // // // // //
//                               //
//             MACROS            //
//                               //
// // // // // // // // // // // //

#define mnModeSelectCheckGetOptionButtonInput(is_button, mask) \
mnCommonCheckGetOptionButtonInput(sMNModeSelectOptionChangeWait, is_button, mask)

#define mnModeSelectCheckGetOptionStickInputUD(stick_range, min, b) \
mnCommonCheckGetOptionStickInputUD(sMNModeSelectOptionChangeWait, stick_range, min, b)

#define mnModeSelectCheckGetOptionStickInputLR(stick_range, min, b) \
mnCommonCheckGetOptionStickInputLR(sMNModeSelectOptionChangeWait, stick_range, min, b)

#define mnModeSelectSetOptionChangeWaitP(is_button, stick_range, div) \
mnCommonSetOptionChangeWaitP(sMNModeSelectOptionChangeWait, is_button, stick_range, div)

#define mnModeSelectSetOptionChangeWaitN(is_button, stick_range, div) \
mnCommonSetOptionChangeWaitN(sMNModeSelectOptionChangeWait, is_button, stick_range, div)

// // // // // // // // // // // //
//                               //
//       INITIALIZED DATA        //
//                               //
// // // // // // // // // // // //

// 0x80132B90
#if defined(PORT) && defined(__vita__)
u32 dMNModeSelectFileIDs[/* */] = { llMNCommonFileID, llMNMainFileID, llMNCommonFontsFileID, llIFCommonDigitsFileID };
#elif defined(PORT)
u32 dMNModeSelectFileIDs[/* */] = { llMNCommonFileID, llMNMainFileID };
#else
u32 dMNModeSelectFileIDs[/* */] = { &llMNCommonFileID, &llMNMainFileID };
#endif

// 0x80133088
Lights1 dMNModeSelectLights1 = gdSPDefLights1(0x20, 0x20, 0x20, 0xFF, 0xFF, 0xFF, 0x3C, 0x3C, 0x3C);

// 0x801330A0
Gfx dMNModeSelectDisplayList[/* */] =
{
    gsSPSetGeometryMode(G_LIGHTING),
    gsSPSetLights1(dMNModeSelectLights1),
    gsSPEndDisplayList()
};

// 0x80132BD8
SYVideoSetup dMNModeSelectVideoSetup = SYVIDEO_SETUP_DEFAULT();

// 0x80132BF4
SYTaskmanSetup dMNModeSelectTaskmanSetup =
{
    // Task Manager Buffer Setup
    {
        0,                              // ???
        gcRunAll,                 	 	// Update function
        gcDrawAll,                      // Frame draw function
        &ovl17_BSS_END,                 // Allocatable memory pool start
        0,                              // Allocatable memory pool size
        1,                              // ???
        2,                              // Number of contexts?
        sizeof(Gfx) * 3750,             // Display List Buffer 0 Size
        0,                              // Display List Buffer 1 Size
        0,                              // Display List Buffer 2 Size
        0,                              // Display List Buffer 3 Size
        0x8000,                         // Graphics Heap Size
        2,                              // ???
        0xC000,                         // RDP Output Buffer Size
        mnModeSelectFuncLights,    	    // Pre-render function
        syControllerFuncRead,           // Controller I/O function
    },

    0,                                  // Number of GObjThreads
    sizeof(u64) * 192,                  // Thread stack size
    0,                                  // Number of thread stacks
    0,                                  // ???
    0,                                  // Number of GObjProcesses
    0,                                  // Number of GObjs
    sizeof(GObj),                       // GObj size
    0,                                  // Number of XObjs
    NULL,                               // Matrix function list
    NULL,                               // DObjVec eject function
    0,                                  // Number of AObjs
    0,                                  // Number of MObjs
    0,                                  // Number of DObjs
    sizeof(DObj),                       // DObj size
    0,                                  // Number of SObjs
    sizeof(SObj),                       // SObj size
    0,                                  // Number of CObjs
    sizeof(CObj),                       // Camera size
    
    mnModeSelectFuncStart          	    // Task start function
};

// // // // // // // // // // // //
//                               //
//   GLOBAL / STATIC VARIABLES   //
//                               //
// // // // // // // // // // // //

// 0x80132C80
s32 sMNModeSelectPad0x80132C80[2];

// 0x80132C88
s32 sMNModeSelectOption;

// 0x80132C8C
GObj *sMNModeSelectOption1PModeGObj;

// 0x80132C90
GObj *sMNModeSelectOptionVSModeGObj;

// 0x80132C94
GObj *sMNModeSelectOptionOptionGObj;

// 0x80132C98
GObj *sMNModeSelectOptionDataGObj;

#if defined(PORT) && defined(__vita__)
GObj *sMNModeSelectOptionMultiplayerGObj;
GObj *sMNModeSelectLabelsGObj;
GObj *sMNModeSelectNetplayGObj;
s32 sMNModeSelectNetplayPage;
s32 sMNModeSelectNetplayOption;
s32 sMNModeSelectNetplayNavWait;
s32 sMNModeSelectNetplayNameCursor;
char sMNModeSelectNetplayNameEdit[13];

void mnModeSelectMakeOptions(void);
void mnModeSelectEjectOptions(void);
void mnModeSelectMakeLabels(void);
#endif

// 0x80132C9C
s32 sMNModeSelectOptionChangeWait;

// 0x80132CA0
s32 sMNModeSelectTotalTimeTics;

// 0x80132CA4
s32 sMNModeSelectReturnTic;

// 0x80132CA8
LBFileNode sMNModeSelectStatusBuffer[24];

// 0x80132D68
void *sMNModeSelectFiles[ARRAY_COUNT(dMNModeSelectFileIDs)];

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x80131B00
void mnModeSelectFuncLights(Gfx **dls)
{
    gSPDisplayList(dls[0]++, dMNModeSelectDisplayList);
}

#if defined(PORT) && defined(__vita__)
typedef enum MNModeSelectNetplayPage
{
    nMNModeSelectNetplayPageRoot,
    nMNModeSelectNetplayPageMode,
    nMNModeSelectNetplayPageFind,
    nMNModeSelectNetplayPageHost,
    nMNModeSelectNetplayPageJoin,
    nMNModeSelectNetplayPageSettings,
    nMNModeSelectNetplayPageNameEdit

} MNModeSelectNetplayPage;

enum
{
    nMNModeSelectNetplayRootAdhoc,
    nMNModeSelectNetplayRootOnline,
    nMNModeSelectNetplayRootSettings,
    nMNModeSelectNetplayRootBack,
    nMNModeSelectNetplayRootCount
};

enum
{
    nMNModeSelectNetplayModeFind,
    nMNModeSelectNetplayModeHost,
    nMNModeSelectNetplayModeJoin,
    nMNModeSelectNetplayModeBack,
    nMNModeSelectNetplayModeCount
};

enum
{
    nMNModeSelectNetplaySettingsName,
    nMNModeSelectNetplaySettingsDelay,
    nMNModeSelectNetplaySettingsStats,
    nMNModeSelectNetplaySettingsReset,
    nMNModeSelectNetplaySettingsBack,
    nMNModeSelectNetplaySettingsCount
};

static intptr_t dMNModeSelectNetplayFontOffsets[26] =
{
    llMNCommonFontsLetterASprite, llMNCommonFontsLetterBSprite,
    llMNCommonFontsLetterCSprite, llMNCommonFontsLetterDSprite,
    llMNCommonFontsLetterESprite, llMNCommonFontsLetterFSprite,
    llMNCommonFontsLetterGSprite, llMNCommonFontsLetterHSprite,
    llMNCommonFontsLetterISprite, llMNCommonFontsLetterJSprite,
    llMNCommonFontsLetterKSprite, llMNCommonFontsLetterLSprite,
    llMNCommonFontsLetterMSprite, llMNCommonFontsLetterNSprite,
    llMNCommonFontsLetterOSprite, llMNCommonFontsLetterPSprite,
    llMNCommonFontsLetterQSprite, llMNCommonFontsLetterRSprite,
    llMNCommonFontsLetterSSprite, llMNCommonFontsLetterTSprite,
    llMNCommonFontsLetterUSprite, llMNCommonFontsLetterVSprite,
    llMNCommonFontsLetterWSprite, llMNCommonFontsLetterXSprite,
    llMNCommonFontsLetterYSprite, llMNCommonFontsLetterZSprite
};

static intptr_t dMNModeSelectNetplayDigitOffsets[10] =
{
    llIFCommonDigits0Sprite, llIFCommonDigits1Sprite, llIFCommonDigits2Sprite, llIFCommonDigits3Sprite,
    llIFCommonDigits4Sprite, llIFCommonDigits5Sprite, llIFCommonDigits6Sprite, llIFCommonDigits7Sprite,
    llIFCommonDigits8Sprite, llIFCommonDigits9Sprite
};

static f32 mnModeSelectNetplayMakeString(GObj *gobj, const char *str, f32 x, f32 y, u8 red, u8 green, u8 blue)
{
    SObj *sobj;
    s32 i;

    for (i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == ' ')
        {
            x += 5.0F;
            continue;
        }
        if ((str[i] >= '0') && (str[i] <= '9'))
        {
            sobj = lbCommonMakeSObjForGObj(gobj,
                lbRelocGetFileData(Sprite*, sMNModeSelectFiles[3], dMNModeSelectNetplayDigitOffsets[str[i] - '0']));
            sobj->sprite.attr &= ~SP_FASTCOPY;
            sobj->sprite.attr |= SP_TRANSPARENT;
            sobj->sprite.red = red;
            sobj->sprite.green = green;
            sobj->sprite.blue = blue;
            sobj->sprite.scalex = 0.70F;
            sobj->sprite.scaley = 0.70F;
            sobj->pos.x = x;
            sobj->pos.y = y - 1.0F;
            x += (sobj->sprite.width * sobj->sprite.scalex) + 1.0F;
            continue;
        }
        if (str[i] == '.')
        {
            sobj = lbCommonMakeSObjForGObj(gobj,
                lbRelocGetFileData(Sprite*, sMNModeSelectFiles[2], llMNCommonFontsSymbolPeriodSprite));
            sobj->sprite.attr &= ~SP_FASTCOPY;
            sobj->sprite.attr |= SP_TRANSPARENT;
            sobj->sprite.red = red;
            sobj->sprite.green = green;
            sobj->sprite.blue = blue;
            sobj->pos.x = x;
            sobj->pos.y = y + 4.0F;
            x += sobj->sprite.width + 1.0F;
            continue;
        }
        if ((str[i] < 'A') || (str[i] > 'Z'))
        {
            continue;
        }
        sobj = lbCommonMakeSObjForGObj(gobj,
            lbRelocGetFileData(Sprite*, sMNModeSelectFiles[2], dMNModeSelectNetplayFontOffsets[str[i] - 'A']));
        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;
        sobj->sprite.red = red;
        sobj->sprite.green = green;
        sobj->sprite.blue = blue;
        sobj->pos.x = x;
        sobj->pos.y = y;
        x += sobj->sprite.width + 1.0F;
    }
    return x;
}

static void mnModeSelectNetplayMakeMarker(GObj *gobj, f32 y)
{
    SObj *sobj = lbCommonMakeSObjForGObj(gobj,
        lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainConsoleIconSprite));

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0xFF;
    sobj->sprite.blue = 0xFF;
    sobj->envcolor.r = 0x00;
    sobj->envcolor.g = 0x00;
    sobj->envcolor.b = 0x00;
    sobj->pos.x = 18.0F;
    sobj->pos.y = y - 8.0F;
    sobj->sprite.scalex = 0.45F;
    sobj->sprite.scaley = 0.45F;
}

static const char* mnModeSelectNetplayGetDelayText(void)
{
    switch (port_netplay_get_input_delay())
    {
    case 0: return "ZERO";
    case 1: return "ONE";
    case 2: return "TWO";
    case 3: return "THREE";
    case 4: return "FOUR";
    default: return "AUTO";
    }
}

static const char* mnModeSelectNetplayGetLobbyStatusText(s32 status)
{
    switch (status)
    {
    case 0: return "OPEN";
    case 1: return "FULL";
    case 2: return "STARTING";
    case 3: return "IN GAME";
    default: return "UNKNOWN";
    }
}

static const char* mnModeSelectNetplayGetSlotStateText(s32 state)
{
    switch (state)
    {
    case 0: return "EMPTY";
    case 1: return "CONNECTED";
    case 2: return "READY";
    case 3: return "DISCONNECTED";
    default: return "UNKNOWN";
    }
}

static void mnModeSelectNetplayMakeProtocolLine(GObj *gobj, f32 y)
{
    char build[24];
    char line[64];

    port_netplay_get_build_id(build, ARRAY_COUNT(build));
    snprintf(line, sizeof(line), "PROTOCOL %d BUILD %s", port_netplay_get_protocol_version(), build);
    mnModeSelectNetplayMakeString(gobj, line, 66.0F, y, 0xB0, 0xB0, 0xB0);
}

static void mnModeSelectNetplayMakeLobby(GObj *gobj, const char *title)
{
    char local_ip[20];
    char player_name[20];
    char line[72];
    char message[64];
    s32 slot;
    s32 state;
    s32 ping;
    s32 jitter;

    mnModeSelectNetplayMakeString(gobj, title, 112.0F, 33.0F, 0xFF, 0x20, 0x20);
    port_netplay_get_local_ip(local_ip, ARRAY_COUNT(local_ip));
    if (port_netplay_lobby_is_host())
    {
        snprintf(line, sizeof(line), (port_netplay_get_mode() == 1) ? "MAC %s" : "IP %s", local_ip);
        mnModeSelectNetplayMakeString(gobj, line, 28.0F, 52.0F, 0xA0, 0xFF, 0xA0);
    }
    mnModeSelectNetplayMakeProtocolLine(gobj, 66.0F);

    for (slot = 0; slot < 4; slot++)
    {
        player_name[0] = '\0';
        state = ping = jitter = 0;
        port_netplay_lobby_get_slot(slot, player_name, ARRAY_COUNT(player_name), &state, &ping, &jitter);
        if (player_name[0] == '\0') snprintf(player_name, sizeof(player_name), "EMPTY");
        snprintf(line, sizeof(line), "P%d %s", slot + 1, player_name);
        mnModeSelectNetplayMakeString(gobj, line, 34.0F, 88.0F + (slot * 28.0F),
            (state == 2) ? 0xA0 : 0xF0, (state == 2) ? 0xFF : 0xD0, (state == 3) ? 0x40 : 0xD0);
        mnModeSelectNetplayMakeString(gobj, mnModeSelectNetplayGetSlotStateText(state), 155.0F,
            88.0F + (slot * 28.0F), (state == 2) ? 0xA0 : 0xD0, (state == 2) ? 0xFF : 0xB0, 0x80);
        if ((state == 1) || (state == 2))
        {
            snprintf(line, sizeof(line), "%d MS", ping);
            mnModeSelectNetplayMakeString(gobj, line, 250.0F, 88.0F + (slot * 28.0F), 0xD0, 0xD0, 0xD0);
        }
    }

    port_netplay_get_lobby_message(message, ARRAY_COUNT(message));
    if (message[0] != '\0')
    {
        mnModeSelectNetplayMakeString(gobj, message, 84.0F, 200.0F, 0xFF, 0xC0, 0x40);
    }
    if (port_netplay_lobby_is_host())
    {
        mnModeSelectNetplayMakeString(gobj, port_netplay_lobby_local_ready() ? "A UNREADY" : "A READY",
            32.0F, 219.0F, 0xFF, 0x40, 0x40);
        mnModeSelectNetplayMakeString(gobj, port_netplay_lobby_can_start() ? "START GO" : "START WAIT",
            126.0F, 219.0F, port_netplay_lobby_can_start() ? 0xA0 : 0x80,
            port_netplay_lobby_can_start() ? 0xFF : 0x80, 0x80);
        mnModeSelectNetplayMakeString(gobj, "B LEAVE", 237.0F, 219.0F, 0xFF, 0x40, 0x40);
    }
    else
    {
        mnModeSelectNetplayMakeString(gobj, port_netplay_lobby_local_ready() ? "A UNREADY" : "A READY",
            80.0F, 219.0F, 0xFF, 0x40, 0x40);
        mnModeSelectNetplayMakeString(gobj, "B LEAVE", 205.0F, 219.0F, 0xFF, 0x40, 0x40);
    }
}

static void mnModeSelectNetplayMakeDiscoveryDetail(GObj *gobj, s32 index, f32 y)
{
    char host[20];
    char ip[20];
    char build[24];
    char line[72];
    s32 players = 0;
    s32 max_players = 0;
    s32 ping = 0;
    s32 protocol = 0;
    s32 status = 0;
    s32 compatible = 0;

    if (!port_netplay_get_discovery_lobby(index, host, ARRAY_COUNT(host), ip, ARRAY_COUNT(ip),
        build, ARRAY_COUNT(build), &players, &max_players, &ping, &protocol, &status, &compatible)) return;

    snprintf(line, sizeof(line), "%s %s", host, mnModeSelectNetplayGetLobbyStatusText(status));
    mnModeSelectNetplayMakeString(gobj, line, 50.0F, y, compatible ? 0xF0 : 0xFF,
        compatible ? 0xD0 : 0x80, compatible ? 0xD0 : 0x40);
    snprintf(line, sizeof(line), "%d OF %d  %d MS  V%d BUILD %s", players, max_players, ping, protocol, build);
    mnModeSelectNetplayMakeString(gobj, line, 50.0F, y + 15.0F, 0xB0, 0xB0, 0xB0);
}

static void mnModeSelectNetplayMakeNameEdit(GObj *gobj)
{
    s32 i;
    char letter[2] = { 0, 0 };

    mnModeSelectNetplayMakeString(gobj, "EDIT PLAYER NAME", 72.0F, 45.0F, 0xFF, 0x20, 0x20);

    for (i = 0; i < 12; i++)
    {
        letter[0] = sMNModeSelectNetplayNameEdit[i];
        if (letter[0] == '\0') letter[0] = ' ';
        mnModeSelectNetplayMakeString(gobj, letter, 48.0F + (i * 18.0F), 93.0F,
            (i == sMNModeSelectNetplayNameCursor) ? 0xFF : 0xD0,
            (i == sMNModeSelectNetplayNameCursor) ? 0xFF : 0x20,
            (i == sMNModeSelectNetplayNameCursor) ? 0xFF : 0x20);
    }
    mnModeSelectNetplayMakeString(gobj, "UP DOWN CHANGE", 82.0F, 137.0F, 0xFF, 0x40, 0x40);
    mnModeSelectNetplayMakeString(gobj, "LEFT RIGHT MOVE", 75.0F, 153.0F, 0xFF, 0x40, 0x40);
    mnModeSelectNetplayMakeString(gobj, "A SAVE B CANCEL", 77.0F, 180.0F, 0xFF, 0x40, 0x40);
}

static void mnModeSelectNetplayRefresh(void)
{
    static const char *root_options[nMNModeSelectNetplayRootCount] =
    {
        "LOCAL ADHOC", "ONLINE", "SETTINGS", "BACK"
    };
    static const char *mode_options[nMNModeSelectNetplayModeCount] =
    {
        "FIND GAME", "HOST GAME", "JOIN GAME", "BACK"
    };
    static const char *settings_options[nMNModeSelectNetplaySettingsCount] =
    {
        "PLAYER NAME", "INPUT DELAY", "NETPLAY STATS", "RESET SETTINGS", "BACK"
    };
    char player_name[16];
    s32 i;
    f32 y;

    if (sMNModeSelectNetplayGObj == NULL)
    {
        sMNModeSelectNetplayGObj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT);
        gcAddGObjDisplay(sMNModeSelectNetplayGObj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);
    }
    else gcRemoveSObjAll(sMNModeSelectNetplayGObj);

    if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageRoot)
    {
        mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "MULTIPLAYER", 111.0F, 42.0F, 0xFF, 0x20, 0x20);
        for (i = 0; i < nMNModeSelectNetplayRootCount; i++)
        {
            y = 75.0F + (i * 27.0F);
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, root_options[i], 70.0F, y,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0xD0,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0x20,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0x20);
        }
        mnModeSelectNetplayMakeMarker(sMNModeSelectNetplayGObj, 75.0F + (sMNModeSelectNetplayOption * 27.0F));
        if (port_netplay_network_initialized() == FALSE)
        {
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "NETWORK INITIALIZING", 92.0F, 218.0F, 0xB0, 0xB0, 0xB0);
        }
        else mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "SELECT CONNECTION", 101.0F, 218.0F, 0xA0, 0xFF, 0xA0);
    }
    else if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageMode)
    {
        const sb32 is_adhoc = (port_netplay_get_mode() == 1);
        const s32 adhoc_dialog_state = is_adhoc ? port_netplay_adhoc_dialog_state() : PORT_NETPLAY_ADHOC_DIALOG_INACTIVE;
        mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj,
            is_adhoc ? "LOCAL ADHOC" : "ONLINE", is_adhoc ? 107.0F : 127.0F, 42.0F,
            0xFF, 0x20, 0x20);
        for (i = 0; i < nMNModeSelectNetplayModeCount; i++)
        {
            y = 79.0F + (i * 29.0F);
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, mode_options[i], 72.0F, y,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0xD0,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0x20,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0x20);
        }
        mnModeSelectNetplayMakeMarker(sMNModeSelectNetplayGObj, 79.0F + (sMNModeSelectNetplayOption * 29.0F));
        if (port_netplay_mode_ready())
        {
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj,
                is_adhoc ? "ADHOC READY" : "ONLINE READY", is_adhoc ? 117.0F : 111.0F,
                211.0F, 0xA0, 0xFF, 0xA0);
        }
        else
        {
            if (is_adhoc)
            {
                const char *status_text = "ADHOC INITIALIZING";
                f32 status_x = 91.0F;

                if (adhoc_dialog_state == PORT_NETPLAY_ADHOC_DIALOG_RUNNING)
                {
                    status_text = "CONNECTING ADHOC";
                    status_x = 98.0F;
                }
                else if (adhoc_dialog_state == PORT_NETPLAY_ADHOC_DIALOG_CANCELED)
                {
                    status_text = "ADHOC CANCELED";
                    status_x = 108.0F;
                }
                else if (adhoc_dialog_state == PORT_NETPLAY_ADHOC_DIALOG_ERROR)
                {
                    status_text = "ADHOC CONNECTION FAILED";
                    status_x = 76.0F;
                }
                mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj,
                    status_text, status_x, 211.0F, 0xFF, 0xA0, 0x40);
            }
            else mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj,
                "WIFI REQUIRED", 108.0F, 211.0F, 0xFF, 0xA0, 0x40);
        }
    }
    else if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageSettings)
    {
        mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "MULTIPLAYER SETTINGS", 82.0F, 42.0F, 0xFF, 0x20, 0x20);
        for (i = 0; i < nMNModeSelectNetplaySettingsCount; i++)
        {
            y = 75.0F + (i * 27.0F);
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, settings_options[i], 56.0F, y,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0xD0,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0x20,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0x20);
        }
        port_netplay_get_player_name(player_name, ARRAY_COUNT(player_name));
        mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, player_name, 190.0F, 75.0F, 0xFF, 0xC0, 0x40);
        mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, mnModeSelectNetplayGetDelayText(), 190.0F, 102.0F, 0xFF, 0xC0, 0x40);
        mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj,
            port_netplay_get_show_stats() ? "ON" : "OFF", 190.0F, 129.0F, 0xFF, 0xC0, 0x40);
        mnModeSelectNetplayMakeMarker(sMNModeSelectNetplayGObj, 75.0F + (sMNModeSelectNetplayOption * 27.0F));
    }
    else if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageNameEdit)
    {
        mnModeSelectNetplayMakeNameEdit(sMNModeSelectNetplayGObj);
    }
    else if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageHost)
    {
        if (port_netplay_lobby_is_connected() && port_netplay_lobby_is_host())
        {
            mnModeSelectNetplayMakeLobby(sMNModeSelectNetplayGObj, "HOST GAME");
        }
        else
        {
            char error[64];

            error[0] = '\0';
            port_netplay_get_last_error(error, ARRAY_COUNT(error));
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "HOST GAME", 120.0F, 58.0F, 0xFF, 0x20, 0x20);
            if (error[0] != '\0')
            {
                mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "LOBBY OPEN FAILED",
                    92.0F, 104.0F, 0xFF, 0x80, 0x40);
                mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "CHECK NETPLAY LOG",
                    95.0F, 126.0F, 0xD0, 0xD0, 0xD0);
            }
            else mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj,
                port_netplay_mode_ready() ? "OPENING LOBBY" :
                ((port_netplay_get_mode() == PORT_NETPLAY_MODE_LOCAL_ADHOC) ? "ADHOC INITIALIZING" : "WIFI DISCONNECTED"),
                105.0F, 112.0F, 0xFF, 0xC0, 0x40);
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "B BACK", 135.0F, 180.0F, 0xFF, 0x40, 0x40);
        }
    }
    else if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageFind)
    {
        s32 count = port_netplay_get_discovery_count();

        if (port_netplay_lobby_is_connected())
        {
            mnModeSelectNetplayMakeLobby(sMNModeSelectNetplayGObj,
                (port_netplay_get_mode() == 1) ? "ADHOC LOBBY" : "ONLINE LOBBY");
        }
        else if (port_netplay_get_state() == 2)
        {
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "FIND GAME", 120.0F, 48.0F, 0xFF, 0x20, 0x20);
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "CONNECTING", 116.0F, 112.0F, 0xFF, 0xC0, 0x40);
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "B CANCEL", 126.0F, 190.0F, 0xFF, 0x40, 0x40);
        }
        else
        {
            char message[64];
            s32 players, max_players, ping, protocol, status, compatible;
            char host[20], ip[20], build[24];

            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "FIND GAME", 120.0F, 40.0F, 0xFF, 0x20, 0x20);
            if (count <= 0)
            {
                mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj,
                    (port_netplay_get_mode() == 1) ? "SEARCHING ADHOC" : "SEARCHING NETWORK",
                    (port_netplay_get_mode() == 1) ? 91.0F : 84.0F, 100.0F, 0xD0, 0xD0, 0xD0);
                port_netplay_get_last_error(message, ARRAY_COUNT(message));
                if (message[0] != '\0')
                {
                    mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, message, 80.0F, 130.0F, 0xFF, 0x80, 0x40);
                }
                mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "A RETRY", 74.0F, 190.0F, 0xFF, 0x40, 0x40);
                mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "B BACK", 205.0F, 190.0F, 0xFF, 0x40, 0x40);
            }
            else
            {
                mnModeSelectNetplayMakeDiscoveryDetail(sMNModeSelectNetplayGObj, 0, 82.0F);
                players = max_players = ping = protocol = status = compatible = 0;
                host[0] = ip[0] = build[0] = '\0';
                port_netplay_get_discovery_lobby(0, host, ARRAY_COUNT(host), ip, ARRAY_COUNT(ip),
                    build, ARRAY_COUNT(build), &players, &max_players, &ping, &protocol, &status, &compatible);
                mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, compatible ? "COMPATIBLE" : "INCOMPATIBLE",
                    105.0F, 128.0F, compatible ? 0xA0 : 0xFF, compatible ? 0xFF : 0x80, 0x40);
                if (compatible && (status == 0) && (players < max_players))
                {
                    mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "A CONFIRM JOIN", 89.0F, 173.0F, 0xFF, 0x40, 0x40);
                }
                else mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "JOIN UNAVAILABLE", 91.0F, 173.0F, 0x90, 0x90, 0x90);
                mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "START REFRESH  B BACK", 59.0F, 205.0F, 0xFF, 0x40, 0x40);
            }
        }
    }
    else
    {
        s32 count = port_netplay_get_discovery_count();
        s32 first;

        if (port_netplay_lobby_is_connected())
        {
            mnModeSelectNetplayMakeLobby(sMNModeSelectNetplayGObj,
                (port_netplay_get_mode() == 1) ? "ADHOC LOBBY" : "ONLINE LOBBY");
        }
        else if (port_netplay_get_state() == 2)
        {
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "JOIN GAME", 120.0F, 48.0F, 0xFF, 0x20, 0x20);
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "CONNECTING", 116.0F, 112.0F, 0xFF, 0xC0, 0x40);
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "B CANCEL", 126.0F, 190.0F, 0xFF, 0x40, 0x40);
        }
        else
        {
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "JOIN GAME", 120.0F, 34.0F, 0xFF, 0x20, 0x20);
            if (count <= 0)
            {
                sMNModeSelectNetplayOption = 0;
                mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "NO LOBBIES FOUND", 91.0F, 104.0F, 0xD0, 0xD0, 0xD0);
                mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "START REFRESH", 100.0F, 170.0F, 0xFF, 0x40, 0x40);
            }
            else
            {
                if (sMNModeSelectNetplayOption >= count) sMNModeSelectNetplayOption = count - 1;
                first = sMNModeSelectNetplayOption - 1;
                if (first < 0) first = 0;
                if (first > count - 3) first = count - 3;
                if (first < 0) first = 0;

                for (i = first; (i < count) && (i < first + 3); i++)
                {
                    y = 59.0F + ((i - first) * 49.0F);
                    mnModeSelectNetplayMakeDiscoveryDetail(sMNModeSelectNetplayGObj, i, y);
                    if (i == sMNModeSelectNetplayOption) mnModeSelectNetplayMakeMarker(sMNModeSelectNetplayGObj, y + 6.0F);
                }
                mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "A JOIN  START REFRESH  B BACK", 34.0F, 211.0F, 0xFF, 0x40, 0x40);
            }
        }
    }
}

static void mnModeSelectNetplayEnter(void)
{
    port_netplay_enter_menu();
    mnModeSelectEjectOptions();
    if (sMNModeSelectLabelsGObj != NULL)
    {
        gcEjectGObj(sMNModeSelectLabelsGObj);
        sMNModeSelectLabelsGObj = NULL;
    }
    sMNModeSelectNetplayPage = nMNModeSelectNetplayPageRoot;
    sMNModeSelectNetplayOption = 0;
    sMNModeSelectNetplayNavWait = 0;
    sMNModeSelectNetplayNameCursor = 0;
    mnModeSelectNetplayRefresh();
}

static void mnModeSelectNetplayExit(void)
{
    port_netplay_leave_menu();
    if (sMNModeSelectNetplayGObj != NULL)
    {
        gcEjectGObj(sMNModeSelectNetplayGObj);
        sMNModeSelectNetplayGObj = NULL;
    }
    mnModeSelectMakeOptions();
    mnModeSelectMakeLabels();
}

static void mnModeSelectNetplayMoveOption(s32 delta, s32 count)
{
    sMNModeSelectNetplayOption += delta;
    if (sMNModeSelectNetplayOption < 0) sMNModeSelectNetplayOption = count - 1;
    if (sMNModeSelectNetplayOption >= count) sMNModeSelectNetplayOption = 0;
    func_800269C0_275C0(nSYAudioFGMMenuScroll2);
    mnModeSelectNetplayRefresh();
}

static void mnModeSelectNetplayBeginNameEdit(void)
{
    char current[16];
    s32 i;

    port_netplay_get_player_name(current, ARRAY_COUNT(current));
    for (i = 0; i < 12; i++)
    {
        sMNModeSelectNetplayNameEdit[i] = (current[i] != '\0') ? current[i] : ' ';
    }
    sMNModeSelectNetplayNameEdit[12] = '\0';
    sMNModeSelectNetplayNameCursor = 0;
    sMNModeSelectNetplayPage = nMNModeSelectNetplayPageNameEdit;
    mnModeSelectNetplayRefresh();
}

static void mnModeSelectNetplayCycleNameLetter(s32 delta)
{
    char c = sMNModeSelectNetplayNameEdit[sMNModeSelectNetplayNameCursor];
    s32 index = (c == ' ') ? 0 : (c - 'A') + 1;

    index += delta;
    if (index < 0) index = 26;
    if (index > 26) index = 0;
    sMNModeSelectNetplayNameEdit[sMNModeSelectNetplayNameCursor] = (index == 0) ? ' ' : ('A' + index - 1);
    func_800269C0_275C0(nSYAudioFGMMenuScroll2);
    mnModeSelectNetplayRefresh();
}

static void mnModeSelectNetplayFuncRun(void)
{
    s32 stick_range;

    if (port_netplay_get_mode() == PORT_NETPLAY_MODE_LOCAL_ADHOC)
    {
        port_netplay_adhoc_dialog_tick();
        if (port_netplay_adhoc_dialog_state() == PORT_NETPLAY_ADHOC_DIALOG_RUNNING)
        {
            if ((sMNModeSelectTotalTimeTics % 15) == 0) mnModeSelectNetplayRefresh();
            return;
        }
    }

    if ((sMNModeSelectNetplayPage == nMNModeSelectNetplayPageMode) &&
        ((sMNModeSelectTotalTimeTics % 15) == 0))
    {
        mnModeSelectNetplayRefresh();
    }

    if (port_netplay_css_active())
    {
        /* Keep the network worker/session alive: this is a scene handoff, not
         * leaving Multiplayer.  The VS CSS will rebuild its four slots from
         * the authoritative lobby membership. */
        gSCManagerTransferBattleState.is_reset_players = TRUE;
        gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;
        gSCManagerSceneData.scene_curr = nSCKindPlayersVS;
        syTaskmanSetLoadScene();
        return;
    }

    sb32 up = scSubsysControllerGetPlayerTapButtons(U_JPAD | U_CBUTTONS);
    sb32 down = scSubsysControllerGetPlayerTapButtons(D_JPAD | D_CBUTTONS);
    sb32 left = scSubsysControllerGetPlayerTapButtons(L_JPAD | L_CBUTTONS);
    sb32 right = scSubsysControllerGetPlayerTapButtons(R_JPAD | R_CBUTTONS);
    sb32 action = scSubsysControllerGetPlayerTapButtons(A_BUTTON);
    sb32 start = scSubsysControllerGetPlayerTapButtons(START_BUTTON);
    sb32 accept = scSubsysControllerGetPlayerTapButtons(A_BUTTON | START_BUTTON);
    sb32 cancel = scSubsysControllerGetPlayerTapButtons(B_BUTTON);

    if (sMNModeSelectNetplayNavWait != 0) sMNModeSelectNetplayNavWait--;
    if (!up && !down && (sMNModeSelectNetplayNavWait == 0))
    {
        stick_range = scSubsysControllerGetPlayerStickUD(20, 1);
        if (stick_range != 0) up = TRUE;
        else
        {
            stick_range = scSubsysControllerGetPlayerStickUD(-20, 0);
            if (stick_range != 0) down = TRUE;
        }
    }

    if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageNameEdit)
    {
        if (up) mnModeSelectNetplayCycleNameLetter(+1);
        else if (down) mnModeSelectNetplayCycleNameLetter(-1);
        else if (left)
        {
            sMNModeSelectNetplayNameCursor = (sMNModeSelectNetplayNameCursor == 0) ? 11 : sMNModeSelectNetplayNameCursor - 1;
            func_800269C0_275C0(nSYAudioFGMMenuScroll2);
            mnModeSelectNetplayRefresh();
        }
        else if (right)
        {
            sMNModeSelectNetplayNameCursor = (sMNModeSelectNetplayNameCursor == 11) ? 0 : sMNModeSelectNetplayNameCursor + 1;
            func_800269C0_275C0(nSYAudioFGMMenuScroll2);
            mnModeSelectNetplayRefresh();
        }
        else if (accept)
        {
            port_netplay_set_player_name(sMNModeSelectNetplayNameEdit);
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageSettings;
            sMNModeSelectNetplayOption = nMNModeSelectNetplaySettingsName;
            func_800269C0_275C0(nSYAudioFGMMenuSelect);
            mnModeSelectNetplayRefresh();
        }
        else if (cancel)
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageSettings;
            sMNModeSelectNetplayOption = nMNModeSelectNetplaySettingsName;
            func_800269C0_275C0(nSYAudioFGMMenuScroll1);
            mnModeSelectNetplayRefresh();
        }
        return;
    }

    if ((sMNModeSelectNetplayPage == nMNModeSelectNetplayPageFind) ||
        (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageHost) ||
        (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageJoin))
    {
        if ((sMNModeSelectTotalTimeTics % 15) == 0)
        {
            mnModeSelectNetplayRefresh();
        }
        if (cancel)
        {
            port_netplay_cancel_activity();
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageMode;
            sMNModeSelectNetplayOption = 0;
            func_800269C0_275C0(nSYAudioFGMMenuScroll1);
            mnModeSelectNetplayRefresh();
            return;
        }

        if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageHost)
        {
            if (action && port_netplay_lobby_is_connected())
            {
                port_netplay_lobby_toggle_ready();
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
                mnModeSelectNetplayRefresh();
            }
            else if (start)
            {
                if (port_netplay_lobby_can_start())
                {
                    port_netplay_lobby_start();
                    func_800269C0_275C0(nSYAudioFGMMenuSelect);
                }
                else func_800269C0_275C0(nSYAudioFGMMenuDenied);
                mnModeSelectNetplayRefresh();
            }
            return;
        }

        if (port_netplay_lobby_is_connected())
        {
            if (action)
            {
                port_netplay_lobby_toggle_ready();
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
                mnModeSelectNetplayRefresh();
            }
            return;
        }

        if (port_netplay_get_state() == 2)
        {
            return;
        }

        if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageFind)
        {
            if (start || (action && (port_netplay_get_discovery_count() == 0)))
            {
                port_netplay_refresh_discovery();
                func_800269C0_275C0(nSYAudioFGMMenuScroll2);
            }
            else if (action)
            {
                if (port_netplay_join_discovered_lobby(0))
                {
                    func_800269C0_275C0(nSYAudioFGMMenuSelect);
                }
                else func_800269C0_275C0(nSYAudioFGMMenuDenied);
            }
            mnModeSelectNetplayRefresh();
            return;
        }

        if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageJoin)
        {
            s32 count = port_netplay_get_discovery_count();

            if ((count > 0) && up)
            {
                sMNModeSelectNetplayOption--;
                if (sMNModeSelectNetplayOption < 0) sMNModeSelectNetplayOption = count - 1;
                sMNModeSelectNetplayNavWait = 8;
                func_800269C0_275C0(nSYAudioFGMMenuScroll2);
            }
            else if ((count > 0) && down)
            {
                sMNModeSelectNetplayOption++;
                if (sMNModeSelectNetplayOption >= count) sMNModeSelectNetplayOption = 0;
                sMNModeSelectNetplayNavWait = 8;
                func_800269C0_275C0(nSYAudioFGMMenuScroll2);
            }
            if (start)
            {
                port_netplay_refresh_discovery();
                func_800269C0_275C0(nSYAudioFGMMenuScroll2);
            }
            else if (action && (count > 0))
            {
                if (port_netplay_join_discovered_lobby(sMNModeSelectNetplayOption))
                {
                    func_800269C0_275C0(nSYAudioFGMMenuSelect);
                }
                else func_800269C0_275C0(nSYAudioFGMMenuDenied);
            }
            mnModeSelectNetplayRefresh();
        }
        return;
    }

    if (up)
    {
        mnModeSelectNetplayMoveOption(-1,
            (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageSettings) ? nMNModeSelectNetplaySettingsCount :
            (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageMode) ? nMNModeSelectNetplayModeCount :
            nMNModeSelectNetplayRootCount);
        sMNModeSelectNetplayNavWait = 8;
    }
    else if (down)
    {
        mnModeSelectNetplayMoveOption(+1,
            (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageSettings) ? nMNModeSelectNetplaySettingsCount :
            (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageMode) ? nMNModeSelectNetplayModeCount :
            nMNModeSelectNetplayRootCount);
        sMNModeSelectNetplayNavWait = 8;
    }

    if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageSettings)
    {
        if ((sMNModeSelectNetplayOption == nMNModeSelectNetplaySettingsDelay) && (left || right || accept))
        {
            s32 delay = port_netplay_get_input_delay();
            delay += left ? -1 : +1;
            if (delay < -1) delay = 4;
            if (delay > 4) delay = -1;
            port_netplay_set_input_delay(delay);
            func_800269C0_275C0(nSYAudioFGMMenuScroll2);
            mnModeSelectNetplayRefresh();
            return;
        }
        if ((sMNModeSelectNetplayOption == nMNModeSelectNetplaySettingsStats) && (left || right || accept))
        {
            port_netplay_set_show_stats(!port_netplay_get_show_stats());
            func_800269C0_275C0(nSYAudioFGMMenuScroll2);
            mnModeSelectNetplayRefresh();
            return;
        }
        if (accept)
        {
            if (sMNModeSelectNetplayOption == nMNModeSelectNetplaySettingsName)
            {
                mnModeSelectNetplayBeginNameEdit();
                return;
            }
            if (sMNModeSelectNetplayOption == nMNModeSelectNetplaySettingsReset)
            {
                port_netplay_reset_settings();
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
                mnModeSelectNetplayRefresh();
                return;
            }
            if (sMNModeSelectNetplayOption == nMNModeSelectNetplaySettingsBack)
            {
                sMNModeSelectNetplayPage = nMNModeSelectNetplayPageRoot;
                sMNModeSelectNetplayOption = nMNModeSelectNetplayRootSettings;
                func_800269C0_275C0(nSYAudioFGMMenuScroll1);
                mnModeSelectNetplayRefresh();
                return;
            }
        }
        if (cancel)
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageRoot;
            sMNModeSelectNetplayOption = nMNModeSelectNetplayRootSettings;
            func_800269C0_275C0(nSYAudioFGMMenuScroll1);
            mnModeSelectNetplayRefresh();
        }
        return;
    }

    if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageMode)
    {
        if (accept)
        {
            if (sMNModeSelectNetplayOption == nMNModeSelectNetplayModeBack)
            {
                sMNModeSelectNetplayPage = nMNModeSelectNetplayPageRoot;
                sMNModeSelectNetplayOption = (port_netplay_get_mode() == 1) ?
                    nMNModeSelectNetplayRootAdhoc : nMNModeSelectNetplayRootOnline;
                func_800269C0_275C0(nSYAudioFGMMenuScroll1);
                mnModeSelectNetplayRefresh();
                return;
            }
            if (!port_netplay_mode_ready())
            {
                func_800269C0_275C0(nSYAudioFGMMenuDenied);
                mnModeSelectNetplayRefresh();
                return;
            }
            switch (sMNModeSelectNetplayOption)
            {
            case nMNModeSelectNetplayModeFind:
                sMNModeSelectNetplayPage = nMNModeSelectNetplayPageFind;
                sMNModeSelectNetplayOption = 0;
                port_netplay_start_discovery();
                break;
            case nMNModeSelectNetplayModeHost:
                sMNModeSelectNetplayPage = nMNModeSelectNetplayPageHost;
                sMNModeSelectNetplayOption = 0;
                port_netplay_host_lobby();
                break;
            case nMNModeSelectNetplayModeJoin:
                sMNModeSelectNetplayPage = nMNModeSelectNetplayPageJoin;
                sMNModeSelectNetplayOption = 0;
                port_netplay_start_discovery();
                break;
            }
            func_800269C0_275C0(nSYAudioFGMMenuSelect);
            mnModeSelectNetplayRefresh();
        }
        else if (cancel)
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageRoot;
            sMNModeSelectNetplayOption = (port_netplay_get_mode() == 1) ?
                nMNModeSelectNetplayRootAdhoc : nMNModeSelectNetplayRootOnline;
            func_800269C0_275C0(nSYAudioFGMMenuScroll1);
            mnModeSelectNetplayRefresh();
        }
        return;
    }

    if (accept)
    {
        switch (sMNModeSelectNetplayOption)
        {
        case nMNModeSelectNetplayRootAdhoc:
            port_netplay_set_mode(1);
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageMode;
            sMNModeSelectNetplayOption = 0;
            break;
        case nMNModeSelectNetplayRootOnline:
            port_netplay_set_mode(2);
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageMode;
            sMNModeSelectNetplayOption = 0;
            break;
        case nMNModeSelectNetplayRootSettings:
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageSettings;
            sMNModeSelectNetplayOption = 0;
            break;
        case nMNModeSelectNetplayRootBack:
            mnModeSelectNetplayExit();
            return;
        }
        func_800269C0_275C0(nSYAudioFGMMenuSelect);
        mnModeSelectNetplayRefresh();
    }
    else if (cancel)
    {
        func_800269C0_275C0(nSYAudioFGMMenuScroll1);
        mnModeSelectNetplayExit();
    }
}
#endif

// 0x80131B24
void mnModeSelectMake1PMode(void)
{
    GObj *gobj;
    SObj *sobj;

    sMNModeSelectOption1PModeGObj = gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT); 
    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);
    
    if (sMNModeSelectOption == nMNModeSelectOption1PMode)
    {
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainControllerIconSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainControllerIconSprite));
#endif

        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;

        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 169.0F;
        sobj->pos.y = 27.0F;

#if defined(REGION_JP)
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMain1PModeTextJapSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMain1PModeTextJapSprite));
#endif

        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;

        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 122.0F;
        sobj->pos.y = 195.0F;

#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonFrameSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], &llMNCommonFrameSprite));
#endif
        
        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;
        
        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 93.0F;
        sobj->pos.y = 189.0F;
#endif
    }
    else
    {
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainControllerIconDarkSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainControllerIconDarkSprite));
#endif
        
        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;
        
        sobj->sprite.red = 0x96;
        sobj->sprite.green = 0x96;
        sobj->sprite.blue = 0x96;
        
        sobj->pos.x = 169.0F;
        sobj->pos.y = 27.0F;
    }
}

// 0x80131C44
void mnModeSelectMakeVSMode(void)
{
    GObj *gobj;
    SObj *sobj;

    sMNModeSelectOptionVSModeGObj = gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT); 
    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);
    
    if (sMNModeSelectOption == nMNModeSelectOptionVSMode)
    {
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainConsoleIconSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainConsoleIconSprite));
#endif

        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;

        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 128.0F;
        sobj->pos.y = 64.0F;

#if defined(REGION_JP)
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainVsModeTextJapSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainVsModeTextJapSprite));
#endif

        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;

        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 126.0F;
        sobj->pos.y = 195.0F;

#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonFrameSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], &llMNCommonFrameSprite));
#endif
        
        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;
        
        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 93.0F;
        sobj->pos.y = 189.0F;
#endif
    }
    else
    {
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainConsoleIconDarkSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainConsoleIconDarkSprite));
#endif
        
        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;
        
        sobj->sprite.red = 0x96;
        sobj->sprite.green = 0x96;
        sobj->sprite.blue = 0x96;
        
        sobj->pos.x = 128.0F;
        sobj->pos.y = 64.0F;
    }
}

// 0x80131D68
void mnModeSelectMakeOption(void)
{
    GObj *gobj;
    SObj *sobj;

    sMNModeSelectOptionOptionGObj = gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT); 
    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);
    
    if (sMNModeSelectOption == nMNModeSelectOptionOption)
    {
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainSettingsIconSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainSettingsIconSprite));
#endif

        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;

        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 87.0F;
        sobj->pos.y = 101.0F;

#if defined(REGION_JP)
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainOptionTextJapSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainOptionTextJapSprite));
#endif

        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;

        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 112.0F;
        sobj->pos.y = 195.0F;

#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonFrameSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], &llMNCommonFrameSprite));
#endif
        
        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;
        
        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 93.0F;
        sobj->pos.y = 189.0F;
#endif
    }
    else
    {
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainSettingsIconDarkSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainSettingsIconDarkSprite));
#endif
        
        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;
        
        sobj->sprite.red = 0x96;
        sobj->sprite.green = 0x96;
        sobj->sprite.blue = 0x96;
        
        sobj->pos.x = 87.0F;
        sobj->pos.y = 101.0F;
    }
}

// 0x80131E8C
void mnModeSelectMakeData(void)
{
    GObj *gobj;
    SObj *sobj;

    sMNModeSelectOptionDataGObj = gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT); 
    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);
    
    if (sMNModeSelectOption == nMNModeSelectOptionData)
    {
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainDataIconSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainDataIconSprite));
#endif

        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;

        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 46.0F;
        sobj->pos.y = 138.0F;

#if defined(REGION_JP)
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainDataTextJapSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainDataTextJapSprite));
#endif

        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;

        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 126.0F;
        sobj->pos.y = 195.0F;

#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonFrameSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], &llMNCommonFrameSprite));
#endif
        
        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;
        
        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
    
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
        
        sobj->pos.x = 93.0F;
        sobj->pos.y = 189.0F;
#endif
    }
    else
    {
#ifdef PORT
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainDataIconDarkSprite));
#else
        sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainDataIconDarkSprite));
#endif
        
        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;
        
        sobj->sprite.red = 0x96;
        sobj->sprite.green = 0x96;
        sobj->sprite.blue = 0x96;
        
        sobj->pos.x = 46.0F;
        sobj->pos.y = 138.0F;
    }
}

#if defined(PORT) && defined(__vita__)
void mnModeSelectMakeMultiplayer(void)
{
    GObj *gobj;
    SObj *sobj;

    sMNModeSelectOptionMultiplayerGObj = gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);

    if (sMNModeSelectOption == nMNModeSelectOptionMultiplayer)
    {
        sobj = lbCommonMakeSObjForGObj(gobj,
            lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainConsoleIconSprite));
        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0xFF;
        sobj->sprite.blue = 0xFF;
        sobj->envcolor.r = 0x00;
        sobj->envcolor.g = 0x00;
        sobj->envcolor.b = 0x00;
    }
    else
    {
        sobj = lbCommonMakeSObjForGObj(gobj,
            lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainConsoleIconDarkSprite));
        sobj->sprite.red = 0x96;
        sobj->sprite.green = 0x96;
        sobj->sprite.blue = 0x96;
    }
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->pos.x = 5.0F;
    sobj->pos.y = 175.0F;

    mnModeSelectNetplayMakeString(gobj, "MULTIPLAYER", 57.0F, 202.0F, 0xFF, 0x00, 0x00);
}
#endif

// 0x80131FB0
void mnModeSelectMakeLabels(void)
{
    GObj *gobj;
    SObj *sobj;

    gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT);
#if defined(PORT) && defined(__vita__)
    sMNModeSelectLabelsGObj = gobj;
#endif
    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMain1PModeTextSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMain1PModeTextSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;

    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0x00;
    sobj->sprite.blue = 0x00;
    
    sobj->pos.x = 224.0F;
    sobj->pos.y = 52.0F;
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainVsModeTextSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainVsModeTextSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;

    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0x00;
    sobj->sprite.blue = 0x00;
    
    sobj->pos.x = 183.0F;
    sobj->pos.y = 89.0F;
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainOptionTextSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainOptionTextSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;

    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0x00;
    sobj->sprite.blue = 0x00;
    
    sobj->pos.x = 142.0F;
    sobj->pos.y = 126.0F;
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainDataTextSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainDataTextSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;

    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0x00;
    sobj->sprite.blue = 0x00;
    
    sobj->pos.x = 102.0F;
    sobj->pos.y = 163.0F;
}

// 0x80132168
void mnModeSelectMakeDecals(void)
{
    GObj *gobj;
    SObj *sobj;

    gobj = gcMakeGObjSPAfter(0, NULL, 2, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 0, GOBJ_PRIORITY_DEFAULT, ~0);
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonSmashBrosCollageSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], &llMNCommonSmashBrosCollageSprite));
#endif
    
    sobj->pos.x = 10.0F;
    sobj->pos.y = 10.0F;
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainDecalBarMiddleSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainDecalBarMiddleSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->sprite.red = 0x08;
    sobj->sprite.green = 0x33;
    sobj->sprite.blue = 0x65;
    
    sobj->cms = 0;
    sobj->cmt = 0;
    
    sobj->masks = 4;
    sobj->maskt = 0;
    
    sobj->lrs = 96;
    sobj->lrt = 38;
    
    sobj->pos.x = 0.0f;
    sobj->pos.y = 37.0f;
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainDecalBarEdgeSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainDecalBarEdgeSprite));
#endif

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->sprite.red = 0x08;
    sobj->sprite.green = 0x33;
    sobj->sprite.blue = 0x65;
    
    sobj->pos.x = 96.0F;
    sobj->pos.y = 37.0F;
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainModeSelectTextSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainModeSelectTextSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->envcolor.r = 0x00;
    sobj->envcolor.g = 0x00;
    sobj->envcolor.b = 0x00;
    
    sobj->sprite.red = 0x3C;
    sobj->sprite.green = 0x73;
    sobj->sprite.blue = 0xB4;
    
    sobj->pos.x = 28.0F;
    sobj->pos.y = 27.0F;
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainSmashLogoSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], &llMNMainSmashLogoSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->sprite.red = 0x08;
    sobj->sprite.green = 0x33;
    sobj->sprite.blue = 0x65;
    
    sobj->pos.x = 226.0F;
    sobj->pos.y = 137.0F;
}

// 0x80132398
void mnModeSelectMakeLabelsCamera(void)
{
    CObj *cobj = CObjGetStruct
    (
        gcMakeCameraGObj
        (
            1,
            NULL,
            1,
            GOBJ_PRIORITY_DEFAULT,
            lbCommonDrawSprite,
            60,
            COBJ_MASK_DLLINK(1),
            ~0,
            FALSE,
            nGCProcessKindFunc,
            NULL,
            1,
            FALSE
        )
    );
    syRdpSetViewport(&cobj->viewport, 10.0F, 10.0F, 310.0F, 230.0F);
}

// 0x80132438
void mnModeSelectMakeDecalsCamera(void)
{
    CObj *cobj = CObjGetStruct
    (
        gcMakeCameraGObj
        (
            1,
            NULL,
            1,
            GOBJ_PRIORITY_DEFAULT,
            lbCommonDrawSprite,
            80,
            COBJ_MASK_DLLINK(0),
            ~0,
            FALSE,
            nGCProcessKindFunc,
            NULL,
            1,
            FALSE
        )
    );
    syRdpSetViewport(&cobj->viewport, 10.0F, 10.0F, 310.0F, 230.0F);
}

// 0x801324D8
void mnModeSelectMakeOptions(void)
{
    mnModeSelectMake1PMode();
    mnModeSelectMakeVSMode();
    mnModeSelectMakeOption();
    mnModeSelectMakeData();
}

// 0x80132510
void mnModeSelectEjectOptions(void)
{
    gcEjectGObj(sMNModeSelectOption1PModeGObj);
    gcEjectGObj(sMNModeSelectOptionVSModeGObj);
    gcEjectGObj(sMNModeSelectOptionOptionGObj);
    gcEjectGObj(sMNModeSelectOptionDataGObj);
#if defined(PORT) && defined(__vita__)
    if (sMNModeSelectOptionMultiplayerGObj != NULL)
    {
        gcEjectGObj(sMNModeSelectOptionMultiplayerGObj);
    }
    sMNModeSelectOption1PModeGObj = NULL;
    sMNModeSelectOptionVSModeGObj = NULL;
    sMNModeSelectOptionOptionGObj = NULL;
    sMNModeSelectOptionDataGObj = NULL;
    sMNModeSelectOptionMultiplayerGObj = NULL;
#endif
}

// 0x80132558
void mnModeSelectInitVars(void)
{
    switch (gSCManagerSceneData.scene_prev)
    {
    default:
        sMNModeSelectOption = nMNModeSelectOption1PMode;
        break;
        
    case nSCKind1PMode:
        sMNModeSelectOption = nMNModeSelectOption1PMode;
        break;
        
    case nSCKindVSMode:
        sMNModeSelectOption = nMNModeSelectOptionVSMode;
        break;
        
    case nSCKindOption:
        sMNModeSelectOption = nMNModeSelectOptionOption;
        break;
        
    case nSCKindData:
        sMNModeSelectOption = nMNModeSelectOptionData;
        break;
    }
    sMNModeSelectOptionChangeWait = 0;
#if defined(PORT) && defined(__vita__)
    sMNModeSelectLabelsGObj = NULL;
    sMNModeSelectNetplayGObj = NULL;
    sMNModeSelectNetplayPage = nMNModeSelectNetplayPageRoot;
    sMNModeSelectNetplayOption = 0;
    sMNModeSelectNetplayNavWait = 0;
#endif
    
    sMNModeSelectTotalTimeTics = 0;
    sMNModeSelectReturnTic = sMNModeSelectTotalTimeTics + I_MIN_TO_TICS(5);
}

// 0x801325E8
void mnModeSelectFuncRun(GObj *gobj)
{
    s32 unused1;
    s32 stick_range;
    s32 unused2;
    sb32 is_button;

    sMNModeSelectTotalTimeTics++;

#if defined(PORT) && defined(__vita__)
    if (sMNModeSelectNetplayGObj != NULL)
    {
        sMNModeSelectReturnTic = sMNModeSelectTotalTimeTics + I_MIN_TO_TICS(5);
        if ((sMNModeSelectTotalTimeTics % 30) == 0)
        {
            mnModeSelectNetplayRefresh();
        }
        mnModeSelectNetplayFuncRun();
        return;
    }
#endif
    
    if (sMNModeSelectTotalTimeTics >= 10)
    {
        if (sMNModeSelectTotalTimeTics == sMNModeSelectReturnTic)
        {
            gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;
            gSCManagerSceneData.scene_curr = nSCKindTitle;

            syTaskmanSetLoadScene();
            return;
        }
        if (scSubsysControllerCheckNoInputAll() == FALSE)
        {
            sMNModeSelectReturnTic = sMNModeSelectTotalTimeTics + I_MIN_TO_TICS(5);
        }
        if (sMNModeSelectOptionChangeWait != 0)
        {
            sMNModeSelectOptionChangeWait--;
        }
        if
        (
            (scSubsysControllerGetPlayerStickInRangeLR(-20, 20) != FALSE)                                &&
            (scSubsysControllerGetPlayerStickInRangeUD(-20, 20) != FALSE)                                &&
            (scSubsysControllerGetPlayerHoldButtons(U_JPAD | R_JPAD | U_CBUTTONS | R_CBUTTONS) == FALSE) &&
            (scSubsysControllerGetPlayerHoldButtons(D_JPAD | L_JPAD | D_CBUTTONS | L_CBUTTONS) == FALSE)
        )
        {
            sMNModeSelectOptionChangeWait = 0;
        }
        if (scSubsysControllerGetPlayerTapButtons(A_BUTTON | START_BUTTON) != FALSE)
        {
            switch (sMNModeSelectOption)
            {
            case nMNModeSelectOption1PMode:
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
                
                gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;
                gSCManagerSceneData.scene_curr = nSCKind1PMode;
                
                syTaskmanSetLoadScene();
                return;
                
            case nMNModeSelectOptionVSMode:
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
                
                gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;
                gSCManagerSceneData.scene_curr = nSCKindVSMode;
                
                syTaskmanSetLoadScene();
                return;
                
            case nMNModeSelectOptionOption:
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
                
                gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;
                gSCManagerSceneData.scene_curr = nSCKindOption;
                
                syTaskmanSetLoadScene();
                return;
                
            case nMNModeSelectOptionData:
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
                
                gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;
                gSCManagerSceneData.scene_curr = nSCKindData;
                
                syTaskmanSetLoadScene();
                return;
#if defined(PORT) && defined(__vita__)

            case nMNModeSelectOptionMultiplayer:
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
                mnModeSelectNetplayEnter();
                return;
#endif
            }
        }
        else
        {
            if (scSubsysControllerGetPlayerTapButtons(B_BUTTON) != FALSE)
            {
                syAudioStopBGMAll();

                gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;
                gSCManagerSceneData.scene_curr = nSCKindTitle;
            
                syTaskmanSetLoadScene();
            }
            if
            (
                (
                    (scSubsysControllerGetPlayerStickUD(20, 1) <= 0) ||
                    (scSubsysControllerGetPlayerStickLR(-20, 0) >= 0)
                )
                &&
                (
                    (scSubsysControllerGetPlayerStickUD(-20, 0) >= 0) ||
                    (scSubsysControllerGetPlayerStickLR(20, 1) <= 0)
                )
            )
            {
                if
                (
                    mnModeSelectCheckGetOptionButtonInput(is_button, U_JPAD | R_JPAD | U_CBUTTONS | R_CBUTTONS) ||
                    (sMNModeSelectOptionChangeWait == 0)
                    &&
                    (
                        ((stick_range) = scSubsysControllerGetPlayerStickUD(20, 1), stick_range != 0) ||
                        ((stick_range) = scSubsysControllerGetPlayerStickLR(20, 1), stick_range != 0)
                    )
                )
                {    
                    func_800269C0_275C0(nSYAudioFGMMenuScroll2);
                    
                    mnModeSelectSetOptionChangeWaitP(is_button, stick_range, 7);
                        
                    if (sMNModeSelectOption == nMNModeSelectOptionStart)
                    {
                        sMNModeSelectOption = nMNModeSelectOptionEnd;
                    }
                    else sMNModeSelectOption--;
                    
                    if (sMNModeSelectOption == nMNModeSelectOptionStart)
                    {
                        sMNModeSelectOptionChangeWait += 8;
                    }
                    mnModeSelectEjectOptions();
                    mnModeSelectMakeOptions();
                }
                if
                (
                    mnModeSelectCheckGetOptionButtonInput(is_button, D_JPAD | L_JPAD | D_CBUTTONS | L_CBUTTONS) ||
                    (sMNModeSelectOptionChangeWait == 0)
                    &&
                    (
                        ((stick_range) = scSubsysControllerGetPlayerStickUD(-20, 0), stick_range != 0) ||
                        ((stick_range) = scSubsysControllerGetPlayerStickLR(-20, 0), stick_range != 0)
                    )
                )
                {    
                    func_800269C0_275C0(nSYAudioFGMMenuScroll2);
                    
                    mnModeSelectSetOptionChangeWaitN(is_button, stick_range, 7);
                        
                    if (sMNModeSelectOption == nMNModeSelectOptionEnd)
                    {
                        sMNModeSelectOption = nMNModeSelectOptionStart;
                    }
                    else sMNModeSelectOption++;
                    
                    if (sMNModeSelectOption == nMNModeSelectOptionEnd)
                    {
                        sMNModeSelectOptionChangeWait += 8;
                    }
                    mnModeSelectEjectOptions();
                    mnModeSelectMakeOptions();
                }
            }
        }
    }
}

// 0x80132A0C
void mnModeSelectFuncStart(void)
{
    LBRelocSetup rl_setup;

    rl_setup.table_addr = (uintptr_t)&lLBRelocTableAddr;
#ifdef PORT
    rl_setup.table_files_num = (u32)llRelocFileCount;
#else
    rl_setup.table_files_num = (u32)&llRelocFileCount;
#endif
    rl_setup.file_heap = NULL;
    rl_setup.file_heap_size = 0;
    rl_setup.status_buffer = sMNModeSelectStatusBuffer;
    rl_setup.status_buffer_size = ARRAY_COUNT(sMNModeSelectStatusBuffer);
    rl_setup.force_status_buffer = NULL;
    rl_setup.force_status_buffer_size = 0;
    
    lbRelocInitSetup(&rl_setup);
    lbRelocLoadFilesListed(dMNModeSelectFileIDs, sMNModeSelectFiles);
    gcMakeGObjSPAfter(0, mnModeSelectFuncRun, 0, GOBJ_PRIORITY_DEFAULT);
    gcMakeDefaultCameraGObj(0, GOBJ_PRIORITY_DEFAULT, 100, 0, GPACK_RGBA8888(0x00, 0x00, 0x00, 0x00));
    
    mnModeSelectInitVars();
    mnModeSelectMakeDecalsCamera();
    mnModeSelectMakeLabelsCamera();
    mnModeSelectMakeDecals();
    mnModeSelectMakeOptions();
    mnModeSelectMakeLabels();
    
    if
    (
        (gSCManagerSceneData.scene_prev != nSCKind1PMode) &&
        (gSCManagerSceneData.scene_prev != nSCKindVSMode) &&
        (gSCManagerSceneData.scene_prev != nSCKindOption) &&
        (gSCManagerSceneData.scene_prev != nSCKindData)
    )
    {
        syAudioPlayBGM(0, nSYAudioBGMModeSelect);
    }
}

// 0x80132B34
void mnModeSelectStartScene(void)
{
    dMNModeSelectVideoSetup.zbuffer = SYVIDEO_ZBUFFER_START(320, 240, 0, 10, u16);
    syVideoInit(&dMNModeSelectVideoSetup);
    
    dMNModeSelectTaskmanSetup.scene_setup.arena_size = (size_t) ((uintptr_t)&ovl1_VRAM - (uintptr_t)&ovl17_BSS_END);
    syTaskmanStartTask(&dMNModeSelectTaskmanSetup);
}
