# DAC

DAC（Digital-to-Analog Converter，数模转换器）是一种将数字信号转换为模拟信号的设备。

给定一个 Vref，一个 N 位的 DAC 可以输出最小分辨率为 Vref/2^N 的电压

## 误差

### 非线性误差

| 类型 |         Full Name         | 名称         |        定义与计算        |
| :--: | :-----------------------: | ------------ | :-----------------------: |
| INL |   Integral Nonlinearity   | 积分非线性度 |    实际输出 - 理想输出    |
| DNL | Differential Nonlinearity | 微分非线性度 | 相邻的实际输出之差 - 1LSB |

LSB（Least Significant Bit，最低有效位）：表示数字值中的最低位或分辨率单位。

我们希望输出结果是单调的，来方便做控制。如果始终有 DNL>-1，则输出是单调的。

### 线性误差

|     类型     |         Full Name         |   名称   |       产生原因       | 表现     |
| :-----------: | :-----------------------: | :------: | :------------------: | -------- |
|  Gain Errors  |   Integral Nonlinearity   | 增益误差 | 主要因为 Vref 不准确 | 影响斜率 |
| Offset Errors | Differential Nonlinearity | 偏置误差 |    运放的适调电压    | 影响截距 |
