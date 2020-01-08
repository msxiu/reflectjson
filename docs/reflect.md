# C反射(reflect_t)的说明
反射在C#，JAVA等语言中用比较广泛，语言本身就支持；C里需要自己定义实现，本文用到的反射是指对结构的成员变量进行赋值与取值的机制。

有了反射机，可以帮我完成很多重复性的工作，从而简化程序的编写，让代码逻辑清析简单，达到提升开发效率的目的。

## 反射的类型
结构体成员有自己的数据类型，本机制提供以下反射类型：
```
/** 反射类型枚举 */
typedef enum REFLECT_TYPE {
	REFLECT_NONE		= 0x00,//未知
	REFLECT_INT			= 0x01,//符号整型
	REFLECT_UINT		= 0x02,//无符号整型
	REFLECT_TIME		= 0x03,//时间(time_t)类型
	REFLECT_FLOAT		= 0x04,//浮点
	REFLECT_BINBUF		= 0x05,//固定大小缓存区
	REFLECT_STRBUF		= 0x06,//字符串缓存区
	REFLECT_STRING		= 0x07,//字符串指针
	REFLECT_OBJBUF		= 0x08,//对象缓存区
	REFLECT_OBJECT		= 0x09,//对象指针
} reflectenum_t;
```
在反射实现时，需要提供一些反射所需的关键信息，以下是反射引用结构体的定义：
````
/** 反射引用结构体，用于存储反射信息 */
struct reflect {
	uint32_t	entire;//被反射结构体总体长度
	const char * fldname;//字段名称
	reflectenum_t type;//数据类型
	uint16_t offset;//相对于结构体偏移
	uint16_t size;//成员在结构体中占用字节长度
	getentityreflect_t getreflect;
};
````
被反射对象都是一组反射引用结构体实例数组，反射引用实例初始化如下：
````
typedef struct author {
    int id;//作者ID
    char name[64];//作者名称
    char summary[256];//作者简介
} author_t, * author_p;
//定义结构体的字段反射对象数组
static reflect_t reflds[] = {
    REFLECT_INIT(REFLECT_INT, author_t, id),
    REFLECT_INIT(REFLECT_STRBUF, author_t, name),
    REFLECT_INIT(REFLECT_STRBUF, author_t, summary),
    REFLECT_EMPTY
};
````
REFLECT_INIT,REFLECT_EMPTY是反射引用实例初始化的宏；反射引用实例初始化宏有如下：
````
/** 实例为空，一般用在数组尾部 */
#define REFLECT_EMPTY
/** 简单初始化 */
#define REFLECT_INIT(reftype, objtype, member)
/** 回调初始化，对于复杂对象，可能本就是一个结构体，可以使用回调返回该成员的反射引用数据 */
#define REFLECT_CALLBACK(reftype, objtype, member, handle)
/** 命名初始化，在反射中使用给定的名称初始化对象 */
#define REFLECT_NAMED(name, reftype, objtype, member)
/** 命名回调初始化，对于复杂对象，可能本就是一个结构体，可以使用回调返回该成员的反射引用数据 */
#define REFLECT_NAMECBK(name, reftype, objtype, member, handle)
````
初始化宏参数说明：
name：在反射中使用名称
reftype：成员的数据类型，enum REFLECT_TYPE;
objtype：结构体名称，如：author_t
member：成员名称
handle：回调返回反射引用数组的函数