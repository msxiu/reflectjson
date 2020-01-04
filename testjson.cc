#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "libreflectjson.h"

typedef struct author {
    int id;
    char name[64];
    char summary[256];
}author_t, * author_p;

typedef struct objentity {
    int id;
    int type;
    int group;
    author_t author;
    char title[255];
    const char* content;
    time_t time;
} objentity_t, * objentity_p;

static reflect_p author_reflectof() {
    static reflect_t reflds[] = {
        REFLECT_INIT(REFLECT_INT, author_t, id),
        REFLECT_INIT(REFLECT_STRBUF, author_t, name),
        REFLECT_INIT(REFLECT_STRBUF, author_t, summary),
        REFLECT_EMPTY
    };
    return reflds;
}

reflect_p objentity_reflectof() {
    static reflect_t reflds[] = {
        REFLECT_INIT(REFLECT_INT, objentity_t, id),
        REFLECT_INIT(REFLECT_INT, objentity_t, type),
        REFLECT_INIT(REFLECT_INT, objentity_t, group),
        REFLECT_CALLBACK(REFLECT_OBJBUF, objentity_t, author, author_reflectof),
        REFLECT_INIT(REFLECT_STRBUF, objentity_t, title),
        REFLECT_INIT(REFLECT_STRING, objentity_t, content),
        REFLECT_INIT(REFLECT_UINT, objentity_t, time),
        REFLECT_EMPTY
    };
    return reflds;
}

void objentity_set_values(objentity_p o) {
    o->id=1;
    o->type = 4;
    o->group = 19;
    o->time = time(NULL);
    strncpy(o->title, "test information!", sizeof(o->title));
    o->author.id = 12;
    strncpy(o->author.name, "json", sizeof(o->author.name));
    strncpy(o->author.summary, "linux c json example", sizeof(o->author.summary));
    o->content = "news value content!";
}
void objentity_show(objentity_p o, FILE* out){
    fprintf(out, "----------- id:%d ------------------------------------------------\n", o->id);
    fprintf(out, "type:\t%d\n", o->type);
    fprintf(out, "group:\t%d\n", o->group);
    fprintf(out, "time:\t%d\n", (int)o->time);
    fprintf(out, "title:\t%s\n", o->title);
    fprintf(out, "content:\t%s\n", o->content);
    fprintf(out, "-----------author.id:%d ------------------------------------------------\n", o->author.id);
    fprintf(out, "author.name:\t%s\n", o->author.name);
    fprintf(out, "author.summary:\t%s\n", o->author.summary);
}

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
void test_serialization() {
    reflect_p flds = objentity_reflectof();
    objentity_t record;
    memset(&record, 0, sizeof(objentity_t));
    jsonentity_deserialization(&record, flds, "./config.json");
    const char* jsonstr = jsonentity2string(&record, flds);
    fprintf(stdout, "%s\n", jsonstr);
    free((void*)jsonstr);
    objentity_show(&record, stdout);
    jsonentity_free(&record, flds);
}


int main(int argc, const char* argv[]) {
    int testtype = 0;
    if(argc > 1) {
        testtype = atoi(argv[1]);
    }
    switch (testtype) {
    case 1:test_serialization(); break;
    default: test_objentity(); break;
    }
    return 0;
}

