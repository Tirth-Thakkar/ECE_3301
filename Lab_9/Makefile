.PHONY: all build clean test

# Convenience targets only. Each Part_N folder is an independent project.
all: build

build:
	$(MAKE) -C Part_1 build
	$(MAKE) -C Part_2 build
	$(MAKE) -C Part_3 build

clean:
	$(MAKE) -C Part_1 clean
	$(MAKE) -C Part_2 clean
	$(MAKE) -C Part_3 clean

test:
	mkdir -p build/tests
	$(CC) -std=c99 -Wall -Wextra -Werror -Itests tests/test_monitor.c \
		Part_3/src/ADC.c Part_3/src/System.c Part_3/src/Dashboard.c \
		Part_3/src/Altimeter.c Part_3/src/MotionSensor.c Part_3/src/LIS3DH.c \
		Common/src/Button.c Common/src/Indicators.c -lm -o build/tests/test_monitor
	./build/tests/test_monitor
	$(CC) -std=c99 -Wall -Wextra -Werror tests/test_part1.c Part_1/src/StateController.c -o build/tests/test_part1
	./build/tests/test_part1
	$(CC) -std=c99 -Wall -Wextra -Werror tests/test_part2.c Part_2/src/StateController.c -o build/tests/test_part2
	./build/tests/test_part2
