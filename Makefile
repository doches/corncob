# Makefile for lda-c and assorted tools (int->int hash, string->int hash, sparsecount, etc.)

CC = gcc-4.3
OPTIMIZATION_FLAGS = -O2
DEBUG_FLAGS = 

UNAME := $(shell uname)
ifeq ($(UNAME),Linux)
CFLAGS = -std=gnu99 -Wall -I lib/ -I vendor/include/ -I tools/ -Werror $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS)
endif
ifeq ($(UNAME),Darwin)
CFLAGS = -std=gnu99 -Wall -I lib/ -I vendor/include/ -I tools/ -Werror $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS)
endif

LFLAGS = -lm -lgsl -lgslcblas $(DEBUG_FLAGS)
EXECUTABLES = lda rmc wordcount ctools_test nlda nlda2 ocw gen_sv focw ntm cw
HEADERS = lib/corpus.h lib/ct_hash.h lib/SparseCounts.h lib/word_hash.h lib/WordMap.h tools/lda.h tools/rmc.h lib/count_list.h lib/context_corpus.h vendor/include/progressbar.h tools/nlda.h vendor/include/statusbar.h tools/ntm.h lib/document_corpus.h tools/cw.h

TEST := ${wildcard test/*.c}
TEST_OBJ := ${foreach src,${TEST},${subst .c,.o, ${lastword ${subst /, ,${src}}}}}
LIB := ${wildcard lib/*.c}
LIB_OBJ := ${foreach src,${LIB},${subst .c,.o, ${lastword ${subst /, ,${src}}}}}

# Test target (default)
#test: ct_hash_test.o word_hash_test.o corpus_test.o SparseCounts_test.o WordMap_test.o test.o context_corpus_test.o line_corpus_test.o unsigned_array_test.o cosine_test.o double_matrix_test.o ${TEST}
#	$(CC) $(LFLAGS) $(CFLAGS) test.o word_hash_test.o ct_hash_test.o corpus_test.o SparseCounts_test.o WordMap_test.o WordMap.o word_hash.o ct_hash.o corpus.o SparseCounts.o context_corpus.o context_corpus_test.o progressbar.o line_corpus_test.o line_corpus.o statusbar.o unsigned_array.o unsigned_array_test.o cosine_test.o cosine.o double_matrix_test.o double_matrix.o -o ctools_test
test: ${TEST_OBJ} ${LIB_OBJ} ${TEST}
	$(CC) $(LFLAGS) $(CFLAGS) ${TEST_OBJ} ${LIB_OBJ} progressbar.o statusbar.o -o ctools_test
	./ctools_test

test.o: test/test.c test/test.h
	$(CC) -c $(CFLAGS) test/test.c

double_matrix_test.o: double_matrix.o test/double_matrix_test.h test/double_matrix_test.c
	$(CC) -c $(CFLAGS) test/double_matrix_test.c

double_matrix.o: lib/double_matrix.h lib/double_matrix.c
	$(CC) -c $(CFLAGS) lib/double_matrix.c

cosine_test.o: cosine.o test/cosine_test.h test/cosine_test.c
	$(CC) -c $(CFLAGS) test/cosine_test.c
	
unsigned_array_test.o: unsigned_array.o test/unsigned_array_test.h test/unsigned_array_test.c
	$(CC) -c $(CFLAGS) test/unsigned_array_test.c

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

line_corpus_test.o: line_corpus.o test/line_corpus_test.h test/line_corpus_test.c statusbar.o
	$(CC) -c $(CFLAGS) test/line_corpus_test.c
	
double_hash_test.o: double_hash.o test/double_hash_test.h test/double_hash_test.c
	$(CC) -c $(CFLAGS) test/double_hash_test.c

# LDA target
lda: ct_hash.o word_hash.o corpus.o lda.o Instance.o
	$(CC) $(LFLAGS) $(CFLAGS) ct_hash.o word_hash.o corpus.o lda.o Instance.o -o lda

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
	$(CC) $(LFLAGS) $(CFLAGS) WordMap.o word_hash.o SparseCounts.o corpus.o ct_hash.o wordcount.o -o wordcount

wordcount.o: tools/wordcount.c
	$(CC) -c $(CFLAGS) tools/wordcount.c
	
WordMap.o: lib/WordMap.h lib/WordMap.c
	$(CC) -c $(CFLAGS) lib/WordMap.c

# RMC target
rmc: rmc.o SparseCounts.o corpus.o ct_hash.o word_hash.o WordMap.o Instance.o count_list.o
	$(CC) $(LFLAGS) $(CFLAGS) WordMap.o SparseCounts.o word_hash.o ct_hash.o corpus.o rmc.o Instance.o count_list.o -o rmc

rmc.o: tools/rmc.h tools/rmc.c
	$(CC) -c $(CFLAGS) tools/rmc.c

count_list.o: lib/count_list.h lib/count_list.c
	$(CC) -c $(CFLAGS) lib/count_list.c

# nLDA target
context_corpus.o: lib/context_corpus.h lib/context_corpus.c progressbar.o
	$(CC) -c $(CFLAGS) lib/context_corpus.c
	
progressbar.o: vendor/lib/progressbar.c vendor/include/progressbar.h
	$(CC) -c $(CFLAGS) vendor/lib/progressbar.c

statusbar.o: vendor/lib/statusbar.c vendor/include/statusbar.h
	$(CC) -c $(CFLAGS) vendor/lib/statusbar.c

nlda.o: tools/nlda.h tools/nlda.c
	$(CC) -c $(CFLAGS) tools/nlda.c

nlda: context_corpus.o progressbar.o nlda.o WordMap.o SparseCounts.o ct_hash.o word_hash.o Instance.o count_list.o
	$(CC) $(LFLAGS) $(CFLAGS) WordMap.o SparseCounts.o word_hash.o ct_hash.o Instance.o count_list.o context_corpus.o progressbar.o nlda.o -o nlda

# nLDA2 target
NLDA2_DEPS = document_corpus.o progressbar.o WordMap.o SparseCounts.o ct_hash.o word_hash.o Instance.o nlda2.o count_list.o
nlda2: $(NLDA2_DEPS)
	$(CC) $(LFLAGS) $(CFLAGS) $(NLDA2_DEPS) -o nlda2

line_corpus.o: lib/line_corpus.h lib/line_corpus.c statusbar.o
	$(CC) -c $(CFLAGS) lib/line_corpus.c

nlda2.o: tools/nlda2.h tools/nlda2.c
	$(CC) -c $(CFLAGS) tools/nlda2.c
	
gen_sv: gen_sv.o line_corpus.o progressbar.o WordMap.o SparseCounts.o ct_hash.o word_hash.o count_list.o statusbar.o
	$(CC) $(LFLAGS) $(CFLAGS) gen_sv.o line_corpus.o progressbar.o WordMap.o SparseCounts.o ct_hash.o word_hash.o count_list.o statusbar.o -o gen_sv

gen_sv.o: tools/gen_sv.c
	$(CC) -c $(CFLAGS) tools/gen_sv.c

ocw: line_corpus.o progressbar.o WordMap.o SparseCounts.o ct_hash.o word_hash.o ocw.o statusbar.o target_corpus.o unsigned_array.o ct_hash.o cosine.o double_matrix.o
	$(CC) $(LFLAGS) $(CFLAGS) line_corpus.o progressbar.o WordMap.o SparseCounts.o ct_hash.o word_hash.o ocw.o statusbar.o target_corpus.o unsigned_array.o cosine.o double_matrix.o -o ocw

ocw.o: tools/ocw.h tools/ocw.c
	$(CC) -c $(CFLAGS) tools/ocw.c

target_corpus.o: lib/target_corpus.c lib/target_corpus.h
	$(CC) -c $(CFLAGS) lib/target_corpus.c

unsigned_array.o: lib/unsigned_array.c lib/unsigned_array.h
	$(CC) -c $(CFLAGS) lib/unsigned_array.c

cosine.o: lib/cosine.c lib/cosine.h
	$(CC) -c $(CFLAGS) lib/cosine.c
	
document_corpus.o: lib/document_corpus.c lib/document_corpus.h
	$(CC) -c $(CFLAGS) lib/document_corpus.c
	
# FoCW target
FOCW_DEP = target_corpus.o WordMap.o progressbar.o statusbar.o focw.o unsigned_array.o word_hash.o LSH.o ct_hash.o double_hash.o ct_hash_print.o double_matrix.o
focw: $(FOCW_DEP)
	$(CC) $(LFLAGS) -lgsl $(FOCW_DEP) -o focw

focw.o: tools/focw.h tools/focw.c
	$(CC) -c $(CFLAGS) tools/focw.c

LSH.o: lib/LSH.h lib/LSH.c
	$(CC) -c $(CFLAGS) lib/LSH.c
	
double_hash.o: lib/double_hash.h lib/double_hash.c
	$(CC) -c $(CFLAGS) lib/double_hash.c
	
ct_hash_print.o: lib/ct_hash_print.h lib/ct_hash_print.c
	$(CC) -c $(CFLAGS) lib/ct_hash_print.c
	
CW_DEP = target_corpus.o WordMap.o progressbar.o statusbar.o cw.o unsigned_array.o word_hash.o LSH.o ct_hash.o double_hash.o ct_hash_print.o double_matrix.o
cw: $(CW_DEP)
	$(CC) $(LFLAGS) -lgsl $(CW_DEP) -o cw

cw.o: tools/cw.h tools/cw.c
	$(CC) -c $(CFLAGS) tools/cw.c

# nTM target
NTM_DEP = line_corpus.o WordMap.o progressbar.o statusbar.o ntm.o word_hash.o ct_hash.o ct_hash_print.o SparseCounts.o count_list.o unsigned_array.o
ntm: $(NTM_DEP)
	$(CC) $(LFLAGS) $(NTM_DEP) -o ntm
	
ntm.o: tools/ntm.h tools/ntm.c
	$(CC) -c $(CFLAGS) tools/ntm.c

.PHONY: clean doc all

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
