#include "unilist.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define S1(N) #N
#define S2(N) S1(N)
#define ERROR(...) fprintf(stderr, __FILE__ ":" S2(__LINE__) __VA_ARGS__ );


unilist_t* unilist_create(size_t elsize){
  unilist_t *al = (unilist_t*) malloc(sizeof(unilist_t));
  if(al == NULL) {
    ERROR("malloc fail\r");
    //XXX: do something when malloc fails
    return NULL;
  }
  al->elsize = elsize;
  al->first = NULL;
  al->last = NULL;
  al->count = 0;
  return al;
}

void unilist_destroy(unilist_t *al){
  while(al->first != NULL) unilist_delete(al, 0);
  free(al);
  
}

unilist_el* unilist_append(unilist_t *al, void* element){
  unilist_el *el = (unilist_el*) malloc(sizeof(unilist_el));
  if(el == NULL) {
    //XXX: do something when malloc fails
  }
  if(al->first == NULL) {
    al->first = el;
    al->last = el;
  }  // Handle adding first element
  
  el->element = malloc(al->elsize);
  memcpy(el->element, element, al->elsize);
  el->next = NULL;
  
  if(al->count !=0) al->last->next = el;
  al->last = el;
  al->count++;
}

void unilist_delete(unilist_t *al, uint32_t index) {
  int i = 0;
  unilist_el *el = al->first;
  unilist_el *prev = NULL;
  unilist_el *next = NULL;
  //find element
  while(i < index && el != NULL){
    prev = el;
    el = el->next;
    next = el->next;
    i++;
  }

  if(prev == NULL){ //First element - special case
    al->first = el->next;
  } else if(next == NULL) { //Last element - special case
    al->last = prev;
    prev->next = NULL;
  } else {
    prev->next = next;
  }
  //Delete element
  free(el->element);
  free(el);
  al->count--;
}

unilist_el* unilist_get(unilist_t *al, uint32_t index) {
  if(al->count < index) return NULL;
  unilist_el *el = al->first;
  int i = 0;
  //find element
  while(i < index && el != NULL){
    el = el->next;
    i++;
  }
  return el;
}


void unilist_print(unilist_t *al) {
  //XXX: check if list is not empty
  unilist_el *el = al->first;
  int i = 0;
  while(el != NULL) {
    printf("%d: el (%p): %p\tnext=%p\n", i, el, el->element, el->next);
    i++;
    el = el->next;
  }
}


void unilist_iter(unilist_t *al, void (*pf)(void* x)){
  unilist_el *el = al->first;
  int i = 0;
  while(el != NULL) {
    pf(el->element);
    i++;
    el = el->next;
  }
}


unilist_el* unilist_first(unilist_t *al) { return al->first; }
