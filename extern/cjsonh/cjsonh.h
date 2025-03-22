#ifndef CJSONH
#define CJSONH

#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct CJHObject {
  FILE *file;
  bool comma;
} CJHObject;

typedef struct CJHArray {
  FILE *file;
  bool comma;
} CJHArray;

typedef void (*CJHWriteObjectCB)(CJHObject *o, void *userdata);
typedef void (*CJHWriteArrayCB)(CJHArray *a, void *userdata);

static inline void cjh_o_add_null(CJHObject *o, const char *key) {
  fprintf(o->file, "%s\"%s\":%s", (o->comma ? "," : ""), key, "null");
  o->comma = true;
}
static inline void cjh_o_add_bool(CJHObject *o, const char *key, bool b) {
  fprintf(o->file, "%s\"%s\":%s", (o->comma ? "," : ""), key, (b ? "true" : "false"));
  o->comma = true;
}
static inline void cjh_o_add_bool_if(CJHObject *o, const char *key, bool b, bool def) {
  if (b != def)
    cjh_o_add_bool(o, key, b);
}
static inline void cjh_o_add_number(CJHObject *o, const char *key, double d) {
  fprintf(o->file, "%s\"%s\":%.*g", (o->comma ? "," : ""), key, DBL_DIG, d);
  o->comma = true;
}
static inline void cjh_o_add_number_if(CJHObject *o, const char *key, double d, double def) {
  if (d != def)
    cjh_o_add_number(o, key, d);
}
static inline void cjh_o_add_string(CJHObject *o, const char *key, const char *val) {
  fprintf(o->file, "%s\"%s\":\"%s\"", (o->comma ? "," : ""), key, val);
  o->comma = true;
}
static inline void cjh_o_add_string_if(CJHObject *o, const char *key, const char *val, const char *def) {
  if (strcmp(val, def) != 0)
    cjh_o_add_string(o, key, val);
}
static inline void cjh_o_add_object(CJHObject *o, const char *key, CJHWriteObjectCB cb, void *userdata) {
  fprintf(o->file, "%s\"%s\":{", (o->comma ? "," : ""), key);
  o->comma = true;
  CJHObject sub = {o->file, false};
  cb(&sub, userdata);
  fprintf(o->file, "}");
}
static inline void cjh_o_add_array(CJHObject *o, const char *key, CJHWriteArrayCB cb, void *userdata) {
  fprintf(o->file, "%s\"%s\":[", (o->comma ? "," : ""), key);
  o->comma = true;
  CJHArray sub = {o->file, false};
  cb(&sub, userdata);
  fprintf(o->file, "]");
}

static inline void cjh_a_add_null(CJHArray *a) {
  fprintf(a->file, "%s%s", (a->comma ? "," : ""), "null");
  a->comma = true;
}
static inline void cjh_a_add_bool(CJHArray *a, bool b) {
  fprintf(a->file, "%s%s", (a->comma ? "," : ""), (b ? "true" : "false"));
  a->comma = true;
}
static inline void cjh_a_add_number(CJHArray *a, double d) {
  fprintf(a->file, "%s%.*g", (a->comma ? "," : ""), DBL_DIG, d);
  a->comma = true;
}
static inline void cjh_a_add_string(CJHArray *a, const char *val) {
  fprintf(a->file, "%s\"%s\"", (a->comma ? "," : ""), val);
  a->comma = true;
}
static inline void cjh_a_add_object(CJHArray *a, CJHWriteObjectCB cb, void *userdata) {
  fprintf(a->file, "%s{", (a->comma ? "," : ""));
  a->comma = true;
  CJHObject sub = {a->file, false};
  cb(&sub, userdata);
  fprintf(a->file, "}");
}
static inline void cjh_a_add_array(CJHArray *a, CJHWriteArrayCB cb, void *userdata) {
  fprintf(a->file, "%s[", (a->comma ? "," : ""));
  a->comma = true;
  CJHArray sub = {a->file, false};
  cb(&sub, userdata);
  fprintf(a->file, "]");
}

static inline bool cjh_write(const char *file, CJHWriteObjectCB cb, void *userdata) {
  CJHObject o = {.file = fopen(file, "w"), false};
  if (!o.file)
    return false;
  fprintf(o.file, "{");
  cb(&o, userdata);
  fprintf(o.file, "}");
  fclose(o.file);
  return true;
}

#endif