# 项目简介 

该项目是人工智能电子秤项目中的子项目
主要负责采集摄像头的图像数据，并编码为RGB24格式和JPEG格式
最终将图像数据存储为JPEG文件(.jpg)
项目使用C语言编写而成，使用到了JPEG库

# 项目安装与使用

## JPEG库安装

* 下载并解压缩JPEG库源码包

	`tar -zxvf jpegsrc.v8.tar.gz`

*  编译JPEG库

	进入到JPEG源码库安装目录
	`cd jpeg-8/`

	配置安装路径，编译器使用默认编译器(gcc)
	`./configure --prefix=/库文件生/成目录的/绝对路径`

* 编译并安装JPEG库

	`make && make install`

* 库的使用说明和示例

	库安装成功之后，库源码目录jpeg-8中的libjpeg.txt和example.c文件描述了库的简介和使用方法
本项目中的`jpeg_util.c`文件封装了jpeg库，提供了更加方便的API

##编译项目

进入到项目源码根目录下，执行make即可生成可执行文件camerashot

`make`

##运行项目

进入项目根目录make生成可执行文件camerashot后，执行如下命令
`./camerashot  /摄像头/设备文件/路径  /生成的/图片文件/保存路径`

示例
>打开/dev/video0对应的设备文件，即我的笔记本上的摄像头，采集一张图片，以JPEG格式存储到1.jpg文件中

`./camerashot /dev/video0  ./1.jpg`


# 项目课程简介

该项目来自于物联网人工智能电子秤项目实战课程的Linux C阶段课程的阶段项目
主要面向Linux C编程的小白玩家以及需要提升C编程能力的同学

**课程封面**

[![Linux C核心编程系列课程封面](https://github.com/xiaoyangzai/cameraShot/raw/master/project_images/C%E8%AF%AD%E8%A8%80%E7%BC%96%E7%A8%8B%E8%AF%BE%E7%A8%8B%E5%B0%81%E9%9D%A2.png)](https://edu.csdn.net/course/detail/24969 "一脸懵逼")


课程视频传送门: <https://edu.csdn.net/course/detail/24969>


#物联网人工智能电子秤项目简介

智能电子秤以物联网技术为基础,通过人工智能技术智能自动识别蔬菜水果种类,从而实现蔬菜
水果自动称重与结算,最大化的降低人工参与度,提高交易效率,降低交易成本。


## 项目特点

	**有意思....**
	**这就是人工智能?啧啧啧**
	**经济实惠**
	**学生党专用**
	**你买不了吃亏....**
	**二营长，把我的.....**

## 项目展示

硬件环境

![智能电子秤](https://github.com/xiaoyangzai/cameraShot/raw/master/project_images/%E7%A1%AC%E4%BB%B6%E7%BB%84%E5%90%88.jpg)

称个辣椒试试

![智能电子秤](https://github.com/xiaoyangzai/cameraShot/raw/master/project_images/%E7%A7%B0%E9%87%8D-%E8%BE%A3%E6%A4%92.jpg)


塑料袋? 不存在的.

![智能电子秤](https://github.com/xiaoyangzai/cameraShot/raw/master/project_images/%E7%A7%B0%E9%87%8D-%E8%BE%A3%E6%A4%92-%E5%A1%91%E6%96%99%E8%A2%8B.jpg)


## 项目模块

该项目分为三大模块: 物品结算可视化模块,种类识别服务模块以及蔬菜水果识别模块。

### 物品结算可视化模块

物品结算可视化模块主要负责实现物品图像采集、物品自动称重与结算,交易明细信息存储等功能,使用 C 语言基于 B/S 架构实现物品图像、单价、重量、类别等信息实时可视化,本模块通过多线程的方式实现,图像采集线程实时采集电子秤上的物品图像,称重线程读取电子秤传感器上物品重量数据,种类识别线程通过网络将物品图像发送给种类识别服务模块,并接收物品类别,通过操作 Mysql 数据库获取物品名称和单价信息,主线程将物品图像、重量、单价、类别等信息封装为 JSON 格式,并通过WebSocket 协议推送到网页中实时显示物品图像、重量、单价等信息,交易结算线程接收页面交易确认信息,并将交易明细存储到 Mysql 数据库中。


### 种类识别服务模块

种类识别服务模块负责接收物品结算可视化模块的物品识别请求,使用 Python 基于 C/S 架构通过多进程的方式实现为多个物品称重可视化模块并发提供物品识别服务,参照 HTTP 协议涉及了应用层通讯协议,该模块首先获取物品称重可视化模块发送的物品图像信息,然后对图像数据格式进行转换,以转换后的图像作为输入,运行蔬菜水果识别模块中的神经网络模型识别该图像的类别,最后将类别回发给物品结算可视化模块。

### 蔬菜水果识别模块

蔬菜水果识别模块中通过深度卷积神经网络实现,主要在 VGG16 模型的基础上,根据本项目的需求对 VGG16 模型进行修改,通过 Tensorflow 深度学习框架构建蔬菜水果识别模型,然后预加载VGG16 模型的权值,并选择交叉熵作为损失函数,根据采集的常见蔬菜水果图像作为训练数据集,通过批量随机梯度下降再对模型进行微调训练,实现蔬菜水果的准确识别,本模块在蔬菜水果识别模型中封装了预测接口,方便种类识别服务模块使用。

## 项目实战课程

物联网人工智能电子秤项目实战课程根据功能模块划分为两部分
	**物联网技术和人工智能技术**
细分为五个阶段课程和阶段项目

### 物联网技术 

本部分由以下课程组成
+ Linux C核心编程系列课程与阶段项目
	* Nginx内存池实现
	* 学生信息管理系统
	* 基于V4L2的摄像头图像采集与处理
	* 屏幕截图程序
	* 课程视频教程传输门: <https://edu.csdn.net/course/detail/24969>
+ Linux环境高级编程系列课程与阶段项目
	* 基于WebSocket的屏幕广播系统
	* 课程视频教程传输门: **稍后更新**
+ MySql数据库编程系列课程
	* 蔬菜水果信息管理系统	
	* 课程视频教程传输门: **稍后更新**

### 人工智能技术 

本部分由以下课程组成
+ Python编程系列课程与阶段项目
	* 远程图像采集服务器
	* 课程视频教程传输门: **稍后更新**
+ 人工智能数学基础
	 该部分主要讲解人工智能中数学基础、概率论基础以及数理统计知识
	* 课程视频教程传输门: **稍后更新**
+ 机器学习基础
	* 课程视频教程传输门: **稍后更新**
+ 深度神经网络原理
	* 课程视频教程传输门: **稍后更新**
+ 深度卷积神经网络
	* 课程视频教程传输门: **稍后更新**
+ Tensorflow教程与阶段项目
	* 验证码识别模型设计与实现
	* 课程视频教程传输门: **稍后更新**
