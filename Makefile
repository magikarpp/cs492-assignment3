CFLAGS += -std=c99 -g -lm -Wno-incompatible-pointer-types -Wextra
src_files=main.c shell.c DTree.c LinkedList.c Queue.c LDisk.c Lfile.c
# Lfile.c Ldisk.c
name=filesystem

$(name): $(src_files)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(name)
