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





## 附录

### AllOP.lua

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





```
void luaV_execute (lua_State *L, int nexeccalls)
```

