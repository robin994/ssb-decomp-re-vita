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
#include <it/itdef.h>
#include <netplay/netplay_bridge.h>
#include <sys/netreplay.h>
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
sb32 gMNNetplayOverlayRequest;
sb32 gMNNetplayReturnToVSMode;
GObj *sMNModeSelectLabelsGObj;
GObj *sMNModeSelectNetplayGObj;
s32 sMNModeSelectNetplayPage;
s32 sMNModeSelectNetplayOption;
s32 sMNModeSelectNetplayNavWait;
s32 sMNModeSelectNetplayImeTarget;

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
    nMNModeSelectNetplayPageTextEntry,
    nMNModeSelectNetplayPageHostRules,
    nMNModeSelectNetplayPageItemToggles

} MNModeSelectNetplayPage;

enum
{
    nMNModeSelectNetplayRuleStage,
    nMNModeSelectNetplayRuleLives,
    nMNModeSelectNetplayRuleTime,
    nMNModeSelectNetplayRuleItems,
    nMNModeSelectNetplayRuleItemList,
    nMNModeSelectNetplayRuleTeam,
    nMNModeSelectNetplayRuleFriendlyFire,
    nMNModeSelectNetplayRuleDamage,
    nMNModeSelectNetplayRuleHandicap,
    nMNModeSelectNetplayRuleCount
};

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
    nMNModeSelectNetplayModeDirect,
    nMNModeSelectNetplayModeBack,
    nMNModeSelectNetplayModeCount
};

enum
{
    nMNModeSelectNetplaySettingsName,
    nMNModeSelectNetplaySettingsDelay,
    nMNModeSelectNetplaySettingsStats,
    nMNModeSelectNetplaySettingsServer,
    nMNModeSelectNetplaySettingsDeterminism,
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

static const intptr_t dMNModeSelectNetplayMenuDigits[10] =
{
    llMNCommonDigit0Sprite, llMNCommonDigit1Sprite, llMNCommonDigit2Sprite, llMNCommonDigit3Sprite,
    llMNCommonDigit4Sprite, llMNCommonDigit5Sprite, llMNCommonDigit6Sprite, llMNCommonDigit7Sprite,
    llMNCommonDigit8Sprite, llMNCommonDigit9Sprite
};

static f32 mnModeSelectNetplayCharSpacing(const char *str, s32 c)
{
    switch (str[c])
    {
    case 'A':
        switch (str[c + 1])
        {
        case 'F': case 'P': case 'T': case 'V': case 'Y':
            return 0.0F;
        default:
            return 1.0F;
        }
    case 'F': case 'P': case 'V': case 'Y':
        switch (str[c + 1])
        {
        case 'A': case 'T':
            return 0.0F;
        default:
            return 1.0F;
        }
    case 'Q': case 'T':
        switch (str[c + 1])
        {
        case '.':
            return 1.0F;
        default:
            return 0.0F;
        }
    case '.':
        return 1.0F;
    default:
        return (str[c + 1] == 'T') ? 0.0F : 1.0F;
    }
}

static f32 mnModeSelectNetplayMakeStringScaled(GObj *gobj, const char *str, f32 x, f32 y,
    u8 red, u8 green, u8 blue, f32 scale)
{
    SObj *sobj;
    s32 i;

    for (i = 0; str[i] != '\0'; i++)
    {
        char ch = str[i];

        if (ch == ' ')
        {
            x += 5.0F * scale;
            continue;
        }
        if ((ch >= '0') && (ch <= '9'))
        {
            sobj = lbCommonMakeSObjForGObj(gobj,
                lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], dMNModeSelectNetplayMenuDigits[ch - '0']));
        }
        else if (ch == ':')
        {
            sobj = lbCommonMakeSObjForGObj(gobj,
                lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonColonSprite));
        }
        else if (ch == '%')
        {
            sobj = lbCommonMakeSObjForGObj(gobj,
                lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonPercentageSprite));
        }
        else if (ch == '-')
        {
            sobj = lbCommonMakeSObjForGObj(gobj,
                lbRelocGetFileData(Sprite*, sMNModeSelectFiles[3], llIFCommonDigitsDashSprite));
        }
        else if (ch == '.')
        {
            sobj = lbCommonMakeSObjForGObj(gobj,
                lbRelocGetFileData(Sprite*, sMNModeSelectFiles[2], llMNCommonFontsSymbolPeriodSprite));
        }
        else if ((ch >= 'A') && (ch <= 'Z'))
        {
            sobj = lbCommonMakeSObjForGObj(gobj,
                lbRelocGetFileData(Sprite*, sMNModeSelectFiles[2], dMNModeSelectNetplayFontOffsets[ch - 'A']));
        }
        else continue;

        sobj->sprite.attr &= ~SP_FASTCOPY;
        sobj->sprite.attr |= SP_TRANSPARENT;
        sobj->sprite.red = red;
        sobj->sprite.green = green;
        sobj->sprite.blue = blue;
        sobj->sprite.scalex = scale;
        sobj->sprite.scaley = scale;
        sobj->pos.x = x;
        sobj->pos.y = (ch == '.') ? y + (3.0F * scale) : y;
        x += (sobj->sprite.width * scale) + (mnModeSelectNetplayCharSpacing(str, i) * scale);
    }
    return x;
}

static f32 mnModeSelectNetplayMakeString(GObj *gobj, const char *str, f32 x, f32 y, u8 red, u8 green, u8 blue)
{
    return mnModeSelectNetplayMakeStringScaled(gobj, str, x, y, red, green, blue, 1.0F);
}

static void mnModeSelectNetplayUpdateTab(SObj *head, s32 status);
static void mnModeSelectNetplayMakeCursor(GObj *gobj, f32 x, f32 y);

static SObj *mnModeSelectNetplayMakeTab(GObj *gobj, f32 x, f32 y, s32 width_units, s32 status)
{
    SObj *first;
    SObj *sobj;

    first = sobj = lbCommonMakeSObjForGObj(gobj,
        lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonOptionTabLeftSprite));
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->pos.x = x;
    sobj->pos.y = y;

    sobj = lbCommonMakeSObjForGObj(gobj,
        lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonOptionTabMiddleSprite));
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->pos.x = x + 16.0F;
    sobj->pos.y = y;
    sobj->cms = 0;
    sobj->cmt = 0;
    sobj->masks = 4;
    sobj->maskt = 0;
    sobj->lrs = width_units * 8;
    sobj->lrt = 0x1D;

    sobj = lbCommonMakeSObjForGObj(gobj,
        lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonOptionTabRightSprite));
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->pos.x = x + 16.0F + (width_units * 8);
    sobj->pos.y = y;

    mnModeSelectNetplayUpdateTab(first, status);
    return first;
}

static void mnModeSelectNetplayMakeTabRow(GObj *gobj, f32 x, f32 y, s32 width_units,
    const char *label, sb32 is_hot)
{
    mnModeSelectNetplayMakeTab(gobj, x, y, width_units,
        is_hot ? nMNOptionTabStatusHighlight : nMNOptionTabStatusNot);
    mnModeSelectNetplayMakeString(gobj, label, x + 20.0F, y + 3.0F, 0x00, 0x00, 0x00);
    if (is_hot)
    {
        mnModeSelectNetplayMakeCursor(gobj, x - 12.0F, y + 1.0F);
    }
}

static void mnModeSelectNetplayUpdateTab(SObj *head, s32 status)
{
    u8 prim_r, prim_g, prim_b;
    u8 env_r, env_g, env_b;
    s32 i;

    switch (status)
    {
    case nMNOptionTabStatusHighlight:
        prim_r = 0x82; prim_g = 0x00; prim_b = 0x28;
        env_r = 0xFF; env_g = 0x00; env_b = 0x28;
        break;
    case nMNOptionTabStatusSelected:
        prim_r = 0x00; prim_g = 0x00; prim_b = 0x00;
        env_r = 0xFF; env_g = 0xFF; env_b = 0xFF;
        break;
    default:
        prim_r = 0x00; prim_g = 0x00; prim_b = 0x00;
        env_r = 0x82; env_g = 0x82; env_b = 0xAA;
        break;
    }
    for (i = 0; (i < 3) && (head != NULL); i++)
    {
        head->envcolor.r = prim_r;
        head->envcolor.g = prim_g;
        head->envcolor.b = prim_b;
        head->sprite.red = env_r;
        head->sprite.green = env_g;
        head->sprite.blue = env_b;
        head = head->next;
    }
}

static void mnModeSelectNetplayMakeFrame(GObj *gobj, f32 x, f32 y, u8 red, u8 green, u8 blue)
{
    SObj *sobj = lbCommonMakeSObjForGObj(gobj,
        lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonFrameSprite));

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->envcolor.r = 0x00;
    sobj->envcolor.g = 0x00;
    sobj->envcolor.b = 0x00;
    sobj->sprite.red = red;
    sobj->sprite.green = green;
    sobj->sprite.blue = blue;
    sobj->pos.x = x;
    sobj->pos.y = y;
}

static void mnModeSelectNetplayMakeCursor(GObj *gobj, f32 x, f32 y)
{
    SObj *sobj = lbCommonMakeSObjForGObj(gobj,
        lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonArrowRSprite));

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0xAE;
    sobj->sprite.blue = 0x00;
    sobj->pos.x = x;
    sobj->pos.y = y;
}

static void mnModeSelectNetplayMakeBackground(void)
{
    GObj *gobj = gcMakeGObjSPAfter(0, NULL, 2, GOBJ_PRIORITY_DEFAULT);
    SObj *sobj;

    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 0, GOBJ_PRIORITY_DEFAULT, ~0);

    sobj = lbCommonMakeSObjForGObj(gobj,
        lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonSmashBrosCollageSprite));
    sobj->pos.x = 10.0F;
    sobj->pos.y = 10.0F;

    sobj = lbCommonMakeSObjForGObj(gobj,
        lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonDecalPaperSprite));
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->sprite.red = 0xA0;
    sobj->sprite.green = 0x78;
    sobj->sprite.blue = 0x14;
    sobj->pos.x = 140.0F;
    sobj->pos.y = 143.0F;

    sobj = lbCommonMakeSObjForGObj(gobj,
        lbRelocGetFileData(Sprite*, sMNModeSelectFiles[0], llMNCommonDecalPaperSprite));
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->sprite.red = 0xA0;
    sobj->sprite.green = 0x78;
    sobj->sprite.blue = 0x14;
    sobj->pos.x = 225.0F;
    sobj->pos.y = 56.0F;

    sobj = lbCommonMakeSObjForGObj(gobj,
        lbRelocGetFileData(Sprite*, sMNModeSelectFiles[1], llMNMainConsoleIconDarkSprite));
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->sprite.red = 0x99;
    sobj->sprite.green = 0x99;
    sobj->sprite.blue = 0x99;
    sobj->pos.x = 10.0F;
    sobj->pos.y = 10.0F;
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

static s32 sMNModeSelectNetplayRuleCursor;
static s32 sMNModeSelectNetplayItemCursor;
static s32 sMNModeSelectNetplayItemScroll;

#define MNNP_ITEM_LIST_COUNT 15
#define MNNP_ITEM_LIST_VIEW 8
#define MNNP_ITEM_CONTAINER_BITS \
    (ITEM_TOGGLE_MASK_KIND(nITKindBox) | ITEM_TOGGLE_MASK_KIND(nITKindTaru) | \
     ITEM_TOGGLE_MASK_KIND(nITKindCapsule) | ITEM_TOGGLE_MASK_KIND(nITKindEgg))
#define MNNP_ITEM_UTILITY_BITS \
    ((ITEM_TOGGLE_MASK_KIND(nITKindUtilityEnd + 1) - 1) & ~(ITEM_TOGGLE_MASK_KIND(nITKindUtilityStart) - 1))

static const s32 sMNModeSelectNetplayItemKinds[MNNP_ITEM_LIST_COUNT] =
{
    nITKindSword, nITKindBat, nITKindHammer, nITKindHarisen, nITKindMSBomb,
    nITKindBombHei, nITKindNBumper, nITKindGShell, nITKindMBall, nITKindLGun,
    nITKindFFlower, nITKindStarRod, nITKindTomato, nITKindHeart, nITKindStar
};

static const char *const sMNModeSelectNetplayItemNames[MNNP_ITEM_LIST_COUNT] =
{
    "BEAM SWORD", "HOME RUN BAT", "HAMMER", "FAN", "MOTION SENSOR",
    "BOB OMB", "BUMPER", "SHELL", "POKE BALL", "RAY GUN",
    "FIRE FLOWER", "STAR ROD", "MAXIM TOMATO", "HEART", "STAR MAN"
};

static const char* mnModeSelectNetplayGetItemRateName(s32 rate)
{
    static const char *const names[] =
    {
        "OFF", "VERY LOW", "LOW", "MIDDLE", "HIGH", "VERY HIGH"
    };
    if ((rate < 0) || (rate >= (s32)ARRAY_COUNT(names))) return "RANDOM";
    return names[rate];
}

static const char* mnModeSelectNetplayGetHandicapName(s32 mode)
{
    switch (mode)
    {
    case nSCBattleHandicapOn: return "ON";
    case nSCBattleHandicapAuto: return "AUTO";
    default: return "OFF";
    }
}

static void mnModeSelectNetplayFormatDamage(s32 value, char *buf, s32 buf_size)
{
    if (value < 0) snprintf(buf, buf_size, "RANDOM");
    else snprintf(buf, buf_size, "%d", value);
}

static s32 mnModeSelectNetplayCycleItemRate(s32 value, s32 dir)
{
    s32 next = value + dir;
    if (next < -1) next = nSCBattleItemSwitchEnumCount - 1;
    if (next >= nSCBattleItemSwitchEnumCount) next = -1;
    return next;
}

static s32 mnModeSelectNetplayCycleDamage(s32 value, s32 dir)
{
    if (value < 0) return (dir > 0) ? 50 : 200;
    value += dir * 10;
    if (value < 50) return -1;
    if (value > 200) return -1;
    return value;
}

static s32 mnModeSelectNetplayCycleHandicap(s32 value, s32 dir)
{
    s32 next = value + dir;
    if (next < 0) next = nSCBattleHandicapAuto;
    if (next > nSCBattleHandicapAuto) next = 0;
    return next;
}

static u32 mnModeSelectNetplayNormalizeItemMask(u32 mask)
{
    if ((mask & MNNP_ITEM_UTILITY_BITS) == 0) return 0;
    return (mask & MNNP_ITEM_UTILITY_BITS) | MNNP_ITEM_CONTAINER_BITS;
}

static void mnModeSelectNetplayToggleItem(s32 index)
{
    s32 kind = sMNModeSelectNetplayItemKinds[index];
    u32 mask = (u32)port_netplay_hostrules_get_item_toggles();
    u32 bit = ITEM_TOGGLE_MASK_KIND(kind);

    if (kind == nITKindGShell) bit |= ITEM_TOGGLE_MASK_KIND(nITKindRShell);
    if (mask & ITEM_TOGGLE_MASK_KIND(kind)) mask &= ~bit;
    else mask |= bit;
    port_netplay_hostrules_set_item_toggles((s32)mnModeSelectNetplayNormalizeItemMask(mask));
}

static const char* mnModeSelectNetplayGetStageName(s32 idx)
{
    static const char *const names[] =
    {
        "CASTLE", "SECTOR Z", "KONGO", "ZEBES", "HYRULE",
        "YOSHI", "DREAM", "SAFFRON", "MUSHROOM"
    };
    if ((idx < 0) || (idx >= (s32)ARRAY_COUNT(names))) return "RANDOM";
    return names[idx];
}

static void mnModeSelectNetplayFormatStocks(s32 value, char *buf, s32 buf_size)
{
    if (value < 0) snprintf(buf, buf_size, "RANDOM");
    else snprintf(buf, buf_size, "%d", value);
}

static void mnModeSelectNetplayFormatTime(s32 units, char *buf, s32 buf_size)
{
    if (units < 0) snprintf(buf, buf_size, "RANDOM");
    else if (units == 0) snprintf(buf, buf_size, "INFINITE");
    else
    {
        s32 secs = units * 30;
        snprintf(buf, buf_size, "%d.%02d", secs / 60, secs % 60);
    }
}

static s32 mnModeSelectNetplayCycleStage(s32 value, s32 dir)
{
    s32 next = value + dir;
    if (next < -1) next = 8;
    if (next > 8) next = -1;
    return next;
}

static s32 mnModeSelectNetplayCycleStocks(s32 value, s32 dir)
{
    s32 next;
    if (value < 0) next = (dir > 0) ? 1 : 5;
    else
    {
        next = value + dir;
        if (next < 1) next = -1;
        if (next > 5) next = -1;
    }
    return next;
}

static s32 mnModeSelectNetplayCycleTime(s32 value, s32 dir)
{
    static const s32 order[] = { -1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    s32 i;
    for (i = 0; i < (s32)ARRAY_COUNT(order); i++)
    {
        if (order[i] == value)
        {
            i = (i + dir + (s32)ARRAY_COUNT(order)) % (s32)ARRAY_COUNT(order);
            return order[i];
        }
    }
    return -1;
}

static void mnModeSelectNetplayCycleRule(s32 cursor, s32 dir)
{
    switch (cursor)
    {
    case nMNModeSelectNetplayRuleStage:
        port_netplay_hostrules_set_stage(mnModeSelectNetplayCycleStage(port_netplay_hostrules_get_stage(), dir));
        break;
    case nMNModeSelectNetplayRuleLives:
        port_netplay_hostrules_set_stocks(mnModeSelectNetplayCycleStocks(port_netplay_hostrules_get_stocks(), dir));
        break;
    case nMNModeSelectNetplayRuleTime:
        port_netplay_hostrules_set_time(mnModeSelectNetplayCycleTime(port_netplay_hostrules_get_time(), dir));
        break;
    case nMNModeSelectNetplayRuleItems:
        port_netplay_hostrules_set_item_rate(mnModeSelectNetplayCycleItemRate(port_netplay_hostrules_get_item_rate(), dir));
        break;
    case nMNModeSelectNetplayRuleTeam:
        port_netplay_hostrules_set_team_battle(!port_netplay_hostrules_get_team_battle());
        break;
    case nMNModeSelectNetplayRuleFriendlyFire:
        port_netplay_hostrules_set_team_attack(!port_netplay_hostrules_get_team_attack());
        break;
    case nMNModeSelectNetplayRuleDamage:
        port_netplay_hostrules_set_damage_ratio(mnModeSelectNetplayCycleDamage(port_netplay_hostrules_get_damage_ratio(), dir));
        break;
    case nMNModeSelectNetplayRuleHandicap:
        port_netplay_hostrules_set_handicap(mnModeSelectNetplayCycleHandicap(port_netplay_hostrules_get_handicap(), dir));
        break;
    default:
        break;
    }
}

static void mnModeSelectNetplayFormatRuleValue(s32 row, sb32 is_host, char *buf, s32 buf_size)
{
    s32 stage = is_host ? port_netplay_hostrules_get_stage() : port_netplay_lobby_get_rule_stage();
    s32 stocks = is_host ? port_netplay_hostrules_get_stocks() : port_netplay_lobby_get_rule_stocks();
    s32 time_units = is_host ? port_netplay_hostrules_get_time() : port_netplay_lobby_get_rule_time();
    s32 item_rate = is_host ? port_netplay_hostrules_get_item_rate() : port_netplay_lobby_get_rule_item_rate();
    s32 team = is_host ? port_netplay_hostrules_get_team_battle() : port_netplay_lobby_get_rule_team_battle();
    s32 friendly = is_host ? port_netplay_hostrules_get_team_attack() : port_netplay_lobby_get_rule_team_attack();
    s32 damage = is_host ? port_netplay_hostrules_get_damage_ratio() : port_netplay_lobby_get_rule_damage_ratio();
    s32 handicap = is_host ? port_netplay_hostrules_get_handicap() : port_netplay_lobby_get_rule_handicap();

    switch (row)
    {
    case nMNModeSelectNetplayRuleStage:
        snprintf(buf, buf_size, "%s", mnModeSelectNetplayGetStageName(stage));
        break;
    case nMNModeSelectNetplayRuleLives:
        mnModeSelectNetplayFormatStocks(stocks, buf, buf_size);
        break;
    case nMNModeSelectNetplayRuleTime:
        mnModeSelectNetplayFormatTime(time_units, buf, buf_size);
        break;
    case nMNModeSelectNetplayRuleItems:
        snprintf(buf, buf_size, "%s", mnModeSelectNetplayGetItemRateName(item_rate));
        break;
    case nMNModeSelectNetplayRuleItemList:
        snprintf(buf, buf_size, "SELECT");
        break;
    case nMNModeSelectNetplayRuleTeam:
        snprintf(buf, buf_size, "%s", (team > 0) ? "ON" : "OFF");
        break;
    case nMNModeSelectNetplayRuleFriendlyFire:
        snprintf(buf, buf_size, "%s", (friendly > 0) ? "ON" : "OFF");
        break;
    case nMNModeSelectNetplayRuleDamage:
        mnModeSelectNetplayFormatDamage(damage, buf, buf_size);
        break;
    case nMNModeSelectNetplayRuleHandicap:
        snprintf(buf, buf_size, "%s", mnModeSelectNetplayGetHandicapName(handicap));
        break;
    default:
        buf[0] = '\0';
        break;
    }
}

static void mnModeSelectNetplayMakeRules(GObj *gobj)
{
    static const char *const labels[] =
    {
        "STAGE", "LIVES", "TIME", "ITEMS", "", "TEAM", "FF", "DMG", "HCAP"
    };
    sb32 is_host = port_netplay_lobby_is_host();
    char value[16];
    char line[80];
    s32 n;
    s32 row;

    n = 0;
    for (row = nMNModeSelectNetplayRuleStage; row <= nMNModeSelectNetplayRuleTime; row++)
    {
        mnModeSelectNetplayFormatRuleValue(row, is_host, value, sizeof(value));
        n += snprintf(line + n, sizeof(line) - n, (row == nMNModeSelectNetplayRuleStage) ? "%s %s" : "  %s %s",
            labels[row], value);
    }
    mnModeSelectNetplayMakeString(gobj, line, 20.0F, 181.0F, 0xB0, 0xB0, 0xB0);

    n = 0;
    for (row = nMNModeSelectNetplayRuleItems; row <= nMNModeSelectNetplayRuleHandicap; row++)
    {
        if (row == nMNModeSelectNetplayRuleItemList) continue;
        mnModeSelectNetplayFormatRuleValue(row, is_host, value, sizeof(value));
        n += snprintf(line + n, sizeof(line) - n, (row == nMNModeSelectNetplayRuleItems) ? "%s %s" : "  %s %s",
            labels[row], value);
    }
    mnModeSelectNetplayMakeString(gobj, line, 20.0F, 193.0F, 0xB0, 0xB0, 0xB0);

    if (is_host)
    {
        mnModeSelectNetplayMakeString(gobj, "R  MATCH RULES", 20.0F, 206.0F, 0xA0, 0xFF, 0xA0);
    }
}

static void mnModeSelectNetplayMakeHostRulesPage(GObj *gobj)
{
    static const char *const labels[nMNModeSelectNetplayRuleCount] =
    {
        "STAGE", "LIVES", "TIME", "ITEMS", "ITEM LIST", "TEAM BATTLE",
        "FRIENDLY FIRE", "DAMAGE", "HANDICAP"
    };
    char value[16];
    char line[48];
    s32 row;

    mnModeSelectNetplayMakeString(gobj, "MATCH RULES", 108.0F, 32.0F, 0xFF, 0x20, 0x20);

    for (row = 0; row < nMNModeSelectNetplayRuleCount; row++)
    {
        f32 y = 54.0F + (row * 17.0F);
        sb32 hot = (row == sMNModeSelectNetplayRuleCursor);

        mnModeSelectNetplayFormatRuleValue(row, TRUE, value, sizeof(value));
        snprintf(line, sizeof(line), "%s", labels[row]);
        mnModeSelectNetplayMakeString(gobj, line, 40.0F, y,
            hot ? 0xFF : 0xD0, hot ? 0xFF : 0xD0, hot ? 0x40 : 0xD0);
        mnModeSelectNetplayMakeString(gobj, value, 200.0F, y,
            hot ? 0xFF : 0xC0, hot ? 0xFF : 0xC0, hot ? 0x40 : 0x40);
    }

    mnModeSelectNetplayMakeString(gobj, "L R CHANGE   A ITEM LIST   B BACK", 44.0F, 214.0F, 0xFF, 0x40, 0x40);
}

static void mnModeSelectNetplayMakeItemTogglesPage(GObj *gobj)
{
    u32 mask = (u32)port_netplay_hostrules_get_item_toggles();
    s32 i;

    mnModeSelectNetplayMakeString(gobj, "ITEM LIST", 118.0F, 32.0F, 0xFF, 0x20, 0x20);

    for (i = 0; i < MNNP_ITEM_LIST_VIEW; i++)
    {
        s32 idx = sMNModeSelectNetplayItemScroll + i;
        f32 y = 54.0F + (i * 19.0F);
        sb32 hot;
        sb32 on;

        if (idx >= MNNP_ITEM_LIST_COUNT) break;
        hot = (idx == sMNModeSelectNetplayItemCursor);
        on = (mask & ITEM_TOGGLE_MASK_KIND(sMNModeSelectNetplayItemKinds[idx])) != 0;
        mnModeSelectNetplayMakeString(gobj, sMNModeSelectNetplayItemNames[idx], 44.0F, y,
            hot ? 0xFF : 0xD0, hot ? 0xFF : 0xD0, hot ? 0x40 : 0xD0);
        mnModeSelectNetplayMakeString(gobj, on ? "ON" : "OFF", 236.0F, y,
            on ? 0xA0 : 0x90, on ? 0xFF : 0x90, on ? 0x40 : 0x90);
    }

    mnModeSelectNetplayMakeString(gobj, "L R TOGGLE   B BACK", 82.0F, 214.0F, 0xFF, 0x40, 0x40);
}

static const char* mnModeSelectNetplayGetDeterminismText(void)
{
    if (syNetReplayGetDeterminismFailed()) return "FAIL";
    if (syNetReplayGetDeterminismVerified()) return "PASS";
    if (syNetReplayDeterminismTestRecordArmed()) return "ARMED";
    if (syNetReplayDeterminismTestTraceAvailable()) return "READY";
    return "NO TRACE";
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

    mnModeSelectNetplayMakeRules(gobj);

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

static void mnModeSelectNetplayMakeTextEntry(GObj *gobj)
{
    const char *label;

    switch (sMNModeSelectNetplayImeTarget)
    {
    case 1:
        label = "HOST IP ADDRESS";
        break;
    case 2:
        label = "LOBBY SERVER";
        break;
    default:
        label = "PLAYER NAME";
        break;
    }

    mnModeSelectNetplayMakeFrame(gobj, 66.0F, 86.0F, 0x82, 0x82, 0xAA);
    mnModeSelectNetplayMakeStringScaled(gobj, label, 96.0F, 100.0F, 0xFF, 0xC0, 0x40, 1.15F);
    mnModeSelectNetplayMakeString(gobj, "KEYBOARD OPEN", 106.0F, 124.0F, 0xA0, 0xFF, 0xA0);
}

static void mnModeSelectNetplayRefresh(void)
{
    static const char *root_options[nMNModeSelectNetplayRootCount] =
    {
        "LOCAL ADHOC", "ONLINE", "SETTINGS", "BACK"
    };
    static const char *mode_options[nMNModeSelectNetplayModeCount] =
    {
        "FIND GAME", "HOST GAME", "JOIN GAME", "DIRECT IP", "BACK"
    };
    static const char *settings_options[nMNModeSelectNetplaySettingsCount] =
    {
        "PLAYER NAME", "INPUT DELAY", "NETPLAY STATS", "SET SERVER", "DETERMINISM TEST",
        "RESET SETTINGS", "BACK"
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
        mnModeSelectNetplayMakeStringScaled(sMNModeSelectNetplayGObj, "NETPLAY", 116.0F, 42.0F, 0xFF, 0x20, 0x20, 1.35F);
        for (i = 0; i < nMNModeSelectNetplayRootCount; i++)
        {
            y = 78.0F + (i * 28.0F);
            mnModeSelectNetplayMakeTabRow(sMNModeSelectNetplayGObj, 90.0F, y, 16, root_options[i],
                (i == sMNModeSelectNetplayOption));
        }
        if (port_netplay_network_initialized() == FALSE)
        {
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "NETWORK INITIALIZING", 92.0F, 206.0F, 0xB0, 0xB0, 0xB0);
        }
        else mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, "SELECT CONNECTION", 101.0F, 206.0F, 0xA0, 0xFF, 0xA0);
    }
    else if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageMode)
    {
        const sb32 is_adhoc = (port_netplay_get_mode() == 1);
        const s32 adhoc_dialog_state = is_adhoc ? port_netplay_adhoc_dialog_state() : PORT_NETPLAY_ADHOC_DIALOG_INACTIVE;
        mnModeSelectNetplayMakeStringScaled(sMNModeSelectNetplayGObj,
            is_adhoc ? "LOCAL ADHOC" : "ONLINE", is_adhoc ? 104.0F : 128.0F, 42.0F,
            0xFF, 0x20, 0x20, 1.35F);
        for (i = 0; i < nMNModeSelectNetplayModeCount; i++)
        {
            y = 70.0F + (i * 26.0F);
            mnModeSelectNetplayMakeTabRow(sMNModeSelectNetplayGObj, 90.0F, y, 16, mode_options[i],
                (i == sMNModeSelectNetplayOption));
        }
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
            y = 70.0F + (i * 24.0F);
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, settings_options[i], 56.0F, y,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0xD0,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0x20,
                (i == sMNModeSelectNetplayOption) ? 0xFF : 0x20);
        }
        {
            char server_buf[40];
            char server_short[16];
            s32 sc;

            port_netplay_get_player_name(player_name, ARRAY_COUNT(player_name));
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, player_name, 190.0F,
                70.0F + (nMNModeSelectNetplaySettingsName * 24.0F), 0xFF, 0xC0, 0x40);
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj, mnModeSelectNetplayGetDelayText(), 190.0F,
                70.0F + (nMNModeSelectNetplaySettingsDelay * 24.0F), 0xFF, 0xC0, 0x40);
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj,
                port_netplay_get_show_stats() ? "ON" : "OFF", 190.0F,
                70.0F + (nMNModeSelectNetplaySettingsStats * 24.0F), 0xFF, 0xC0, 0x40);

            port_netplay_get_rendezvous_server(server_buf, ARRAY_COUNT(server_buf));
            for (sc = 0; sc < 15 && server_buf[sc] != '\0'; sc++) server_short[sc] = server_buf[sc];
            server_short[sc] = '\0';
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj,
                (server_short[0] != '\0') ? server_short : "OFF", 190.0F,
                70.0F + (nMNModeSelectNetplaySettingsServer * 24.0F), 0xFF, 0xC0, 0x40);

            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj,
                mnModeSelectNetplayGetDeterminismText(), 190.0F,
                70.0F + (nMNModeSelectNetplaySettingsDeterminism * 24.0F), 0xFF, 0xC0, 0x40);
        }
        mnModeSelectNetplayMakeMarker(sMNModeSelectNetplayGObj, 70.0F + (sMNModeSelectNetplayOption * 24.0F));
        if (sMNModeSelectNetplayOption == nMNModeSelectNetplaySettingsDeterminism)
        {
            mnModeSelectNetplayMakeString(sMNModeSelectNetplayGObj,
                "A VERIFY  START RECORD", 70.0F, 218.0F, 0xD0, 0xD0, 0xD0);
        }
    }
    else if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageTextEntry)
    {
        mnModeSelectNetplayMakeTextEntry(sMNModeSelectNetplayGObj);
    }
    else if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageHostRules)
    {
        if (port_netplay_lobby_is_host())
        {
            mnModeSelectNetplayMakeHostRulesPage(sMNModeSelectNetplayGObj);
        }
        else
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageHost;
        }
    }
    else if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageItemToggles)
    {
        if (port_netplay_lobby_is_host())
        {
            mnModeSelectNetplayMakeItemTogglesPage(sMNModeSelectNetplayGObj);
        }
        else
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageHost;
        }
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

static void mnModeSelectNetplayExit(void)
{
    port_netplay_leave_menu();
    if (sMNModeSelectNetplayGObj != NULL)
    {
        gcEjectGObj(sMNModeSelectNetplayGObj);
        sMNModeSelectNetplayGObj = NULL;
    }
    gMNNetplayReturnToVSMode = TRUE;
    gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;
    gSCManagerSceneData.scene_curr = nSCKindVSMode;
    syTaskmanSetLoadScene();
}

static void mnModeSelectNetplayMoveOption(s32 delta, s32 count)
{
    sMNModeSelectNetplayOption += delta;
    if (sMNModeSelectNetplayOption < 0) sMNModeSelectNetplayOption = count - 1;
    if (sMNModeSelectNetplayOption >= count) sMNModeSelectNetplayOption = 0;
    func_800269C0_275C0(nSYAudioFGMMenuScroll2);
    mnModeSelectNetplayRefresh();
}

static void mnModeSelectNetplayBeginTextEntry(s32 target)
{
    char current[64];
    const char *title;
    s32 max_len;
    s32 mode;

    current[0] = '\0';
    sMNModeSelectNetplayImeTarget = target;

    switch (target)
    {
    case 1:
        port_netplay_get_join_address(current, ARRAY_COUNT(current));
        title = "HOST IP ADDRESS";
        max_len = 15;
        mode = 1;
        break;
    case 2:
        port_netplay_get_rendezvous_server(current, ARRAY_COUNT(current));
        title = "LOBBY SERVER";
        max_len = 60;
        mode = 0;
        break;
    default:
        port_netplay_get_player_name(current, ARRAY_COUNT(current));
        title = "PLAYER NAME";
        max_len = 12;
        mode = 0;
        break;
    }

    if (port_netplay_ime_begin(title, current, max_len, mode))
    {
        sMNModeSelectNetplayPage = nMNModeSelectNetplayPageTextEntry;
    }
    else func_800269C0_275C0(nSYAudioFGMMenuDenied);
    mnModeSelectNetplayRefresh();
}

static void mnModeSelectNetplayFinishTextEntry(void)
{
    s32 state = port_netplay_ime_state();
    s32 target = sMNModeSelectNetplayImeTarget;

    if (state == 2)
    {
        char buf[64];

        port_netplay_ime_result(buf, ARRAY_COUNT(buf));
        port_netplay_ime_cancel();

        if (target == 1)
        {
            if (port_netplay_join_address(buf) != 0)
            {
                sMNModeSelectNetplayPage = nMNModeSelectNetplayPageJoin;
                sMNModeSelectNetplayOption = 0;
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
            }
            else
            {
                sMNModeSelectNetplayPage = nMNModeSelectNetplayPageMode;
                sMNModeSelectNetplayOption = nMNModeSelectNetplayModeDirect;
                func_800269C0_275C0(nSYAudioFGMMenuDenied);
            }
        }
        else if (target == 2)
        {
            port_netplay_set_rendezvous_server(buf);
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageSettings;
            sMNModeSelectNetplayOption = nMNModeSelectNetplaySettingsServer;
            func_800269C0_275C0(nSYAudioFGMMenuSelect);
        }
        else
        {
            port_netplay_set_player_name(buf);
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageSettings;
            sMNModeSelectNetplayOption = nMNModeSelectNetplaySettingsName;
            func_800269C0_275C0(nSYAudioFGMMenuSelect);
        }
    }
    else
    {
        port_netplay_ime_cancel();
        if (target == 1)
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageMode;
            sMNModeSelectNetplayOption = nMNModeSelectNetplayModeDirect;
        }
        else
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageSettings;
            sMNModeSelectNetplayOption = (target == 2)
                ? nMNModeSelectNetplaySettingsServer
                : nMNModeSelectNetplaySettingsName;
        }
        func_800269C0_275C0(nSYAudioFGMMenuScroll1);
    }
    sMNModeSelectNetplayNavWait = 10;
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

    if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageTextEntry)
    {
        port_netplay_ime_tick();
        if (port_netplay_ime_state() != 1)
        {
            mnModeSelectNetplayFinishTextEntry();
        }
        else if ((sMNModeSelectTotalTimeTics % 15) == 0)
        {
            mnModeSelectNetplayRefresh();
        }
        return;
    }

    if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageHostRules)
    {
        sb32 rule_left = left;
        sb32 rule_right = right;

        if (!port_netplay_lobby_is_host())
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageHost;
            mnModeSelectNetplayRefresh();
            return;
        }
        if (!rule_left && !rule_right && (sMNModeSelectNetplayNavWait == 0))
        {
            if (scSubsysControllerGetPlayerStickLR(20, 1) != 0) rule_right = TRUE;
            else if (scSubsysControllerGetPlayerStickLR(-20, 0) != 0) rule_left = TRUE;
        }
        if (up || down)
        {
            sMNModeSelectNetplayRuleCursor =
                (sMNModeSelectNetplayRuleCursor + (up ? (nMNModeSelectNetplayRuleCount - 1) : 1)) % nMNModeSelectNetplayRuleCount;
            sMNModeSelectNetplayNavWait = 10;
            func_800269C0_275C0(nSYAudioFGMMenuScroll1);
            mnModeSelectNetplayRefresh();
            return;
        }
        if (accept && (sMNModeSelectNetplayRuleCursor == nMNModeSelectNetplayRuleItemList))
        {
            sMNModeSelectNetplayItemCursor = 0;
            sMNModeSelectNetplayItemScroll = 0;
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageItemToggles;
            func_800269C0_275C0(nSYAudioFGMMenuSelect);
            mnModeSelectNetplayRefresh();
            return;
        }
        if ((rule_left || rule_right) && (sMNModeSelectNetplayRuleCursor != nMNModeSelectNetplayRuleItemList))
        {
            mnModeSelectNetplayCycleRule(sMNModeSelectNetplayRuleCursor, rule_right ? 1 : -1);
            sMNModeSelectNetplayNavWait = 10;
            func_800269C0_275C0(nSYAudioFGMMenuScroll2);
            mnModeSelectNetplayRefresh();
            return;
        }
        if (cancel)
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageHost;
            func_800269C0_275C0(nSYAudioFGMMenuScroll1);
            mnModeSelectNetplayRefresh();
        }
        return;
    }

    if (sMNModeSelectNetplayPage == nMNModeSelectNetplayPageItemToggles)
    {
        sb32 toggle_left = left;
        sb32 toggle_right = right;

        if (!port_netplay_lobby_is_host())
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageHost;
            mnModeSelectNetplayRefresh();
            return;
        }
        if (!toggle_left && !toggle_right && (sMNModeSelectNetplayNavWait == 0))
        {
            if (scSubsysControllerGetPlayerStickLR(20, 1) != 0) toggle_right = TRUE;
            else if (scSubsysControllerGetPlayerStickLR(-20, 0) != 0) toggle_left = TRUE;
        }
        if (up || down)
        {
            sMNModeSelectNetplayItemCursor += up ? -1 : 1;
            if (sMNModeSelectNetplayItemCursor < 0) sMNModeSelectNetplayItemCursor = MNNP_ITEM_LIST_COUNT - 1;
            if (sMNModeSelectNetplayItemCursor >= MNNP_ITEM_LIST_COUNT) sMNModeSelectNetplayItemCursor = 0;
            if (sMNModeSelectNetplayItemCursor < sMNModeSelectNetplayItemScroll)
                sMNModeSelectNetplayItemScroll = sMNModeSelectNetplayItemCursor;
            if (sMNModeSelectNetplayItemCursor >= sMNModeSelectNetplayItemScroll + MNNP_ITEM_LIST_VIEW)
                sMNModeSelectNetplayItemScroll = sMNModeSelectNetplayItemCursor - MNNP_ITEM_LIST_VIEW + 1;
            sMNModeSelectNetplayNavWait = 10;
            func_800269C0_275C0(nSYAudioFGMMenuScroll1);
            mnModeSelectNetplayRefresh();
            return;
        }
        if (toggle_left || toggle_right || accept)
        {
            mnModeSelectNetplayToggleItem(sMNModeSelectNetplayItemCursor);
            sMNModeSelectNetplayNavWait = 10;
            func_800269C0_275C0(nSYAudioFGMMenuScroll2);
            mnModeSelectNetplayRefresh();
            return;
        }
        if (cancel)
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageHostRules;
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
            if (port_netplay_lobby_is_host() &&
                scSubsysControllerGetPlayerTapButtons(Z_TRIG))
            {
                sMNModeSelectNetplayRuleCursor = 0;
                sMNModeSelectNetplayPage = nMNModeSelectNetplayPageHostRules;
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
                mnModeSelectNetplayRefresh();
                return;
            }
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
        if ((sMNModeSelectNetplayOption == nMNModeSelectNetplaySettingsDeterminism) && start)
        {
            if (syNetReplayArmDeterminismTestRecord())
            {
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
            }
            else func_800269C0_275C0(nSYAudioFGMMenuDenied);
            mnModeSelectNetplayRefresh();
            return;
        }
        if ((sMNModeSelectNetplayOption == nMNModeSelectNetplaySettingsDeterminism) && action)
        {
            if (syNetReplayDeterminismTestTraceAvailable())
            {
                port_netplay_leave_menu();
                if (!syNetReplayLaunchDeterminismTestPlayback())
                {
                    port_netplay_enter_menu();
                    func_800269C0_275C0(nSYAudioFGMMenuDenied);
                    mnModeSelectNetplayRefresh();
                    return;
                }
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
                return;
            }
            if (syNetReplayArmDeterminismTestRecord())
            {
                func_800269C0_275C0(nSYAudioFGMMenuSelect);
            }
            else func_800269C0_275C0(nSYAudioFGMMenuDenied);
            mnModeSelectNetplayRefresh();
            return;
        }
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
                mnModeSelectNetplayBeginTextEntry(0);
                return;
            }
            if (sMNModeSelectNetplayOption == nMNModeSelectNetplaySettingsServer)
            {
                mnModeSelectNetplayBeginTextEntry(2);
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
                sb32 was_adhoc = (port_netplay_get_mode() == PORT_NETPLAY_MODE_LOCAL_ADHOC);
                if (was_adhoc)
                {
                    port_netplay_set_mode(PORT_NETPLAY_MODE_NONE);
                }
                sMNModeSelectNetplayPage = nMNModeSelectNetplayPageRoot;
                sMNModeSelectNetplayOption = was_adhoc ?
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
            case nMNModeSelectNetplayModeDirect:
                mnModeSelectNetplayBeginTextEntry(1);
                return;
            }
            func_800269C0_275C0(nSYAudioFGMMenuSelect);
            mnModeSelectNetplayRefresh();
        }
        else if (cancel)
        {
            sb32 was_adhoc = (port_netplay_get_mode() == PORT_NETPLAY_MODE_LOCAL_ADHOC);
            if (was_adhoc)
            {
                port_netplay_set_mode(PORT_NETPLAY_MODE_NONE);
            }
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageRoot;
            sMNModeSelectNetplayOption = was_adhoc ?
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
    sMNModeSelectOption1PModeGObj = NULL;
    sMNModeSelectOptionVSModeGObj = NULL;
    sMNModeSelectOptionOptionGObj = NULL;
    sMNModeSelectOptionDataGObj = NULL;
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
    sMNModeSelectNetplayRuleCursor = 0;
    sMNModeSelectNetplayItemCursor = 0;
    sMNModeSelectNetplayItemScroll = 0;
    sMNModeSelectNetplayImeTarget = 0;
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
#if defined(PORT) && defined(__vita__)
    if (gMNNetplayOverlayRequest || (port_netplay_get_mode() != PORT_NETPLAY_MODE_NONE))
    {
        sb32 fresh_request = gMNNetplayOverlayRequest;
        s32 np_state = port_netplay_get_state();

        gMNNetplayOverlayRequest = FALSE;
        mnModeSelectNetplayMakeBackground();
        port_netplay_enter_menu();
        if (!fresh_request &&
            ((np_state == PORT_NETPLAY_STATE_HOSTING_LOBBY) || (np_state == PORT_NETPLAY_STATE_CLIENT_LOBBY)))
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageHost;
        }
        else if (!fresh_request)
        {
            sMNModeSelectNetplayPage = nMNModeSelectNetplayPageMode;
        }
        sMNModeSelectNetplayOption = 0;
        mnModeSelectNetplayRefresh();
    }
    else
#endif
    {
        mnModeSelectMakeDecals();
        mnModeSelectMakeOptions();
        mnModeSelectMakeLabels();
    }

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
