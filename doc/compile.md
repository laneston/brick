# 主机编译
```
cmake ..
```

# 交叉编译
```
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain/toolchain_mtk.cmake
```


# 是否编译动态库

如果需要将 NNG 编译成动态库，则需要使能 BUILD_SHARED_LIBS 参数。

```
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain/toolchain_mtk.cmake -DBUILD_SHARED_LIBS=ON
```
