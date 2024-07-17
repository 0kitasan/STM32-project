# CLion

暴论：CubeIDE 就是一坨狗屎。这玩意 eclipse 这个垃圾 IDE 魔改，速度慢的要死。特别是在 ubuntu 下，还有不少 bug ，比如配置完 CubeMX 后，无法在 ide 中编写代码，必须手动重启，真给他逆天完了。

因此，本人于一气之下，改用 CLion 来作为嵌入式开发的 IDE。

本文介绍如何在 ubuntu 中使用 CLion 搭建 stm32 开发环境。方案为：CLion + CubeMX + arm-none-eabi + OpenOCD

## 环境搭建

### 安装 CLion 和 CubeMX

CLion(https://www.jetbrains.com/clion/download)

CubeMX(https://www.st.com/en/development-tools/stm32cubemx.html)

### 安装 arm 编译器：arm-none-eabi

`arm-none-eabi` 是一个常见的工具链前缀，专门用于 ARM 架构的嵌入式系统开发。它包括编译器、汇编器、链接器和调试器等工具，主要针对不运行操作系统（"none"）且符合嵌入式应用二进制接口（EABI）的 ARM 设备。

ubuntu 可以直接使用 apt 安装：

```bash
sudo apt install gcc-arm-none-eabi
```

### 安装 OpenOCD

OpenOCD（Open On-Chip Debugger）是一款开源的调试与烧写工具，主要用于嵌入式系统的开发和调试。它支持多种架构和调试接口，如 JTAG 和 SWD，常用于调试 ARM Cortex-M 系列微控制器。

同样可以直接使用 apt 安装：

```bash
sudo apt install openocd
```

## 配置 CLion

中文语言包（可选）
编译器和调试器配置：arm-none-eabi

## 配置 CubeMX

## 配置 OpenOCD（.cfg 文件）
