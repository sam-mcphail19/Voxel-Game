import matplotlib.pyplot as plt
import numpy as np
import re

def parse_vec_string(vec: str):
	match = re.search(r"glm::vec2\(([^,]+)f, ([^,]+)f\)", vec)
	return (float(match.group(1)), float(match.group(2)))

def main():
	# Points provided
	points = [
		"glm::vec2(0.00f, 0.04f)",
		"glm::vec2(0.15f, 0.07f)",
		"glm::vec2(0.42f, 0.09f)",
		"glm::vec2(0.47f, 0.25f)",
		"glm::vec2(0.59f, 0.26f)",
		"glm::vec2(0.71f, 0.29f)",
		"glm::vec2(0.74f, 0.54f)",
		"glm::vec2(0.77f, 0.75f)",
		"glm::vec2(0.81f, 0.76f)",
		"glm::vec2(0.88f, 0.76f)",
		"glm::vec2(1.00f, 0.77f)",
	]

	points = [parse_vec_string(point) for point in points]

	x_values, y_values = zip(*points)

	plt.figure(figsize=(8, 6))
	plt.plot(x_values, y_values, marker='o', linestyle='-', color='b')

	plt.ylim(0, 1)

	plt.grid(True)

	plt.show()

if __name__ == "__main__":
	main()