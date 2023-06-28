# 5.1.4源码分析



## 5.1格式文件(LuaQ)

```
print("Hello World!")
```

使用`luac`生成文件如下

```
1B 4C 75 61 51 00 01 04 04 04 08 00 13 00 00 00
40 2E 5C 68 65 6C 6C 6F 5F 77 6F 72 6C 64 2E 6C
75 61 00 00 00 00 00 00 00 00 00 00 00 02 02 04
00 00 00 05 00 00 00 41 40 00 00 1C 40 00 01 1E
00 80 00 02 00 00 00 04 06 00 00 00 70 72 69 6E
74 00 04 0D 00 00 00 48 65 6C 6C 6F 20 57 6F 72
6C 64 21 00 00 00 00 00 04 00 00 00 01 00 00 00
01 00 00 00 01 00 00 00 01 00 00 00 00 00 00 00
00 00 00 00
```

```c
void luaU_header (char* h)
{
 int x=1;
 memcpy(h,LUA_SIGNATURE,sizeof(LUA_SIGNATURE)-1);
 h+=sizeof(LUA_SIGNATURE)-1;
 *h++=(char)LUAC_VERSION;
 *h++=(char)LUAC_FORMAT;
 *h++=(char)*(char*)&x;				/* endianness */
 *h++=(char)sizeof(int);
 *h++=(char)sizeof(size_t);
 *h++=(char)sizeof(Instruction);
 *h++=(char)sizeof(lua_Number);
 *h++=(char)(((lua_Number)0.5)==0);		/* is lua_Number integral? */
}

#define DumpMem(b,n,size,D)	DumpBlock(b,(n)*(size),D)
#define DumpVar(x,D)	 	DumpMem(&x,1,sizeof(x),D)
#define DumpCode(f,D)	    DumpVector(f->code,f->sizecode,sizeof(Instruction),D)

static void DumpFunction(const Proto* f, const TString* p, DumpState* D)
{
 DumpString((f->source==p || D->strip) ? NULL : f->source,D);
 DumpInt(f->linedefined,D);
 DumpInt(f->lastlinedefined,D);
 DumpChar(f->nups,D);
 DumpChar(f->numparams,D);
 DumpChar(f->is_vararg,D);
 DumpChar(f->maxstacksize,D);
 DumpCode(f,D);
 DumpConstants(f,D);
 DumpDebug(f,D);
}

static void DumpString(const TString* s, DumpState* D)
{
 if (s==NULL || getstr(s)==NULL)
 {
  size_t size=0;
  DumpVar(size,D);
 }
 else
 {
  size_t size=s->tsv.len+1;		/* include trailing '\0' */
  DumpVar(size,D); //size
  DumpBlock(getstr(s),size,D); //data
 }
}

static void DumpVector(const void* b, int n, size_t size, DumpState* D)
{
 DumpInt(n,D);
 DumpMem(b,n,size,D);
}
```



```
1B 4C 75 61 //LUA_SIGNATURE
51 //LUAC_VERSION
00 //LUAC_FORMAT
01 //endianness
04 //sizeof(int)
04 //sizeof(size_t)
04 //sizeof(Instruction)
08 //sizeof(lua_Number)
00 //is lua_Number integral

//DumpString("@.\hello_world.lua")
13 00 00 00 //DumpVar(strsize)
40 2E 5C 68 65 6C 6C 6F 5F 77 6F 72 6C 64 2E 6C 75 61 00 //DumpBlock(strdata)

//DumpInt(f->linedefined,D);
00 00 00 00
//DumpInt(f->lastlinedefined,D);
00 00 00 00
//DumpChar(f->nups,D);
00
//DumpChar(f->numparams,D);
00
//DumpChar(f->is_vararg,D);
02
//DumpChar(f->maxstacksize,D);
02

//#define DumpCode(f,D)	 DumpVector(f->code,f->sizecode,sizeof(Instruction),D)
//DumpInt(f->sizecode,D);
04 00 00 00
//DumpBlock(f->code,f->sizecode*sizeof(Instruction),D);
05 00 00 00 //GETGLOBAL       0 -1    ; print
41 40 00 00 //LOADK           1 -2    ; "Hello World!"
1C 40 00 01 //CALL            0 2 1
1E 00 80 00 //RETURN          0 1

//DumpConstants
//DumpInt(f->sizek,D);
02 00 00 00
//DumpChar(ttype(&f->k[i]),D);
04 //Tstring
//DumpString(rawtsvalue(o),D);
06 00 00 00
70 72 69 6E 74 00 //print
04 
0D 00 00 00 
48 65 6C 6C 6F 20 57 6F 72 6C 64 21 00 //Hello World!
//DumpInt(f->sizep,D);
00 00 00 00 

//DumpDebug
//DumpVector(f->lineinfo,f->sizelineinfo,sizeof(int),D);
04 00 00 00//size
//DumpMem(b,n,size,D);
01 00 00 00
//DumpInt(f->sizelocvars,D);
01 00 00 00
//DumpString(f->locvars[i].varname,D);
01 00 00 00 
//DumpInt(f->locvars[i].startpc,D);
01 00 00 00
//DumpInt(f->locvars[i].endpc,D);
00 00 00 00
//DumpInt(f->sizeupvalues,D)
00 00 00 00
//DumpString(f->upvalues[i],D);
//
```









## 基础类型

`lua.h`

```c
//类型枚举
#define LUA_TNONE		  (-1)//不存在
#define LUA_TNIL		   0  //无
#define LUA_TBOOLEAN	    1 //布尔
#define LUA_TLIGHTUSERDATA	2 //Userdata : void* 无自动GC
#define LUA_TNUMBER		    3 //数值
//-------GC--------
#define LUA_TSTRING		    4 //字符串
#define LUA_TTABLE		    5 //表 
#define LUA_TFUNCTION		6 //函数
#define LUA_TUSERDATA		7 //Userdata : void* 有自动GC (iscollectable)
#define LUA_TTHREAD		    8 //线程

//从LUA_TSTRING(4)以后的类型都需要自动GC
#define iscollectable(o)  (ttype(o) >= LUA_TSTRING)
```

### GC实现

```c
(lobject.h)
//GC类型基类，用于GC管理
#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked
typedef struct GCheader {
  CommonHeader;
} GCheader;

(lstate.h)
//包括了所有可GC类型
union GCObject {
  GCheader gch;
  union TString ts;
  union Udata u;
  union Closure cl;
  struct Table h;
  struct Proto p;
  struct UpVal uv;
  struct lua_State th;  /* 线程 */
};

(lobject.h)
//Lua所有类型
typedef union {
  GCObject *gc; //可GC类型 (4-8)
  void *p;      //LIGHTUSERDATA (2)
  lua_Number n; //数值(3)
  int b;        //布尔(1)
} Value;

//完整Lua类型字段
#define TValuefields	Value value; int tt //内容,类型
typedef struct lua_TValue {
  TValuefields; //lua_TValue.value, lua_TValue.tt
} TValue;
```

类型操作

```c
//类型判断 只需要看 tt字段
#define ttype(o)	((o)->tt)
#define ttisnil(o)	    (ttype(o) == LUA_TNIL)//只有类型没有值 0
...
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA)
//数值获取 先检验类型后取值
#define check_exp(c,e)		(lua_assert(c), (e))//取决于lua_assert定义
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b) //布尔 1
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n) //数值 2
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p)//lightuserdata 3
    
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc)//gc对象 4-8
#define rawtsvalue(o)	check_exp(ttisstring(o), &(o)->value.gc->ts)//string 4
#define hvalue(o)	check_exp(ttistable(o), &(o)->value.gc->h) //table 5
#define rawuvalue(o)	check_exp(ttisuserdata(o), &(o)->value.gc->u)//userdata 6
    
#define tsvalue(o)	(&rawtsvalue(o)->tsv)
#define uvalue(o)	(&rawuvalue(o)->uv)

#define clvalue(o)	check_exp(ttisfunction(o), &(o)->value.gc->cl)
#define thvalue(o)	check_exp(ttisthread(o), &(o)->value.gc->th)
#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))
```





```c
typedef union TString {
  L_Umaxalign dummy;  //对齐字长
  struct {
    CommonHeader;
    lu_byte reserved;  //保留字符串 GC开关
    unsigned int hash; //hash表索引
    size_t len;
  } tsv;
} TString;
typedef LUAI_USER_ALIGNMENT_T L_Umaxalign;
#define LUAI_USER_ALIGNMENT_T union { double u; void *s; long l; }

(lstate.h)
//全局字符串表 global_state.strt
typedef struct stringtable {
  GCObject **hash; //global_state.strt.hash[ts->hash] hash冲突时通过GCOject结构串链表
  lu_int32 nuse;  //使用计数器
  int size;
} stringtable;

```



```c
typedef union TKey {
  struct {
    TValuefields;
    struct Node *next;  /* for chaining */
  } nk;
  TValue tvk;
} TKey;

typedef struct Node {
  TValue i_val;
  TKey i_key;
} Node;

typedef struct Table {
  CommonHeader;
  lu_byte flags;  //缓存表里的meta method 1代表不存在 
  lu_byte lsizenode;  //hash桶大小的log2  hash桶总是按翻倍扩增
  struct Table *metatable; //meta
  TValue *array;  //[]数组部分 仅当key是小于数组大小的正整数 `ipairs(t)`
  Node *node; //Hash桶  除了上述情况的key都先hash然后访问hash桶
  Node *lastfree;  //指向Hash桶尾部
  GCObject *gclist; 
  int sizearray;  //[]数组部分大小
} Table;

(ltm.h)
typedef enum {
  TM_INDEX,
  TM_NEWINDEX,
  TM_GC,
  TM_MODE,
  TM_EQ,  /* last tag method with `fast' access */
  TM_ADD,
  TM_SUB,
  TM_MUL,
  TM_DIV,
  TM_MOD,
  TM_POW,
  TM_UNM,
  TM_LEN,
  TM_LT,
  TM_LE,
  TM_CONCAT,
  TM_CALL,
  TM_N		/* number of elements in the enum */
} TMS;
```







```c
typedef union Udata {
  L_Umaxalign dummy;  /* ensures maximum alignment for `local' udata */
  struct {
    CommonHeader;
    struct Table *metatable;
    struct Table *env;
    size_t len;
  } uv;
} Udata;
```



```c
//函数原型
typedef struct Proto {
  CommonHeader;
  TValue *k;  /* constants used by the function */
  Instruction *code;
  struct Proto **p;  /* functions defined inside the function */
  int *lineinfo;  /* map from opcodes to source lines */
  struct LocVar *locvars;  /* information about local variables */
  TString **upvalues;  /* upvalue names */
  TString  *source;
  int sizeupvalues;
  int sizek;  /* size of `k' */
  int sizecode;
  int sizelineinfo;
  int sizep;  /* size of `p' */
  int sizelocvars;
  int linedefined;
  int lastlinedefined;
  GCObject *gclist;
  lu_byte nups;  /* number of upvalues */
  lu_byte numparams;
  lu_byte is_vararg;
  lu_byte maxstacksize;
} Proto;
```








```c
struct lua_State {
  //CommonHeader
   GCObject *next;
   	//union GCObject {
   	//  GCheader gch;// GCObject *next; lu_byte tt; lu_byte marked
   	//  union TString ts; // 字符串类型
   	//  union Udata u;    // Userdata类型
   	//  union Closure cl; // Closure
   	//  struct Table h;
   	//  struct Proto p;
   	//  struct UpVal uv;
   	//  struct lua_State th;  /* thread */
   	//};
  lu_byte tt;//lu_byte==unsigned char
  lu_byte marked;
    
    
  lu_byte status;
  StkId top;  /* first free slot in the stack */
  StkId base;  /* base of current function */
  global_State *l_G;
  CallInfo *ci;  /* call info for current function */
  const Instruction *savedpc;  /* `savedpc' of current function */
  StkId stack_last;  /* last free slot in the stack */
  StkId stack;  /* stack base */
  CallInfo *end_ci;  /* points after end of ci array*/
  CallInfo *base_ci;  /* array of CallInfo's */
  int stacksize;
  int size_ci;  /* size of array `base_ci' */
  unsigned short nCcalls;  /* number of nested C calls */
  unsigned short baseCcalls;  /* nested C calls when resuming coroutine */
  lu_byte hookmask;
  lu_byte allowhook;
  int basehookcount;
  int hookcount;
  lua_Hook hook;
  TValue l_gt;  /* table of globals */
  TValue env;  /* temporary place for environments */
  GCObject *openupval;  /* list of open upvalues in this stack */
  GCObject *gclist;
  struct lua_longjmp *errorJmp;  /* current error recover point */
  ptrdiff_t errfunc;  /* current error handling function (stack index) */
};
```





## 还原OpCode乱序

### Dump对比

allopcodes.lua

```lua
local u1,u2,u3
function f1(a1,a2,...)
    local l0 = a1; -- move
    local l1 = 1 -- loadk
    local l2 = true -- loadbool
    local l3 = nil -- loadnil
    local l4 = u1[g1] -- gettupval, getglobal, gettable
    g1 = l1 -- setglobal
    u2 = l2 -- setupval
    l3[l2] = l1 -- settable
    local l5 = { -- newtable
        l1, l2; -- move, setlist
        x = l2 -- settable
    }
    local l6 = l5:x() -- self, call
    local l7 = -((l0+l1-l2)*l3/l4%l5)^l6 -- add, sub, mul, div, mod, pow, unm
    local l8 = #(not l7) -- not, len
    local l9 = l7..l8 -- concat
    if l1==l2 and l2<l3 or l3<=l4 then -- eq, lt, le, jmp
        for i = 1, 10, 2 do -- forprep
            l0 = l0 and l2 -- test
        end -- forloop
    else -- jmp
        for k,v in ipairs(l5) do
            l4 = l5 or l6 -- testset
        end -- tforloop
    end
    do
        local l21, l22 = ... -- vararg
        local function f2() -- closure
            return l21, l22
        end
        f2(k,v) -- call
    end --close
    return f1() -- return, tailcall
end
```



C API

```c
lua_State* L = luaL_newstate();
luaL_loadbuffer or luaL_loadstring or luaL_loadfile
Closure* c=(Closure*)lua_topointer(L, -1);
Proto* f = c->l.p;//Closure如果改了偏移会不同
luaU_dump(L, f, writer, D, 0);//非导出函数， string.dump
lua_close(L);
```



Lua API

```
-- luaopen_string 此库必须打开
allopcodes() allopcodes.lua文件内容 end
chunk = string.dump(allopcodes)
if luaopen_io then --如果 io 可用，则可以直接写文件
    file:write(chunk)
else
    output(chunk,to_any_where) --想办法把 chunk 搞出来,直接读内存也行
    return chunk
end
```



#### 缓解措施

避免相关函数导出





### luaP数组

luaP_opnames

luaP_opmodes

#### 缓解措施

删除/注释相关代码



### OpMode

```c++
/*
** masks for instruction properties. The format is:
** bits 0-1: op mode
** bits 2-3: C arg mode
** bits 4-5: B arg mode
** bit 6: instruction set register A
** bit 7: operator is a test
*/  
enum OpArgMask {
  OpArgN,  /* argument is not used */
  OpArgU,  /* argument is used */
  OpArgR,  /* argument is a register or a jump offset */
  OpArgK   /* argument is a constant or register/constant */
};
enum OpMode {iABC, iABx, iAsBx};  /* basic instruction format */

#define opmode(t,a,b,c,m) (((t)<<7) | ((a)<<6) | ((b)<<4) | ((c)<<2) | (m))
const lu_byte luaP_opmodes[NUM_OPCODES] = {
/*       T  A    B       C     mode		   opcode	*/
  opmode(0, 1, OpArgR, OpArgN, iABC) 		/* OP_MOVE */
 ,opmode(0, 1, OpArgK, OpArgN, iABx)		/* OP_LOADK */
 ,opmode(0, 1, OpArgU, OpArgU, iABC)		/* OP_LOADBOOL */
 ,opmode(0, 1, OpArgR, OpArgN, iABC)		/* OP_LOADNIL */
 ,opmode(0, 1, OpArgU, OpArgN, iABC)		/* OP_GETUPVAL */
 ,opmode(0, 1, OpArgK, OpArgN, iABx)		/* OP_GETGLOBAL */
 ,opmode(0, 1, OpArgR, OpArgK, iABC)		/* OP_GETTABLE */
 ,opmode(0, 0, OpArgK, OpArgN, iABx)		/* OP_SETGLOBAL */
 ,opmode(0, 0, OpArgU, OpArgN, iABC)		/* OP_SETUPVAL */
 ,opmode(0, 0, OpArgK, OpArgK, iABC)		/* OP_SETTABLE */
 ,opmode(0, 1, OpArgU, OpArgU, iABC)		/* OP_NEWTABLE */
 ,opmode(0, 1, OpArgR, OpArgK, iABC)		/* OP_SELF */
 ,opmode(0, 1, OpArgK, OpArgK, iABC)		/* OP_ADD */
 ,opmode(0, 1, OpArgK, OpArgK, iABC)		/* OP_SUB */
 ,opmode(0, 1, OpArgK, OpArgK, iABC)		/* OP_MUL */
 ,opmode(0, 1, OpArgK, OpArgK, iABC)		/* OP_DIV */
 ,opmode(0, 1, OpArgK, OpArgK, iABC)		/* OP_MOD */
 ,opmode(0, 1, OpArgK, OpArgK, iABC)		/* OP_POW */
 ,opmode(0, 1, OpArgR, OpArgN, iABC)		/* OP_UNM */
 ,opmode(0, 1, OpArgR, OpArgN, iABC)		/* OP_NOT */
 ,opmode(0, 1, OpArgR, OpArgN, iABC)		/* OP_LEN */
 ,opmode(0, 1, OpArgR, OpArgR, iABC)		/* OP_CONCAT */
 ,opmode(0, 0, OpArgR, OpArgN, iAsBx)		/* OP_JMP */
 ,opmode(1, 0, OpArgK, OpArgK, iABC)		/* OP_EQ */
 ,opmode(1, 0, OpArgK, OpArgK, iABC)		/* OP_LT */
 ,opmode(1, 0, OpArgK, OpArgK, iABC)		/* OP_LE */
 ,opmode(1, 1, OpArgR, OpArgU, iABC)		/* OP_TEST */
 ,opmode(1, 1, OpArgR, OpArgU, iABC)		/* OP_TESTSET */
 ,opmode(0, 1, OpArgU, OpArgU, iABC)		/* OP_CALL */
 ,opmode(0, 1, OpArgU, OpArgU, iABC)		/* OP_TAILCALL */
 ,opmode(0, 0, OpArgU, OpArgN, iABC)		/* OP_RETURN */
 ,opmode(0, 1, OpArgR, OpArgN, iAsBx)		/* OP_FORLOOP */
 ,opmode(0, 1, OpArgR, OpArgN, iAsBx)		/* OP_FORPREP */
 ,opmode(1, 0, OpArgN, OpArgU, iABC)		/* OP_TFORLOOP */
 ,opmode(0, 0, OpArgU, OpArgU, iABC)		/* OP_SETLIST */
 ,opmode(0, 0, OpArgN, OpArgN, iABC)		/* OP_CLOSE */
 ,opmode(0, 1, OpArgU, OpArgN, iABx)		/* OP_CLOSURE */
 ,opmode(0, 1, OpArgU, OpArgN, iABC)		/* OP_VARARG */
};
```



### OpCode

```
  Instructions can have the following fields:
	`A' : 8 bits
	`B' : 9 bits
	`C' : 9 bits
	`Bx' : 18 bits (`B' and `C' together)
	`sBx' : signed Bx
```



```
iABC | Opcode:0-7 A:8-15   C:16-24 B:24-31 
iABx | Opcode:0-7 A:8-15  Bx:16-31
iAsBx| Opcode:0-7 A:8-15 sBx:16-31
iAx  | Opcode:0-7 A:8-31
```





```c
typedef enum {
/*----------------------------------------------------------------------
name		args	description
------------------------------------------------------------------------*/
OP_MOVE,/*	A B	R(A) := R(B)					*/
OP_LOADK,/*	A Bx	R(A) := Kst(Bx)					*/
OP_LOADBOOL,/*	A B C	R(A) := (Bool)B; if (C) pc++			*/
OP_LOADNIL,/*	A B	R(A) := ... := R(B) := nil			*/
OP_GETUPVAL,/*	A B	R(A) := UpValue[B]				*/

OP_GETGLOBAL,/*	A Bx	R(A) := Gbl[Kst(Bx)]				*/
OP_GETTABLE,/*	A B C	R(A) := R(B)[RK(C)]				*/

OP_SETGLOBAL,/*	A Bx	Gbl[Kst(Bx)] := R(A)				*/
OP_SETUPVAL,/*	A B	UpValue[B] := R(A)				*/
OP_SETTABLE,/*	A B C	R(A)[RK(B)] := RK(C)				*/

OP_NEWTABLE,/*	A B C	R(A) := {} (size = B,C)				*/

OP_SELF,/*	A B C	R(A+1) := R(B); R(A) := R(B)[RK(C)]		*/

OP_ADD,/*	A B C	R(A) := RK(B) + RK(C)				*/
OP_SUB,/*	A B C	R(A) := RK(B) - RK(C)				*/
OP_MUL,/*	A B C	R(A) := RK(B) * RK(C)				*/
OP_DIV,/*	A B C	R(A) := RK(B) / RK(C)				*/
OP_MOD,/*	A B C	R(A) := RK(B) % RK(C)				*/
OP_POW,/*	A B C	R(A) := RK(B) ^ RK(C)				*/
OP_UNM,/*	A B	R(A) := -R(B)					*/
OP_NOT,/*	A B	R(A) := not R(B)				*/
OP_LEN,/*	A B	R(A) := length of R(B)				*/

OP_CONCAT,/*	A B C	R(A) := R(B).. ... ..R(C)			*/

OP_JMP,/*	sBx	pc+=sBx					*/

OP_EQ,/*	A B C	if ((RK(B) == RK(C)) ~= A) then pc++		*/
OP_LT,/*	A B C	if ((RK(B) <  RK(C)) ~= A) then pc++  		*/
OP_LE,/*	A B C	if ((RK(B) <= RK(C)) ~= A) then pc++  		*/

OP_TEST,/*	A C	if not (R(A) <=> C) then pc++			*/ 
OP_TESTSET,/*	A B C	if (R(B) <=> C) then R(A) := R(B) else pc++	*/ 

OP_CALL,/*	A B C	R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1)) */
OP_TAILCALL,/*	A B C	return R(A)(R(A+1), ... ,R(A+B-1))		*/
OP_RETURN,/*	A B	return R(A), ... ,R(A+B-2)	(see note)	*/

OP_FORLOOP,/*	A sBx	R(A)+=R(A+2);
			if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }*/
OP_FORPREP,/*	A sBx	R(A)-=R(A+2); pc+=sBx				*/

OP_TFORLOOP,/*	A C	R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2)); 
                        if R(A+3) ~= nil then R(A+2)=R(A+3) else pc++	*/ 
OP_SETLIST,/*	A B C	R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B	*/

OP_CLOSE,/*	A 	close all variables in the stack up to (>=) R(A)*/
OP_CLOSURE,/*	A Bx	R(A) := closure(KPROTO[Bx], R(A), ... ,R(A+n))	*/

OP_VARARG/*	A B	R(A), R(A+1), ..., R(A+B-1) = vararg		*/
} OpCode;

```





### execute对比

```c+
//lvm.c
void luaV_execute (lua_State *L, int nexeccalls);
```





#### case OP_MOVE

```c++
/* from stack to (same) stack */
const TValue *o2=(rb);
TValue *o1=(ra); 
o1->value = o2->value;
o1->tt=o2->tt;
//checkliveness(G(L),o1);
```

```c++
        v147 = (int *)&level[v149 >> 23];//o2 = RB(i)
        v146 = val;//o1=ra
        v146->value.b = *v147;//o1->value = o2->value;
        *(&v146->value.b + 1) = v147[1];
        v146->tt = v147[2];//o1->tt=o2->tt;
        continue;
```

```c++
        v16 = v6 >> 23;
        v9->value.b = v4[v16].value.b;
        *(&v9->value.b + 1) = *(&v4[v16].value.b + 1);
        v9->tt = v4[v16].tt;
        continue;
```

#### case OP_LOADK

```
KBx(i)
/* to stack (not from same stack) */
const TValue *o2=(rb);
TValue *o1=(ra); 
o1->value = o2->value;
o1->tt=o2->tt;
```



```c++
        v145 = (int *)(v151 + 16 * (v149 >> 14));
        v144 = val;
        v4 = v145[1];
        v5 = val;
        val->value.b = *v145;
        *(&v5->value.b + 1) = v4;
        v144->tt = v145[2];
        continue;
```



#### case OP_LOADBOOL



#### arith_op

```c++
   arith_op(op,tm) { 
        TValue *rb = RKB(i); 
        TValue *rc = RKC(i);
       //如果两个操作数都是数值 直接通过展开宏里的算式计算数值
        if (ttisnumber(rb) && ttisnumber(rc)) { 
          lua_Number nb = nvalue(rb), nc = nvalue(rc); 
          TValue *i_o=ra; 
          i_o->value.n=op(nb, nc); 
          i_o->tt=LUA_TNUMBER;
        } 
        else 
            L->savedpc = pc;
       		//否则通过luaV_tonumber类型转换成数值类型再通过算式计算
       		Arith(L, ra, rb, rc, tm); 
       		base = L->base;
      }
  case OP_ADD: {//12
    arith_op(luai_numadd, TM_ADD); //5
    continue;
  }
  case OP_SUB: {
    arith_op(luai_numsub, TM_SUB); //6
    continue;
  }
  case OP_MUL: {
    arith_op(luai_nummul, TM_MUL); //7
    continue;
  }
  case OP_DIV: {
    arith_op(luai_numdiv, TM_DIV); //8
    continue;
  }
  case OP_MOD: {
    arith_op(luai_nummod, TM_MOD); //9
    continue;
  }
  case OP_POW: {//17
    arith_op(luai_numpow, TM_POW); //10
    continue;
  }
 case OP_UNM: { //一元操作 没有用arith宏
        TValue *rb = RB(i);
        if (ttisnumber(rb)) {
          lua_Number nb = nvalue(rb);
          setnvalue(ra, luai_numunm(nb));
        }
        else {
          Protect(Arith(L, ra, rb, rb, TM_UNM));
        }
        continue;
      }
```

核心特征

```
Arith(a1, a2, a3, a4, TM_CODE)
//x86
.text:10806469 6A 05                         push    5                               ; lua_TValue *
.text:1080646B 53                            push    ebx                             ; lua_TValue *
.text:1080646C 8B D1                         mov     nexeccalls, L
.text:1080646E 89 77 18                      mov     [edi+18h], esi
.text:10806471 50                            push    eax                             ; lua_State *
.text:10806472 8B CF                         mov     L, edi
.text:10806474 DD D8                         fstp    st
.text:10806476 E8 95 F9 FF FF                call    Arith
```

```
call_binTM(L, rb, luaO_nilobject, ra, TM_LEN)
call_binTM(L, rb, rc, ra, op)

typedef enum {
  TM_INDEX,//0
  TM_NEWINDEX,//1
  TM_GC,  //2
  TM_MODE,//3
  TM_EQ,  //4  /* last tag method with `fast' access */
  TM_ADD, //5
  TM_SUB, //6
  TM_MUL, //7
  TM_DIV, //8
  TM_MOD, //9
  TM_POW, //10
  TM_UNM, //11
  TM_LEN, //12
  TM_LT,  //13
  TM_LE,  //14
  TM_CONCAT,//15
  TM_CALL,//16
  TM_N	  //17	/* number of elements in the enum */
} TMS;
```



#### API

```c++
//luaV_gettable
case OP_GETGLOBAL:
	TValue *rb = KBx(i);
    ttype(rb) == LUA_TSTRING; //4
	luaV_gettable(L, &g, rb, ra);
case OP_GETTABLE:
	luaV_gettable(L, RB(i), RKC(i), ra);
case OP_SELF:
	luaV_gettable(L, rb, RKC(i), ra);

//luaV_settable
case OP_SETGLOBAL:
    luaV_settable(L, &g, KBx(i), ra);
case OP_SETTABLE:
    luaV_settable(L, ra, RKB(i), RKC(i));

//luaC_barrierf
case OP_SETUPVAL:
	luaC_barrier(L, uv, ra);
	//if (valiswhite(v) && isblack(obj2gco(p)))  \
	//	luaC_barrierf(L,obj2gco(p),gcvalue(v));

//luaH_new
case OP_NEWTABLE:
	sethvalue(L, ra, luaH_new(L, luaO_fb2int(b), luaO_fb2int(c)));

//Arith
//见上一节


//luaH_getn
case OP_LEN:
	setnvalue(ra, cast_num(luaH_getn(hvalue(rb))));

//luaV_concat
case OP_CONCAT:
 	Protect(luaV_concat(L, c-b+1, c); luaC_checkGC(L));
	//luaC_step

//luaV_equalval 
case OP_EQ:
 	luaV_equalval((lua_State *)v69, (const lua_TValue *)v122, (const lua_TValue *)v123);

//luaV_lessthan
case OP_LT:
	if (luaV_lessthan(L, RKB(i), RKC(i)) == GETARG_A(i))dojump(L, pc, GETARG_sBx(*pc));

//lessequal
case OP_LE: 
	if (lessequal(L, RKB(i), RKC(i)) == GETARG_A(i))dojump(L, pc, GETARG_sBx(*pc));

//luaD_precall
case OP_CALL:
	switch (luaD_precall(L, ra, nresults) case 0/1;
case OP_TAILCALL:
    switch (luaD_precall(L, ra, LUA_MULTRET))case 0/1; //LUA_MULTRET==-1
            luaF_close(L, ci->base);

case OP_RETURN:
    luaF_close(L, base);
    luaD_poscall(L, ra);
	//lua_assert(GET_OPCODE(*((L->ci)->savedpc - 1)) == OP_CALL);

case OP_FORPREP:
	luaG_runerror(L, LUA_QL("for") " initial value must be a number");
	luaG_runerror(L, LUA_QL("for") " limit must be a number");
	luaG_runerror(L, LUA_QL("for") " step must be a number");

case OP_SETLIST:
	luaH_resizearray(L, h, last);
	luaH_setnum(L, h, last--), val);

case OP_CLOSE:
	luaF_close(L, ra);
case OP_CLOSURE:
	luaF_newLclosure(L, nup, cl->env);
	luaF_findupval(L, base + GETARG_B(*pc));
	luaD_reallocstack(L, L->stacksize - EXTRA_STACK - 1);
	luaC_step(L);
case OP_VARARG:
	luaD_growstack(L, n);
	luaD_reallocstack(L, L->stacksize - EXTRA_STACK - 1)
 
 case OP_FORLOOP:
    luaD_call(L, funca, (v149 >> 14) & 0x1FF);
```



```
case OP_NOT:
 case OP_JMP:

 case OP_TEST: 
 case OP_TESTSET:

```





#### 有明显字符串特征

```c++
void luaG_typeerror (lua_State *L, const TValue *o, const char *op) {
  const char *name = NULL;
  const char *t = luaT_typenames[ttype(o)];
  const char *kind = (isinstack(L->ci, o)) ?
                         getobjname(L, L->ci, cast_int(o - L->base), &name) :
                         NULL;
  if (kind)
    luaG_runerror(L, "attempt to %s %s " LUA_QS " (a %s value)",
                op, kind, name, t);
  else
    luaG_runerror(L, "attempt to %s a %s value", op, t);
}

//luaV_concat
void luaG_concaterror (lua_State *L, StkId p1, StkId p2) {
  if (ttisstring(p1) || ttisnumber(p1)) p1 = p2;
  lua_assert(!ttisstring(p1) && !ttisnumber(p1));
  luaG_typeerror(L, p1, "concatenate");
}


void luaG_aritherror (lua_State *L, const TValue *p1, const TValue *p2) {
  TValue temp;
  if (luaV_tonumber(p1, &temp) == NULL)
    p2 = p1;  /* first operand is wrong */
  luaG_typeerror(L, p2, "perform arithmetic on");
}


int luaG_ordererror (lua_State *L, const TValue *p1, const TValue *p2) {
  const char *t1 = luaT_typenames[ttype(p1)];
  const char *t2 = luaT_typenames[ttype(p2)];
  if (t1[2] == t2[2])
    luaG_runerror(L, "attempt to compare two %s values", t1);
  else
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
  return 0;
}
```





```c++
case OP_LEN : luaG_typeerror(L, rb, "get length of");
case OP_FORPREP: luaG_runerror(L, LUA_QL("for") " initial value must be a number");
                 luaG_runerror(L, LUA_QL("for") " limit must be a number");
                 luaG_runerror(L, LUA_QL("for") " step must be a number");
```

