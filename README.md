# redis-counter
高效读取计数数据的微型数据库，灵感来自redis5.0.3

项目仅供学习，目的是更好地学习redis源码和redis运行原理


想要调试sds数据结构命令

    cd ./src && make sds && ./sds 

运行服务器

    cd ./src && make server && ./server

运行客户端

    cd ./src && make client && ./client


客户端执行命令
```sh
set page_view 1
ok
incr page_view
ok
get page_view
2
```