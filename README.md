# 1.配置USB串口
将70-ttyusb.rules放到/etc/udev/rules.d/目录下，重新插入串口

# 2.编译gst-plugins-base:
```sehll
./configure
make
sudo make install
```

# 3.这里有两个API版本，先编译ndicapi:
```sehll
mkdir build
cmake ..
make 
sudo make install
```
然后可以通过find_package 找到 ndicapi

# 4.编译NDI-master
直接make
运行:
```sehll
./build/linux/capisample /dev/ttyUSB0 
```
