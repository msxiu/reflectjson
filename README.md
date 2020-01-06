# C反射JSON(reflectjson)原理及使用说明

> 在C程序里，现在使用比较广泛的是CJSON完成json字符串的解析，使用较复，需要掌握CJSON的语法等知识较多，难于掌握，不便于使用；
***
reflectjson 实现将json字符串赋值给C结构体对象，调用简单，无需了解json底层解析与实现过程，大大提升使用与开发效率。

###一个简单的调用列子：
```
//定义一个结构体
typedef struct objentity {
    int id;
    int type;
    int group;
    char title[255];
    const char* content;
    time_t time;
} objentity_t, * objentity_p;


//定义反射函数，用于反射结构体
reflect_p objentity_reflectof() {
    static reflect_t reflds[] = {
        REFLECT_INIT(REFLECT_INT, objentity_t, id),
        REFLECT_INIT(REFLECT_INT, objentity_t, type),
        REFLECT_INIT(REFLECT_INT, objentity_t, group),
        REFLECT_INIT(REFLECT_STRBUF, objentity_t, title),
        REFLECT_INIT(REFLECT_STRING, objentity_t, content),
        REFLECT_INIT(REFLECT_UINT, objentity_t, time),
        REFLECT_EMPTY
    };
    return reflds;
}

//给对象设置初始值
void objentity_set_values(objentity_p o) {
    o->id=1;
    o->type = 4;
    o->group = 19;
    o->time = time(NULL);
    strncpy(o->title, "test information!", sizeof(o->title));
    o->content = "news value content!";
}
//显示对象成员信息
void objentity_show(objentity_p o, FILE* out){
    fprintf(out, "----------- id:%d ------------------------------------------------\n", o->id);
    fprintf(out, "type:\t%d\n", o->type);
    fprintf(out, "group:\t%d\n", o->group);
    fprintf(out, "time:\t%d\n", (int)o->time);
    fprintf(out, "title:\t%s\n", o->title);
    fprintf(out, "content:\t%s\n", o->content);
}

//测试json解析与json字符串生成函数
void test_objentity() {
    reflect_p flds = objentity_reflectof();
    objentity_t record;
    objentity_t item;
    char* strjson = NULL;
    jsonobject_p json = NULL;
    memset(&record, 0, sizeof(objentity_t));
    objentity_set_values(&record);

    if((json = jsonobject_formentity(&record, flds))){
        if((strjson = jsonobject_tojsonstring(json))) {
            printf("%s\n", strjson); 
        }
        jsonobject_destory(json);

    }
    if(strjson) {
        json = jsonobject_parseof(strjson, CHARASCII);
        free(strjson);
        memset(&item, 0, sizeof(objentity_t));
        jsonobject_toentity(json, flds, &item);
        jsonobject_destory(json);
        objentity_show(&item, stdout);
        jsonentity_free(&item, flds);
    }
}

```
****
程序运行结果如下：

****

是不是很神奇？那一起来使用吧！
