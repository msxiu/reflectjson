#ifndef JSON_OBJECT_H_
#define JSON_OBJECT_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "llqvarymem.h"
#include "libreflectjson.h"

#ifdef __GNUC__
#define CSET_GBK    "GBK"
#define CSET_UTF8   "UTF-8"
#define LC_NAME_zh_CN   "zh_CN"
#include <locale.h>
#include <iconv.h>

#elif defined(_MSC_VER)

#define CSET_GBK    "936"
#define CSET_UTF8   "65001"
#define LC_NAME_zh_CN   "Chinese_People's Republic of China"

#endif

#define LC_NAME_zh_CN_GBK       LC_NAME_zh_CN "." CSET_GBK
#define LC_NAME_zh_CN_UTF8      LC_NAME_zh_CN "." CSET_UTF8
#define LC_NAME_zh_CN_DEFAULT   LC_NAME_zh_CN_GBK

static INLINE int wcharconvertascii(wchar_t c, char *pcmb) {
	int len = 0;
	setlocale(LC_ALL, LC_NAME_zh_CN_GBK);
	len = wctomb (pcmb, c);
	pcmb[len] = 0;
	return 1;
}
static INLINE int wcharconvertutf8(wchar_t c, char *pcmb) {
	int len = 0;
	setlocale(LC_ALL, LC_NAME_zh_CN_UTF8);
	len = wctomb (pcmb, c);
	pcmb[len] = 0;
	return 1;
}


extern char* jsonmapconvert(unsigned char c);/**UNSAFECHAR_JSONMAP的字答映射转换函数*/
extern int jsonobject_child_append(jsonobject_p o, jsonobject_p child);
extern jsonobject_p jsonobject_indexAt(jsonobject_p o, int idx);
/**数组对象，通过下标方式获取子数据*/
extern jsonobject_p jsonobject_indexof(jsonobject_p o, int idx);
/**获取子节点json对象*/
extern jsonobject_p jsonobject_getchild(jsonobject_p o, const char* key);
/**设置对属子节点json对象，如果查找到则替换，否则添加，未添加成功返回0*/
extern int jsonobject_setchild(jsonobject_p o, const char* key, jsonobject_p c);



//基本类型空间申请
static INLINE char* jsonobject_allocbasevalue(){ return (char*)calloc(1, 64); }

/**节点类型比较,相同返回1,否则返回0*/
static INLINE int jsonobject_typeof(jsonobject_p o, jsonenum_t t){
	return (o && t == o->type);
}


#endif