#ifndef CJSONH
#define CJSONH

#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *space = "                                ";
static int indent = 0;

static inline bool streq(const char *a, const char *b) { return strcmp(a, b) == 0; }

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

typedef struct CJHObjectR {
  const char *start;
} CJHObjectR;

typedef struct CJHArrayR {
  const char *start;
} CJHArrayR;

typedef void (*CJHReadObjectCB)(CJHObjectR *o, const char *key, void *userdata);
typedef void (*CJHReadArrayCB)(CJHArrayR *a, int index, void *userdata);

static inline const char *skip_white_space(const char *content) {
  while (*content && isspace(*content))
    content++;
  return content;
}

static inline const char *end_of_string(const char *c) {
  while (*c && (*c != '"' || *(c - 1) == '\\')) // does not catch "...\\"
    c++;
  return c;
}
static inline const char *cjh__read_object(const char *c, CJHReadObjectCB cb, void *userdata) {
  c = skip_white_space(c);
  assert(*c == '{');
  c++;

  while (*c) {
    c = skip_white_space(c);
    if (*c == '}') {
      c++;
      break;
    }

    assert(*c == '"');
    c++;
    const char *key = c;
    c = end_of_string(c);
    assert(*c);
    char *key_end = (char *)c;
    *key_end = '\0';

    c++;
    c = skip_white_space(c);
    assert(*c == ':');
    c++;
    c = skip_white_space(c);

    CJHObjectR sub = {c};
    cb(&sub, key, userdata);

    *key_end = '"';

    c = sub.start;
    assert(c);
    c = skip_white_space(c);
    if (*c == ',')
      c++;
    else
      assert(*c == '}');
  }

  return c;
}

static inline void cjh_o_read_object(CJHObjectR *o, CJHReadObjectCB cb, void *userdata) {
  o->start = cjh__read_object(o->start, cb, userdata);
}
static inline void cjh_a_read_object(CJHArrayR *a, CJHReadObjectCB cb, void *userdata) {
  a->start = cjh__read_object(a->start, cb, userdata);
}

static inline const char *cjh__read_array(const char *c, CJHReadArrayCB cb, void *userdata) {
  c = skip_white_space(c);
  assert(*c == '[');
  c++;

  int index = 0;
  while (*c) {
    c = skip_white_space(c);
    if (*c == ']') {
      c++;
      break;
    }

    CJHArrayR sub = {c};
    cb(&sub, index++, userdata);
    c = sub.start;

    assert(c);
    c = skip_white_space(c);
    if (*c == ',')
      c++;
    else
      assert(*c == ']');
  }

  return c;
}

static inline void cjh_o_read_array(CJHObjectR *o, CJHReadArrayCB cb, void *userdata) {
  o->start = cjh__read_array(o->start, cb, userdata);
}
static inline void cjh_a_read_array(CJHArrayR *a, CJHReadArrayCB cb, void *userdata) {
  a->start = cjh__read_array(a->start, cb, userdata);
}

static inline bool cjh_read(const char *file, CJHReadObjectCB cb, void *userdata) {
  CJHObject o = {.file = fopen(file, "r"), false};
  if (!o.file)
    return false;

  fseek(o.file, 0, SEEK_END);
  const size_t fsize = ftell(o.file);
  fseek(o.file, 0, SEEK_SET);

  char *content = malloc(fsize + 1);
  fread(content, fsize, 1, o.file);
  fclose(o.file);

  cjh_o_read_object(&(CJHObjectR){content}, cb, userdata);

  free(content);
  return true;
}

static inline const char *cjh__skip_block(const char *c, const char b, const char e) {
  assert(*c == b);
  int count = 1;
  c++;
  while (*c && count > 0) {
    if (*c == b)
      count++;
    else if (*c == e)
      count--;
    c++;
  }
  return c;
}
static inline const char *cjh__skip_word(const char *c) {
  while (*c && !isspace(*c) && *c != ',' && *c != '}' && *c != '{')
    ++c;
  return c;
}
static inline const char *cjh__skip(const char *c) {
  c = skip_white_space(c);

  if (*c == '"') {
    c++;
    c = end_of_string(c);
    assert(*c == '"');
    c++;

  } else if (*c == '{') {
    c = cjh__skip_block(c, '{', '}');

  } else if (*c == '[') {
    c = cjh__skip_block(c, '[', ']');

  } else {
    c = cjh__skip_word(c);
  }

  return c;
}
static inline void cjh_o_skip(CJHObjectR *o) { o->start = cjh__skip(o->start); }
static inline void cjh_a_skip(CJHArrayR *a) { a->start = cjh__skip(a->start); }

static inline double cjh__read_bool(const char **cp) {
  const char *c = skip_white_space(*cp);
  char *e = (char *)cjh__skip_word(c);
  assert(e > c);
  const char old = *e;
  *e = '\0';
  assert(streq(c, "true") || streq(c, "false"));
  bool b = streq(c, "true");
  *e = old;
  *cp = e;
  return b;
}
static inline double cjh_o_read_bool(CJHObjectR *o) { return cjh__read_bool(&o->start); }
static inline double cjh_a_read_bool(CJHArrayR *a) { return cjh__read_bool(&a->start); }

static inline double cjh__read_number(const char **cp) {
  const char *c = skip_white_space(*cp);

  char *e;
  double num = strtod(c, &e);
  assert(e > c);
  *cp = e;
  return num;
}
static inline double cjh_o_read_number(CJHObjectR *o) { return cjh__read_number(&o->start); }
static inline double cjh_a_read_number(CJHArrayR *a) { return cjh__read_number(&a->start); }

typedef struct StrView {
  const char *s;
  const int len;
} StrView;
static inline StrView cjh__read_string(const char **cp) {
  const char *c = skip_white_space(*cp);
  assert(*c == '"');
  c++;
  const char *e = end_of_string(c);

  *cp = e + 1;
  return (StrView){c, (int)(e - c)};
}
static inline StrView cjh_o_read_string(CJHObjectR *o) { return cjh__read_string(&o->start); }
static inline StrView cjh_a_read_string(CJHArrayR *a) { return cjh__read_string(&a->start); }

#endif