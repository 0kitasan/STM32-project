# adc

## 1adc-1ch


## 1adc-2ch

用dma

discontinuous mode

* enable，则启动一次，转换第一个通道，再启动一次，转换第二个通道
* disable，转换完第一个通道后直接转换下一个，也就是说转换是自动接上的

如果


## 2adc-2ch


如果要对两个信号进行严格的同时采样，必然要用到两个adc

由于运行ADC的START程序会消耗时间，调用两个adc启动程序会导致采样时间差

因此我们将adc配置成dual模式，此模式下，从adc会跟随主adc，主adc开始转换，从adc同时开始转换

注意，程序中一定要先初始化从adc再初始化主adc，否则主adc开的时候，从adc还没配置好
