# Others

## View

view::mask为什么只支持写, 而不支持读, 因为读不好确定该返回什么值

view::mask的类型转换需要进一步完善

## 零和一

在我们的卷积运算，矩阵运算中除了乘法， 加法外我们还需要零和一， 所以我们需要定义one和zero

one和zero以value()的形式返回，若可编译时求得则加constexpr修饰

## CI

### jenkins

[Jenkinsfile](../Jenkinsfile) 是项目的CI脚本，其会定义CI的执行方式

### 目标平台

* linux x86
* windows x86
* linux aarch64
* linux armv7
* linux x86 cuda
* linux aarch64 cuda

## build

cmake版本太高也会导致问题， 比如对于cuda9.0 需要使用cmake10.0

尽量使用离当前系统的默认编译工具， 否则容易出现库依赖的问题

benchmark不work和环境有关， centos和ubuntu均出现该现象， 未定位 

## Heterogeneouse

gnu compiler向量化扩展是不支持的， 因为在cu文件里MATAZURE_GENERAL会被激活， 所以得确保gcc得特性是被device代码所支持， 显然向量化拓展的语法是不支持的， 需要自定义simd类型， 并用__CUDA_ARCH__来切换才行

## compile error

```c++
static_assert(_T1::rank == _T2::rank, "the ranks are not matched");;
static_assert(std::is_same<typename _T1::value_type, typename _T2::value_type>::value, "the value types are not matched");
static_assert(std::is_same<runtime_t<_T1>, runtime_t<_T2>>::value, "the runtime types are not matched");
```

## runtime error

```c++
MATAZURE_ASSERT(equal(ts1.shape(), ts2.shape()), "the shapes are not matched");
```

## 代码何如原则

dev分支通过后推送到master上， master和dev是一致的， 但确保dev正确再推送， 否则回撤。

定期发布release标签，

若有bug则直接再master上发布

如要补长期release， 则需要开辟release分支，便于稳定修复bug
