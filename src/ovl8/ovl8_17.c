#include <sys/objman.h>
#include <sys/develop.h>
#include <db/debug.h>

typedef struct dbUnknown17 {
	s32 dbUnknown17_0x0;
	s32 dbUnknown17_0x4;
	s32* dbUnknown17_0x8;
	s32 dbUnknown17_0xC;
	s32 dbUnknown17_0x10;
} dbUnknown17;

typedef struct {
	s16 unk0;
	s32 unk4[0x8/4];
	s32* unkC;
} dbUnknown18;

typedef struct {
	s32 unk0[0x8/4];
	dbUnknown18* unk8;
	s32 unkC;
	s32 unk10;
} dbUnknown19;

typedef struct {
	s32 unk0[0x4/4];
	s32 unk4;
	dbUnknownLink** unk8;
	s32 unkC;
	s32 unk10;
	s32 unk14;
	s32 unk18[0x4/4];
	s32 unk1C;
} dbUnknown20;

typedef struct {
#ifdef PORT
    s32 unk0[0x30 / 4];
    struct dbUnknown23* handler;
#else
	s32 unk0[0x30 / 4];
	struct dbUnknown23* handler;
#endif
} dbUnknown21;

typedef struct {
#ifdef PORT
    s32 unk0;
    dbUnknown21* obj;
    s32* table;      // pointer table (used in indexed lookup)
    s32 count;
#else
	s32 unk0;
	dbUnknown21* obj;
	s32* table;      // pointer table (used in indexed lookup)
	s32 count;
#endif
} dbUnknown22;

typedef struct dbUnknown23 {
#ifdef PORT
    s32 unk0[0x70 / 4];
    s16 offset;
    void (*callback)(void* base, void* arg);
#else
	s32 unk0[0x70 / 4];
	s16 offset;
	void (*callback)(void* base, void* arg);
#endif
} dbUnknown23;

typedef struct {
#ifdef PORT
    s32 unk0[0x8/4];
    s32* unk8;
    s32 unkC;
#else
	s32 unk0[0x8/4];
	s32* unk8;
	s32 unkC;
#endif
} dbUnknown24;

#ifdef PORT
extern s32 D_ovl8_8038BC30;
#else
void func_ovl8_8038120C();
void func_ovl8_8038125C();
void func_ovl8_80381274();
void func_ovl8_803812BC();
extern sb32 func_ovl8_80381308();
void func_ovl8_80381710();
void func_ovl8_803817C0(dbUnknown22*, f32);
void func_ovl8_8038185C();
void func_ovl8_80381908();
void func_ovl8_803819F4();
void func_ovl8_80381A58();
void func_ovl8_80381A88();
void func_ovl8_80381AF0();
void func_ovl8_80381B04();
void func_ovl8_80381B10();
#endif

#ifdef PORT
void func_ovl8_8038185C(dbUnknown24 *arg0, s32 *arg1);
void func_ovl8_80381908(dbUnknownLink* arg0);
#else
s32 D_ovl8_8038BC30 = 0x2D2D2D00;
#endif

#ifdef PORT
extern dbUnknownLinkStruct D_ovl8_8038BC34;
extern dbUnknownLinkStruct D_ovl8_8038BC8C;
#else
dbFunction D_ovl8_8038BC34[] = {
	{0, NULL},
	{0, (sb32(*)())func_ovl8_8038120C},
	{0, (sb32(*)())func_ovl8_8038125C},
	{0, (sb32(*)())func_ovl8_80381274},
	{0, (sb32(*)())func_ovl8_803812BC},
	{0, func_ovl8_80381308},
	{0, (sb32(*)())func_ovl8_80381710},
	{0, (sb32(*)())func_ovl8_803817C0},
	{0, (sb32(*)())func_ovl8_8038185C},
	{0, (sb32(*)())func_ovl8_80381908},
	{0, NULL},
};
#endif

#ifdef PORT
//0x80381130
#else
dbFunction D_ovl8_8038BC8C[] = {
	{0, NULL},
	{0, (sb32(*)())func_ovl8_80381B10},
	{0, (sb32(*)())func_ovl8_80381A58},
	{0, (sb32(*)())func_ovl8_803819F4},
	{0, (sb32(*)())func_ovl8_80381A88},
	{0, func_ovl8_80381308},
	{0, (sb32(*)())func_ovl8_80381AF0},
	{0, (sb32(*)())func_ovl8_80381B04},
	{0, (sb32(*)())func_ovl8_8038185C},
	{0, (sb32(*)())func_ovl8_80381908},
	{0, NULL},
};

DBMenu D_ovl8_8038BCE4 = {
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0xff },
	{ 0, 0, 0, 0xff },
	1, 0, 0x504F504D, 0, 0, 0, 0
};

// 80381130
#endif
dbUnknownLinkStruct* func_ovl8_80381130(dbUnknownLinkStruct* arg0)
{
	if ((arg0 != NULL) || ((arg0 = func_ovl8_803717A0(0x20)) != NULL))
	{
		arg0->unk_dbunkstruct_0x1C.link = &D_ovl8_8038BC34;
	}
	return arg0;
}

// 0x8038116C a fn that resets dbUnknownLinkStruct to some degree
dbUnknownLinkStruct* func_ovl8_8038116C(dbUnknownLinkStruct* targetLinkStruct, s32 arg1, dbUnknownS38* arg2) {
#ifdef PORT
    if ( targetLinkStruct || (targetLinkStruct = func_ovl8_803717A0(32))) {
        
        targetLinkStruct->unk_dbunkstruct_0x1C.link = &D_ovl8_8038BC34;
        *(dbUnknownS38**)&targetLinkStruct->position.y = arg2;
        
        arg2->unk_dbunks38_0x30[11].unk_dbfunc_0x4(arg2->unk_dbunks38_0x30[11].unk_dbfunc_0x0 + (uintptr_t) arg2, 0x4CBEBC20, arg2);
        
        func_ovl8_8038185C(targetLinkStruct, arg1);
       
        stringCopy((char*)*(dbUnknownS38**)&targetLinkStruct->position.y + 0xC, &D_ovl8_8038BC30);
        
        *(u32*)&targetLinkStruct->text_color = 0;
        *(u32*)&targetLinkStruct->bg_color  = 0;
        targetLinkStruct->id = (s32) targetLinkStruct->unk_dbunkstruct_0xC;
    }
    
    return targetLinkStruct;
#else
	if ( targetLinkStruct || (targetLinkStruct = func_ovl8_803717A0(32))) {
		
		targetLinkStruct->unk_dbunkstruct_0x1C.link = &D_ovl8_8038BC34;
		*(dbUnknownS38**)&targetLinkStruct->position.y = arg2;
		
		arg2->unk_dbunks38_0x30[11].unk_dbfunc_0x4(arg2->unk_dbunks38_0x30[11].unk_dbfunc_0x0 + (uintptr_t) arg2, 0x4CBEBC20, arg2);
		
		func_ovl8_8038185C(targetLinkStruct, arg1);
	   
		stringCopy((char*)*(dbUnknownS38**)&targetLinkStruct->position.y + 0xC, &D_ovl8_8038BC30);
		
		*(u32*)&targetLinkStruct->text_color = 0;
		*(u32*)&targetLinkStruct->bg_color  = 0;
		targetLinkStruct->id = (s32) targetLinkStruct->unk_dbunkstruct_0xC;
	}
	
	return targetLinkStruct;
#endif
}

#ifdef PORT
// 0x8038120C
#else
// 8038120C
#endif
void func_ovl8_8038120C(dbUnknownLinkStruct* arg0, s32 arg1)
{
	if (arg0 == NULL)
		return;

	arg0->unk_dbunkstruct_0x1C.link = &D_ovl8_8038BC34;
	func_ovl8_80381908(arg0);
	if (arg1 & 1)
		func_ovl8_803717C0(arg0);
}

#ifdef PORT
// 0x8038125C
#else
// 8038125C
#endif
void func_ovl8_8038125C(s32 **arg0, s32 arg1, s32 *arg2)
{
	*arg2 = (arg0[2] + (arg1))[-1];
}

#ifdef PORT
// 0x80381274
#else
// 80381274
#endif
void func_ovl8_80381274(dbUnknown17* arg0, s32 arg1)
{
	if (arg1 == 0)
	{
		arg0->dbUnknown17_0xC = 0;
		*arg0->dbUnknown17_0x8 = 0;
	}
	else
	{
		arg0->dbUnknown17_0x8[arg0->dbUnknown17_0xC - arg1] = 0;
		arg0->dbUnknown17_0xC = arg0->dbUnknown17_0xC - arg1;
	}
}

#ifdef PORT
// 0x803812BC
#else
// 803812BC
#endif
void func_ovl8_803812BC(dbUnknown17* arg0, s32* arg1)
{
	if (arg0->dbUnknown17_0xC < arg0->dbUnknown17_0x10)
	{
		arg0->dbUnknown17_0xC = arg0->dbUnknown17_0xC + 1;
		arg0->dbUnknown17_0x8[arg0->dbUnknown17_0xC - 1] = *arg1;
		arg0->dbUnknown17_0x8[arg0->dbUnknown17_0xC] = 0;
	}
}

#ifdef PORT
// 0x80381308
#else
// 80381308
// TODO: when this function is matched, remove D_ovl8_8038BCEC, D_ovl8_8038BCF0, and D_ovl8_8038BCF4 from not_found.txt
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/ovl8/ovl8_17/func_ovl8_80381308.s")

#ifdef PORT
// 0x80381710
#else
// 80381710
#endif
void func_ovl8_80381710(dbUnknownStructSC* arg0, s32 arg1) {
#ifdef PORT
    dbUnknown5_2* temp_v0;
    dbFunction* temp_v1;
#else
	dbUnknown5_2* temp_v0;
	dbFunction* temp_v1;
#endif

#ifdef PORT
    arg0->unk_0x4->dbUnknown5_2_unk_f32_0x0 = arg1;
    temp_v0 = arg0->unk_0x4;
    
    if ((temp_v0->dbUnknown5_2_unk_f32_0x0 > 0.0f) && (temp_v0->dbUnknown5_2_unk_f32_0x0 <= arg0->unk_0xC)) {
        temp_v1 = temp_v0->dbUnknown5_2_db_func;
        temp_v1[14].unk_dbfunc_0x4(temp_v1[14].unk_dbfunc_0x0 + (uintptr_t)temp_v0, arg0->unk_0x8[arg1 - 1]);
        return;
    }
    
    temp_v1 = temp_v0->dbUnknown5_2_db_func;
    temp_v1[14].unk_dbfunc_0x4(temp_v1[14].unk_dbfunc_0x0 + (uintptr_t) temp_v0, &D_ovl8_8038BC30);
#else
	arg0->unk_0x4->dbUnknown5_2_unk_f32_0x0 = arg1;
	temp_v0 = arg0->unk_0x4;
	
	if ((temp_v0->dbUnknown5_2_unk_f32_0x0 > 0.0f) && (temp_v0->dbUnknown5_2_unk_f32_0x0 <= arg0->unk_0xC)) {
		temp_v1 = temp_v0->dbUnknown5_2_db_func;
		temp_v1[14].unk_dbfunc_0x4(temp_v1[14].unk_dbfunc_0x0 + (uintptr_t)temp_v0, arg0->unk_0x8[arg1 - 1]);
		return;
	}
	
	temp_v1 = temp_v0->dbUnknown5_2_db_func;
	temp_v1[14].unk_dbfunc_0x4(temp_v1[14].unk_dbfunc_0x0 + (uintptr_t) temp_v0, &D_ovl8_8038BC30);
#endif
}


#ifdef PORT
// 0x803817C0
#else
// 803817C0
#endif
void func_ovl8_803817C0(dbUnknown22* arg0, f32 arg1)
{
#ifdef PORT
    s32 index;
    dbUnknown23* handler;
#else
	s32 index;
	dbUnknown23* handler;
#endif

#ifdef PORT
    if (arg1 > 0.0f)
    {
        index = arg1;
#else
	if (arg1 > 0.0f)
	{
		index = arg1;
#endif

#ifdef PORT
        if (arg0->count >= index)
        {
            handler = arg0->obj->handler;
#else
		if (arg0->count >= index)
		{
			handler = arg0->obj->handler;
#endif

#ifdef PORT
            handler->callback(
                (void*)(handler->offset + (uintptr_t)arg0->obj),
                arg0->table[index - 1]
            );
            return;
        }
    }
#else
			handler->callback(
				(void*)(handler->offset + (uintptr_t)arg0->obj),
				arg0->table[index - 1]
			);
			return;
		}
	}
#endif

#ifdef PORT
    handler = arg0->obj->handler;
#else
	handler = arg0->obj->handler;
#endif

#ifdef PORT
    handler->callback(
        (void*)(handler->offset + (uintptr_t)arg0->obj),
        &D_ovl8_8038BC30
    );
#else
	handler->callback(
		(void*)(handler->offset + (uintptr_t)arg0->obj),
		&D_ovl8_8038BC30
	);
#endif
}

#ifdef PORT
// 0x8038185C
#else
// 8038185C
#endif
void func_ovl8_8038185C(dbUnknown24 *arg0, s32 *arg1) 
{
#ifdef PORT
    s32 i;
    s32 sp40;
    s32 sp3C;
    s32 sp38;
    s32 *var_a1;
    s32 *var_v1;
#else
	s32 i;
	s32 sp40;
	s32 sp3C;
	s32 sp38;
	s32 *var_a1;
	s32 *var_v1;
#endif

#ifdef PORT
    func_ovl8_8037FF40(arg1, &arg0->unkC, &sp40, &sp3C, &sp38);
    arg0->unk8 = func_ovl8_803716D8((arg0->unkC * 4) + 4);
    
    if (arg0->unk8 == NULL || arg0->unkC == 0)
        return;
    
    var_a1 = arg0->unk8;
    var_v1 = arg1;
    
    for (i = 0; arg0->unkC > i; i++) var_a1[i] = *var_v1++;
#else
	func_ovl8_8037FF40(arg1, &arg0->unkC, &sp40, &sp3C, &sp38);
	arg0->unk8 = func_ovl8_803716D8((arg0->unkC * 4) + 4);
	
	if (arg0->unk8 == NULL || arg0->unkC == 0)
		return;
	
	var_a1 = arg0->unk8;
	var_v1 = arg1;
	
	for (i = 0; arg0->unkC > i; i++) var_a1[i] = *var_v1++;
#endif

#ifdef PORT
    var_a1[i] = 0;
#else
	var_a1[i] = 0;
#endif
}

#ifdef PORT
// 0x80381908
#else
// 80381908
#endif
void func_ovl8_80381908(dbUnknownLink* arg0)
{
	if (arg0->unk_dbunklink_0x8 != NULL)
	{
		func_ovl8_80371764(arg0->unk_dbunklink_0x8);
	}
}

#ifdef PORT
// 0x80381934
#else
// 80381934
#endif
void *func_ovl8_80381934(dbUnknown20 *arg0, s16 *arg1, dbUnknownS38 *arg2)
{
	s32 sp24;
	s16 temp_v1_2;

	if ((arg0 != NULL) || (arg0 = func_ovl8_803717A0(0x20), (arg0 != NULL)))
	{
		func_ovl8_80381130(arg0);
		arg0->unk1C = &D_ovl8_8038BC8C;
		arg0->unk4 = arg2;
		arg2->unk_dbunks38_0x30[11].unk_dbfunc_0x4(arg2->unk_dbunks38_0x30[11].unk_dbfunc_0x0 + arg0->unk4, 0x4CBEBC20, arg2);
		temp_v1_2 = *arg1;
		arg0->unk14 = 1;
		arg0->unkC = (s32) temp_v1_2;
		arg0->unk10 = (s32) temp_v1_2;
		sp24 = (temp_v1_2 * 8) + 0xC;
		arg0->unk8 =  func_ovl8_803716D8(sp24);
		memcpy(arg0->unk8, arg1, sp24);
	}
	return arg0;
}

#ifdef PORT
// 0x803819F4
#else
// 803819F4
#endif
void func_ovl8_803819F4(dbUnknown19 *arg0, s32 arg1)
{
	s16 *temp_v0;

	if (arg1 == 0)
	{
		arg0->unkC = 0;
		*arg0->unk8->unkC = 0;
		arg0->unk8->unk0 = 0;
		return;
	}

	*(arg0->unk8->unkC + ((arg0->unkC - arg1) * 2)) = 0;
	temp_v0 = arg0->unk8;
	arg0->unkC = (s32) (arg0->unkC - arg1);
	temp_v0[0] = (s16) (temp_v0[0] - arg1);
}

#ifdef PORT
// 0x80381A58
#else
// 80381A58
#endif
void func_ovl8_80381A58(dbUnknown19 *arg0, s32 arg1, s32 *arg2)
{
	s32 temp_v0;

	temp_v0 = arg1 * 2;
	arg2[0x0/4] = ((s32*)(arg0->unk8->unkC + temp_v0))[-8/4];
	arg2[0x4/4] = ((s32*)(arg0->unk8->unkC + temp_v0))[-4/4];
}

#ifdef PORT
// 0x80381A88
#else
// 80381A88
#endif
void func_ovl8_80381A88(dbUnknown19 *arg0, s32 *arg1)
{
	s16 *temp_v1;

	if (arg0->unkC >= arg0->unk10)
		return;

	arg0->unkC = arg0->unkC + 1;
	(arg0->unk8->unkC + (arg0->unkC * 2))[-8/4] = arg1[0/4];
	(arg0->unk8->unkC + (arg0->unkC * 2))[-4/4] = arg1[4/4];
	temp_v1 = arg0->unk8;
	temp_v1[0]++;
}

#ifdef PORT
// 0x80381AF0
#else
// 80381AF0
#endif
void func_ovl8_80381AF0(f32** arg0, s32 arg1)
{
	*arg0[1] = (f32) arg1;
}

#ifdef PORT
// 0x80381B04
#else
// 80381B04
#endif
void func_ovl8_80381B04(s32 arg0, s32 arg1) {}

#ifdef PORT
// 0x80381B10
#else
// 80381B10
#endif
void func_ovl8_80381B10(dbUnknownLinkStruct* arg0, s32 arg1)
{
	if (arg0 == NULL)
		return;

	arg0->unk_dbunkstruct_0x1C.link = &D_ovl8_8038BC8C;
	func_ovl8_8038120C(arg0, 0);
	if (arg1 & 1)
		func_ovl8_803717C0(arg0);
}