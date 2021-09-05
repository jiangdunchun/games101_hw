# feature
本次作业实现基于快速傅立叶变换(Fast Fourier Transform, FFT)的水体渲染方法的特点是真实感出色，全局可控，细节丰富，但计算量相对较大。

- [ ] 次表面散射(SSS)

- [ ] 水面高光及粗糙度

- [ ] 白沫

- [ ] 菲涅尔现象


# image


# development
## evironment
使用了作业8的开发环境。

## build and debug
```
// build
mkdir build 
cd build
cmake ..
make

// bebug(use gl_version 3.3)
MESA_GL_VERSION_OVERRIDE=3.3 ./fft_water
```

# reference
[FFT海洋水体渲染学习笔记](https://zhuanlan.zhihu.com/p/335045713) -- Yuumu