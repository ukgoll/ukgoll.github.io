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