# STM32-project

STM32 project with CubeIDE

备忘录。

## 一些注解

### spi控制ws2812

很奇怪的事情：在main函数的while循环使用spi传输一个字节，信号中间会间隔很长一段时间，所以如果要连续不断的输出信号，只能使用数组
