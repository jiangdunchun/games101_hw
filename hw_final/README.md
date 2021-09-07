# feature
本次作业实现基于快速傅立叶变换(Fast Fourier Transform, FFT)实现海面的渲染。该方法的特点是真实感出色，全局可控，细节丰富，但计算量相对较大。

- [ ] 次表面散射(SSS)

- [ ] 水面高光及粗糙度

- [ ] 白沫

- [ ] 菲涅尔现象


# image


# development
## evironment
```
sudo sh install.sh
```

## build and run
```
// build
mkdir build 
cd build
cmake ..
make

// run
./FFT_water_simulation
```

# reference
[FFT海洋水体渲染学习笔记](https://zhuanlan.zhihu.com/p/335045713)

[Unity 基于GPU FFT海洋的实现-理论篇](https://zhuanlan.zhihu.com/p/95482541)

[快速傅立叶变换(FFT)的海面模拟](https://blog.csdn.net/qq_39300235/article/details/103582460)
