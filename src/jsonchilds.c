#include "jsonobject.h"


int jsonobject_child_append(jsonobject_p o, jsonobject_p child) {
	if(o && child) {
        size_t childsize = sizeof(jsonobject_p);
		uint32_t offset = (o->childs.used * childsize);
        if(memrealloc_nextof((void**)(&(o->childs.ptr)), &(o->childs.size), &offset, childsize)) {
            o->childs.ptr[o->childs.used++] = child;
            return 1;
        }
	}
	return 0;
}
jsonobject_p jsonobject_indexAt(jsonobject_p o, int idx) {
	jsonobject_p node = NULL;
	if(o && idx < o->childs.used) {
        node = o->childs.ptr[idx];
	}
    return node;
}

/**数组对象，通过下标方式获取子数据*/
jsonobject_p jsonobject_indexof(jsonobject_p o, int idx) {
	if(o && (JSONFIELD_ARRAY == o->type) && (o->childs.used > idx)) {
		return jsonobject_indexAt(o, idx);
	}
	return NULL;
}
/**获取子节点json对象*/
jsonobject_p jsonobject_getchild(jsonobject_p o, const char* key) {
	int i;
	jsonobject_p node = NULL;
	if(o && key && JSONFIELD_OBJECT == o->type && o->childs.used) {
		for(i=0;i<o->childs.used;i++) {//查找子节点
			node = jsonobject_indexAt(o, i);
			if(!strcasecmp(key, node->name)) return node;
		}
	}
	return NULL;
}
/**设置对属子节点json对象，如果查找到则替换，否则添加，未添加成功返回0*/
int jsonobject_setchild(jsonobject_p o, const char* key, jsonobject_p c) {
	int i;
	jsonobject_p node = NULL;
	if(o && key && JSONFIELD_OBJECT == o->type && o->childs.used) {
		for(i=0;i<o->childs.used;i++) {//查找子节点
			node = jsonobject_indexAt(o, i);
			if(!strcasecmp(key, node->name)) {
				jsonobject_destory(node);//回收原对象
				strncpy(c->name, key, sizeof(c->name));//复制名字
				memcpy(node, c, sizeof(void*));
				return i;
			}
		}
		return jsonobject_child_append(o, c);
	}
	return 0;
}

