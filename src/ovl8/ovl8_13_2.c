#include <sys/objman.h>
#include <sys/develop.h>
#include <db/debug.h>

typedef struct dbUnknown8_13
{
	u16 unk_dbunknown8_13_0x0;
	u16 width;
	u16 height;
	DBFontPadding padding;
	u8 bits_per_pixel;
	u8 *glyphs;
} dbUnknown8_13;

extern db4Bytes D_ovl8_80389F4C;
extern db4Bytes D_ovl8_80389F50;
extern GObj* D_ovl8_8038A860;
extern s16 D_ovl8_8038A864;
extern db4Bytes D_ovl8_8038A890[];
extern dbUnknownLinkStruct D_ovl8_8038A8A0;
extern dbFunction D_ovl8_8038A980;
extern dbUnknownLink D_ovl8_8038AAD8;
#ifdef PORT
extern dbUnknownLinkStruct D_ovl8_8038AB00;
#else
void func_ovl8_8037EB00();
extern sb32 func_ovl8_8037C9E4();
extern sb32 func_ovl8_803721E8();
extern sb32 func_ovl8_80372224();
extern sb32 func_ovl8_80372250();
extern sb32 func_ovl8_803722A0();
extern sb32 func_ovl8_803722F0();
extern sb32 func_ovl8_80372348();
extern sb32 func_ovl8_80372358();
extern sb32 func_ovl8_80372360();
extern sb32 func_ovl8_803726CC();

dbFunction D_ovl8_8038AB00[] = {
	{0, NULL},
	{(s16)0xFFC0, (sb32(*)())func_ovl8_8037EB00},
	{(s16)0xFFC0, func_ovl8_8037C9E4},
	{0, func_ovl8_803721E8},
	{0, func_ovl8_80372224},
	{0, func_ovl8_80372250},
	{0, func_ovl8_803722A0},
	{0, func_ovl8_803722F0},
	{0, func_ovl8_80372348},
	{0, func_ovl8_80372358},
	{0, func_ovl8_80372360},
	{0, func_ovl8_803726CC},
	{0, NULL},
	{0, NULL},
};
#endif

extern Vec3i D_8038EFE0_1AB830;
extern s32 D_8038EFEC_1AB83C;
extern Vec3i D_8038EFF0_1AB840;
extern s16 D_8038EFFC_1AB84C;
extern db4Shorts D_8038F000_1AB850;
extern DBFont D_8038F008_1AB858;
extern dbUnknown8_13 D_8038F020_1AB870;
extern u16 D_8038F030_1AB880;
extern u16 D_8038F032_1AB882;
extern f32 D_8038F034_1AB884;
extern f32 D_8038F038_1AB888;
extern void (*D_8038F03C_1AB88C)(u16, u16, u8*, u8*);
extern void (*D_8038F040_1AB890)(u16, u16, u8*, u8*);
extern u16 D_8038F044_1AB894;
extern u16 D_8038F046_1AB896;
extern u16 D_8038F048_1AB898;
extern u8* D_8038F050_1AB8A0;
extern s32* D_8038F290_1ABAE0;

extern dbUnknownS14 D_8038FB90_1AC3E0;
extern db4Bytes D_8038FB98_1AC3E8;

extern void func_ovl8_8037D990(s32);
extern void func_ovl8_8037D9B4(db4Bytes*);
extern void func_ovl8_8037D9D0(db4Bytes*);
void func_ovl8_8037DAA0(s32, u32);
void func_ovl8_8037DFF8(Sprite*, u16, u16, u8, s32, s32*, s32*, f32);

#ifdef PORT
// 0x8037DD60
#else
// 8037DD60
#endif
void func_ovl8_8037DD60(DBMenuPosition* pos, char* text)
{
#ifdef PORT
    s16 temp_s5;
    char current;
    
    temp_s5 = D_8038F000_1AB850.arr[0];
    current = *text;
    
    while (current)
    {
        if (current == '\n')
        {
            D_8038F000_1AB850.arr[0] = temp_s5;
            D_8038F000_1AB850.arr[1] += D_8038F008_1AB858.height + D_8038F046_1AB896;
        }
        else
            func_ovl8_8037DAA0(pos, current);
#else
	s16 temp_s5;
	char current;
	
	temp_s5 = D_8038F000_1AB850.arr[0];
	current = *text;
	
	while (current)
	{
		if (current == '\n')
		{
			D_8038F000_1AB850.arr[0] = temp_s5;
			D_8038F000_1AB850.arr[1] += D_8038F008_1AB858.height + D_8038F046_1AB896;
		}
		else
			func_ovl8_8037DAA0(pos, current);
#endif

#ifdef PORT
        current = text[1];
        text++;
    }
#else
		current = text[1];
		text++;
	}
#endif
}

#ifdef PORT
// 0x8037DE1C
#else
// 8037DE1C
#endif
void func_ovl8_8037DE1C(DBMenuPosition* pos, char *str, DBMenuPosition *arg2) 
{
#ifdef PORT
    s16 temp_s6;
    s32 var_s3;
    s32 var_v1;
    u8 chr;
    DBFontPadding *temp_v0;
#else
	s16 temp_s6;
	s32 var_s3;
	s32 var_v1;
	u8 chr;
	DBFontPadding *temp_v0;
#endif

#ifdef PORT
    chr = *str;
    temp_s6 = D_8038F000_1AB850.arr[0];
    
    while (chr) 
    {
        if (chr == 0xA) 
        {
            D_8038F000_1AB850.arr[0] = temp_s6;
            D_8038F000_1AB850.arr[1] = D_8038F000_1AB850.arr[1] + D_8038F008_1AB858.height + D_8038F046_1AB896;
        } 
        else 
        {
            if (!(D_8038F048_1AB898 & 0xFF00)) 
            {
                if (chr & 0x80) 
                {
                    var_s3 = D_8038F020_1AB870.width + D_8038F044_1AB894;
                } 
                else if (((chr >= ' ') && (chr <= '~')) || ((chr >= 0xA0) && (chr < 0xE0))) 
                {
                    var_v1 = (chr >= 0xA1) ? chr - 'A' : chr - ' ';
                    var_s3 = ((D_8038F008_1AB858.width - D_8038F008_1AB858.padding[var_v1].left_padding) - D_8038F008_1AB858.padding[var_v1].right_padding) + D_8038F044_1AB894;
                }
                
                if ((D_8038F000_1AB850.arr[0] + var_s3) >= (arg2->x + arg2->w)) 
                {
                    D_8038F000_1AB850.arr[0] = temp_s6;
                    D_8038F000_1AB850.arr[1] = D_8038F000_1AB850.arr[1] + D_8038F008_1AB858.height + D_8038F046_1AB896;
                    chr = *str;
                }
            }
            
            func_ovl8_8037DAA0(pos, chr);
        }
        
        chr = str[1];
        str++;
    }
#else
	chr = *str;
	temp_s6 = D_8038F000_1AB850.arr[0];
	
	while (chr) 
	{
		if (chr == 0xA) 
		{
			D_8038F000_1AB850.arr[0] = temp_s6;
			D_8038F000_1AB850.arr[1] = D_8038F000_1AB850.arr[1] + D_8038F008_1AB858.height + D_8038F046_1AB896;
		} 
		else 
		{
			if (!(D_8038F048_1AB898 & 0xFF00)) 
			{
				if (chr & 0x80) 
				{
					var_s3 = D_8038F020_1AB870.width + D_8038F044_1AB894;
				} 
				else if (((chr >= ' ') && (chr <= '~')) || ((chr >= 0xA0) && (chr < 0xE0))) 
				{
					var_v1 = (chr >= 0xA1) ? chr - 'A' : chr - ' ';
					var_s3 = ((D_8038F008_1AB858.width - D_8038F008_1AB858.padding[var_v1].left_padding) - D_8038F008_1AB858.padding[var_v1].right_padding) + D_8038F044_1AB894;
				}
				
				if ((D_8038F000_1AB850.arr[0] + var_s3) >= (arg2->x + arg2->w)) 
				{
					D_8038F000_1AB850.arr[0] = temp_s6;
					D_8038F000_1AB850.arr[1] = D_8038F000_1AB850.arr[1] + D_8038F008_1AB858.height + D_8038F046_1AB896;
					chr = *str;
				}
			}
			
			func_ovl8_8037DAA0(pos, chr);
		}
		
		chr = str[1];
		str++;
	}
#endif
}

void func_ovl8_8037DFCC(s16 arg0, s16 arg1)
{
	D_8038F000_1AB850.arr[1] = arg1;
	D_8038F000_1AB850.arr[0] = arg0;
}

#ifdef PORT
// 0x8037DFF8
#else
// 8037DFF8
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/ovl8/ovl8_13_2/func_ovl8_8037DFF8.s")

#ifdef PORT
// 0x8037E6F4
#else
// 8037E6F4
#endif
u32 func_ovl8_8037E6F4(u8 arg0) {
#ifdef PORT
    u32 retVal;
    u32 var_a0;
    DBFontPadding* temp_v0;
#else
	u32 retVal;
	u32 var_a0;
	DBFontPadding* temp_v0;
#endif

#ifdef PORT
    if (D_8038F048_1AB898 & 0xFF00) 
    {
        D_8038F048_1AB898 = 0;
        
        retVal = D_8038F020_1AB870.width;
    }
    else if (arg0 & 0x80) 
    {
        D_8038F048_1AB898 = (arg0 & 0xFF) << 8;
        
        retVal = 0;
    }
    else if (((arg0 >= ' ') && (arg0 <= '~')) || ((arg0 >= 0xA0) && (arg0 < 0xE0))) 
    {
        var_a0 = (arg0 >= 0xA1) ? arg0 - 'A' : arg0 - ' ';
        
        retVal = (D_8038F008_1AB858.width - D_8038F008_1AB858.padding[var_a0].left_padding) - D_8038F008_1AB858.padding[var_a0].right_padding;
    }
    else retVal = 0;
    
    return retVal;
#else
	if (D_8038F048_1AB898 & 0xFF00) 
	{
		D_8038F048_1AB898 = 0;
		
		retVal = D_8038F020_1AB870.width;
	}
	else if (arg0 & 0x80) 
	{
		D_8038F048_1AB898 = (arg0 & 0xFF) << 8;
		
		retVal = 0;
	}
	else if (((arg0 >= ' ') && (arg0 <= '~')) || ((arg0 >= 0xA0) && (arg0 < 0xE0))) 
	{
		var_a0 = (arg0 >= 0xA1) ? arg0 - 'A' : arg0 - ' ';
		
		retVal = (D_8038F008_1AB858.width - D_8038F008_1AB858.padding[var_a0].left_padding) - D_8038F008_1AB858.padding[var_a0].right_padding;
	}
	else retVal = 0;
	
	return retVal;
#endif
}

#ifdef PORT
// 0x8037E7A8
#else
// 8037E7A8
#endif
s32 func_ovl8_8037E7A8(u8 * s) 
{
#ifdef PORT
    s32 width = 0;
#else
	s32 width = 0;
#endif

#ifdef PORT
    if (s == NULL) {
        return 0;
    }
    
    while (*s != 0) {
        u8 *p = s++;\
        width += func_ovl8_8037E6F4(*p);   
    }
    
    return width;
#else
	if (s == NULL) {
		return 0;
	}
	
	while (*s != 0) {
		u8 *p = s++;\
		width += func_ovl8_8037E6F4(*p);   
	}
	
	return width;
#endif
}

#ifdef PORT
// 0x8037E80C
#else
// 8037E80C
#endif
u16 func_ovl8_8037E80C()
{
#ifdef PORT
    return D_8038F008_1AB858.height;
#else
	return D_8038F008_1AB858.height;
#endif
}

#ifdef PORT
// 0x8037E818
#else
// 8037E818
#endif
u16 func_ovl8_8037E818()
{
#ifdef PORT
    return D_8038F044_1AB894;
#else
	return D_8038F044_1AB894;
#endif
}

#ifdef PORT
// 0x8037E824
#else
// 8037E824
#endif
u16 func_ovl8_8037E824()
{
#ifdef PORT
    return D_8038F046_1AB896;
#else
	return D_8038F046_1AB896;
#endif
}

#ifdef PORT
// 0x8037E830
#else
// 8037E830
#endif
typedef struct {
#ifdef PORT
    s32 unk0[0x10/4];
    u16 unk10;
    s32 unk14[0x10/4];
    s32 unk24;
#else
	s32 unk0[0x10/4];
	u16 unk10;
	s32 unk14[0x10/4];
	s32 unk24;
#endif
} UnkStruct8037E830;
s32 func_ovl8_8037E830(s32 arg0, UnkStruct8037E830 *arg1)
{
#ifdef PORT
    s32 sp34;
    s32 temp_v0;
    s32 sp2C;
    s32 var_v1;
#else
	s32 sp34;
	s32 temp_v0;
	s32 sp2C;
	s32 var_v1;
#endif

#ifdef PORT
    temp_v0 = arg1->unk10;
#else
	temp_v0 = arg1->unk10;
#endif

#ifdef PORT
    if (temp_v0 == 2)
        sp2C = 2;
    else if (temp_v0 == 5)
        sp2C = 3;
    
    sp34 = func_ovl8_803717A0(0x120);
    if (sp34 != 0)
    {
        func_ovl8_8037E97C(sp34, 0, 0, arg1, sp2C, arg1->unk24, arg0);
        var_v1 = sp34;
    }
    else
        var_v1 = 0;
    
    return var_v1;
#else
	if (temp_v0 == 2)
		sp2C = 2;
	else if (temp_v0 == 5)
		sp2C = 3;
	
	sp34 = func_ovl8_803717A0(0x120);
	if (sp34 != 0)
	{
		func_ovl8_8037E97C(sp34, 0, 0, arg1, sp2C, arg1->unk24, arg0);
		var_v1 = sp34;
	}
	else
		var_v1 = 0;
	
	return var_v1;
#endif
}

#ifdef PORT
// 0x8037E8C8
#else
// 8037E8C8
#endif
dbUnknown5* func_ovl8_8037E8C8(dbUnknown5 *arg0, dbUnknownLinkStruct *arg1, dbUnknownLink *arg2) 
{
#ifdef PORT
    if ((arg0 != 0) || ((arg0 = func_ovl8_803717A0(0x120)) != 0)) 
    {
        if (arg1 == NULL) 
        {
            arg1 = (dbUnknownLinkStruct *)&arg0->unk_dbunk5_0xB8;
            arg2 = (dbUnknownLink *)&arg0->unk_dbunk5_0x114;
            #line 238
            func_ovl8_803717E0(arg1);
            func_ovl8_8037C2D0(arg2);
            #line 245
        }
        
        func_ovl8_8037C710(arg0, arg1, arg2);
        
        arg0->unk_dbunk5_0x30 = &D_ovl8_8038A8A0;
        arg1->db_func = &D_ovl8_8038A980;
        arg2->unk_dbunklink_0x8 = (dbUnknownLink *)&D_ovl8_8038AAD8;
        arg0->unk_dbunk5_0x4C = (dbUnknownLinkStruct *)&D_ovl8_8038AB00;
    }
    
    return arg0;
#else
	if ((arg0 != 0) || ((arg0 = func_ovl8_803717A0(0x120)) != 0)) 
	{
		if (arg1 == NULL) 
		{
			arg1 = (dbUnknownLinkStruct *)&arg0->unk_dbunk5_0xB8;
			arg2 = (dbUnknownLink *)&arg0->unk_dbunk5_0x114;
			#line 265
			func_ovl8_803717E0(arg1);
			func_ovl8_8037C2D0(arg2);
			#line 272
		}
		
		func_ovl8_8037C710(arg0, arg1, arg2);
		
		arg0->unk_dbunk5_0x30 = &D_ovl8_8038A8A0;
		arg1->db_func = &D_ovl8_8038A980;
		arg2->unk_dbunklink_0x8 = (dbUnknownLink *)&D_ovl8_8038AAD8;
		arg0->unk_dbunk5_0x4C = (dbUnknownLinkStruct *)&D_ovl8_8038AB00;
	}
	
	return arg0;
#endif
}

#ifdef PORT
// 0x8037E97C
#else
// 8037E97C
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/ovl8/ovl8_13_2/func_ovl8_8037E97C.s")

#ifdef PORT
// 0x8037EB00
#else
// 8037EB00
#endif
void func_ovl8_8037EB00(dbUnknown5* arg0, s32 arg1)
{
#ifdef PORT
    s32 temp_a0;
#else
	s32 temp_a0;
#endif

#ifdef PORT
    if (arg0 != NULL)
    {
        arg0->unk_dbunk5_0x30 = &D_ovl8_8038A8A0,
        arg0->unk_dbunk5_0x38->db_func = &D_ovl8_8038A980;
        arg0->unk_dbunk5_0x34->unk_dbunklink_0x8 = &D_ovl8_8038AAD8;
        arg0->unk_dbunk5_0x4C = &D_ovl8_8038AB00;
#else
	if (arg0 != NULL)
	{
		arg0->unk_dbunk5_0x30 = &D_ovl8_8038A8A0,
		arg0->unk_dbunk5_0x38->db_func = &D_ovl8_8038A980;
		arg0->unk_dbunk5_0x34->unk_dbunklink_0x8 = &D_ovl8_8038AAD8;
		arg0->unk_dbunk5_0x4C = &D_ovl8_8038AB00;
#endif

#ifdef PORT
        temp_a0 = arg0->unk_dbunk5_0xB0;
        arg0->unk_dbunk5_0x48 = 0;
        arg0->unk_dbunk5_0xB0 = 0;
        arg0->unk_dbunk5_0xB4 = 0;
#else
		temp_a0 = arg0->unk_dbunk5_0xB0;
		arg0->unk_dbunk5_0x48 = 0;
		arg0->unk_dbunk5_0xB0 = 0;
		arg0->unk_dbunk5_0xB4 = 0;
#endif

#ifdef PORT
        if (temp_a0 != 0) {
            func_ovl8_8037B3E4(temp_a0);
        }
#else
		if (temp_a0 != 0) {
			func_ovl8_8037B3E4(temp_a0);
		}
#endif

#ifdef PORT
        func_ovl8_8037C92C(arg0, 0);
#else
		func_ovl8_8037C92C(arg0, 0);
#endif

#ifdef PORT
        if (arg1 != 0)
        {
            func_ovl8_8037C30C(arg0->unk_dbunk5_0x34, 0);
            func_ovl8_803718C4(arg0->unk_dbunk5_0x38, 0);
        }
        if (arg1 & 1)
        {
            func_ovl8_803717C0(arg0);
        }
    }
#else
		if (arg1 != 0)
		{
			func_ovl8_8037C30C(arg0->unk_dbunk5_0x34, 0);
			func_ovl8_803718C4(arg0->unk_dbunk5_0x38, 0);
		}
		if (arg1 & 1)
		{
			func_ovl8_803717C0(arg0);
		}
	}
#endif
}

#ifdef PORT
// 0x8037EBC8
#else
// 8037EBC8
#endif
void func_ovl8_8037EBC8(dbUnknown5* arg0) 
{
#ifdef PORT
    u16 sp56;
    u16 sp54;
    s32* sp50;
    s32 sp4C;
    char* str;
    s32 unused2;
    s32 a0;
    s32 a1;
    dbUnknownS14* sp38;
    Vec2h sp34;
    db4Bytes sp30;
    dbFunction* temp_v1;
    f32 unused3;
#else
	u16 sp56;
	u16 sp54;
	s32* sp50;
	s32 sp4C;
	char* str;
	s32 unused2;
	s32 a0;
	s32 a1;
	dbUnknownS14* sp38;
	Vec2h sp34;
	db4Bytes sp30;
	dbFunction* temp_v1;
	f32 unused3;
#endif

#ifdef PORT
    str = &arg0->unk_dbunk5_0xC.str;
    
    if (arg0->unk_dbunk5_0x48 != NULL) 
    {
        sp4C = func_ovl8_8037E7A8(str);
        
        temp_v1 = arg0->unk_dbunk5_0x4C;
        temp_v1[4].unk_dbfunc_0x4(temp_v1[4].unk_dbfunc_0x0 + (uintptr_t)&arg0->unk_dbunk5_0x40, &sp50);
        
        func_ovl8_8037D95C(&sp38);
        func_ovl8_8037D990(0x10);
#else
	str = &arg0->unk_dbunk5_0xC.str;
	
	if (arg0->unk_dbunk5_0x48 != NULL) 
	{
		sp4C = func_ovl8_8037E7A8(str);
		
		temp_v1 = arg0->unk_dbunk5_0x4C;
		temp_v1[4].unk_dbfunc_0x4(temp_v1[4].unk_dbfunc_0x0 + (uintptr_t)&arg0->unk_dbunk5_0x40, &sp50);
		
		func_ovl8_8037D95C(&sp38);
		func_ovl8_8037D990(0x10);
#endif

#ifdef PORT
        if (arg0->unk_dbunk5_0x0 == (0.0F, 0.0F)) 
        {
            func_ovl8_8037B434(arg0->unk_dbunk5_0xB4, &sp50, 0, &arg0->unk_dbunk5_0x38->bg_color);
            func_ovl8_8037D9D0(&D_ovl8_80389F4C);
            
            sp34.x = sp34.y = 6;
            func_ovl8_8037A5B8(arg0->unk_dbunk5_0xB4, &sp34, &sp30);
            func_ovl8_8037D9B4(&sp30);
#else
		if (arg0->unk_dbunk5_0x0 == (0.0F, 0.0F)) 
		{
			func_ovl8_8037B434(arg0->unk_dbunk5_0xB4, &sp50, 0, &arg0->unk_dbunk5_0x38->bg_color);
			func_ovl8_8037D9D0(&D_ovl8_80389F4C);
			
			sp34.x = sp34.y = 6;
			func_ovl8_8037A5B8(arg0->unk_dbunk5_0xB4, &sp34, &sp30);
			func_ovl8_8037D9B4(&sp30);
#endif

#ifdef PORT
            a0 = (sp54 / 2) - (sp4C / 2);
            a1 = (sp56 / 2) - (func_ovl8_8037E80C() / 2);
            func_ovl8_8037DFCC(a0, a1);
            func_ovl8_8037DD60(arg0->unk_dbunk5_0xB4, str);
        } 
        else 
        {
            func_ovl8_8037B434(arg0->unk_dbunk5_0xB0, &sp50, 1, &arg0->unk_dbunk5_0x38->bg_color);
            func_ovl8_8037D9D0(&D_ovl8_80389F50);
            
            sp34.x = sp34.y = 6;
            func_ovl8_8037A5B8(arg0->unk_dbunk5_0xB0, &sp34, &sp30);
            func_ovl8_8037D9B4(&sp30);
#else
			a0 = (sp54 / 2) - (sp4C / 2);
			a1 = (sp56 / 2) - (func_ovl8_8037E80C() / 2);
			func_ovl8_8037DFCC(a0, a1);
			func_ovl8_8037DD60(arg0->unk_dbunk5_0xB4, str);
		} 
		else 
		{
			func_ovl8_8037B434(arg0->unk_dbunk5_0xB0, &sp50, 1, &arg0->unk_dbunk5_0x38->bg_color);
			func_ovl8_8037D9D0(&D_ovl8_80389F50);
			
			sp34.x = sp34.y = 6;
			func_ovl8_8037A5B8(arg0->unk_dbunk5_0xB0, &sp34, &sp30);
			func_ovl8_8037D9B4(&sp30);
#endif

#ifdef PORT
            a0 = (sp54 / 2) - (sp4C / 2);
            a1 = (sp56 / 2) - (func_ovl8_8037E80C() / 2);
            func_ovl8_8037DFCC(a0 + 1, a1 + 1);
            func_ovl8_8037DD60(arg0->unk_dbunk5_0xB0, str);
        }
        
        func_ovl8_8037D908(&sp38);
    }
}
#else
			a0 = (sp54 / 2) - (sp4C / 2);
			a1 = (sp56 / 2) - (func_ovl8_8037E80C() / 2);
			func_ovl8_8037DFCC(a0 + 1, a1 + 1);
			func_ovl8_8037DD60(arg0->unk_dbunk5_0xB0, str);
		}
		
		func_ovl8_8037D908(&sp38);
	}
}
#endif
