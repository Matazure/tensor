# [Tensor](https://github.com/Matazure/tensor) [![Build Status](https://travis-ci.org/Matazure/tensor.svg?branch=master)](https://travis-ci.org/Matazure/tensor)  [![AppVeyor](https://img.shields.io/appveyor/ci/zhangzhimin/tensor.svg)](https://ci.appveyor.com/project/zhangzhimin/tensor) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/Matazure/tensor/blob/master/LICENSE)
Tensor是一个C++实现的异构计算库，基于for_index算法引擎和多维数组(tensor)来构建一个通用的异构计算库。可广泛适用于图像处理，深度学习，数值计算等领域。 目前支持C++和CUDA，即将对OpenCL和Metal进行验证。Tensor以最为优雅的设计来追求最为极致的性能。

## 为什么使用Tensor
* 避免在算法实现的过程中花太多精力在坐标计算，元素遍历等繁琐处理上
* Tensor拥有丰富的基础操作，并且会随着大家的参与越来越丰富
* Tensor可以使延迟计算，向量化，并行等技术的完美配合，可以让代码保持优雅结构的同时，拥有极佳的性能
* 异构编程的强有力支持，让你可以轻松使用GPU来运行算法，性能媲美原生异构语言。

## 特点
* 统一的跨平台异构编程接口
* 泛型设计，元编程支持
* 内存自动回收
* 向量化操作
* 延迟计算
* Header only


## 示例
下面的程序用于rgb图像归一化和通道分离，可兼容CPU和GPU。
``` cpp
#include <matazure/tensor>
#include <image_utility.hpp>

using namespace matazure;

typedef pointb<3> rgb;

int main(int argc, char *argv[]) {
	//加载图像
	if (argc < 2){
		printf("please input a 3 channel(rbg) image path");
		return -1;
	}

	auto ts_rgb = read_rgb_image(argv[1]);

	//选择是否使用CUDA
#ifdef USE_CUDA
	auto gts_rgb = mem_clone(ts_rgb, device_tag{});
#else
	auto &gts_rgb = ts_rgb;
#endif
	//图像像素归一化
	auto glts_rgb_shift_zero = gts_rgb - rgb::all(128);
	auto glts_rgb_stride = stride(glts_rgb_shift_zero, 2);
	auto glts_rgb_normalized = cast<pointf<3>>(glts_rgb_stride) / pointf<3>::all(128.0f);
	//前面并未进行实质的计算，这一步将上面的运算合并处理并把结果写入到memory中, 避免了额外的内存开销
	auto gts_rgb_normalized = glts_rgb_normalized.persist();
#ifdef USE_CUDA
	cuda::device_synchronize();
	auto ts_rgb_normalized = mem_clone(gts_rgb_normalized, host_tag{});
#else
	auto &ts_rgb_normalized = gts_rgb_normalized;
#endif
	//定义三个通道的图像数据
	tensor<float, 2> ts_red(ts_rgb_normalized.shape());
	tensor<float, 2> ts_green(ts_rgb_normalized.shape());
	tensor<float, 2> ts_blue(ts_rgb_normalized.shape());
	//zip操作，就返回tuple数据，tuple的元素为上面三个通道对应元素的引用
	auto ts_zip_rgb = zip(ts_red, ts_green, ts_blue);
	//让tuple元素可以和point<byte, 3>可以相互转换
	auto ts_zip_point = point_view(ts_zip_rgb);
	//拷贝结果到ts_red, ts_green, ts_blue中，因为ts_zip_point的元素是指向这三个通道的引用
	copy(ts_rgb_normalized, ts_zip_point);

	//保存raw数据
	auto output_red_path = argc < 3 ? "red.raw_data" : argv[2];
	auto output_green_path = argc < 4 ? "green.raw_data" : argv[3];
	auto output_blue_path = argc < 5 ? "blue.raw_data" : argv[4];
	io::write_raw_data(output_red_path, ts_red);
	io::write_raw_data(output_green_path, ts_green);
	io::write_raw_data(output_blue_path, ts_blue);

	return 0;
}
```
丰富的tensor操作，向量化的接口使代码看起来清晰整洁，延迟计算的使用，避免了额外的内存读写，让程序拥有极佳的性能。
## 开发环境
### 仅CPU支持
Tensor的代码规范遵循C++14标准， 所以只需编译器支持C++14即可。
### CUDA支持
在符合CPU支持的基础上，需要安装[CUDA 9.0](https://developer.nvidia.com/cuda-downloads)，详情可查看[CUDA官方文档](http://docs.nvidia.com/cuda/index.html#axzz4kQuxAvUe)

## 编译项目
需先安装[git](https://git-scm.com/)和[CMake](https://cmake.org/)及相应的编译工具，然后运行script下对应的编译脚本即可。  
* build_windows.bat编译windows版本
* build_native.sh编译unix版本(linux及mac)
* build_android.bat可以在windows主机编译android版本，需要cmake-3.6.3或者android studio自带的cmake
* build_android.sh可以在linux主机编译android版本。

查看example下的简单示例会是一个很好的开始。  

## 使用
```
git clone https://github.com/Matazure/tensor.git
```
使用上面指令获取tensor项目后，将根目录（默认是tensor）加入到目标项目的头文件路径即可，无需编译和其他第三方库依赖，nvcc的编译参数需要加入"--expt-extended-lambda -std=c++11"。 CUDA的官方文档有nvcc编译参数设置，CUDA项目创建等的详细说明，也可参考项目的CMakeLists.txt。

## 性能
Tensor编写了大量性能测试用例来确保其优异的性能，可以在目标平台上运行生成的benchmark程来评估性能情况。 直接运行tensor_benchmark, hete_host_tensor_benchmark或者hete_cu_tensor_benchmark.

## 平台支持情况
| 设备  | Windows | Linux | OSX | Android | IOS |
| --- | --- | --- | --- | --- | --- |
| C++ | 支持 | 支持 | 支持 | 支持 | 支持
| CUDA | 支持 | 支持 | 支持 |  |  |
| OpenMP | 支持 | 支持 | 支持 | 支持 | 支持 |
|向量化|支持|支持|支持|支持|支持 |

## 许可证书
该项目使用MIT证书授权，具体可查看LICENSE文件
