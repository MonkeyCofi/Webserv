import math

def sumsum(x, y=None):
	if y is None:
		return (x * (x + 1)) // 2
	return ((x - y + 1) * (x + y)) // 2

def closest(x):
	return int(math.sqrt(x * 2))

def issum(x):
	return sumsum(closest(x)) == x

x = 295
for i in range(612):
	x += i
	if issum(x):
		print(f"FOUND IT! {x} / {closest(x)}\nLeaves: {i + 1} to 611 = {sumsum(611, i + 1)}")
