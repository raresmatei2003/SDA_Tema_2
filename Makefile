build: main.c
	gcc -std=c99 -g main.c -o quadtree

run: quadtree
	./quadtree

clean:
	rm quadtree