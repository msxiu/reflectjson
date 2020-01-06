#include "jsonobject.h"


//申请一个jsonobject 对象
jsonobject_p jsonobject_alloc(jsonenum_t t, const char* nodename) {
	jsonobject_p r=NULL;
	if((r=(jsonobject_p)calloc(1, sizeof(jsonobject_t)))){
		r->type = t;
		if(nodename) {
			strncpy(r->name, nodename, sizeof(r->name));
		}
	}
	return r;
}

extern jsonobject_p jsonobject_indexAt(jsonobject_p o, int idx);
//json对象销毁，回收json子对象以及本身
void jsonobject_destory(jsonobject_p o) {
	int i;
	jsonobject_p node = NULL;
	if(o) {
		if(o->childs.used) {
			for(i=0; i<o->childs.used; i++) {
				node = jsonobject_indexAt(o, i);
				jsonobject_destory(node);
			}
			free(o->childs.ptr);//回收缓存区
		}
		if(o->value){ free(o->value); }
		free(o);
	}
}


/**UNSAFECHAR_JSONMAP的字答映射转换函数*/
char* jsonmapconvert(unsigned char c) {
	switch(c){
	case '\a':return "\\a";//Sound alert
	case '\b':return "\\b";//退格
	case '\f':return "\\f";//Form feed
	case '\n':return "\\n";//换行
	case '\r':return "\\r";//回车
	case '\t':return "\\t";//水平制表符
	case '\v':return "\\v";//垂直制表符
	case '"':return "\\\"";//双引号
	case '\'':return "\\'";//单引号
	case '/':return "\\/";//反斜杠
	case '\\':return "\\\\";//反斜杠
	default:return NULL;
	}
}


/**添加一个字符串的子节点*/
static int jsonobject_addchildnode(jsonobject_p o, jsonenum_t type, const char* key, const char* v) {
	int ret = 0;
	jsonobject_p c = NULL;
	if(o && key && v && (JSONFIELD_ARRAY == o->type || JSONFIELD_OBJECT == o->type)) {
		if((c = jsonobject_alloc(type, key))) {
			if((c->value = (char*)calloc(1, strlen(v) + 1))) {
				//strncpy(c->name, key, sizeof(c->name));//赋值键名
				strcpy(c->value, v);//赋值键值
				if(jsonobject_child_append(o, c)) {
					ret = 1;
				} else {
					jsonobject_destory(c);//回收对象
				}
			} else {
				free(c);
			}
		}
	}
	return ret;
}
/**添加一个字符串的子节点*/
int jsonobject_addchildstring(jsonobject_p o, const char* key, const char* v) {
	return jsonobject_addchildnode(o, JSONFIELD_STRING, key, v);
}
/**添加一个整数类型的子节点*/
int jsonobject_addchildint(jsonobject_p o, const char* key, int64_t v) {
	char buf[64];
	sprintf(buf, "%ld", v);
	return jsonobject_addchildnode(o, JSONFIELD_BASE, key, buf);
}
/**添加一个小数类型的子节点*/
int jsonobject_addchildfloat(jsonobject_p o, const char* key, double v) {
	char buf[64];
	sprintf(buf, "%lf", v);
	return jsonobject_addchildnode(o, JSONFIELD_BASE, key, buf);
}


