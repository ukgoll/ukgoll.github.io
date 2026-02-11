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