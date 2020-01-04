#ifndef LLQ_JSON_OBJECT_H_
#define LLQ_JSON_OBJECT_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "llqreflect.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif


typedef enum JSONFIELD_TYPE {//JSON节点类型
	JSONFIELD_BASE		= 0,//表示基本类型,如int,long,date等
	JSONFIELD_STRING	= 1,//字符串类型
	JSONFIELD_OBJECT	= 2,//表示复合类型
	JSONFIELD_ARRAY		= 3,//数组对象
} jsonenum_t;
typedef enum CHARSET_ENCODING {//字符编码方式
	CHARUTF8 = 0,		//UTF-8编编
	CHARASCII = 1		//GBK，ASCII编译
} charenum_t;

struct jsonobject;

typedef struct jsonobject jsonobject_t, * jsonobject_p;
struct jsonobject {//JSON中的一个字段描述,为了节省空间,以流的形式存储
	jsonenum_t type;//类型
	char name[256];//字段名称
	char* value;//表示对象是JSONFIELD_BASE
	struct {
        unsigned int size;//申请数据字节长度
		unsigned int used;//已使用记录个数
        jsonobject_p* ptr;//数据指针地址
    } childs;//子节点对象
};

//申请一个jsonobject 对象
extern jsonobject_p jsonobject_alloc(jsonenum_t t, const char* nodename);
/**将实体v生成json对象, flds为选取字段*/
extern jsonobject_p jsonobject_formentity(void* v, reflect_p flds);
/**将实体v生成json对象, flds为选取字段, 可通过以NULL结尾的可变参数选取字段*/
extern jsonobject_p jsonobject_fromentityflds(void* v, reflect_p flds, ...);
/**从字符串中解析json对象,返回解析到的json对象实体，失败返回NULL*/
extern jsonobject_p jsonobject_parseof(const char* json, charenum_t ascii);


/**添加一个整数类型的子节点*/
extern int jsonobject_addchildint(jsonobject_p o, const char* key, int64_t v);
/**添加一个小数类型的子节点*/
extern int jsonobject_addchildfloat(jsonobject_p o, const char* key, double v);
/**添加一个字符串的子节点*/
extern int jsonobject_addchildstring(jsonobject_p o, const char* key, const char* v);
/** 给数组json对象o添加数组成员,返回添加记录条数*/
extern size_t jsonobject_arraychilds(jsonobject_p o, reflect_p flds, void* records, size_t recordsize, size_t recordcount);
/**给json对象o添加一个数组对象的子节点,返回添加记录条数*/
extern size_t jsonobject_addarraychild(jsonobject_p o, const char* key, reflect_p flds, void* records, size_t recordsize, size_t recordcount);
/**添加实体子成员,成功返回1,失败返回0*/
extern size_t jsonobject_addchildentity(jsonobject_p o, const char* key, reflect_p flds, void* obj);



/**将jsonobject对象转换成字符串形式*/
extern char* jsonobject_tojsonstring(jsonobject_p node);
/**将一个JSON对象还原到实体对象*/
extern size_t jsonobject_toentity(jsonobject_p o, reflect_p flds, void* e);

extern void jsonobject_destory(jsonobject_p o);//消毁jsonobject对象


extern const char* jsonentity2string(void* entity, reflect_p flds);//将结构体对象转换json字符串
extern int jsonentity4string(void* entity, reflect_p flds, const char* str);//将json字符串解析并反射绑定到结构体对象

/**序列化，将结构体对象中的数据生成json字符串，保存到文件中*/
extern int jsonentity_serialization(void* entity, reflect_p flds, const char* faddr);
/**反序列化，从文件中读取json字符串，反射绑定到结构体对象*/
extern int jsonentity_deserialization(void* entity, reflect_p flds, const char* faddr);
extern void jsonentity_free(void* entity, reflect_p flds);//释放对象申请的内存空间


/**获取节点值数据*/
static INLINE char* jsonobject_getvalue(jsonobject_p o) {
	if(o && (JSONFIELD_BASE == o->type || JSONFIELD_STRING == o->type)) {
		return o->value; 
	}
	return NULL; 
}
/**获取节点值数据*/
static INLINE size_t jsonobject_getvalueint64(jsonobject_p o, int64_t *v){
	if(o && (JSONFIELD_BASE == o->type || JSONFIELD_STRING == o->type)) {
		(*v) = atol(o->value);
		return 1;
	}
	return 0; 
}
/**获取节点值数据*/
static INLINE size_t jsonobject_getvaluefloat(jsonobject_p o, double *v){
	if(o && (JSONFIELD_BASE == o->type || JSONFIELD_STRING == o->type)) {
		(*v) = atof(o->value);
		return 1;
	}
	return 0; 
}

/**设置节点值*/
static INLINE size_t jsonobject_valueint64(jsonobject_p o, int64_t v){
	if(o && JSONFIELD_BASE == o->type) {
		if(o->value) { free(o->value); o->value = NULL; }
		if((o->value = jsonobject_allocbasevalue())) {
			sprintf(o->value, "%lu", v);
		}
		return 1;
	}
	return 0;
}
/**设置节点值*/
static INLINE size_t jsonobject_valuefloat(jsonobject_p o, double v){
	if(o && JSONFIELD_BASE == o->type) {
		if(o->value) { free(o->value); o->value = NULL; }
		if((o->value = jsonobject_allocbasevalue())) {
			sprintf(o->value, "%lf", v); 
		}
		return 1;
	}
	return 0;
}
/**设置节点值*/
static INLINE size_t jsonobject_valuestring(jsonobject_p o, const char* v){
	if(o && JSONFIELD_STRING == o->type) {
		if(o->value) { free(o->value); o->value = NULL; }
		if(v) {
			size_t sln = strlen(v);
			if((o->value = (char*)calloc(1, sln + 1))) { strcpy(o->value, v); }
		}
		return 1;
	}
	return 0;
}


//**** chile node operator ********************************************************************************************************
//设置节点节称
static INLINE void jsonobject_setnodename(jsonobject_p o, const char* name) {
	if(o) {
		strncpy(o->name, name, sizeof(o->name));
	}
}

static INLINE int jsonobject_getchildvalue(jsonobject_p o, const char* key, const char **v) {
	jsonobject_p child;
	if(o && key && v && (child = jsonobject_getchild(o, key))) {
		(*v) = child->value;
		return 1;
	}
	(*v) = NULL;
	return 0;
}
static INLINE int jsonobject_getchildint64(jsonobject_p o, const char* key, int64_t *v) {
	jsonobject_p child;
	if(o && key && v && (child = jsonobject_getchild(o, key))) {
		return jsonobject_getvalueint64(child, v);
	}
	return 0;
}
static INLINE int jsonobject_getchildfloat(jsonobject_p o, const char* key, double *v) {
	jsonobject_p child;
	if(o && key && v && (child = jsonobject_getchild(o, key))) {
		return jsonobject_getvaluefloat(child, v);
	}
	return 0;
}


#ifdef __cplusplus
};
#endif

#endif