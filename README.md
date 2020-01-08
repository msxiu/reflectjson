# C反射JSON(reflectjson)使用说明

> 在C程序里，现在使用比较广泛的是CJSON完成json字符串的解析，使用较复，需要掌握CJSON的语法等知识较多，难于掌握，不便于使用；
***
reflectjson 实现将json字符串赋值给C结构体对象，调用简单，无需了解json底层解析与实现过程，几需简短的几行代码，就能完成json字符串与结构体对象之间的转换，大大提升使用与开发效率。

### 编译与安装
```
cd src
chmod +x configure
./configure
make
make install
```
### 一个简单的调用列子：
```
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


```
****
程序运行结果如下：
```
[root@bogon src]# ./testentity -f ./example/testentity.json 
id:102
name:reflectjson
summary:A simple use example of reflectjson
```
****

是不是很简单，方便？那快来试试吧！
