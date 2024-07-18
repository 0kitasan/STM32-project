# DAC

DAC（Digital-to-Analog Converter，数模转换器）是一种将数字信号转换为模拟信号的设备。

给定一个 Vref，一个 N 位的 DAC 可以输出最小分辨率为 Vref/2^N 的电压

## DAC 架构

- Sting
- R-2R

## DAC 误差

### 非线性误差

| 类型 |         Full Name         | 名称         |        定义与计算         |
| :--: | :-----------------------: | ------------ | :-----------------------: |
| INL  |   Integral Nonlinearity   | 积分非线性度 |    实际输出 - 理想输出    |
| DNL  | Differential Nonlinearity | 微分非线性度 | 相邻的实际输出之差 - 1LSB |

LSB（Least Significant Bit，最低有效位）：表示数字值中的最低位或分辨率单位。

我们希望输出结果是单调的，来方便做控制。如果始终有 DNL>-1，则输出是单调的。

### 线性误差

|     类型      |         Full Name         |   名称   |       产生原因       | 表现     |
| :-----------: | :-----------------------: | :------: | :------------------: | -------- |
|  Gain Errors  |   Integral Nonlinearity   | 增益误差 | 主要因为 Vref 不准确 | 影响斜率 |
| Offset Errors | Differential Nonlinearity | 偏置误差 |    运放的适调电压    | 影响截距 |

## DAC 误差实验

用 ADS1115（高精度）测量 stm32 的 12 位 dac 输出的电压，用上位机控制，遍历 4096 个点绘制成图

分别查看 INL 与 DNL

## DAC 抖动与 Sigma-Delta 调制实验

注意，这里都在上位机中控制

通过抖动（Dithering）可以实现 DAC 的超分

现在，我们希望将一个 12 位 DAC 超分成 16 位 DAC

如果我希望他输出电压（Vref/2^N 省略）：`2000+5/16` ，我们就需要准备一个长度为 16 的数组：其中 5 个数为 2001，剩下的均为 2000

个人理解：如果前面 11 个为 2000，后面 5 个为 2001，那么这个抖动就不够“随机”。

为了生成更“随机”与“均匀”的抖动，受 Bresenham 直线算法启发，我们使用基于这个算法的 Sigma-Delta 调制

### Bresenham 直线算法

布雷森汉姆直线算法（Bresenham's line algorithm）是用来描绘由两点所决定的直线的算法，它会算出一条线段在 n 维位图上最接近的点。
这个算法只会用到较为快速的整数加法、减法和位元移位，常用于绘制电脑画面中的直线。是计算机图形学中最先发展出来的算法。

### Sigma-Delta 调制

Delta-Sigma（ΔΣ）调制（或称 Sigma-Delta（ΣΔ）调制、SDM，中文译作积分-微分调制）是一种数字模拟互相转换的实做方法，它是把高比特清晰度低频率信号用脉冲密度调制编码为低比特清晰度高频率信号的一种方法（PCM 转 PWM），
可以将量化失真移往更高频率、减少滤除时对目标频率的影响，推导自 delta 调制原理的模拟至数字或是数字至模拟转换技术。

## DAC 波形生成

这部分属于拓展内容

- 三角波
- 锯齿波
- 噪声

## DAC vs DDS

DAC:低频下更准确

DDS:高频下更准确

## CORDIC

CORDIC (Coordinate Rotation Digital Computer，坐标旋转数字计算器)，是一个可以计算三角函数，简单且高效的算法。

在 stm32g4 上有配备 CORDIC 模块，如果需要用到三角函数计算，最好使用他，c 语言库函数 sinf 以及 arm 计算库的开销和速度都不如 CORDIC。
