#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "libreflectjson.h"
#include "jsonobject.h"


//设置反射对象的一个字段
static size_t jsonobject_setentityfield(jsonobject_p o, reflect_p f, const void* e) {
	if(o && f && e) {
		reflect_p innerflds;
		const unsigned char* v = (const unsigned char*)e;
		switch(f->type){
		case REFLECT_INT: 
			jsonobject_addchildint(o, f->fldname, reflect_getintvalue(f, v)); 
			return 1;
		case REFLECT_UINT: 
		case REFLECT_TIME:
			jsonobject_addchildint(o, f->fldname, reflect_getuintvalue(f, v)); 
			return 1;
		case REFLECT_FLOAT: 
			jsonobject_addchildfloat(o, f->fldname, reflect_getfloatvalue(f, v)); 
			return 1;
		case REFLECT_STRBUF:
		case REFLECT_STRING:
			jsonobject_addchildstring(o, f->fldname, reflect_getstring(f, v));
			return 1;
		case REFLECT_OBJBUF:
			if(f->getreflect && (innerflds = f->getreflect())) {
				void* childentry = (void*)(v + f->offset);
				jsonobject_p child = jsonobject_formentity(childentry, innerflds);
				jsonobject_setnodename(child, f->fldname);
				jsonobject_child_append(o, child);
			}
			break;
		case REFLECT_OBJECT:
			if(f->getreflect && (innerflds = f->getreflect())) {
				void* childentry = *((void**)(v + f->offset));
				jsonobject_p child = jsonobject_formentity(childentry, innerflds);
				jsonobject_setnodename(child, f->fldname);
				jsonobject_child_append(o, child);
			}
			break;
		default: return 0;
		}
	}
	return 0;
}

/**将实体v生成json对象, flds为选取字段, 可通过以NULL结尾的可变参数选取字段*/
jsonobject_p jsonobject_fromentityflds(void* v, reflect_p flds, ...) {
	jsonobject_p o = NULL;
	va_list ap;
	reflect_p f;
	char* fldname;//字段名
	if(v && flds && (o = jsonobject_alloc(JSONFIELD_OBJECT, NULL))) {
		va_start(ap, flds);//变参处理开始
		while ((fldname = va_arg(ap, char*))) {
			if((f = reflect_find(flds, fldname))) {//通过名称查找到字段
				jsonobject_setentityfield(o, f, v);
			}
		}
		va_end(ap);//变参处理结束
	}
	return o;
}



/**将实体v生成json对象, flds为选取字段*/
jsonobject_p jsonobject_formentity(void* v, reflect_p flds) {
	size_t i=0;
	jsonobject_p o = NULL;
	reflect_p f;
	if(v && flds && (o = jsonobject_alloc(JSONFIELD_OBJECT, NULL))) {//设置对象属性
		while((f = (flds + i)) && f->fldname && (REFLECT_NONE != f->type) && f->entire) {
			jsonobject_setentityfield(o, f, v);
			i++;
		}
	}
	return o;
}
/** 给数组json对象o添加数组成员,返回添加记录条数*/
size_t jsonobject_arraychilds(jsonobject_p o, reflect_p flds, void* records, size_t recordsize, size_t recordcount) {
	size_t i = 0, ret = 0;
	jsonobject_p jnode;//
	const char *r, *ptr = (const char*)records;
	if(o && JSONFIELD_ARRAY == o->type && flds && records && recordsize && recordcount) {
		for(i=0; i<recordcount; i++) {
			r = (ptr + (i*recordsize));//指向到第i条记录
			if((jnode = jsonobject_formentity((void*)r, flds))) {//申请并解析实体
				ret += jsonobject_child_append(o, jnode);//加入到子节点中
			}
		}
	}
	return ret;
}

/**给json对象o添加一个数组对象的子节点,返回添加记录条数*/
size_t jsonobject_addarraychild(jsonobject_p o, const char* key, reflect_p flds, void* records, size_t recordsize, size_t recordcount) {
	size_t ret = 0;
	jsonobject_p c = NULL;
	if(o && (c = jsonobject_alloc(JSONFIELD_ARRAY, key))) {
		//strncpy(c->name, key, sizeof(c->name));//赋值键名
		jsonobject_arraychilds(c, flds, records, recordsize, recordcount);
		if(!(ret = jsonobject_child_append(o, c))){//添加子对象
			jsonobject_destory(c);//添加不成功时,需要回收对象
		}
	}
	return ret;
}
/**添加实体子成员,成功返回1,失败返回0*/
size_t jsonobject_addchildentity(jsonobject_p o, const char* key, reflect_p flds, void* obj) {
	size_t ret = 0;
	jsonobject_p c = NULL;
	if(o && (c = jsonobject_formentity(obj, flds))) {
		strncpy(c->name, key, sizeof(c->name));//赋值键名
		if(!(ret = jsonobject_child_append(o, c))) {//添加子对象
			jsonobject_destory(c);//销毁对象
		}
	}
	return ret;
}

