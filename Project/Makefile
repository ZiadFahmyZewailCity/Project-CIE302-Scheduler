build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc scheduler.c -o scheduler.out
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out

debug:
	gcc process_generator.c -g -o process_generator.out
	gcc clk.c -g -o clk.out
	gcc scheduler.c -g -o scheduler.out
	gcc process.c -g -o process.out
	gcc test_generator.c -o test_generator.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out
