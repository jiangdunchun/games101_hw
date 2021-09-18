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
[真实感水体渲染技术总结](https://zhuanlan.zhihu.com/p/95917609)

[FFT海洋水体渲染学习笔记（二）](https://zhuanlan.zhihu.com/p/335946333)

[FFT海洋水体渲染学习笔记](https://zhuanlan.zhihu.com/p/335045713)

[Unity 基于GPU FFT海洋的实现-理论篇](https://zhuanlan.zhihu.com/p/95482541)

[Unity 基于GPU FFT海洋的实现-实践篇](https://zhuanlan.zhihu.com/p/96811613)

[快速傅立叶变换(FFT)的海面模拟](https://blog.csdn.net/qq_39300235/article/details/103582460)

[海面模拟以及渲染(计算着色器、FFT、Reflection Matrix)](https://blog.csdn.net/xiewenzhao123/article/details/79111004)

[快速傅里叶变换(蝶形变换)-FFT](https://zhuanlan.zhihu.com/p/374489378)

[Realtime GPGPU FFT Ocean Water Simulation](https://tore.tuhh.de/bitstream/11420/1439/1/GPGPU_FFT_Ocean_Simulation.pdf)

[Shader相册第6期 --- 实时水面模拟与渲染](https://zhuanlan.zhihu.com/p/31670275)
