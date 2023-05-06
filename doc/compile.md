# 主机编译
```
cmake ..
```



# 交叉编译
```
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain/toolchain_openwrt.cmake
```


# 是否编译动态库

如果需要将 NNG/MQTT 编译成动态库，则需要使能 BUILD_SHARED_LIBS 参数。

```
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain/toolchain_openwrt.cmake -DNNG_TESTS=OFF -DBUILD_SHARED_LIBS=ON
```


```
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain/toolchain_openwrt.cmake -DNNG_TESTS=OFF -DBUILD_SHARED_LIBS=OFF
```


```
mkdir build && cd build

cmake .. \
-DCMAKE_TOOLCHAIN_FILE=../toolchain/toolchain_openwrt.cmake \
-DBUILD_SHARED_LIBS=OFF \
-DPAHO_WITH_SSL=TRUE \
-DPAHO_BUILD_SHARED=FALSE \
-DPAHO_BUILD_STATIC=TRUE \
-DNNG_TESTS=OFF \
-DPAHO_ENABLE_TESTING=NO \
-DRSYSLOG=OFF
```