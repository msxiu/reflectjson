#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "libreflectjson.h"
#include "jsonobject.h"

const char* jsonentity2string(void* entity, reflect_p flds) {
    const char* res = NULL;
    if(entity && flds) {
        jsonobject_p json = jsonobject_formentity(entity, flds);
        if(json) {
            res = jsonobject_tojsonstring(json);
            jsonobject_destory(json);
        }
    }
    return res;
}

int jsonentity4string(void* entity, reflect_p flds, const char* str) {
    int ret = 0;
    if(entity && flds && str) {
        jsonobject_p json = jsonobject_parseof(str, CHARUTF8);
        if(json) {
            ret = jsonobject_toentity(json, flds, entity);
            jsonobject_destory(json);
        }
    }
    return ret;
}

/**序列化，将结构体对象中的数据生成json字符串，保存到文件中*/
int jsonentity_serialization(void* entity, reflect_p flds, const char* faddr)  {
	int ret = 0;
    size_t wl = 0, tl, bln;
	FILE *fp = NULL;
	if(entity && flds && faddr) {
        const char* buffer = jsonentity2string(entity, flds);
        if(buffer && (fp = fopen(faddr, "wb"))) {
            bln = strlen(buffer);
            while(wl < bln) {
                if((tl = fwrite(((char*)buffer) + wl, 1, (bln - wl), fp)) <= 0) {
                    break;//写出现错误了
                }
                wl += tl;
            }
            free((char*)buffer);
            fclose(fp);
            ret = (wl == bln);//数据写完整了才表示正确
        }
	}
    return ret;
}

/**反序列化，从文件中读取json字符串，反射绑定到结构体对象*/
int jsonentity_deserialization(void* entity, reflect_p flds, const char* faddr) {
    FILE* fp;
	int ret=0, rlen = 0, flen=0, tln;
	char* buffer = NULL;
    if(entity && flds && faddr && (fp = fopen(faddr, "rb"))) {
		fseek(fp, 0L, SEEK_END);
		flen = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		if((buffer = (char*)calloc(1, flen + 1))) {
			while(rlen < flen){
				tln = fread(buffer + rlen, 1, (flen - rlen), fp);
				rlen += tln;
			}
            jsonentity4string(entity, flds, buffer);
            free(buffer);
            ret = 1;
		}
		fclose(fp);
    }
    return ret;
}


/**将一个JSON对象还原到实体对象*/
size_t jsonobject_toentity(jsonobject_p o, reflect_p flds, void* e) {
	size_t i=0;
	reflect_p f;
	if(o && (o->type == JSONFIELD_OBJECT) && flds && e) {
		jsonobject_p jnode;
		reflect_p innerflds;
		void* childentry;
		const unsigned char* v = (const unsigned char*)e;
		while((f = (flds + i)) && f->fldname && (REFLECT_NONE != f->type) && f->entire) {
			if((jnode = jsonobject_getchild(o, f->fldname))) {
				switch(f->type){
				case REFLECT_INT:
					if(jnode->value) {
						reflect_setintvalue(f, e, atol(jnode->value));
					}
					break;
				case REFLECT_UINT: 
				case REFLECT_TIME:
					if(jnode->value) {
						reflect_setuintvalue(f, e, atol(jnode->value)); 
					}
					break;
				case REFLECT_FLOAT: 
					if(jnode->value) {
						reflect_setfloatvalue(f, e, atof(jnode->value)); 
					}
					break;
				case REFLECT_STRBUF:
				case REFLECT_STRING: 
					if(jnode->value) {
						reflect_mallocstring(f, e, jnode->value);
					}
					break;
				case REFLECT_OBJBUF:
					if(f->getreflect && (innerflds = f->getreflect())) {
						childentry = (void*)(v + f->offset);
						jsonobject_toentity(jnode, innerflds, childentry);
					}
					break;
				case REFLECT_OBJECT:
					if(f->getreflect && (innerflds = f->getreflect())) {
						if((childentry = (*((void**)(v + f->offset)) = calloc(1, innerflds->entire)))) {
							jsonobject_toentity(jnode, innerflds, childentry);
						}
					}
					break;
				default:break;
				}
			}
			i++;
		}
	}
	return i;
}
void jsonentity_free(void* entity, reflect_p flds) {
	if(entity && flds) {
		int i =0;
        reflect_p f, innerflds;
		const unsigned char* v = (const unsigned char*)entity;
        void* straddr;
        while((f = (flds + i)) && (f->fldname) && (REFLECT_NONE != f->type) && (f->size)) {
            //printf("%p=>%d:%s\n", f, i, f->fldname);
            switch(f->type){
            case REFLECT_STRING:
                if((straddr = *((char**)(v + f->offset)))) { free(straddr); }
                break;
            case REFLECT_OBJBUF:
                if(f->getreflect && (innerflds = f->getreflect())) {
                    void* childentry = (void*)(v + f->offset);
                    jsonentity_free(childentry, innerflds);
                }
                break;
            case REFLECT_OBJECT:
                if(f->getreflect && (innerflds = f->getreflect())) {
                    void* childentry = *((void**)(v + f->offset));
                    jsonentity_free(childentry, innerflds);
                    free(childentry);//释放对象
                }
                break;
            default: break;
            }
            i++;
        }
	}
}
