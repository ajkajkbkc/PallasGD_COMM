### 项目默认使用Bootloader，下面讲一下如何去掉Bootloader

1. 关掉编译开关： `XXXX_IAP`

2. 修改IROM1的Start为0x8000000，如下图红框:
![haha](./README.png)

*把 `0x8008000`改为 `0x8000000`*

3. 全部编译链接
