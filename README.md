# My_Project
本项目是用stm32h750vbt6作为的主控芯片，包含有4G模块、以太网模块、RS485模块、LoRA模块、大彩的屏幕以及SD卡这些外设、其中以太网模块使用了lwip协议栈、RS485是基于modbus修改的私有协议栈其他模块基本都是透传，
操作系统选用的是LiteOS，以及使用了华为云的IoTDA实现在线的数据呈现和命令下发。操作系统和协议栈都未上传需自己移植，关键代码已经给出。

## 目录
- [HardWare](#外设模块驱动)
- [OS_CONFIG](#LiteOS的配置文件)
- [TASK](#项目的所有进程任务)
- [Inc](#cubemax生成的头文件带部分修改)
- [Src](#cubemax生成的源文件带部分修改)

## HardWare
外设模块驱动

## OS_CONFIG
LiteOS的配置文件

## Inc
cubemax生成的头文件带部分修改

## TASK
项目的所有进程任务
