// #include <time.h>
// #define DR_WAV_IMPLEMENTATION
// #include "dr/dr_wav.h"

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

#define FONT_KC853 (0)
#define FONT_KC854 (1)
#define FONT_Z1013 (2)
#define FONT_CPC (3)
#define FONT_C64 (4)
#define FONT_ORIC (5)

typedef struct vertex_t {
  float x, y, z;
  uint16_t u, v;
} vertex_t;

typedef struct Buffer {
  sg_buffer vertices, indices;
  int num_elements;
} Buffer;

static struct {
  sg_pipeline pipeline;

  Buffer tilemap_buffer;
  Buffer wearisome_buffer;
  sg_image tilemap;
  sg_image wearisome;
  sg_sampler pixel_sampler;

  double time;
} state = {};

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

void update_state(double dt) { state.time += dt; }

static void audio_cb(float *buffer, int num_frames, int num_channels, void *ud) {
  (void)ud;
  for (int i = 0; i < num_frames; ++i) {
    for (int j = 0; j < num_channels; ++j) {
      buffer[i * num_channels + j] = 0.0f;
    }
  }
}

typedef struct Mat4 {
  float m[4][4];
} Mat4;
Mat4 orthographic(float Left, float Right, float Bottom, float Top, float Near, float Far) {
  Mat4 O = (Mat4){{
      {1.0f, 0.0f, 0.0f, 0.0f},
      {0.0f, 1.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
  }};

  O.m[0][0] = 2.0f / (Right - Left);
  O.m[1][1] = 2.0f / (Top - Bottom);
  O.m[2][2] = 2.0f / (Near - Far);

  O.m[3][0] = (Left + Right) / (Left - Right);
  O.m[3][1] = (Bottom + Top) / (Bottom - Top);
  O.m[3][2] = (Far + Near) / (Near - Far);
  return O;
}

Mat4 mul(Mat4 Left, Mat4 Right) {
  Mat4 Result = {.m = {{0.0f}}};
  for (int c = 0; c < 4; ++c) {
    for (int r = 0; r < 4; ++r) {
      float Sum = 0.0f;
      for (int cm = 0; cm < 4; ++cm)
        Sum += Left.m[cm][r] * Right.m[c][cm];
      Result.m[c][r] = Sum;
    }
  }
  return Result;
}

Mat4 translation3f(float x, float y, float z) {
  Mat4 tr = {};
  tr.m[0][0] = tr.m[1][1] = tr.m[2][2] = tr.m[3][3] = 1.0f;
  tr.m[3][0] = x;
  tr.m[3][1] = y;
  tr.m[3][2] = z;
  return tr;
}

typedef struct Rect {
  float x, y, w, h;
} Rect;

typedef struct SubImage {
  int i, j, ni, nj;
} SubImage;

void add_quad(vertex_t *vertices, Rect r, SubImage img) {
  const int i = img.i;
  const int j = img.j;
  const int oi = 65535 / img.ni;
  const int oj = 65535 / img.nj;
  vertices[0] = (vertex_t){r.x + 0, r.y + 0, 0, (i + 0) * oi, (j + 1) * oj};
  vertices[1] = (vertex_t){r.x + r.w, r.y + 0, 0, (i + 1) * oi, (j + 1) * oj};
  vertices[2] = (vertex_t){r.x + r.w, r.y + r.h, 0, (i + 1) * oi, (j + 0) * oj};
  vertices[3] = (vertex_t){r.x + 0, r.y + r.h, 0, (i + 0) * oi, (j + 0) * oj};
}

Buffer quad_buffer(float x, float y, float w, float h) {
  vertex_t vertices[8];
  add_quad(vertices, (Rect){x, y, w, h}, (SubImage){0, 0, 2, 2});
  add_quad(&vertices[4], (Rect){x, y, w, h}, (SubImage){1, 0, 2, 2});
  uint16_t indices[] = {0, 2, 1, 0, 3, 2, 4 + 0, 4 + 2, 4 + 1, 4 + 0, 4 + 3, 4 + 2};

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
      .num_elements = 6 * 2,
  };
}

bool is_set(uint8_t *map, int i, int j, int w, int h) {
  if (i < 0 || j < 0 || i >= w || j >= h)
    return false;
  return map[j * h + i] != 0;
}

uint8_t tile_code(uint8_t *map, int i, int j, int w, int h) {
  uint8_t code = 0;
  code += is_set(map, i + 0, j + 0, w, h) * 1;
  code += is_set(map, i + 1, j + 0, w, h) * 2;
  code += is_set(map, i + 1, j + 1, w, h) * 4;
  code += is_set(map, i + 0, j + 1, w, h) * 8;
  return code;
}

Buffer create_tile_map() {
  uint8_t map[8 * 8] = {
      0, 1, 1, 1, 1, 1, 1, 0, //
      0, 1, 1, 1, 1, 1, 1, 1, //
      0, 1, 1, 1, 1, 1, 0, 1, //
      1, 1, 1, 1, 0, 1, 1, 1, //
      0, 1, 1, 0, 1, 1, 1, 1, //
      1, 1, 1, 0, 1, 1, 1, 1, //
      0, 1, 0, 1, 1, 1, 1, 0, //
      0, 0, 0, 0, 1, 1, 0, 0, //
  };

  static int lu[16][2] = {
      {0, 3}, {0, 0}, {1, 3}, {3, 0}, {0, 2}, {2, 3}, {1, 0}, {1, 1},
      {3, 3}, {3, 2}, {0, 1}, {2, 0}, {1, 2}, {3, 1}, {2, 2}, {2, 1},
  };

  vertex_t vertices[4 * 9 * 9];
  uint16_t indices[6 * 9 * 9];
  int ov = 0, oi = 0;
  for (int i = -1; i < 8; ++i) {
    for (int j = -1; j < 8; ++j) {
      uint8_t tc = tile_code(map, i, j, 8, 8);
      if (tc == 0)
        continue;
      // printf("code %d %d %d\n", i, j, (int)tc);
      float x = i * 16.0f;
      float y = j * 16.0f;
      add_quad(&vertices[ov], (Rect){x, y, 16, 16}, (SubImage){lu[tc][0], lu[tc][1], 4, 4});
      // = {0, 2, 1, 0, 3, 2};
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

static void init(void) {
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
      .sample_rate = 44100, .num_channels = 2, .stream_userdata_cb = audio_cb,
      // .user_data = game,
  });

  const char *vs = "#version 330\n"
                   "\n"
                   "uniform mat4 mvp;\n"
                   "\n"
                   "layout(location=0) in vec4 position;\n"
                   "layout(location=1) in vec2 texcoord;\n"
                   "\n"
                   "out vec2 uv;\n"
                   "\n"
                   "void main() {\n"
                   "  gl_Position = mvp * position;\n"
                   "  uv = texcoord;\n"
                   "}\n";

  const char *fs = "#version 330\n"
                   "\n"
                   "uniform sampler2D tex;\n"
                   "uniform vec4 color;\n"
                   "uniform float noise;\n"
                   "uniform int rand;\n"
                   "\n"
                   "in vec2 uv;\n"
                   "\n"
                   "out vec4 frag_color;\n"
                   "\n"
                   "void main() {\n"
                   "  vec4 c = texture(tex, uv);\n"
                   "  int xx = int(uv.x * 64)/2 * 1024 * 17;\n"
                   "  int yy = int(uv.y * 64)/2 * 128 * 57;\n"
                   "  float n = c.a * noise * ((((rand ^ xx ^ yy) % 2000) - 1000)/1000.0);\n"
                   "  if (noise > 0)\n"
                   "    frag_color = color * c + vec4(n,n,n, c.a);\n"
                   "  else\n"
                   "    frag_color = color * c;\n"
                   "}\n";

  sg_shader
      shader =
          sg_make_shader(
              &(sg_shader_desc){
                  .attrs = {{.name = "position"}, {.name = "texcoord"}},
                  .vs = {.source = vs,
                         .uniform_blocks = {{.size = sizeof(float[4][4]),
                                             .layout = SG_UNIFORMLAYOUT_NATIVE,
                                             .uniforms = {{"mvp", SG_UNIFORMTYPE_MAT4, 1}}}}},
                  .fs =
                      {
                          .source = fs,
                          .uniform_blocks = {{.size = sizeof(float[5]) + sizeof(int),
                                              .layout = SG_UNIFORMLAYOUT_NATIVE,
                                              .uniforms =
                                                  {
                                                      {"color", SG_UNIFORMTYPE_FLOAT4, 1},
                                                      {"noise", SG_UNIFORMTYPE_FLOAT, 1},
                                                      {"rand", SG_UNIFORMTYPE_INT, 1},
                                                  }}},
                          .images[0] =
                              {.used = true, .image_type = SG_IMAGETYPE_2D, .sample_type = SG_IMAGESAMPLETYPE_FLOAT},
                          .samplers[0] = {.used = true, .sampler_type = SG_SAMPLERTYPE_FILTERING},
                          .image_sampler_pairs[0] =
                              {.used = true, .image_slot = 0, .sampler_slot = 0, .glsl_name = "tex"},
                      },
              });

  state.pipeline = sg_make_pipeline(&(sg_pipeline_desc){
      .shader = shader,
      .layout =
          (sg_vertex_layout_state){.buffers = {{.stride = (int)sizeof(vertex_t)}},
                                   .attrs =
                                       {
                                           {.offset = (int)offsetof(vertex_t, x), .format = SG_VERTEXFORMAT_FLOAT3},
                                           {.offset = (int)offsetof(vertex_t, u), .format = SG_VERTEXFORMAT_USHORT2N},
                                       }},
      .index_type = SG_INDEXTYPE_UINT16,
      .cull_mode = SG_CULLMODE_BACK,
      .color_count = 1,
      .colors = {{
          .pixel_format = SG_PIXELFORMAT_RGBA8,
          .write_mask = SG_COLORMASK_RGBA,
          .blend =
              {
                  .enabled = true,
                  .src_factor_rgb = SG_BLENDFACTOR_ONE,
                  .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                  .src_factor_alpha = SG_BLENDFACTOR_ONE,
                  .dst_factor_alpha = SG_BLENDFACTOR_ZERO,
              },
      }},
      .depth =
          {
              .compare = SG_COMPAREFUNC_LESS_EQUAL,
              .write_enabled = true,
          },
  });

  state.tilemap_buffer = create_tile_map();
  state.wearisome_buffer = quad_buffer(0, 0, 16, 16);

  state.tilemap = img_load("assets/tilemap.png");
  state.wearisome = img_load("assets/wearisome.png");
  state.pixel_sampler = sg_make_sampler(&(sg_sampler_desc){
      .min_filter = SG_FILTER_NEAREST,
      .mag_filter = SG_FILTER_NEAREST,
      .mipmap_filter = SG_FILTER_NEAREST,
      .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
      .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
      .label = "tilemap_sampler",
  });
}

void jump_to(float l, float c) {
  sdtx_home();
  sdtx_origin(c, l);
}

static void frame(void) {
  update_state(sapp_frame_duration());

  sdtx_canvas(sapp_width() * 0.5f, sapp_height() * 0.5f);
  sdtx_font(FONT_KC853);

  jump_to(0, 0);
  sdtx_color3b(0x42, 0x53, 0x47);
  sdtx_printf("%f", state.time);

  sg_begin_pass(&(sg_pass){
      .action = {.colors[0] = {.load_action = SG_LOADACTION_CLEAR, .clear_value = {0.0f, 0.125f, 0.25f, 1.0f}}},
      .swapchain = sglue_swapchain(),
  });

  sdtx_draw();

  sg_apply_pipeline(state.pipeline);

  Mat4 camera = orthographic(0.0f, sapp_width() * 0.35f, 0.0f, sapp_height() * 0.35f, -1.0f, 1.0f);

  struct {
    float color[4];
    float noise;
    int rand;
  } fs_param = {{1, 1, 1, 1}, 0.004f, rand()};

  Mat4 mvp = mul(camera, translation3f(128 - 8, 64 + 8, 0.0f));
  sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &(sg_range){mvp.m, sizeof(float[4][4])});
  sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &SG_RANGE(fs_param));
  sg_apply_bindings(&(sg_bindings){
      .fs = {.images = {state.tilemap}, .samplers = {state.pixel_sampler}},
      .vertex_buffers = {state.tilemap_buffer.vertices},
      .index_buffer = state.tilemap_buffer.indices,
  });
  sg_draw(0, state.tilemap_buffer.num_elements, 1);

  mvp = mul(camera, translation3f(128, 64, 0.0f));
  sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &(sg_range){mvp.m, sizeof(float[4][4])});
  fs_param.noise = 0.1f;
  sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &SG_RANGE(fs_param));

  sg_apply_bindings(&(sg_bindings){
      .fs = {.images = {state.wearisome}, .samplers = {state.pixel_sampler}},
      .vertex_buffers = {state.wearisome_buffer.vertices},
      .index_buffer = state.wearisome_buffer.indices,
  });
  sg_draw(6 * (((int)(state.time) % 2)), 6, 1);

  sg_end_pass();
  sg_commit();
}

static void cleanup(void) {
  sdtx_shutdown();
  saudio_shutdown();
  sg_shutdown();
}

void events(const sapp_event *e) {
  if (e->type == SAPP_EVENTTYPE_MOUSE_DOWN) {

  } else if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE) {

  } else if ((e->type == SAPP_EVENTTYPE_KEY_DOWN)) {
    switch (e->key_code) {
    case SAPP_KEYCODE_SPACE:
      break;
    default:
      break;
    }
  }
}

int main() {
  sapp_run(&(sapp_desc){
      .init_cb = init,
      .frame_cb = frame,
      .cleanup_cb = cleanup,
      .event_cb = events,
      .width = 800,
      .height = 600,
      .window_title = "a 2D thing",
      .icon.sokol_default = true,
      .logger.func = slog_func,
  });

  return 0;
}