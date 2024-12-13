test.img: fatgen
	./fatgen > test.img

fatgen : fat.c fs.c testmain.c fat.h
	gcc -g fat.c fs.c testmain.c -o fatgen

fs.c : genfs.py fatname.py testdir
	python genfs.py > fs.c

clean :
	rm -rf *.pyc fatgen test.img *.dSYM
