#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "llqparameters.h"
#include "libreflectjson.h"

//作者结构体
typedef struct author {
    int id;//作者ID
    char name[64];//作者名称
    char summary[256];//作者简介
}author_t, * author_p;

//展示作者信息
static void show_author(author_p author) {
    fprintf(stdout, "id:%d\nname:%s\nsummary:%s\n", author->id, author->name, author->summary);
}

int main(int argc, const char* argv[]) {
    static reflect_t reflds[] = {//定义结构体的字段反射对象数组
        REFLECT_INIT(REFLECT_INT, author_t, id),
        REFLECT_INIT(REFLECT_STRBUF, author_t, name),
        REFLECT_INIT(REFLECT_STRBUF, author_t, summary),
        REFLECT_EMPTY
    };
    const char* value = NULL;
    author_t item = {0, "", ""};
    if(parameters_find(argc, argv, "-f", &value) && value) {//从指定文件中加载json字符串
        memset(&item, 0, sizeof(author_t));
        if(jsonentity_deserialization(&item, reflds, value)) {
            show_author(&item);
        }
    }
    if(parameters_find(argc, argv, "-s", &value) && value) {//参数为json字符串
        memset(&item, 0, sizeof(author_t));
        if(jsonentity4string(&item, reflds, value)) {
            show_author(&item);
        }
    }
}
