# 5.1.4源码分析



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

