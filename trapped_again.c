// #include <time.h>
// #define DR_WAV_IMPLEMENTATION
// #include "dr/dr_wav.h"
#include <stdlib.h>
#include <string.h>

#ifdef __TINYC__
#include <math.h>
#define fmodf fmod
#define sinf sin
#endif

#ifdef _WIN32
#ifndef CLANGD_ANALYSIS
#include <windows.h>
#endif
#else
#include <dirent.h>
#endif

#include "game/assets.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#define _POSIX_C_SOURCE 200809L
#include <sys/stat.h>
#include <sys/types.h>

#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define SOKOL_NO_ENTRY
#define SOKOL_GLCORE
#define SOKOL_DEBUGTEXT_IMPL

#include "sokol_gfx.h"

#include "sokol_app.h"
#include "sokol_audio.h"

#include "sokol_glue.h"
#include "sokol_log.h"

#include "util/sokol_debugtext.h"

#include "gc/gc.h"
#include "math/Rect.h"
#include "math/Vec2.h"

#include "game/GameScene.h"

#define FONT_KC853 (0)
#define FONT_KC854 (1)
#define FONT_Z1013 (2)
#define FONT_CPC (3)
#define FONT_C64 (4)
#define FONT_ORIC (5)

typedef struct vertex_t {
  Vec2 p;
  uint16_t u, v;
} vertex_t;

typedef struct Buffer {
  sg_buffer vertices, indices;
  int num_elements;
} Buffer;

typedef struct vs_param_t {
  Vec2 to_screen_scale, pan;
  float rot;
} vs_param_t;

typedef struct fs_param_t {
  float color[4];
  float noise;
  int rand;
} fs_param_t;

typedef struct Assets {
  sg_image tilemap;
  sg_image wearisome;
} Assets;

typedef struct TileRectBuffer {
  Buffer *buffer;
  int w, h;
} TileRectBuffer;

typedef struct Game {
  sg_pipeline pipeline;

  struct {
    vs_param_t vs_param;
    fs_param_t fs_param;
    Vec2 camera_pan;
    float camera_scale;
    float overlay_scale;
  } render;

  TileRectBuffer tilerect_buffer[16];
  Buffer animation_buffer_4x4;
  sg_sampler pixel_sampler;

  sg_image images[NB_Img];

  Scene scene;

  double time;
} Game;

void g_set_scene(Game *g, Scene scene) { g->scene = scene; }

float g_time(Game *g) { return g->time; }
int g_frame(Game *g) { return (int)(g->time * 8.0f); }

void g_color(Game *game, Color c) {
  game->render.fs_param.color[0] = c.r;
  game->render.fs_param.color[1] = c.g;
  game->render.fs_param.color[2] = c.b;
  game->render.fs_param.color[3] = c.a;
}
void g_noise(Game *game, float n) { game->render.fs_param.noise = n; }

void g_buffer(Game *g, const Buffer *buffer, const sg_image *img, Vec2 pan) {
  g->render.vs_param.pan = v_add(g->render.camera_pan, pan);

  sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(g->render.vs_param));
  sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &SG_RANGE(g->render.fs_param));
  sg_apply_bindings(&(sg_bindings){
      .fs = {.images = {*img}, .samplers = {g->pixel_sampler}},
      .vertex_buffers = {buffer->vertices},
      .index_buffer = buffer->indices,
  });
  sg_draw(0, buffer->num_elements, 1);
}

void g_object(Game *g, const Buffer *buffer, const sg_image *tex, Vec2 pan, float rot, int frame) {
  g->render.vs_param.pan = v_add(g->render.camera_pan, pan);
  g->render.vs_param.rot = rot;

  sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(g->render.vs_param));
  sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &SG_RANGE(g->render.fs_param));
  sg_apply_bindings(&(sg_bindings){
      .fs = {.images = {*tex}, .samplers = {g->pixel_sampler}},
      .vertex_buffers = {buffer->vertices},
      .index_buffer = buffer->indices,
  });

  sg_draw(6 * frame, 6, 1);
}

sg_image img_load(const char *path) {
  int ww = 0, hh = 0, channel = 0;

  uint8_t *pixels = stbi_load(path, &ww, &hh, &channel, 4);
  printf("img: %s (%d,%d,%d)\n", path, ww, hh, channel);
  if (pixels) {
    sg_image img = sg_alloc_image();
    sg_init_image(img, &(sg_image_desc){.width = ww,
                                        .height = hh,
                                        .pixel_format = SG_PIXELFORMAT_RGBA8,
                                        .data = {.subimage[0][0] = {pixels, (ww * hh * 4)}}});
    stbi_image_free(pixels);
    return img;
  }
  assert(false);
  return (sg_image){};
}

const sg_image *g_image(Game *g, Image img) {
  if (g->images[img].id == 0)
    g->images[img] = img_load(image_paths[img]);
  return &g->images[img];
}

void Game_update_state(Game *g, double dt) {
  g->time += dt;

  if (g->scene.update)
    g->scene.update(g->scene.context, g, dt);
}

static void audio_cb(float *buffer, int num_frames, int num_channels, void *ud) {
  (void)ud;
  for (int i = 0; i < num_frames; ++i) {
    for (int j = 0; j < num_channels; ++j) {
      buffer[i * num_channels + j] = 0.0f;
    }
  }
}

typedef struct SubImage {
  int i, j, ni, nj;
} SubImage;

void add_quad(vertex_t *vertices, Rect r, SubImage img) {
  const int i = img.i;
  const int j = img.j;
  const int oi = 65535 / img.ni;
  const int oj = 65535 / img.nj;
  vertices[0] = (vertex_t){(Vec2){r.pos.x + 0, r.pos.y + 0}, (i + 0) * oi, (j + 1) * oj};
  vertices[1] = (vertex_t){(Vec2){r.pos.x + r.size.x, r.pos.y + 0}, (i + 1) * oi, (j + 1) * oj};
  vertices[2] = (vertex_t){(Vec2){r.pos.x + r.size.x, r.pos.y + r.size.y}, (i + 1) * oi, (j + 0) * oj};
  vertices[3] = (vertex_t){(Vec2){r.pos.x + 0, r.pos.y + r.size.y}, (i + 0) * oi, (j + 0) * oj};
}

Buffer quad_animation_buffer(float x, float y, float w, float h, int ni, int nj) {
  vertex_t vertices[4 * ni * nj];
  uint16_t indices[6 * ni * nj];
  int ov = 0;
  int oi = 0;
  for (int i = 0; i < ni; ++i) {
    for (int j = 0; j < nj; ++j) {
      add_quad(&vertices[ov], (Rect){{x, y}, {w, h}}, (SubImage){j, i, ni, nj});
      indices[oi + 0] = ov + 0;
      indices[oi + 1] = ov + 2;
      indices[oi + 2] = ov + 1;
      indices[oi + 3] = ov + 0;
      indices[oi + 4] = ov + 3;
      indices[oi + 5] = ov + 2;
      ov += 4;
      oi += 6;
    }
  }

  return (Buffer){
      .vertices = sg_make_buffer(&(sg_buffer_desc){
          .type = SG_BUFFERTYPE_VERTEXBUFFER,
          .data = (sg_range){vertices, sizeof(vertex_t) * 4 * ni * nj},
          .label = "vertex-buffer",
      }),
      .indices = sg_make_buffer(&(sg_buffer_desc){
          .type = SG_BUFFERTYPE_INDEXBUFFER,
          .data = (sg_range){indices, sizeof(uint16_t) * 6 * ni * nj},
          .label = "index-buffer",
      }),
      .num_elements = 6 * 2,
  };
}
uint8_t tile_code(int i, int j, IsSetCB is_set, void *data) {
  uint8_t code = 0;
  code += is_set(data, i + 0, j + 0) * 1;
  code += is_set(data, i + 1, j + 0) * 2;
  code += is_set(data, i + 1, j + 1) * 4;
  code += is_set(data, i + 0, j + 1) * 8;
  return code;
}
Buffer *create_tile_rect_buffer(int ni, int nj, IsSetCB is_set, void *data) {
  static int lu[16][2] = {
      {0, 3}, {0, 0}, {1, 3}, {3, 0}, {0, 2}, {2, 3}, {1, 0}, {1, 1},
      {3, 3}, {3, 2}, {0, 1}, {2, 0}, {1, 2}, {3, 1}, {2, 2}, {2, 1},
  };

  vertex_t vertices[4 * (ni + 1) * (nj + 1)];
  uint16_t indices[6 * (ni + 1) * (nj + 1)];
  int ov = 0, oi = 0;
  for (int i = -1; i < ni; ++i) {
    for (int j = -1; j < nj; ++j) {
      uint8_t tc = tile_code(i, j, is_set, data);
      if (tc == 0)
        continue;
      float x = i * 16.0f;
      float y = j * 16.0f;
      add_quad(&vertices[ov], (Rect){{x, y}, {16, 16}}, (SubImage){lu[tc][0], lu[tc][1], 4, 4});
      indices[oi++] = ov + 0;
      indices[oi++] = ov + 2;
      indices[oi++] = ov + 1;
      indices[oi++] = ov + 0;
      indices[oi++] = ov + 3;
      indices[oi++] = ov + 2;
      ov += 4;
    }
  }
  Buffer *b = (Buffer *)gc_malloc(&gc, sizeof(Buffer));
  *b = (Buffer){
      .vertices = sg_make_buffer(&(sg_buffer_desc){
          .type = SG_BUFFERTYPE_VERTEXBUFFER,
          .data = (sg_range){vertices, sizeof(vertex_t) * 4 * (ni + 1) * (nj + 1)},
          .label = "vertex-buffer",
      }),
      .indices = sg_make_buffer(&(sg_buffer_desc){
          .type = SG_BUFFERTYPE_INDEXBUFFER,
          .data = (sg_range){indices, sizeof(uint16_t) * 6 * (ni + 1) * (nj + 1)},
          .label = "index-buffer",
      }),
      .num_elements = oi,
  };
  return b;
}

void Buffer_free(Buffer *b) {
  sg_destroy_buffer(b->vertices);
  sg_destroy_buffer(b->indices);
  gc_free(&gc, b);
}

bool rect_is_set(Recti *r, int i, int j) {
  if (i < 0 || j < 0 || i >= r->w || j >= r->h)
    return false;
  return true;
}

const Buffer *g_tilerect_buffer(Game *g, int w, int h) {
  TileRectBuffer *tb = NULL;
  for (int i = 0; i < 16; ++i) {
    if (g->tilerect_buffer[i].w == w && g->tilerect_buffer[i].h == h)
      return g->tilerect_buffer[i].buffer;
    if (g->tilerect_buffer[i].w == 0 && g->tilerect_buffer[i].h == 0) {
      tb = &g->tilerect_buffer[i];
      break;
    }
  }
  if (tb) {
    tb->buffer = create_tile_rect_buffer(w, h, (IsSetCB)rect_is_set, &(Recti){0, 0, w, h});
    tb->w = w;
    tb->h = h;
  }

  return tb ? tb->buffer : NULL;
}

const Buffer *g_animation_buffer(Game *g) { return &g->animation_buffer_4x4; }

static void Game_init(Game *g) {
  g->render.camera_pan = (Vec2){32.0f, 32.0f};
  g->render.camera_scale = 2.0f;
  g->render.overlay_scale = 2.0f;
  g->render.vs_param = (vs_param_t){
      {2.0f / sapp_width() * g->render.camera_scale, 2.0f / sapp_height() * g->render.camera_scale},
      {1.0f, 1.0f},
      0.0f,
  };
  g->render.fs_param = (fs_param_t){{1, 1, 1, 1}, 0.0, 0};

  sg_setup(&(sg_desc){
      .environment = sglue_environment(),
      .logger.func = slog_func,
  });

  sdtx_setup(&(sdtx_desc_t){
      .fonts = {[FONT_KC853] = sdtx_font_kc853(),
                [FONT_KC854] = sdtx_font_kc854(),
                [FONT_Z1013] = sdtx_font_z1013(),
                [FONT_CPC] = sdtx_font_cpc(),
                [FONT_C64] = sdtx_font_c64(),
                [FONT_ORIC] = sdtx_font_oric()},
      .logger.func = slog_func,
  });

  saudio_setup(&(saudio_desc){
      .sample_rate = 44100,
      .num_channels = 2,
      .stream_userdata_cb = audio_cb,
      .user_data = g,
  });

  const char *vs = "#version 330\n"
                   "\n"
                   "uniform vec2 to_screen_scale;\n"
                   "uniform vec2 pan;\n"
                   "uniform float rot;\n"
                   "\n"
                   "layout(location=0) in vec4 position;\n"
                   "layout(location=1) in vec2 texcoord;\n"
                   "\n"
                   "out vec2 p;\n"
                   "out vec2 uv;\n"
                   "\n"
                   "vec2 rotate(vec2 v, float a) {\n"
                   "	float s = sin(a);\n"
                   "	float c = cos(a);\n"
                   "	mat2 m = mat2(c, -s, s, c);\n"
                   "	return m * v;\n"
                   "}\n"
                   "\n"
                   "void main() {\n"
                   //  "  gl_Position = mvp * position;\n"
                   "  p = rotate(position.xy, rot) + pan;\n"
                   "  gl_Position = vec4(p.x * to_screen_scale.x - 1, p.y * to_screen_scale.y - 1, 0, 1);\n"
                   "  uv = texcoord;\n"
                   "}\n";

  const char *fs = "#version 330\n"
                   "\n"
                   "uniform sampler2D tex;\n"
                   "uniform vec4 color;\n"
                   "uniform float noise;\n"
                   "uniform int rand;\n"
                   "\n"
                   "in vec2 p;\n"
                   "in vec2 uv;\n"
                   "\n"
                   "out vec4 frag_color;\n"
                   "\n"
                   "void main() {\n"
                   "  vec4 c = texture(tex, uv) * vec4(vec3(1.0),1.0);\n"
                   "  int xx = int(uv.x * 64)/2 * 1024 * 17;\n"
                   "  int yy = int(uv.y * 64)/2 * 128 * 57;\n"
                   "  float n = c.a * noise * ((((rand ^ xx ^ yy) % 2000) - 1000)/1000.0);\n"
                   "  vec3 cc = (vec3(color) * vec3(c)) + vec3(n);\n"
                   "  float ca = color.a * c.a;\n"
                   "  frag_color = vec4(cc, ca);\n"
                   "}\n";

  sg_shader shader = sg_make_shader(&(sg_shader_desc){
      .attrs = {{.name = "position"}, {.name = "texcoord"}},
      .vs = {.source = vs,
             .uniform_blocks = {{
                 .size = sizeof(vs_param_t),
                 .layout = SG_UNIFORMLAYOUT_NATIVE,
                 .uniforms =
                     {
                         {"to_screen_scale", SG_UNIFORMTYPE_FLOAT2, 1},
                         {"pan", SG_UNIFORMTYPE_FLOAT2, 1},
                         {"rot", SG_UNIFORMTYPE_FLOAT, 1},
                     },
             }}},
      .fs =
          {
              .source = fs,
              .uniform_blocks = {{
                  .size = sizeof(fs_param_t),
                  .layout = SG_UNIFORMLAYOUT_NATIVE,
                  .uniforms =
                      {
                          {"color", SG_UNIFORMTYPE_FLOAT4, 1},
                          {"noise", SG_UNIFORMTYPE_FLOAT, 1},
                          {"rand", SG_UNIFORMTYPE_INT, 1},
                      },
              }},
              .images[0] = {.used = true, .image_type = SG_IMAGETYPE_2D, .sample_type = SG_IMAGESAMPLETYPE_FLOAT},
              .samplers[0] = {.used = true, .sampler_type = SG_SAMPLERTYPE_FILTERING},
              .image_sampler_pairs[0] = {.used = true, .image_slot = 0, .sampler_slot = 0, .glsl_name = "tex"},
          },
  });

  g->pipeline = sg_make_pipeline(&(sg_pipeline_desc){
      .shader = shader,
      .layout =
          (sg_vertex_layout_state){
              .buffers = {{.stride = (int)sizeof(vertex_t)}},
              .attrs =
                  {
                      {.offset = (int)offsetof(vertex_t, p), .format = SG_VERTEXFORMAT_FLOAT2},
                      {.offset = (int)offsetof(vertex_t, u), .format = SG_VERTEXFORMAT_USHORT2N},
                  },
          },
      .index_type = SG_INDEXTYPE_UINT16,
      .cull_mode = SG_CULLMODE_BACK,
      .color_count = 1,
      .colors = {{
          .pixel_format = SG_PIXELFORMAT_RGBA8,
          .write_mask = SG_COLORMASK_RGBA,
          .blend =
              {
                  .enabled = true,
                  .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                  .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                  .src_factor_alpha = SG_BLENDFACTOR_SRC_ALPHA,
                  .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
              },
      }},
      .depth =
          {
              .compare = SG_COMPAREFUNC_LESS_EQUAL,
              .write_enabled = true,
          },
  });

  memset(g->tilerect_buffer, 0, sizeof(g->tilerect_buffer));
  g->animation_buffer_4x4 = quad_animation_buffer(-8, -8, 16, 16, 4, 4);

  g->pixel_sampler = sg_make_sampler(&(sg_sampler_desc){
      .min_filter = SG_FILTER_LINEAR,
      .mag_filter = SG_FILTER_LINEAR,
      .mipmap_filter = SG_FILTER_NEAREST,
      .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
      .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
      .label = "pixel_sampler",
  });

  GameScene_init(g);
}

void Game_update_console(Game *g) {
  sdtx_canvas(sapp_width(), sapp_height());
  sdtx_font(FONT_KC853);

  sdtx_home();
  sdtx_origin(0, 0);
  sdtx_color3b(0x42, 0x53, 0x47);
  sdtx_printf("%f\n", g_time(g));
  sdtx_printf("%f\n", sapp_frame_duration());
}

void Game_draw_scene(Game *g) {
  if (g->scene.draw) {
    g->render.vs_param.to_screen_scale =
        (Vec2){2.0f / sapp_width() * g->render.camera_scale, 2.0f / sapp_height() * g->render.camera_scale};
    g->scene.draw(g->scene.context, g);
  }

  if (g->scene.draw_overlay) {
    Vec2 pan = g->render.camera_pan;
    g->render.vs_param.to_screen_scale =
        (Vec2){2.0f / sapp_width() * g->render.overlay_scale, 2.0f / sapp_height() * g->render.overlay_scale};
    g->render.camera_pan = (Vec2){0.0f, 0.0};
    g->scene.draw_overlay(g->scene.context, g);
    g->render.camera_pan = pan;
  }
}

static void Game_draw(Game *g) {
  Game_update_state(g, sapp_frame_duration());

  Game_update_console(g);

  sg_begin_pass(&(sg_pass){
      .action = {.colors[0] = {.load_action = SG_LOADACTION_CLEAR, .clear_value = {1.0f, 1.0f, 1.0f, 1.0f}}},
      .swapchain = sglue_swapchain(),
  });

  sg_apply_pipeline(g->pipeline);
  g->render.fs_param.rand = rand();

  Game_draw_scene(g);
  sdtx_draw();

  sg_end_pass();
  sg_commit();
}

static void Game_cleanup(Game *g) {
  (void)g;

  sdtx_shutdown();
  saudio_shutdown();
  sg_shutdown();
}

static Vec2 to_scene(Game *g, float x, float y) {
  return v_sub(v_diff((Vec2){x, sapp_height() - y}, g->render.camera_scale), g->render.camera_pan);
}
static Vec2 to_overlay(Game *g, float x, float y) {
  return v_diff((Vec2){x, sapp_height() - y}, g->render.overlay_scale);
}

bool mid_down = false;
static void Game_handel_events(const sapp_event *e, Game *g) {
  if (e->type == SAPP_EVENTTYPE_MOUSE_SCROLL) {
    Vec2 mp_b = to_scene(g, e->mouse_x, e->mouse_y);
    g->render.camera_scale += e->scroll_y * 0.1f;
    Vec2 mp_a = to_scene(g, e->mouse_x, e->mouse_y);
    g->render.camera_pan = v_add(g->render.camera_pan, v_sub(mp_a, mp_b));

  } else if (e->type == SAPP_EVENTTYPE_MOUSE_DOWN) {
    if (e->mouse_button == 2)
      mid_down = true;

    if (g->scene.mouse_down)
      g->scene.mouse_down(g->scene.context, g, to_scene(g, e->mouse_x, e->mouse_y),
                          to_overlay(g, e->mouse_x, e->mouse_y), e->mouse_button);
  } else if (e->type == SAPP_EVENTTYPE_MOUSE_UP) {
    if (e->mouse_button == 2)
      mid_down = false;
    if (g->scene.mouse_up)
      g->scene.mouse_up(g->scene.context, g, to_scene(g, e->mouse_x, e->mouse_y), to_overlay(g, e->mouse_x, e->mouse_y),
                        e->mouse_button);
  } else if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
    if (mid_down)
      g->render.camera_pan =
          v_add(g->render.camera_pan, v_diff((Vec2){e->mouse_dx, -e->mouse_dy}, g->render.camera_scale));
    if (g->scene.mouse_move)
      g->scene.mouse_move(g->scene.context, g, to_scene(g, e->mouse_x, e->mouse_y),
                          to_overlay(g, e->mouse_x, e->mouse_y));
  } else if ((e->type == SAPP_EVENTTYPE_KEY_DOWN)) {
    switch (e->key_code) {
    case SAPP_KEYCODE_SPACE:
      break;
    default:
      break;
    }
  }
}

typedef void (*dirCB)(const char *p, void *ud);
void eachFileIn(const char *sDir, dirCB cb, void *ud) {
  char sPath[2048];

#ifdef CLANGD_ANALYSIS
  (void)sDir;
  (void)cb;
  (void)ud;
  (void)sPath;
#elif _WIN32
  WIN32_FIND_DATA fdFile;
  HANDLE hFind = NULL;

  sprintf(sPath, "%s\\*.*", sDir);

  if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    return;

  do {
    if (strcmp(fdFile.cFileName, ".") == 0 || strcmp(fdFile.cFileName, "..") == 0 ||
        (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      continue;

    // Build up our file path using the passed in
    //   [sDir] and the file/foldername we just found:
    sprintf(sPath, "%s/%s", sDir, fdFile.cFileName);
    cb(sPath, ud);
  } while (FindNextFile(hFind, &fdFile)); // Find the next file.

  FindClose(hFind); // Always, Always, clean things up!
#else
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(sDir)) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_type != DT_REG)
        continue;
      sprintf(sPath, "%s/%s", sDir, ent->d_name);
      cb(sPath, ud);
    }
    closedir(dir);
  } else {
  }
#endif
}

bool str_ends_with(const char *str, const char *suffix) {
  if (!str || !suffix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return 0;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void export_svg__with_inkscape(const char *fp, void *ud) {
  char call[2048];

  if (!ud || !str_ends_with(fp, ".svg"))
    return;

  sprintf(call, "%s --export-type=png -d 192 %s", (const char *)ud, fp);
  printf("%s\n", call);
  system(call);
}

int main(int argc, char *argv[]) {
  gc_start(&gc, &argc);

  for (int i = 0; i < argc - 1; ++i)
    if (strcmp(argv[i], "--export-svg") == 0) {
      eachFileIn("assets", export_svg__with_inkscape, argv[i + 1]);
      return 0;
    }

  Game g = (Game){0};
  sapp_run(&(sapp_desc){
      .init_userdata_cb = (void (*)(void *))Game_init,
      .frame_userdata_cb = (void (*)(void *))Game_draw,
      .cleanup_userdata_cb = (void (*)(void *))Game_cleanup,
      .event_userdata_cb = (void (*)(const sapp_event *, void *))Game_handel_events,
      .user_data = &g,
      .width = 1024,
      .height = 690,
      .window_title = "trapped again",
      .icon.sokol_default = true,
      .logger.func = slog_func,
  });

  gc_stop(&gc);
  return 0;
}