#include "jsonobject.h"
#include "llqvarymem.h"
#include "llqcharsetdetect.h"


//JSON字符串编码
static size_t jsonstrencoding(sizeptr_p bs, uint32_t *maxsize, const char* str) {
	unsigned char c;
	const char *s;
	char buffer[16];
	uint32_t ret = 0, i = 0, tmpsize = 0;
	unsafechar_t safetype = UNSAFECHAR_NONE;
	if(str && bs) {
		while((c = *(str + i))) {
			safetype = (3 & charectersafedetect(c));
			switch (safetype) {
			case UNSAFECHAR_JSONMAP:
				if((s = jsonmapconvert(c))) {
					ret += varysizeptr_writestr(bs, maxsize, s);
				}
				break;
			case UNSAFECHAR_JSONCODE: 
				tmpsize =sprintf(buffer, "\\x%2x", c);
				ret += varysizeptr_write(bs, maxsize, buffer, tmpsize); 
				break;
			default: 
				ret += varysizeptr_writeint8(bs, maxsize, c); 
				break;
			}
			i++;
		}
	}
	return ret;
}


static size_t jsonobject_getjson(jsonobject_p o, sizeptr_p bs, uint32_t* maxsize);
/**返回jsonobject_p node的json字符串*/
static void jsonobject_nodejson(jsonobject_p node, sizeptr_p bs, uint32_t* maxsize) {
	if(node && bs) {
		switch(node->type) {
		case JSONFIELD_OBJECT:
		case JSONFIELD_ARRAY: 
			jsonobject_getjson(node, bs, maxsize); 
			break;
		case JSONFIELD_STRING: 
			varysizeptr_writeint8(bs, maxsize, '"');
			jsonstrencoding(bs, maxsize, node->value);
			varysizeptr_writeint8(bs, maxsize, '"');
			break;
		case JSONFIELD_BASE: 
		default: 
			varysizeptr_writestr(bs, maxsize, node->value);
			break;
		}
	}
}

/**返回JSON对象的字符串形式**/
static size_t jsonobject_getjson(jsonobject_p o, sizeptr_p bs, uint32_t* maxsize) {
	int i;
	jsonobject_p node = NULL;
	if(o && bs) {
		switch(o->type) {
		case JSONFIELD_OBJECT:
			varysizeptr_writeint8(bs, maxsize, '{');
			if(o->childs.used) {
				for(i=0; i<o->childs.used; i++) {
					if((node = jsonobject_indexAt(o, i))) {
						if(i > 0) { varysizeptr_writeint8(bs, maxsize, ','); }
						varysizeptr_writeint8(bs, maxsize, '"');
						varysizeptr_writestr(bs, maxsize, node->name);
						varysizeptr_writeint8(bs, maxsize, '"');
						varysizeptr_writeint8(bs, maxsize, ':');
						jsonobject_nodejson(node, bs, maxsize);
					}
				}
			}
			varysizeptr_writeint8(bs, maxsize, '}');
			break;
		case JSONFIELD_ARRAY:
			varysizeptr_writeint8(bs, maxsize, '[');
			if(o->childs.used) {
				for(i=0;i<o->childs.used;i++) {
					if(i > 0) { varysizeptr_writeint8(bs, maxsize, ','); }
					jsonobject_nodejson(node, bs, maxsize);
				}
			}
			varysizeptr_writeint8(bs, maxsize, ']');
			break;
		default:
			printf("error type of %d!\n", o->type);
			break;
		}
		return bs->size;
	}
	return 0;
}

/**将jsonobject对象转换成字符串形式*/
char* jsonobject_tojsonstring(jsonobject_p node) {
	sizeptr_t vdat = {0, NULL};
	uint32_t szmax = 0;
	jsonobject_nodejson(node, &vdat, &szmax);
	return (char*)vdat.ptr;
}
