// #include <time.h>
// #define DR_WAV_IMPLEMENTATION
// #include "dr/dr_wav.h"

#define _POSIX_C_SOURCE 200809L
#include <sys/stat.h>
#include <sys/types.h>

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
  void *click_context;
  char caption[32];
} Button;

typedef struct Producer {
  const char *name;
  int count;
  int base_cost;
  int multiplier;
  int level;
  float cost_growth;
} Producer;

int Producer_cost(const Producer *e) { return (int)(e->base_cost * pow(e->cost_growth, e->count)); }

typedef struct Enhancement {
  int number_applied;
  int base_tick_expect;
  float tick_expect_growth;
  int base_cost;
  float cost_growth;
  bool selection_active;
} Enhancement;

void on_beat_click(Button *);
void on_producer_button_click(Button *);
void on_enhancement_button_click(Button *);
void on_new_producer_click(Button *);

#define BUTTON_COUNT 12
static struct {
  size_t beat_count;
  size_t beat_count_old;

  double time;
  double trigger_time;
  Producer life, joke, cool, mine, crow;
  Enhancement enhancement;

  Button buttons[BUTTON_COUNT];

  int mouse_line;
  int mouse_column;
} state = {
    .beat_count = 0,

    .life = {"life", 0, 24, 1, 1, 1.14f},
    .joke = {"joke", 0, 125, 5, 1, 1.155f},
    .cool = {"cool", 0, 600, 5 * 4, 1, 1.144f},
    .mine = {"mine", 0, 3200, 5 * 4 * 3, 1, 1.1375f},
    .crow = {"crow", 0, 15000, 5 * 4 * 3 * 2, 1, 1.1365f},

    .enhancement =
        {
            .number_applied = 0,
            .base_tick_expect = 10,
            .tick_expect_growth = 1.95f,
            .base_cost = 300,
            .cost_growth = 1.185f,
            .selection_active = false,
        },

    .buttons =
        {
            {27, 18, true, on_beat_click, NULL, "<00>"}, {1, 1, false, on_producer_button_click, &state.life, ""},
            // {2, 1, false, on_producer_button_click, &state.joke, ""},
            // {3, 1, false, on_producer_button_click, &state.cool, ""},
            // {4, 1, false, on_producer_button_click, &state.mine, ""},
            // {5, 1, false, on_producer_button_click, &state.crow, ""},
        },
};

size_t tick_update_of(const Producer *p) {
  size_t multiplier = (size_t)(p->multiplier * pow(p->level, 0.85));
  return p->count * multiplier;
}

size_t tick_update_count() {
  return tick_update_of(&state.life) + tick_update_of(&state.joke) + tick_update_of(&state.cool) +
         tick_update_of(&state.mine) + tick_update_of(&state.crow);
}

void update_enhancement(Enhancement *e) {
  if (e->selection_active) {
    int cost =
        (int)(state.enhancement.base_cost * pow(state.enhancement.cost_growth, state.enhancement.number_applied));
    for (int i = 6; i < 9; ++i)
      state.buttons[i].enabled = cost < state.beat_count;
    return;
  }

  size_t tick_update = tick_update_count();
  size_t min_tick_update = (int)(e->base_tick_expect * pow(e->tick_expect_growth, e->number_applied));
  if (tick_update < min_tick_update)
    return;

  e->selection_active = true;

  struct WP {
    void *context;
    const char *text;
    OnClick click;
    float cw;
    float w;
    bool used;
  } all[] = {
      {&state.joke, "new joke", on_new_producer_click,
       state.buttons[2].caption[0] || !state.buttons[1].caption[0] ? 0.0 : 1000.0f},
      {&state.cool, "new cool", on_new_producer_click,
       state.buttons[3].caption[0] || !state.buttons[2].caption[0] ? 0.0 : 1000.0f},
      {&state.mine, "new mine", on_new_producer_click,
       state.buttons[4].caption[0] || !state.buttons[3].caption[0] ? 0.0 : 1000.0f},
      {&state.crow, "new crow", on_new_producer_click,
       state.buttons[5].caption[0] || !state.buttons[4].caption[0] ? 0.0 : 1000.0f},

      {&state.life, "more life", on_enhancement_button_click,
       !state.buttons[1].caption[0] ? 0.0 : (1000.0f / state.life.level)},
      {&state.joke, "more joke", on_enhancement_button_click,
       !state.buttons[2].caption[0] ? 0.0 : (1000.0f / state.joke.level)},
      {&state.cool, "more cool", on_enhancement_button_click,
       !state.buttons[3].caption[0] ? 0.0 : (1000.0f / state.cool.level)},
      {&state.mine, "more mine", on_enhancement_button_click,
       !state.buttons[4].caption[0] ? 0.0 : (1000.0f / state.mine.level)},
      {&state.crow, "more crow", on_enhancement_button_click,
       !state.buttons[5].caption[0] ? 0.0 : (1000.0f / state.crow.level)},
  };
  const int count = sizeof(all) / sizeof(struct WP);

  for (int i = 6; i < 9; ++i) {
    all[0].w = all[0].used ? 0.0f : all[0].cw;
    for (int i = 1; i < count; ++i)
      all[i].w = all[i - 1].w + (all[i].used ? 0.0f : all[i].cw);

    if (all[count - 1].w == 0.0)
      break;

    const float select = ((rand() % 10000) / 10000.0) * all[count - 1].w;

    // for (int i = 0; i < count; ++i)
    //   printf("%s(%f,%d) ", all[i].text, all[i].w, all[i].used);
    // printf("\nselected: %f\n", select);

    struct WP *wp = &all[0];
    for (int i = 1; i < count; ++i) {
      if (all[i - 1].w < select && select <= all[i].w) {
        wp = &all[i];
        break;
      }
    }
    // printf("-> %s\n", wp->text);

    wp->used = true;
    Button *b = &state.buttons[i];
    *b = (Button){i - 3, 22, false, wp->click, wp->context, ""};
    snprintf(b->caption, sizeof(b->caption), "%s", wp->text);
  }
  // printf("\n");
}

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

void update_state(double dt) {
  state.time += dt;

  if (state.trigger_time <= 0.0) {
    state.trigger_time = 1.0;
    state.beat_count += tick_update_count();
  } else
    state.trigger_time -= dt;

  update_beat_count();

  state.buttons[1].enabled = state.buttons[1].caption[0] && Producer_cost(&state.life) <= state.beat_count;
  state.buttons[2].enabled = state.buttons[2].caption[0] && Producer_cost(&state.joke) <= state.beat_count;
  state.buttons[3].enabled = state.buttons[3].caption[0] && Producer_cost(&state.cool) <= state.beat_count;
  state.buttons[4].enabled = state.buttons[4].caption[0] && Producer_cost(&state.mine) <= state.beat_count;
  state.buttons[5].enabled = state.buttons[5].caption[0] && Producer_cost(&state.crow) <= state.beat_count;

  update_enhancement(&state.enhancement);
}

void update_producer_button_text(Producer *p, Button *b) {
  snprintf(b->caption, sizeof(b->caption), "%s %03d(B%d)", p->name, p->count, Producer_cost(p));
}

void on_producer_click(Producer *p, Button *b) {
  int cost = Producer_cost(p);
  if (cost > state.beat_count)
    return;

  state.beat_count -= cost;
  p->count++;

  update_producer_button_text(p, b);
}

void on_beat_click(Button *b) { state.beat_count++; }
void on_producer_button_click(Button *b) { on_producer_click((Producer *)b->click_context, b); }
void on_enhancement_button_click(Button *b) {
  int cost = (int)(state.enhancement.base_cost * pow(state.enhancement.cost_growth, state.enhancement.number_applied));
  if (cost > state.beat_count)
    return;

  Producer *p = (Producer *)b->click_context;
  p->level++;

  state.beat_count -= cost;
  state.enhancement.number_applied++;
  state.enhancement.selection_active = false;
  state.buttons[6] = (Button){};
  state.buttons[7] = (Button){};
  state.buttons[8] = (Button){};
}

void on_new_producer_click(Button *b) {
  int cost = (int)(state.enhancement.base_cost * pow(state.enhancement.cost_growth, state.enhancement.number_applied));
  if (cost > state.beat_count)
    return;

  if (b->click_context == &state.life) {
    state.buttons[1] = (Button){1, 1, false, on_producer_button_click, &state.life, ""};
    update_producer_button_text((Producer *)state.buttons[1].click_context, &state.buttons[1]);
  } else if (b->click_context == &state.joke) {
    state.buttons[2] = (Button){2, 1, false, on_producer_button_click, &state.joke, ""};
    update_producer_button_text((Producer *)state.buttons[2].click_context, &state.buttons[2]);
  } else if (b->click_context == &state.cool) {
    state.buttons[3] = (Button){3, 1, false, on_producer_button_click, &state.cool, ""};
    update_producer_button_text((Producer *)state.buttons[3].click_context, &state.buttons[3]);
  } else if (b->click_context == &state.mine) {
    state.buttons[4] = (Button){4, 1, false, on_producer_button_click, &state.mine, ""};
    update_producer_button_text((Producer *)state.buttons[4].click_context, &state.buttons[4]);
  } else if (b->click_context == &state.crow) {
    state.buttons[5] = (Button){5, 1, false, on_producer_button_click, &state.crow, ""};
    update_producer_button_text((Producer *)state.buttons[5].click_context, &state.buttons[5]);
  }

  state.beat_count -= cost;
  state.enhancement.number_applied++;
  state.enhancement.selection_active = false;
  state.buttons[6] = (Button){};
  state.buttons[7] = (Button){};
  state.buttons[8] = (Button){};
}

static void init(void) {
  for (int i = 1; i < 2; ++i)
    update_producer_button_text((Producer *)state.buttons[i].click_context, &state.buttons[i]);

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
  return state.mouse_line == b->l && state.mouse_column >= b->c && state.mouse_column < b->c + strlen(b->caption);
}

void jump_to(float l, float c) {
  sdtx_home();
  sdtx_origin(c, l);
}

static void frame(void) {

  update_state(sapp_frame_duration());

  sdtx_canvas(sapp_width() * 0.5f, sapp_height() * 0.5f);
  sdtx_font(FONT_KC853);

  for (int i = 0; i < BUTTON_COUNT; ++i) {
    if (!state.buttons[i].caption[0])
      continue;

    jump_to(state.buttons[i].l, state.buttons[i].c);
    if (!state.buttons[i].enabled)
      sdtx_color3b(0x42, 0x53, 0x47);
    else if (Button_hovered(&state.buttons[i]))
      sdtx_color3b(0xf4, 0x43, 0x36);
    else
      sdtx_color3b(0xa2, 0xb3, 0xa7);
    sdtx_puts(state.buttons[i].caption);
  }

  if (state.enhancement.selection_active) {
    jump_to(1, 20);
    sdtx_color3b(0xa2, 0xb3, 0xa7);
    int cost =
        (int)(state.enhancement.base_cost * pow(state.enhancement.cost_growth, state.enhancement.number_applied));
    sdtx_printf("enhancement (%dB)\n", cost);
    jump_to(2, 20);
    sdtx_color3b(0x42, 0x53, 0x47);
    sdtx_printf("-----------------------)", cost);
  } else {
    Enhancement *e = &state.enhancement;
    size_t tick_update = tick_update_count();
    size_t min_tick_update = (int)(e->base_tick_expect * pow(e->tick_expect_growth, e->number_applied));
    jump_to(1, 20);
    sdtx_color3b(0x42, 0x53, 0x47);
    sdtx_printf("next level (%d)\n", (int)(min_tick_update - tick_update));
  }

  if (state.life.count > 0) {
    jump_to(28, 18);
    sdtx_color3b(0x33, 0x33, 0x33);
    sdtx_puts(".  .");

    const float t = (1.0 - state.trigger_time);
    sdtx_color3f(t * t * 0.8, 0.125f + t * t * 0.5, 0.25f + t * t * 0.3f);
    jump_to(28, 18.0f + t * 1.5f);
    sdtx_puts(":");

    jump_to(28, 21.0f - t * 1.5f);
    sdtx_puts(":");
  }

  sdtx_color3b(0x33, 0x33, 0x33);
  jump_to(0, 0);
  sdtx_printf("-------------------------------------------------\n");
  for (int i = 0; i < 7; ++i)
    sdtx_printf("(                 :                             )\n");
  sdtx_printf("-------------------------------------------------\n");
  for (int i = 0; i < 17; ++i)
    sdtx_printf("(                 :                             )\n");
  sdtx_printf("-------------------------------------------------\n");
  sdtx_printf("(                                               )\n");
  sdtx_printf("(                                               )\n");
  sdtx_printf("-------------------------------------------------\n");

  if (state.life.count > 0) {
    sdtx_canvas(sapp_width(), sapp_height());
    sdtx_home();
    sdtx_origin(1.0f, 60.0f);
    sdtx_color3b(0x63, 0x63, 0x63);
    sdtx_printf("%lld per tick", tick_update_count());
  }

  sg_begin_pass(&(sg_pass){
      .action = {.colors[0] = {.load_action = SG_LOADACTION_CLEAR, .clear_value = {0.0f, 0.125f, 0.25f, 1.0f}}},
      .swapchain = sglue_swapchain(),
  });
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
    for (int i = 0; i < BUTTON_COUNT; ++i) {
      if (state.buttons[i].caption[0] && Button_hovered(&state.buttons[i]) && state.buttons[i].on_click)
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
      .width = 800,
      .height = 600,
      .window_title = "beat",
      .icon.sokol_default = true,
      .logger.func = slog_func,
  });

  return 0;
}