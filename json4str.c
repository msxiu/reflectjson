#include "jsonobject.h"
#include "llqvarymem.h"

#define CHARIS_EMPTY(c)			((c) < 0x21)

typedef struct {
	int maxwrap;	//json解析时允许的最大包裹层数
	int wrap;		//当前包裹层数
	charenum_t conding;	//字符编码
} jsonparseparam_t, *jsonparseparam_p;//解析json字符串时的参数

/** 解析字符串到整型, @return :成功保存返回1,失败返回0 * */
static int chrbufferintegerof(
	const char *buffer,	//字符串缓存
	int sln,			//取缓存区长度,为0则表示字符串的长度
	unsigned char hex,	//表示进制(8:10:16)
	int *v				//
) {
	int ret = 0, px=1, sl;
	if(buffer && sln > 0 && v) {
		sl = (0 == sln ? strlen(buffer) : sln);
		while(sl) {
			char c = buffer[--sl];
			int i = -1;
			if(c >= 'a' && c <= 'f') {
				i = (c - 'a') + 10;
			} else if(c >= 'A' && c <= 'F') {
				i = (c - 'A') + 10;
			} else if(c >= '0' && c <= '9') {
				i = c - '0';
			} else {
				return 0;
			}
			ret += (i * px);
			px *= hex;
		}
		*v = ret;
		return 1;
	}
	return 0;
}


/**去掉左边的空格*/
static void jsonstrltrim(const char* json, size_t *position) {
	if(json && position) {
		while (CHARIS_EMPTY(*(json + (*position)))) { (*position)++; }
	}
}
/**吃掉解析一个对象后的逗号及前后空格*/
static size_t jsonstreatcomma(const char* json, size_t* position){
	size_t i=0;
	if(json && position) {
		i = *position;
		jsonstrltrim(json, &i);//跳过前面空格
		if(',' == *(json + i)){
			i++;//吃掉属性尾部逗号,开始下一个读取
			jsonstrltrim(json, &i);//跳过前面空格
		}
		*position = i;
	}
	return i;
}

/**特殊解码*/
static size_t jsonstrdecoding(char* str, charenum_t ascii) {
	size_t i=0, ret = 0;
	uint32_t pos = 0;
	int ch;
	char c, c2, ws[16];//, vbuf[64]
	sizeptr_t vdat={0, NULL}, *bs = &vdat;
	if(str) {
		while((c = *(str + i))) {
			switch (c) {
				case '\\':
					c2 = *(str + (i+1));
					switch (c2) {
						case 'u'://\uFC4A
							if (chrbufferintegerof(str+i+2, 4, 16, &ch)) {
								switch(ascii) {
									case CHARASCII:
										wcharconvertascii((wchar_t)ch, ws);
										varysizeptr_writestr(bs, &pos, ws);
										break;
									default:
										wcharconvertutf8((wchar_t)ch, ws);
										varysizeptr_writestr(bs, &pos, ws);
										break;
								}
								i += 5;
							} else {
								varysizeptr_writeint8(bs, &pos, c);
							}
							break;
						case 'x'://\xF3
							if (chrbufferintegerof(str+i+2, 2, 16, &ch)) {
								varysizeptr_writeint8(bs, &pos, (unsigned char)ch);
								i += 3;
							} else {
								varysizeptr_writeint8(bs, &pos, c);
							}
							break;
						case '0':
						case 'a': i++; break;
						case 'b': varysizeptr_writeint8(bs, &pos, '\b'); i++; break;
						case 'f': varysizeptr_writeint8(bs, &pos, '\f'); i++; break;
						case 't': varysizeptr_writeint8(bs, &pos, '\t'); i++; break;
						case 'r': varysizeptr_writeint8(bs, &pos, '\r'); i++; break;
						case 'n': varysizeptr_writeint8(bs, &pos, '\n'); i++; break;
						case 'v': varysizeptr_writeint8(bs, &pos, '\v'); i++; break;
						default: varysizeptr_writeint8(bs, &pos, c2); i++; break;//{"'\/.....}
					}
					break;
				default:
					varysizeptr_writeint8(bs, &pos, c);
					break;
			}
			i++;
		}
		memset(str, 0, i);
		strcpy(str, bs->ptr);
		varysizeptr_reclaim(bs);
		ret = bs->size;
	}
	return ret;
}

/**读取json字符串中一个字符串对象*/
static size_t jsonstrreader(const char* json, size_t *position, char* quote, sizeptr_p bs, uint32_t *mxsize) {
	if(json && position && bs) {
		size_t i = *position;//获取位置, c2
		char first = *(json+i);
		char c, ch = (*quote = (('\'' == first || '"' == first) ? first : 0));
		if ('\0' != ch) i++;//跳过引号
		while ((c = (*(json + i)))) {
			switch (c) {
				case '\\':
					varysizeptr_writeint8(bs, mxsize, '\\');
					varysizeptr_writeint8(bs, mxsize, *(json + (i+1)));
					i++;
					break;
				case '"':
				case '\'':
					if (c == ch) {
						ch = '\0';
					} else {
						varysizeptr_writeint8(bs, mxsize, c);
					}
					break;
				case 32://空格处理
				case '\t':
				case '\v':
				case '\r':
				case '\n':
				case '}':
				case ']':
				case ',':
				case ':':
					if ('\0' == ch) {//对象结束标志
						*position = i;
						return 1;
					}
					varysizeptr_writeint8(bs, mxsize, c);
					break;
				default:
					varysizeptr_writeint8(bs, mxsize, c);
					break;
			}
			i++;
		}
		*position = i;//赋值读取值
		return 1;
	}
	return 0;
}




static size_t jsonobject_parseobject(jsonobject_p o, jsonparseparam_p p, const char* json, size_t* position);
/**解析数据*/
static int jsonobject_parsearray(jsonobject_p o, jsonparseparam_p p, const char* json, size_t* position) {
	size_t i = 0, subsize;
	uint32_t szvdat = 0;
	char c, quote;
	jsonobject_p jnode = NULL;
	sizeptr_t vdat = {0, NULL};
	if(o && p && json && position && ((++p->wrap) < p->maxwrap)) {
		i=*position;
		i++;//跳过'['标记
		jsonstrltrim(json, &i);//跳过前面空格
		while ((c = *(json + i)) && ']' != c) {
			switch (c) {
				case ',':
					i++;
					break;
				case '[':
					jnode = jsonobject_alloc(JSONFIELD_ARRAY, NULL);
					subsize = jsonobject_parsearray(jnode, p, json, &i);
					jsonobject_child_append(o, jnode);//加入到子节点中
					if(!subsize) {
						return subsize;
					}
					jsonstreatcomma(json, &i);
					break;
				case '{':
					jnode = jsonobject_alloc(JSONFIELD_OBJECT, NULL);
					subsize = jsonobject_parseobject(jnode, p, json, &i);
					jsonobject_child_append(o, jnode);//加入到子节点中
					if(!subsize) {
						return subsize;
					}
					jsonstreatcomma(json, &i);
					break;
				default:
					jnode = jsonobject_alloc(JSONFIELD_BASE, NULL);
					jsonstrreader(json, &i, &quote, &vdat, &szvdat);//读取基本键数据
					jnode->type = (0 == quote ? JSONFIELD_BASE : JSONFIELD_STRING);
					jnode->value = (char*)vdat.ptr;////赋值数据
					if(JSONFIELD_STRING == jnode->type) {//解码字符串
						jsonstrdecoding(jnode->value, p->conding);
					}
					jsonobject_child_append(o, jnode);//加入到子节点中
					jsonstreatcomma(json, &i);
					break;
			}
		}
		i++;//跳过']'标记
		*position = i;
	}
	return i;
}
/***解析对象*/
static size_t jsonobject_parseobject(jsonobject_p o, jsonparseparam_p p, const char* json, size_t* position) {
	size_t i=0, subsize;
	char c, quote;
	jsonobject_p jnode = NULL;
	sizeptr_t vkey = {0, NULL};
	sizeptr_t vdat = {0, NULL};

	if(o && p && json && position && (++p->wrap) < p->maxwrap) {
		i=*position;
		i++;//跳过'{'标记
		jsonstrltrim(json, &i);//跳过前面空格
		while ((c=*(json + i)) && ('}' !=  c)) {
			uint32_t szkey = 0, szval = 0;
			jsonstrreader(json, &i, &quote, &vkey, &szkey);//读取键名
			switch ((c=*(json + i))) {
				case ':':
					i++;
					jsonstrltrim(json, &i);//跳过前面空格
					switch ((c=*(json + i))) {
						case '{':
							jnode = jsonobject_alloc(JSONFIELD_OBJECT, NULL);
							strncpy(jnode->name, (char*)vkey.ptr, sizeof(jnode->name));//考贝名称
							subsize = jsonobject_parseobject(jnode, p, json, &i);//读取对象
							jsonobject_child_append(o, jnode);//加入到子节点中
							if(!subsize) {
								return subsize;
							}
							break;
						case '[':
							jnode = jsonobject_alloc(JSONFIELD_ARRAY, NULL);
							strncpy(jnode->name, (char*)vkey.ptr, sizeof(jnode->name));//考贝名称
							subsize = jsonobject_parsearray(jnode, p, json, &i);//读取数组
							jsonobject_child_append(o, jnode);//加入到子节点中
							if(!subsize) {
								return subsize;
							}
							break;
						default:
							jnode = jsonobject_alloc(JSONFIELD_BASE, NULL);
							strncpy(jnode->name, (char*)vkey.ptr, sizeof(jnode->name));//考贝名称
							szval = 0;
							memset(&vdat, 0, sizeof(sizeptr_t));
							jsonstrreader(json, &i, &quote, &vdat, &szval);//读取基本键数据
							jnode->type = (0 == quote ? JSONFIELD_BASE : JSONFIELD_STRING);
							jnode->value = (char*)vdat.ptr;//弹出数据区
							if(JSONFIELD_STRING == jnode->type) {//解码字符串
								jsonstrdecoding(jnode->value, p->conding);
							}
							jsonobject_child_append(o, jnode);//加入到子节点中
							break;
					}
					jsonstreatcomma(json, &i);
					break;
				case ','://表示下一个属性开始
				case '}':
				case ']':
					printf("json format error of %lu:'%c'!\n", i, c);
					return -1;//表示读取错误!不正确的JSON字符串
			}
			varysizeptr_reclaim(&vkey);//回收对象
		}
		i++;//跳过'}'标记
		*position = i;
	}
	return i;
}

/**从字符串中解析json对象,返回解析到的json对象实体，失败返回NULL*/
jsonobject_p jsonobject_parseof(const char* json, charenum_t ascii) {
	size_t i = 0;
	jsonobject_p jnode = NULL;
	jsonparseparam_t p = { 20, 0, ascii };
	if(json) {
		jsonstrltrim(json, &i);//跳过前面空格
		if ('{' == *(json + i)) {
			if((jnode = jsonobject_alloc(JSONFIELD_OBJECT, NULL))) {
				jsonobject_parseobject(jnode, &p, json, &i);
			}
		} else if ('[' == *(json + i)) {
			if((jnode = jsonobject_alloc(JSONFIELD_ARRAY, NULL))) {
				jsonobject_parsearray(jnode, &p, json, &i);
			}
		}
	}
	return jnode;
}


