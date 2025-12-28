---
title: "Python yield 协程函数"
date: 2025-12-28 19:19:00 +0800
categories: [Python, Async]
tags: [Python, Async]
---

# Preface
近日重新读了一下 《流畅的Python》这本书籍里关于协程的这一块，重读带来的感受还是不一样，对于 Python 协程的理解和应用又多了一些感悟。

# yield 和 yield from 关键字
在 Python3.5 以前，没有 `async` 和 `await` 两个关键字，在这之前大部分都是使用的是 `yield` 和 `yield from` 两个关键字。这两个关键词只能用在函数里面，使用了这两个关键词的函数被称之为 **协程函数**，协程函数不会直接调用，他会返回一个 **生成器** 对象。使用 [next()](https://docs.python.org/3/library/functions.html#next) 函数启动。使用 `async` 定义的协程返回的是 `协程对象`(实现了 `__await__` 协议)。


## 协程函数
一般是指使用了 `yield` 和 `yield from` 的函数，其余没有什么很特殊的地方，调用方通过 `co.send` 向协程函数发送数据，这样一来，可以自己来实现调度算法了。同时当 yield 执行完毕之后，协程函数会抛出 `StopIteration` 异常，调用方需要处理。（对于 `yield from` 就相当好，他会自动激活协程函数以及处理 `StopIteration` 异常。

一般有四个状态，我们可以通过 `inspect` 模块的 `getgeneratorstate` 函数来获取协程的状态，[inspec](https://docs.python.org/3/library/inspect.html)这个模块很强大，对于很多低层次的判断都有函数调用。
有四个状态

1. `GEN_CREATED` 等待开始执行，这个时候表示协程刚创建，使用 `next(co)` 或者 `co.send(None)` 来激活。（对于 send 函数，未激活的时候，只能发送 `None` 来激活。
2. `GEN_RUNNING`
3. `GEN_SUSPENDED` 暂停在 `yield` 表达式处
4. `GEN_CLOSED` 已关闭的状态


``` python
from inspect import getgeneratorstate

def coro_state():
	input_co = yield "retrun value"
	print(f"state: {getgeneratorstate(input_co)}")
	return "coro_state done!"
		

if __name__ == "__main__":
	co = coro_state()
	print(f"state: {getgeneratorstate(co)}")
	ret = next(co)
	print(f"ret is {ret}")
	print(f"state: {getgeneratorstate(co)}")
	try:
		co.send(co)
	except StopIteration as e:
		print(f"e.value is {e.value} and state: {getgeneratorstate(co)}")
	co.close()
	print(f"state: {getgeneratorstate(co)}")

# state: GEN_CREATED
# ret is retrun value
# state: GEN_SUSPENDED
# state: GEN_RUNNING
# e.value is coro_state done! and state: GEN_CLOSED
# state: GEN_CLOSED
```

### 操作
对于协程函数，操作的方法也就那么几个，python 的官方文档中也有写明 [generator-iterator-methods](https://docs.python.org/3/reference/expressions.html#generator-iterator-methods)

#### send 
> 恢复执行并向生成器函数“发送”一个值。 value 参数将成为当前 yield 表达式的结果。 send() 方法会返回生成器所产生的下一个值，或者如果生成器没有产生下一个值就退出则会引发 StopIteration。 当调用 send() 来启动生成器时，它必须以 None 作为调用参数，因为这时没有可以接收值的 yield 表达式。(当然也可以用 next 函数来激活协程)


```python
def send_coro():
	i_str = yield "return when start"
	return f"i_str is {i_str}"

if __name__ == "__main__":
	sc = send_coro()
	print(sc)
	# print(sc.send(None))
	print(next(sc))
	try:
		sc.send("vzgoll")
	except StopIteration as e:
		print(f"e.value is {e.value}")
# <generator object send_coro at 0x1026ddcc0>
# return when start
# e.value is i_str is vzgoll
```
从上面就可以很好的看出来，send 函数的作用。用 `next` 函数传递一个默认值可以规避掉处理 `StopIteration`，当然这是建立在你不需要接受调用方传递值到协程前提下。
#### throw
> 在生成器暂停的位置引发一个异常，并返回该生成器函数所产生的下一个值。 如果生成器没有产生下一个值就退出，则将引发 StopIteration 异常。 如果生成器函数没有捕获传入的异常，或是引发了另一个异常，则该异常会被传播给调用方

从下面这段代码就可以看出来（在看这段代码之前，需要了解 `yield` 返回和接受值的逻辑：当 `yield` `等待的时候，yield` 后面的值就会返回给调用方，同时等待调用方发送值来），在我们 `tc.send(None)` 之后，这个协程函数就启动了，此时到达第一个 `yield` 等待，同时返回 1。接着我们 `next` ，发送一个 `None` 过去，同时返回 `qq2`。前面这两个在知道 `yield` 是怎么工作之后还是很好理解的。然后，我们 `tc.throw(ValueError("wasd"))` 抛出一个错误，这个错误我们在协程里面手动处理了，所以和普通的 `next` 调用没什么区别：运行到下一个 `yield`，同时返回这个 `yield` 之后的值，也就是 3。随后就是普通的判断了。

当我们注释掉 `except Exception as e` 这个通用错误处理，以及使用 `throw` 抛出 `TypeError`，这样我们会发现错误抛出到了调用方这里，这也很符合错误处理逻辑。


所以说，`throw` 这个看起来麻烦一点，本质就是在考验当前这个 `yield` 有没有处理掉调用方手动 `throw` 的错误类型，处理了就和 `yield` 没有什么两样，如果没有处理的话。


```python
from inspect import getgeneratorstate

def throw_coro():
	i_str = yield 1
	print(f"i_str is {i_str}")
	try:
		i_str = yield "qq2"
		print(f"i_str is {i_str}")
	except ValueError as e:
		i_str = e.args
		print(f"i_str is {i_str}")
	except Exception as e:
		print(f"Exception {e}")
	i_str = yield 3
	print(f"i_str is {i_str}")
	print("throw coro end")

if __name__ == "__main__":
	tc = throw_coro()
	print(tc)
	print(getgeneratorstate(tc))
	print(tc.send(None))
	print(getgeneratorstate(tc))
	print("----")
	print(next(tc))
	print(tc.throw(ValueError("wasd"))) # 这里
	print(getgeneratorstate(tc))
	print(next(tc, "end close"))
	print(getgeneratorstate(tc))
	# try:
	# 	print(next(tc))
	# except StopIteration as e:
	# 	print('normal end')
	# tc.close()

# <generator object throw_coro at 0x1007dcf20>
# GEN_CREATED
# 1
# GEN_SUSPENDED
# ----
# i_str is None
# qq2
# i_str is ('wasd',)
# 3
# GEN_SUSPENDED
# i_str is None
# throw coro end
# end close
```

#### close
> 在生成器函数暂停的位置引发 GeneratorExit。 如果生成器函数随后正常退出、已经关闭，或者引发了 GeneratorExit (由于未捕获异常)，执行关闭将返回其调用方。 如果生成器产生了一个值，则将引发 RuntimeError。 如果生成器引发了任何其他异常，它将被传播给调用方。 如果生成器已经由于异常或以正常退出方式结束执行则 close() 将不会做任何事。

关于 close 的调用逻辑就更加的简单了，协程函数的内部处理 GeneratorExit 来退出函数，我们一般不需要做什么处理，如果需要手动捕获这个错误，也不能在在调用 close 之后，yield 返回值，简而言之，我们一般用不上。


## yield
先从基础的 yield 开始，yield 也是一种流程控制语句，在调用 [next()](https://docs.python.org/3/library/functions.html#next) 函数激活 `coroutine` 之后，整个协程函数就会卡在 yield 这里，直到调用方传递数据过来（这么一听，就很符合协程的风格了，适合调度器来调度）。 yield 后面后面的值就是暂停到 yield 这里时候返回给调用方的值，默认就是 `None`。

```python
def gen_coro():
	print("gen coro start")
	in_str = yield "return value"
	print(f"gen coro {in_str}")

if __name__ == "__main__":
	co = gen_coro()
	print(co)
	ret = next(co)
	print(f"ret is {ret}")
	try:
		co.send("yield done")
	except StopIteration as e:
		print(f"e.value {e.value}")

# <generator object gen_coro at 0x018611B0>
# gen coro start
# ret is return value
# gen coro yield done
# e.value None
```

## yield from 
对于 《流畅的Python》关于 `yield from` 中的案例 16.7 我个人觉得他写的并不是很好，他的委派生成器

```python
def grouper(results, key):
	while True:
		results[key] = yield from subgen()
```
用一个 while 循环巧妙的处理 `StopIteration`，不结束就不会抛出这个异常，但是其实这个 grouper 在每次循环都会重复的创建。只是使用一次，完全可以在循环的外面只创建一次，但是多次使用。

```python
def grouper(results):
	while True:
		key = yield
		if key == Ellipsis:
			print("because receipt Ellipsis end!!")
			break
		sub_ret = yield from sub_gen() 
		results[key] = sub_ret
	return "done"
```

这样这个 grouper 就可以重复的利用。


```python
from inspect import getgeneratorstate, isgeneratorfunction
from functools import wraps

def corowrap(func):
	if not isgeneratorfunction(func):
		raise ValueError(f"{func.__name__} must be generator")
	@wraps(func)
	def primer(*args, **kwargs):
		func_obj = func(*args, **kwargs)
		next(func_obj)
		return func_obj
	return primer


def sub_gen():
	total = 0
	count = 0
	avg = 0
	while True:
		nv = yield
		if nv is None:
			break
		count += 1
		total += nv
		avg = total / count
	return (count, avg)

@corowrap
def grouper(results):
	while True:
		key = yield
		if key == Ellipsis:
			print("because receipt Ellipsis end!!")
			break
		sub_ret = yield from sub_gen() 
		results[key] = sub_ret
	return "done"

def main(data):
	results = {}
	group = grouper(results)
	for key, values in data.items():
		group.send(key)
		for value in values:
			print(f"send value: {value}")
			group.send(value)
		else:
			print("main send None to break")
			print(getgeneratorstate(group))
			print(group.send(None))
	else:
		try:
			group.send(Ellipsis)
		except StopIteration as e:
			print(f"e is {e.value}")
	print(getgeneratorstate(group))
	print(f"results: {results}")
	group.close()
	print(getgeneratorstate(group))
	# try:
	# 	# group.send(Ellipsis)
	# 	group.close()
	# except StopIteration as e:
	# 	print(f"grouper fin {e}")
	# except Exception as e:
	# 	print(f"other exception: {e}")


if __name__ == "__main__":
	data = {
		"kg": [1,2,3,4,5],
		# "weight": [2,3,4]
	}
	main(data)
```