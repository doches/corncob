#include "ct_hash_print.h"

WordMap *static_hash_print_wordmap;   
FILE *static_hplh_file;   
void hash_print_labeled_helper(hash_element *element) 
{ 
  fprintf(static_hplh_file,"%s (%d), ",WordMap_reverse_lookup(static_hash_print_wordmap, element->key),element->value);   
} 

void hash_print_labeled(ct_hash *hash,char *label,WordMap *wordmap)   
{ 
  hash_fprint_labeled(stdout,hash,label,wordmap); 
} 

void hash_fprint_labeled(FILE *fout, ct_hash *hash, char *label, WordMap *wordmap)
{ 
  fprintf(fout,"%s <",label); 

  static_hplh_file = fout;
	static_hash_print_wordmap = wordmap;
  hash_foreach(hash, &hash_print_labeled_helper); 
	static_hash_print_wordmap = NULL;   
  fprintf(fout,">\n");
}

void hash_fprint_helper(hash_element *element)
{
    fprintf(static_hplh_file,"%d (%d), ",element->key,element->value);
}

void hash_fprint(FILE *fout, ct_hash *hash, char *label)
{
    fprintf(fout,"%s <",label);
    static_hplh_file = fout;
    hash_foreach(hash, &hash_fprint_helper);
    static_hplh_file = NULL;
    fprintf(fout,">\n");
}
