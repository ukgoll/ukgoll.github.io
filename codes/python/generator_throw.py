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
	# except Exception as e:
	# 	print(f"Exception {e}")
	i_str = yield 3
	tc.close();
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
	print(tc.throw(TypeError("aflka"))) # 这里
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
# i_str is ('aflka',)
# 3
# GEN_SUSPENDED
# i_str is None
# throw coro end
# end close