<script src="https://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML" type="text/javascript"></script>

<script type="text/x-mathjax-config">
  MathJax.Hub.Config({
    tex2jax: {
      inlineMath: [ ['$','$'], ["\\(","\\)"] ],
      processEscapes: true
    }
  });
</script>

# 基于FFT的海面渲染

## 简介
本项目基于快速傅立叶变换(Fast Fourier Transform, FFT)实现海面的渲染。该方法的特点是真实感出色，全局可控，细节丰富，但计算量相对较大。 主要包含以下内容:

- [x] 高度场

- [x] 白沫

- [x] 菲涅尔现象

## 理论基础
理论实在太复杂，没必要了解全部原理，知道公式即可。若需要，强烈建议参考[[1]](#refer-anchor-1)。
### IDFT
我们先给出IDFT(逆离散傅里叶变换 Inverse Discrete Fourier Transform)公式，该公式表示如何从离散的频域计算出时域:

$$
f(x) = \frac{1}{N} \sum_{m=0}^\infty F(μ) {\rm e}^{i {\frac{2πμx}{N}}}
$$

其中:

> $f(x)$ 时域函数

> $F(μ)$ 频域函数

> ${\rm e}^{i{\frac{2πμx}{N}}}$ 计算结果为一个复数，在实际计算中需要采用欧拉公式展开: ${\rm e}^{ix} = \cos x + i \sin x$

### IFFT 蝶形算法
IDFT 尽管解决了频域离散化的问题，但运算量很大($O(N^2)$)，IFFT(快速逆傅里叶变换 Inverse Fast Fourier transform)则将该问题的时间复杂度优化为 $O(N \log_2N)$。下面是IFFT的蝶形算法示意图[<sup>[2]</sup>](#refer-anchor-2):

![](images/fft.jpg)

## FFT海面模型
$h(\vec{x}, t)$ 是时域函数: 在时间 $t$ 时， $\vec{x}$ 处的海面高度。这里我们先给出公式，在对其做解释:

$$
h(\vec{x}, t) = \sum_{k} \widetilde{h} (\vec{k}, t) {\rm e}^{i \vec{k} \vec{x}}
$$

其中:

> $\vec{x}$ $=(x, z)$ 是改定在在 x-z 平面的坐标

> $t$ 是时间

> $\vec{k}$ $=(k_x, k_y)=(\frac{2πn}{L_x}, \frac{2πm}{L_z})$，其中 $-\frac{N}{2} \leq n \leq \frac{N}{2}$ 和 $-\frac{M}{2} \leq m \leq \frac{M}{2}$，$N$ 和 $M$ 分别是x和z轴上采样离散点的数量， $L_x$ 和 $L_y$ 分别是x和z轴上海平面的大小。

> $\widetilde{h} (\vec{k}, t)$ 频域函数

可以对该公式理解为 $h(\vec{x}, t)$ 就是对 $\widetilde{h} (\vec{k}, t)$ 进行 IDFT。 $\widetilde{h} (\vec{k}, t)$ 的公式如下：

$$
\widetilde{h} (\vec{k}, t) = \widetilde{h}_0(\vec{k}){\rm e}^{i ω(k) t} + \widetilde{h}^*_0(-\vec{k}){\rm e}^{-i ω(k) t}
$$

其中：

> $k$ 是 $\vec{k}$ 的模长

> $ω(k)$ $=\sqrt{G k}$，其中 $G$ 为重力加速度($9.81$)

> $\widetilde{h}_0$ 与 $\widetilde{h}^*_0$ 共轭

$\widetilde{h}_0(\vec{k})$ 的公式如下：

$$
\widetilde{h}_0(\vec{k}) = \frac{1}{\sqrt{2}}(ε_r + i ε_i) \sqrt{P_h(\vec{k})}
$$

其中：

> $ε_r$ 和 $ε_i$ 是两个相互独立服从均值为0，标准差为1的高斯随机数

> $P_h(\vec{k})$ 是方向波谱

$P_h(\vec{k})$ 公式如下：

$$
P_h(\vec{k}) = A \frac{{\rm e}^{-1 / (k L)^2}}{k^4} |\vec{k} \cdot \vec{ω} |^2
$$

其中：

> $A$ 是波高系数

> $L$ $=V^2 / G$, $V$ 是风速

> $\vec{ω}$ 是风向

目前，我们只得到了海面高度的模型，即在y轴上的时域函数，在x和z轴上的时域函数分别为:

$$
D_x(\vec{x}, t) = \sum_{k} -i \frac{\vec{k_x}}{k} \widetilde{h} (\vec{k}, t) {\rm e}^{i \vec{k} \vec{x}}
$$

$$
D_z(\vec{x}, t) = \sum_{k} -i \frac{\vec{k_z}}{k} \widetilde{h} (\vec{k}, t) {\rm e}^{i \vec{k} \vec{x}}
$$

## 实践
目前，~~假设~~我们已经掌握所有公式了，那我们将按照以下流程来实现我们的算法逻辑(感谢[RenderDoc](https://renderdoc.org/)，所有中间过程纹理均来自于RenderDoc的渲染流程捕获)：

![](images/progress.PNG)

### $\widetilde{h}_0(\vec{k})$ 和 $\widetilde{h}^*_0(-\vec{k})$ 的计算
即使 $\widetilde{h}^*_0(-\vec{k})$ 可以通过 $\widetilde{h}_0(\vec{k})$ 获得，但我仍然生成了两张纹理(为了好看)。$\widetilde{h}_0(\vec{k})$ 的数据仅仅与 波高系数、风速、风向、采样离散点的数量等数据相关，只需计算一次即可，除非动态修改这些值。因此这两张纹理的数据，我选择在CPU上计算(因为懒)。

### $\widetilde{h} (\vec{k}, t)$、$-i \frac{\vec{k_x}}{k} \widetilde{h} (\vec{k}, t)$ 和 $-i \frac{\vec{k_z}}{k} \widetilde{h} (\vec{k}, t)$ 的计算
这一步直接套公式即可。我在一个 compute shader 中同时计算了计算这三个纹理。

### $h(\vec{x}, t)$、$D_x(\vec{x}, t)$ 和 $D_z(\vec{x}, t)$ 的计算
这三个纹理分别是对$\widetilde{h} (\vec{k}, t)$、$-i \frac{\vec{k_x}}{k} \widetilde{h} (\vec{k}, t)$ 和 $-i \frac{\vec{k_z}}{k} \widetilde{h} (\vec{k}, t)$的IFFT。

关于 IFFT 算法的实现，我使用了以下流程[<sup>[1]</sup>](#refer-anchor-1)：

![](images/ifft_horizoncal.PNG)

![](images/ifft_vertical.PNG)

值得注意的是，该流程中使用到了 butterfly indices 一维纹理，该纹理的生成算法可参考[[2]](#refer-anchor-2)，我选择在cpu中计算了该纹理。基于该流程，我在一次compute shader 计算中，对一个方向进行一次蝶形算法迭代。因此，为了计算这三个纹理，该 IFFT 的 compute shader 总共运行了 $3 * 2 * \log_2N$次 ($3$张纹理的IFFT，$2$个方向，$\log_2N$次迭代)。

因为 OpenGL 上下文的频繁切换，这样的 IFFT 运算效率很低，因此，代码中[<sup>[3]</sup>](#refer-anchor-3)提供了另外一种基于 Shader Storage Buffer Object 技术的 IFFT compute shader。该 compute shader 在一次计算中，对一个方向进行 $\log_2N$ 次蝶形算法迭代。但是，该 compute shader 需要固定内存大小的Shader Storage Buffer Object，因此，在 $N$ 改变时无法自适应(我实在想不出其他办法)。

### $displacement$、$normal$ 和 $bubble$ 的计算
$displacement$ 的计算套公式即可。

$normal$ 的计算我使用了便于理解的差分法，虽然精度不高。

$bubble$ 的计算我们使用了雅可比行列式，可参考[[4]](#refer-anchor-4)的 5.2.1。

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

<div id="refer-anchor-1"></div>

- [1] [Realtime GPGPU FFT Ocean Water Simulation](https://tore.tuhh.de/bitstream/11420/1439/1/GPGPU_FFT_Ocean_Simulation.pdf)

<div id="refer-anchor-2"></div>

- [2] [零知识证明 - 理解FFT的蝶形运算](https://zhuanlan.zhihu.com/p/277849135)

<div id="refer-anchor-3"></div>

- [3] [海面模拟以及渲染（计算着色器、FFT、Reflection Matrix)](https://blog.csdn.net/xiewenzhao123/article/details/79111004)

<div id="refer-anchor-4"></div>

- [4] [真实感水体渲染技术总结](https://zhuanlan.zhihu.com/p/95917609)

- [5] [FFT海洋水体渲染学习笔记](https://zhuanlan.zhihu.com/p/335045713)
