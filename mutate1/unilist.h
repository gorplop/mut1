#ifndef _UNILIST_H
#define _UNILIST_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  void *next;
  void *element;
} unilist_el;

typedef struct {
  unilist_el* first;
  unilist_el* last;
  size_t elsize;
  bool (*contains)(uint32_t);
  uint32_t count;
} unilist_t;

unilist_t* unilist_create(size_t elsize);
void unilist_destroy(unilist_t *al);
unilist_el* unilist_append(unilist_t *al, void* element);
void unilist_delete(unilist_t *al, uint32_t index);
unilist_el* unilist_get(unilist_t *al, uint32_t index);
unilist_el* unilist_first(unilist_t *al);
void unilist_print(unilist_t *al);
void unilist_iter(unilist_t *al, void (*pf)(void* x));
//bool unilist_contains(unilist_t *al, uint32_t uniess);
  
#endif // _UNILIST_H
