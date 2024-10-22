import matplotlib.pyplot as plt
import numpy as np
import re

def parse_vec_string(vec: str):
	match = re.search(r"glm::vec2\(([^,]+)f, ([^,]+)f\)", vec)
	return (float(match.group(1)), float(match.group(2)))

def main():
	# Points provided
	points = [
		"glm::vec2(0.0f, 80.0f)",
		"glm::vec2(0.05f, 30.0f)",
		"glm::vec2(0.3f, 30.0f)",
		"glm::vec2(0.5f, 65.0f)",
		"glm::vec2(0.65f, 70.0f)",
		"glm::vec2(0.8f, 120.0f)",
		"glm::vec2(1.0f, 120.0f)",
	]

	points = [parse_vec_string(point) for point in points]

	x_values, y_values = zip(*points)

	plt.figure(figsize=(8, 6))
	plt.plot(x_values, y_values, marker='o', linestyle='-', color='b')

	plt.grid(True)
	plt.legend()

	plt.show()

if __name__ == "__main__":
	main()