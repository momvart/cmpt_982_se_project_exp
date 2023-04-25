void swap(char **p, char **q){
  char* t = *p; 
       *p = *q; 
       *q = t;
}
int main(){
      char a1, b1, c1; 
      char *a = &a1;
      char *b = &b1;
      char *c = &c1;
      swap(&a,&b);
      swap(&b,&c);
      swap(&c,&a);
}
