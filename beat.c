// #include <time.h>
// #define DR_WAV_IMPLEMENTATION
// #include "dr/dr_wav.h"

#include "game/assets.h"
#include <assert.h>
#include <stdint.h>
#define _POSIX_C_SOURCE 200809L
#include <sys/stat.h>
#include <sys/types.h>

#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#ifdef __TINYC__
#include <math.h>
#define fmodf fmod
#define sinf sin
#endif

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
  Vec2 to_screen_scale, scale, pan;
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

typedef struct Game {
  sg_pipeline pipeline;

  struct {
    vs_param_t vs_param;
    fs_param_t fs_param;
    Vec2 camera;
  } render;

  Buffer tilemap_buffer;
  Buffer animation_buffer_4x4;
  sg_sampler pixel_sampler;

  sg_image images[NB_Img];

  Scene scene;

  double time;
} Game;

void game_set_scene(Game *g, Scene scene) { g->scene = scene; }

float Game_time(Game *g) { return g->time; }

const Buffer *d_tilemap_buffer(Game *g) { return &g->tilemap_buffer; }
const Buffer *d_animation_buffer(Game *g) { return &g->animation_buffer_4x4; }

void d_color(Game *game, Color c) {
  game->render.fs_param.color[0] = c.r;
  game->render.fs_param.color[1] = c.g;
  game->render.fs_param.color[2] = c.b;
  game->render.fs_param.color[3] = c.a;
}
void d_noise(Game *game, float n) { game->render.fs_param.noise = n; }

void d_buffer(Game *g, const Buffer *buffer, const sg_image *img, Vec2 pan) {
  g->render.vs_param.pan = v_add(g->render.camera, pan);

  sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(g->render.vs_param));
  sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &SG_RANGE(g->render.fs_param));
  sg_apply_bindings(&(sg_bindings){
      .fs = {.images = {*img}, .samplers = {g->pixel_sampler}},
      .vertex_buffers = {buffer->vertices},
      .index_buffer = buffer->indices,
  });
  sg_draw(0, buffer->num_elements, 1);
}

void d_object(Game *g, const Buffer *buffer, const sg_image *tex, Vec2 pan, int frame) {
  g->render.vs_param.pan = v_add(g->render.camera, pan);

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

void update_state(Game *g, double dt) {
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

bool is_set(uint8_t *map, int i, int j, int w, int h) {
  if (i < 0 || j < 0 || i >= w || j >= h)
    return false;
  return map[j * w + i] != 0;
}

uint8_t tile_code(uint8_t *map, int i, int j, int w, int h) {
  uint8_t code = 0;
  code += is_set(map, i + 0, j + 0, w, h) * 1;
  code += is_set(map, i + 1, j + 0, w, h) * 2;
  code += is_set(map, i + 1, j + 1, w, h) * 4;
  code += is_set(map, i + 0, j + 1, w, h) * 8;
  return code;
}

Buffer create_tile_map_buffer() {
#define ni 20
#define nj 12
  uint8_t map[ni * nj] = {
      0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, //
      0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
      0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, //
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, //
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, //
      0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, //
      0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, //
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, //
      1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, //
      1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, //
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, //
      0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, //
  };

  static int lu[16][2] = {
      {0, 3}, {0, 0}, {1, 3}, {3, 0}, {0, 2}, {2, 3}, {1, 0}, {1, 1},
      {3, 3}, {3, 2}, {0, 1}, {2, 0}, {1, 2}, {3, 1}, {2, 2}, {2, 1},
  };

  vertex_t vertices[4 * (ni + 1) * (nj + 1)];
  uint16_t indices[6 * (ni + 1) * (nj + 1)];
  int ov = 0, oi = 0;
  for (int i = -1; i < ni; ++i) {
    for (int j = -1; j < nj; ++j) {
      uint8_t tc = tile_code(map, i, j, ni, nj);
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
  return (Buffer){
      .vertices = sg_make_buffer(&(sg_buffer_desc){
          .type = SG_BUFFERTYPE_VERTEXBUFFER,
          .data = SG_RANGE(vertices),
          .label = "vertex-buffer",
      }),
      .indices = sg_make_buffer(&(sg_buffer_desc){
          .type = SG_BUFFERTYPE_INDEXBUFFER,
          .data = SG_RANGE(indices),
          .label = "index-buffer",
      }),
      .num_elements = oi,
  };
}

static void Game_init(Game *g) {
  g->render.vs_param = (vs_param_t){
      {2.0f / sapp_width() * 2.0, 2.0f / sapp_height() * 2.0},
      {1.0f, 1.0f},
      {0.0f, 0.0f},
  };
  g->render.fs_param = (fs_param_t){{1, 1, 1, 1}, 0.0, 0};
  g->render.camera = (Vec2){32.0f, 32.0f};

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
                   "uniform vec2 scale;\n"
                   "uniform vec2 pan;\n"
                   "\n"
                   "layout(location=0) in vec4 position;\n"
                   "layout(location=1) in vec2 texcoord;\n"
                   "\n"
                   "out vec2 p;\n"
                   "out vec2 uv;\n"
                   "\n"
                   "void main() {\n"
                   //  "  gl_Position = mvp * position;\n"
                   "  p = position.xy + pan;\n"
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
                   "  vec4 c = texture(tex, uv);\n"
                   "  float row = (int(p.x) % 16 == 0) ? 0.9 : 1.0;\n"
                   "  float col = (int(p.y) % 16 == 0) ? 0.9 : 1.0;\n"
                   "  int xx = int(uv.x * 64)/2 * 1024 * 17;\n"
                   "  int yy = int(uv.y * 64)/2 * 128 * 57;\n"
                   "  float n = c.a * noise * ((((rand ^ xx ^ yy) % 2000) - 1000)/1000.0);\n"
                   "  frag_color =  vec4((min(row,col) * vec3(color) * vec3(c)) + vec3(n), color.a * c.a);\n"
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
                         {"scale", SG_UNIFORMTYPE_FLOAT2, 1},
                         {"pan", SG_UNIFORMTYPE_FLOAT2, 1},
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

  g->tilemap_buffer = create_tile_map_buffer();
  g->animation_buffer_4x4 = quad_animation_buffer(0, 0, 16, 16, 2, 2);

  g->pixel_sampler = sg_make_sampler(&(sg_sampler_desc){
      .min_filter = SG_FILTER_NEAREST,
      .mag_filter = SG_FILTER_NEAREST,
      .mipmap_filter = SG_FILTER_NEAREST,
      .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
      .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
      .label = "pixel_sampler",
  });

  GameScene_init(g);
}

void jump_to(float l, float c) {
  sdtx_home();
  sdtx_origin(c, l);
}

static void Game_frame(Game *g) {
  update_state(g, sapp_frame_duration());

  sdtx_canvas(sapp_width() * 0.5f, sapp_height() * 0.5f);
  sdtx_font(FONT_KC853);

  jump_to(0, 0);
  sdtx_color3b(0x42, 0x53, 0x47);
  sdtx_printf("%f", g->time);

  sg_begin_pass(&(sg_pass){
      .action = {.colors[0] = {.load_action = SG_LOADACTION_CLEAR, .clear_value = {0.0f, 0.125f, 0.25f, 1.0f}}},
      .swapchain = sglue_swapchain(),
  });

  sdtx_draw();

  sg_apply_pipeline(g->pipeline);
  g->render.fs_param.rand = rand();

  if (g->scene.draw)
    g->scene.draw(g->scene.context, g);

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
  return v_sub(v_diff((Vec2){x, sapp_height() - y}, 2.0f), g->render.camera);
}

static void Game_handel_events(const sapp_event *e, Game *g) {

  if (e->type == SAPP_EVENTTYPE_MOUSE_DOWN) {
    if (g->scene.mouse_click)
      g->scene.mouse_click(g->scene.context, g, to_scene(g, e->mouse_x, e->mouse_y), e->mouse_button);

  } else if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
    if (g->scene.mouse_move)
      g->scene.mouse_move(g->scene.context, g, to_scene(g, e->mouse_x, e->mouse_y));
  } else if ((e->type == SAPP_EVENTTYPE_KEY_DOWN)) {
    switch (e->key_code) {
    case SAPP_KEYCODE_SPACE:
      break;
    default:
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  gc_start(&gc, &argc);
  (void)argv;

  Game g = (Game){0};
  sapp_run(&(sapp_desc){
      .init_userdata_cb = (void (*)(void *))Game_init,
      .frame_userdata_cb = (void (*)(void *))Game_frame,
      .cleanup_userdata_cb = (void (*)(void *))Game_cleanup,
      .event_userdata_cb = (void (*)(const sapp_event *, void *))Game_handel_events,
      .user_data = &g,
      .width = 800,
      .height = 600,
      .window_title = "a 2D thing",
      .icon.sokol_default = true,
      .logger.func = slog_func,
  });

  gc_stop(&gc);
  return 0;
}