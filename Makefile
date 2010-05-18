# Makefile for lda-c and assorted tools (int->int hash, string->int hash, sparsecount, etc.)

CC = gcc
CFLAGS = -g -std=c99 -Wall -I lib/ -I vendor/include/ -I tools/ -O2
EXECUTABLES = lda rmc wordcount ctools_test nlda
HEADERS = lib/corpus.h lib/ct_hash.h lib/SparseCounts.h lib/word_hash.h lib/WordMap.h tools/lda.h tools/rmc.h lib/count_list.h lib/context_corpus.h vendor/include/progressbar.h tools/nlda.h

# Test target (default)
test: ct_hash_test.o word_hash_test.o corpus_test.o SparseCounts_test.o WordMap_test.o test.o context_corpus_test.o
	$(CC) $(CFLAGS) test.o word_hash_test.o ct_hash_test.o corpus_test.o SparseCounts_test.o WordMap_test.o WordMap.o word_hash.o ct_hash.o corpus.o SparseCounts.o context_corpus.o context_corpus_test.o progressbar.o -o ctools_test
	./ctools_test
	
ctools_test: test

test.o: test/test.c test/test.h
	$(CC) -c $(CFLAGS) test/test.c
	
context_corpus_test.o: context_corpus.o test/context_corpus_test.h test/context_corpus_test.c
	$(CC) -c $(CFLAGS) test/context_corpus_test.c

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
rmc: rmc.o SparseCounts.o corpus.o ct_hash.o word_hash.o WordMap.o Instance.o count_list.o
	$(CC) $(CFLAGS) WordMap.o SparseCounts.o word_hash.o ct_hash.o corpus.o rmc.o Instance.o count_list.o -o rmc

rmc.o: tools/rmc.h tools/rmc.c
	$(CC) -c $(CFLAGS) tools/rmc.c

count_list.o: lib/count_list.h lib/count_list.c
	$(CC) -c $(CFLAGS) lib/count_list.c

# nLDA target
context_corpus.o: lib/context_corpus.h lib/context_corpus.c progressbar.o
	$(CC) -c $(CFLAGS) lib/context_corpus.c
	
progressbar.o: vendor/lib/progressbar.c vendor/include/progressbar.h
	$(CC) -c $(CFLAGS) vendor/lib/progressbar.c

nlda.o: tools/nlda.h tools/nlda.c
	$(CC) -c $(CFLAGS) tools/nlda.c

nlda: context_corpus.o progressbar.o nlda.o WordMap.o SparseCounts.o ct_hash.o word_hash.o Instance.o count_list.o
	$(CC) $(CFLAGS) WordMap.o SparseCounts.o word_hash.o ct_hash.o Instance.o count_list.o context_corpus.o progressbar.o nlda.o -o nlda

# Clean target
clean:
	rm -f *.o
	rm -rf doc/
	rm -f $(EXECUTABLES)
	rm -f **/*~
	rm -f *~

# Doc target
doc: lib/*.h tools/*.h
	rm -rf doc/
	crud

all: $(EXECUTABLES) doc
