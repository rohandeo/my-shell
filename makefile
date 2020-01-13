run: final.c
	gcc final.c -o output
	./output

clean: output
	rm output