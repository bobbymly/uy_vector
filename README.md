# uy_vector
学习 STL 实现的 vector
提供以下基础功能
> 迭代器
> * begin()
> * end()

> 大小
> * size()
> * capacity()
> * empty()

> 修改
> * push_back()
> * pop_back()
> * erase()
> * clear()
> * insert()

> 元素访问
> * vec[1]
> * front()
> * back()

## 迭代器
对于顺序容器vector ，迭代器选择使用简单包装的指针。

## 空间配置器
空间配置器使用 uy_allocator 封装的 simple_alloc

[uy_allocator](https://github.com/bobbymly/uy_allocator)

## 空间配置策略
vector 的实现，关键在于对其大小的控制以及空间重新配置时的数据移动效率。

vector 的动态大小实质是在旧空间不足时， 申请新空间->移动数据->释放旧空间。

这里采用的空间增长策略是每次增长为原来的两倍。

## 注意事项
每一次空间重新配置时，旧的内存空间就会失效（导致迭代器失效）

空间的重新配置通常发生在有新的元素插入（使用空间变大时），使用中应当尽可能的重新获取迭代器（避免一直使用旧时存储下来的迭代器）

注意对象的构造析构过程，而不仅仅只是数据的拷贝与删除
