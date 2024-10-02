//------------------------------------------------------------------------------
//  debugtext-sapp.c
//  Text rendering with sokol_debugtext.h, test builtin fonts.
//---------------------------------------------------------------------------
// #include <time.h>
// #define DR_WAV_IMPLEMENTATION
// #include "dr/dr_wav.h"

#define _POSIX_C_SOURCE 200809L
#include <sys/stat.h>
#include <sys/types.h>
// #include <unistd.h>

#ifdef __TINYC__
#include <math.h>
#define fmodf fmod
#define sinf sin
#endif

#define SOKOL_GLCORE
#define SOKOL_DEBUGTEXT_IMPL

#include "sokol_app.h"
#include "sokol_audio.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "util/sokol_debugtext.h"

#define FONT_KC853 (0)
#define FONT_KC854 (1)
#define FONT_Z1013 (2)
#define FONT_CPC (3)
#define FONT_C64 (4)
#define FONT_ORIC (5)

typedef struct Button Button;
typedef void (*OnClick)(Button *);

typedef struct Button {
  int l, c;
  bool enabled;
  OnClick on_click;
  char caption[32];
} Button;

typedef struct Producer {
  int count;
  int base_cost;
  int multiplier;
  float cost_growth;
} Producer;

int Producer_cost(const Producer *e) { return (int)(e->base_cost * pow(e->cost_growth, e->count)); }

void on_beat_click(Button *);
void on_life_click(Button *);
void on_joke_click(Button *);
void on_cool_click(Button *);
void on_mine_click(Button *);
void on_crow_click(Button *);

#define BUTTON_COUNT 8
static struct {
  size_t beat_count;
  size_t beat_count_old;

  double time;
  double trigger_time;
  Producer life, joke, cool, mine, crow;

  Button buttons[BUTTON_COUNT];

  int mouse_line;
  int mouse_column;

  sg_pass_action pass_action;
} state = {
    .beat_count = 0,

    .life = {0, 24, 1, 1.14f},
    .joke = {0, 125, 5, 1.155f},
    .cool = {0, 600, 5 * 4, 1.144f},
    .mine = {0, 3200, 5 * 4 * 3, 1.375f},
    .crow = {0, 15000, 5 * 4 * 3 * 2, 1.365f},

    .buttons =
        {
            {27, 18, true, on_beat_click, "<00>"},
            {1, 1, false, on_life_click, "life 000(B24)"},
            {2, 1, false, on_joke_click, "joke 000(B125)"},
            {3, 1, false, on_cool_click, "cool 000(B600)"},
            {4, 1, false, on_mine_click, "mine 000(B3200)"},
            {5, 1, false, on_crow_click, "crow 000(B15000)"},
        },

    .pass_action =
        {
            .colors[0] =
                {
                    .load_action = SG_LOADACTION_CLEAR,
                    .clear_value = {0.0f, 0.125f, 0.25f, 1.0f},
                },
        },
};

void update_beat_count() {
  if (state.beat_count_old == state.beat_count)
    return;

  int digits = 0;
  size_t c = state.beat_count;
  while (c > 0) {
    digits += 2;
    c /= 100;
  }
  if (digits < 2)
    digits = 2;

  Button *b = &state.buttons[0];
  snprintf(b->caption, sizeof(b->caption), "<%0*lld>", digits, state.beat_count);
  b->c = 19 - digits / 2;
}

size_t tick_update_count() {
  return state.life.count * state.life.multiplier + state.joke.count * state.joke.multiplier +
         state.cool.count * state.cool.multiplier + state.mine.count * state.mine.multiplier +
         state.crow.count * state.crow.multiplier;
}

void update_state(double dt) {
  state.time += dt;

  if (state.trigger_time <= 0.0) {
    state.trigger_time = 1.0;
    state.beat_count += tick_update_count();
  } else
    state.trigger_time -= dt;

  update_beat_count();

  state.buttons[1].enabled = Producer_cost(&state.life) <= state.beat_count;
  state.buttons[2].enabled = Producer_cost(&state.joke) <= state.beat_count;
  state.buttons[3].enabled = Producer_cost(&state.cool) <= state.beat_count;
  state.buttons[4].enabled = Producer_cost(&state.mine) <= state.beat_count;
  state.buttons[5].enabled = Producer_cost(&state.crow) <= state.beat_count;
}

void on_enhancement_click(Producer *e, Button *b, const char *n) {
  int cost = Producer_cost(e);
  if (cost > state.beat_count)
    return;

  state.beat_count -= cost;
  e->count++;

  cost = Producer_cost(e);
  snprintf(b->caption, sizeof(b->caption), "%s %03d(B%d)", n, e->count, cost);
}

void on_beat_click(Button *b) { state.beat_count++; }

void on_life_click(Button *b) { on_enhancement_click(&state.life, b, "life"); }
void on_joke_click(Button *b) { on_enhancement_click(&state.joke, b, "joke"); }
void on_cool_click(Button *b) { on_enhancement_click(&state.cool, b, "cool"); }
void on_mine_click(Button *b) { on_enhancement_click(&state.mine, b, "mine"); }
void on_crow_click(Button *b) { on_enhancement_click(&state.crow, b, "crow"); }

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
}

bool Button_hovered(const Button *b) {
  return state.mouse_line == b->l && state.mouse_column >= b->c && state.mouse_column < b->l + strlen(b->caption);
}

static void frame(void) {

  update_state(sapp_frame_duration());

  sdtx_canvas(sapp_width() * 0.5f, sapp_height() * 0.5f);
  sdtx_home();

  sdtx_font(FONT_KC853);
  sdtx_home();

  for (int i = 0; i < BUTTON_COUNT && state.buttons[i].caption[0]; ++i) {
    sdtx_home();
    sdtx_origin((float)state.buttons[i].c, (float)state.buttons[i].l);
    if (!state.buttons[i].enabled)
      sdtx_color3b(0x42, 0x53, 0x47);
    else if (Button_hovered(&state.buttons[i]))
      sdtx_color3b(0xf4, 0x43, 0x36);
    else
      sdtx_color3b(0xa2, 0xb3, 0xa7);
    sdtx_puts(state.buttons[i].caption);
  }
  if (state.life.count > 0) {
    sdtx_home();
    sdtx_origin(18, 28);
    sdtx_color3b(0x33, 0x33, 0x33);
    sdtx_puts(".  .");
    const float t = (1.0 - state.trigger_time);
    sdtx_color3f(t * t * 0.8, 0.125f + t * t * 0.5, 0.25f + t * t * 0.3f);
    sdtx_home();
    sdtx_origin(18 + t * 1.5f, 28);
    sdtx_puts(":");
    sdtx_home();
    sdtx_origin(21 - t * 1.5f, 28);
    sdtx_puts(":");
  }

  if (state.life.count > 0) {
    sdtx_canvas(sapp_width(), sapp_height());
    sdtx_home();
    sdtx_origin(1.0f, 58.0f);
    sdtx_color3b(0x33, 0x33, 0x33);
    sdtx_printf("%lld per tick", tick_update_count());
  }

  // sdtx_home();
  // sdtx_origin(0.0f, 14.0f);
  // sdtx_color3b(0x33, 0x33, 0x33);
  // sdtx_printf("%0.2d:%0.2d", state.mouse_line, state.mouse_column);

  sg_begin_pass(&(sg_pass){.action = state.pass_action, .swapchain = sglue_swapchain()});
  sdtx_draw();
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
    for (int i = 0; i < BUTTON_COUNT && state.buttons[i].caption[0]; ++i) {
      if (Button_hovered(&state.buttons[i]) && state.buttons[i].on_click)
        state.buttons[i].on_click(&state.buttons[i]);
    }

  } else if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
    state.mouse_column = (int)e->mouse_x / 2 / 8;
    state.mouse_line = (int)e->mouse_y / 2 / 8;

  } else if ((e->type == SAPP_EVENTTYPE_KEY_DOWN)) {
    switch (e->key_code) {
    case SAPP_KEYCODE_SPACE:
      on_beat_click(&state.buttons[0]);
      break;
    default:
      break;
    }
  }
}

int main(int argc, char *argv[]) {

  sapp_run(&(sapp_desc){
      .init_cb = init,
      .frame_cb = frame,
      .cleanup_cb = cleanup,
      .event_cb = events,
      .width = 640,
      .height = 480,
      .window_title = "beat",
      .icon.sokol_default = true,
      .logger.func = slog_func,
  });

  return 0;
}