### 容器
- 本库的主要目的在于实现常用的高级数据结构，称为***容器***，比如集合等，而非线性表等简单数据结构
- 实现的容器
	1. 集合（基于红黑树构造）
	2. 列表（基于线性表或链表，实现随机、顺序存取和堆栈、队列方式的存取）
	3. 优先级队列（基于二叉堆构造的优先级队列）
	4. 池（基于线性表和单链表构造）
	5. 哈希表（基于线性探测的三值哈希构造）
	6. 目录（基于树林和映射构造）
	7. 报表（基于二维线性表构造）
	8. 网络（基于有向加权图构造）
- 自定义结构类型`Container`，枚举类型`ContainerType`
```
// 容器结构
typedef struct {
	void *container;
	ContainerType type;
} Container, *Container_p;
// 容器类型的枚举
typedef enum {
	Set,
	List,
	PriorityQueue,
	Pool,
	HashTable,
	Catalogue,
	Report,
	Network
} ContainerType;
```

### 元素
- 本库定义容器中存放的内容称为***元素***，元素分为四种类型：integer, real, string, object，分别用以存放整数、实数、字符串和对象元素
```
// 元素的数据类型
typedef long long Integer;
typedef long double Real;
typedef char *String;
typedef void *Object;
// 元素类型名称枚举
typedef enum {
	integer,
	real,
	string,
	object
} ElementType;
// 元素的类型
typedef void *Element;
```
- 元素存入容器时进行一次完整复制，副本及其长度保存在容器之中，存入后对原元素的修改不影响容器中的副本
	- 元素存入容器时需要提供容器、元素首地址、元素类型、元素长度四个参数
	- 当元素类型为integer, real时，元素长度参数应使用sizeof(实际类型)来确定
	- 当元素类型为string时，元素长度一般为strlen(string)，也可以用元素长度限定存入容器的字符串的最大长度，即前len个字符，中文字符串要注意汉字截断问题
	- 当元素类型为object时，元素长度应为sizeof(object)
	- 例如集合容器的添加元素函数：`void set_add(Set set, Element element, ElementType type, size_t len);`
- 从容器中读取元素时返回一个`Element`类型（即`void *`）的结果，客户程序可以对其进行类型强制转换，但不恰当的类型强制转换可能获得无法预料的结果
- 容器在元素删除、容器清空、容器销毁的时候会自动销毁容器中的元素
- 容器在读取元素时，返回元素的一个完整副本，修改这个返回的元素不会影响容器中的元素，使用完毕后必须使用`free()`函数进行销毁
- 容器元素可以通过比较函数来进行比较，元素比较函数比较两个元素的大小，第一个元素大于第二个元素时返回一个小于0的整数，反之返回一个大于0的整数，相等时返回0
- 元素比较函数定义了指针类型，客户程序可以指定自定义的比较函数
```
typedef int (*CmpFunc)(const Element, const Element, size_t, size_t);
```
- 用户不指定自定义的比较函数时，容器使用其元素类型对应的默认比较函数进行元素比较，规则如下：
	- Integer: 比较实际的数值大小，返回-1, 0, 或1
	- Real: 比较实际的数值大小，返回-1, 0, 或1
	- String: 调用标准库函数`strcmp()`进行比较并返回其返回值
	- Object: 首先比较两者的字节数，不相等即返回-1或1，长度相等情况下使用`memcmp()`函数进行逐字节比较并返回结果

### 迭代器
- 容器除了自身特性决定的访问方式外还实现迭代访问元素的功能，不同容器迭代器的迭代算法不同但迭代器的客户程序接口则是统一的，部分容器不支持迭代，如池容器
- 迭代过程可以读取和删除元素，删除元素后迭代器将进入下一个迭代位置，读取或删除最后一个元素后一次迭代结束，再次访问返回NULL
- 一个容器可以同时获得多个迭代器，各个迭代器的迭代步骤互不干扰
- 迭代接口（以集合容器的迭代器为例）：
	- 获取容器的迭代器
	```
	Iterator it = set_iterator(Container_p container, IterateDirection dir);	// 部分容器可能不支持反向迭代
	```
	- 迭代读取元素
	```
	Element e;
	while ((e = it_next(it))) {
		// do something...
		free(e);
	}
	```
	- 迭代删除元素（部分容器的迭代器不支持元素删除功能，如集合，对不支持删除元素的迭代器调用`it_remove()`函数将直接返回0）
	```
	Element e = it_next(it);	// 先迭代一次
	size_t ret = it_remove(it);	// 删除当前迭代位置的元素，返回被删除的元素数量，删除失败返回0
	```
	- 迭代器重置，回到初始状态，从头开始迭代
	```
	it_reset(it);
	```
	- 迭代器销毁
	```
	it_destroy(it);			// 迭代结束或不再使用后必须及时销毁迭代器
	```

### 线程安全
- 所有实现的容器均为线程安全，但线程安全必然会影响部分性能，因此单线程环境时容器操作性能会比不提供线程安全时略有下降
- 迭代器使用Fast-Fail模式，在迭代过程中如果容器内容发生变化，则迭代立即结束，并返回NULL

### 字符串处理
- 支持UTF-8编码字符串的处理，支持以***字数***为基本长度单位进行处理
- 基本字符串处理功能包括：空串判断、空白符处理、子串查找、子串提取、按拼音顺序比较
- 高级字符串处理功能包括：分割、合并、子串替换、UTF-8和GB18030编码互换

### 通用算法
- 连续和离散存放两种存储形式下元素的快速排序
- 连续和离散存放两种存储形式下元素的插入排序
- MD5、SHA1算法
- 部分常用的经典哈希算法

### 错误处理
- 库函数使用统一的规则通过函数返回值向调用者发送错误信息
	- 返回值为整数类型的，-1表示出错
	- 返回值为指针类型的，包括Element，NULL表示出错
	- 在不会造成二义性的上下文中，部分函数返回-1或NULL并不代表出错，此类函数一般为不会导致错误发生的函数
	- 返回值为`void, size_t`或其他无法表示出错信息的函数，一般为此函数不会导致错误或此函数使用默认值静默地忽略错误
	- 存在特殊情况的参见头文件中函数声明处的注释
