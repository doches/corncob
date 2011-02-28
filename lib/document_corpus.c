/*
 *  document_corpus.c
 *  corncob
 *
 *  Created by Trevor Fountain on 11/10/10.
 *  Copyright 2010 Expat Games. All rights reserved.
 *
 */

#include "document_corpus.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

document_corpus_document *document_corpus_document_new(char *directory, char *filename)
{
    document_corpus_document *new = (document_corpus_document *)malloc(sizeof(document_corpus_document));
    new->filename = (char *)malloc(sizeof(char)*100);
    new->filename[0] = '\0';
    strcpy(new->filename,directory);
    strcat(new->filename,"/");
    strcat(new->filename,filename);
    new->next = NULL;
    return new;
}

document_corpus *document_corpus_new(char *directory)
{
    document_corpus *new = (document_corpus *)malloc(sizeof(document_corpus));
    new->documents = NULL;
    new->wordmap = WordMap_new(1000);
    new->filename = (char *)malloc(sizeof(char)*100);
    new->filename[0] = '\0';
    new->document_count = 0;
    strcpy(new->filename,directory);
    
    document_corpus_scan_directory(new,directory);
    
    return new;
}

void document_corpus_scan_directory(document_corpus *corpus, char *directory)
{
    DIR *dir_handle;
    struct dirent *dir_entry;
    
    dir_handle = opendir(directory);
    if (dir_handle != NULL) {
        while ((dir_entry = readdir(dir_handle))) {
            char subfile[120];
            strcpy(subfile,directory);
            strcat(subfile,"/");
            strcat(subfile,dir_entry->d_name);
            DIR *subdir = opendir(subfile);
            if (subdir != NULL) {
                closedir(subdir);
                if (dir_entry->d_name[0] != '.') {
                    document_corpus_scan_directory(corpus,subfile);
                }
            } else {
                document_corpus_document *document = document_corpus_document_new(directory, dir_entry->d_name);
//                printf("%s/%s\n",directory,dir_entry->d_name);
                document->next = corpus->documents;
                corpus->documents = document;
                corpus->document_count++;
            }
        }
        closedir(dir_handle);
    } else {
        fprintf(stderr,"[document_corpus]: Failed to open directory '%s'\n",directory);
    }
}

void document_corpus_each_document(document_corpus *corpus, void (*document_callback)(unsigned int *,unsigned int))
{
    document_corpus_document *current_document = corpus->documents;
    while (current_document != NULL) {
        char *filename = current_document->filename;
        filename[strlen(corpus->filename)+1+strlen(current_document->filename)] = '\0';
        FILE *fin = fopen(filename,"r");
        char line[READLINE_LENGTH];
        
        unsigned int max_words = 8192;
        unsigned int num_words = 0;
        unsigned int *words = (unsigned int *)malloc(sizeof(unsigned int)*max_words);
        
        while(!feof(fin)) {
            char *ret = fgets(line, READLINE_LENGTH, fin);
            if (ret == NULL) {
                break;
            }
            
            char *token = strtok(line," \t\n");
            while (token != NULL) {
                int index = WordMap_index(corpus->wordmap, token);
                
                words[num_words] = index;
                num_words++;
                if (num_words >= max_words) {
                    max_words *= 2;
                    unsigned int *new_words = (unsigned int *)malloc(sizeof(unsigned int)*max_words);
                    for(int i=0;i<num_words;i++) {
                        new_words[i] = words[i];
                    }
                    unsigned int *old_words = words;
                    words = new_words;
                    free(old_words);
                }
                token = strtok(NULL," \t\n");
            }
        }
        printf("%s (%d words):\n",filename,num_words);
        (*document_callback)(words,num_words);
        fclose(fin);
        free(words);
        
        current_document = current_document->next;
    }
}

