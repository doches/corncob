# Makefile for lda-c and assorted tools (int->int hash, string->int hash, sparsecount, etc.)

CC = gcc
CFLAGS = -g -std=c99 -Wall -I lib/

HEADERS = lib/corpus.h lib/ct_hash.h lib/SparseCounts.h lib/word_hash.h lib/WordMap.h tools/lda.h tools/rmc.h

# Test target (default)
test: ct_hash_test.o word_hash_test.o corpus_test.o SparseCounts_test.o WordMap_test.o test.o
	$(CC) $(CFLAGS) test.o word_hash_test.o ct_hash_test.o corpus_test.o SparseCounts_test.o WordMap_test.o WordMap.o word_hash.o ct_hash.o corpus.o SparseCounts.o -o ctools_test
	./ctools_test

test.o: test/test.c test/test.h
	$(CC) -c $(CFLAGS) test/test.c

ct_hash_test.o: ct_hash.o test/ct_hash_test.c test/ct_hash_test.h
	$(CC) -c $(CFLAGS) test/ct_hash_test.c

word_hash_test.o: word_hash.o test/word_hash_test.c test/word_hash_test.h
	$(CC) -c $(CFLAGS) test/word_hash_test.c
	
corpus_test.o: corpus.o test/corpus_test.c test/corpus_test.h
	$(CC) -c $(CFLAGS) test/corpus_test.c

SparseCounts_test.o: SparseCounts.o test/SparseCounts_test.c test/SparseCounts_test.h
	$(CC) -c $(CFLAGS) test/SparseCounts_test.c
	
WordMap_test.o: WordMap.o test/WordMap_test.c test/WordMap_test.h
	$(CC) -c $(CFLAGS) test/WordMap_test.c

# LDA target
lda: ct_hash.o word_hash.o corpus.o lda.o Instance.o
	$(CC) $(CFLAGS) ct_hash.o word_hash.o corpus.o lda.o Instance.o -o lda

Instance.o: lib/Instance.c lib/Instance.h
	$(CC) -c $(CFLAGS) lib/Instance.c

ct_hash.o: lib/ct_hash.c lib/ct_hash.h
	$(CC) -c $(CFLAGS) lib/ct_hash.c

word_hash.o: lib/word_hash.c lib/word_hash.h
	$(CC) -c $(CFLAGS) lib/word_hash.c

corpus.o: lib/corpus.c lib/corpus.h
	$(CC) -c $(CFLAGS) lib/corpus.c
	
SparseCounts.o: lib/SparseCounts.c lib/SparseCounts.h
	$(CC) -c $(CFLAGS) lib/SparseCounts.c

lda.o: tools/lda.c tools/lda.h
	$(CC) -c $(CFLAGS) tools/lda.c

# WordCount target
wordcount: WordMap.o SparseCounts.o corpus.o wordcount.o ct_hash.o word_hash.o
	$(CC) $(CFLAGS) WordMap.o word_hash.o SparseCounts.o corpus.o ct_hash.o wordcount.o -o wordcount

wordcount.o: tools/wordcount.c
	$(CC) -c $(CFLAGS) tools/wordcount.c
	
WordMap.o: lib/WordMap.h lib/WordMap.c
	$(CC) -c $(CFLAGS) lib/WordMap.c

# RMC target
rmc: rmc.o SparseCounts.o corpus.o ct_hash.o word_hash.o WordMap.o Instance.o
	$(CC) $(CFLAGS) WordMap.o SparseCounts.o word_hash.o ct_hash.o corpus.o rmc.o Instance.o -o rmc

rmc.o: tools/rmc.h tools/rmc.c
	$(CC) -c $(CFLAGS) tools/rmc.c

# Clean target
clean:
	rm -f *.o
	rm -rf doc/

# Doc target
doc: lib/*.h tools/*.h
	rm -rf doc/
	crud
