#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screens.h"
#include "buttons.h"
#include "screen_config.h"
#include "heartrate.h"
#include "screen_data.h"
#ifdef ENABLE_LOCALIZE
  #include "localize.h"
#endif
#ifdef PBL_HEALTH
  #include "health.h"
#endif


FieldConfig *cur_fieldconfig;
FieldLayer *cur_fieldlayer;
bool config_hidden = false;
static AppTimer *config_timer;
uint8_t config_screen = CONFIG_SCREEN_DISABLED;
uint8_t config_field = CONFIG_FIELD_SCREEN__MIN;

#define CONFIG_NB_FIELD_ORDER 30
uint8_t config_order[CONFIG_NB_FIELD_ORDER] = {
    FIELD_ACCURACY,
    FIELD_ALTITUDE,
    FIELD_ALTITUDE_GRAPH_ONLY,
#ifdef PBL_COLOR
    FIELD_ALTITUDE_DATA_AND_GRAPH,
#endif
    FIELD_ASCENT,
    FIELD_ASCENTRATE,
    FIELD_ASCENTRATE_GRAPH_ONLY,
#ifdef PBL_COLOR
    FIELD_ASCENTRATE_DATA_AND_GRAPH,
#endif
    FIELD_AVGSPEED,
    FIELD_BEARING,
    FIELD_CADENCE,
    FIELD_DISTANCE,
    FIELD_DURATION,
    FIELD_HEARTRATE,
    FIELD_HEARTRATE_GRAPH_ONLY,
#ifdef PBL_COLOR
    FIELD_HEARTRATE_DATA_AND_GRAPH,
#endif
    //FIELD_LAT,
    //FIELD_LON,
    FIELD_MAXSPEED,
    //FIELD_NBASCENT,
    FIELD_NAV_ESTIMATED_TIME_ARRIVAL,
    FIELD_NAV_DISTANCE_NEXT,
    FIELD_NAV_DISTANCE_TO_DESTINATION,
    FIELD_NAV_TIME_TO_DESTINATION,
    FIELD_SLOPE,
    FIELD_SPEED,
    FIELD_SPEED_GRAPH_ONLY,
#ifdef PBL_COLOR
    FIELD_SPEED_DATA_AND_GRAPH,
#endif
#ifdef PBL_HEALTH
    FIELD_STEPS,
    FIELD_STEPS_CADENCE,
#endif
    FIELD_TEMPERATURE,
    FIELD_TIME,
    FIELD__UNUSED,
};

ConfigData config;

const char *field_get_title(uint8_t field) {
  switch(field) {
    case FIELD_AVGSPEED: return _("Avg speed"); break;
    case FIELD_DISTANCE: return _("Distance"); break;
    case FIELD_ALTITUDE: return _("Altitude"); break;
#ifdef PBL_COLOR
    case FIELD_ALTITUDE_DATA_AND_GRAPH: return _("Altitude all"); break;
#endif
    case FIELD_ALTITUDE_GRAPH_ONLY: return _("Altitude graph"); break;
    case FIELD_ASCENT: return _("Ascent"); break;
    case FIELD_SPEED: return _("Speed"); break;
#ifdef PBL_COLOR
    case FIELD_SPEED_DATA_AND_GRAPH: return _("Speed all"); break;
#endif
    case FIELD_SPEED_GRAPH_ONLY: return _("Speed graph"); break;
    case FIELD_BEARING: return _("Bearing"); break;
    case FIELD_DURATION: return _("Duration"); break;
    case FIELD_MAXSPEED: return _("Max speed"); break;
    case FIELD_NAV_ESTIMATED_TIME_ARRIVAL:  return _("Nav:Time Arrival"); break;
    case FIELD_NAV_DISTANCE_NEXT:           return _("Nav:Dist next point"); break;
    case FIELD_NAV_DISTANCE_TO_DESTINATION: return _("Nav:Dist to dest"); break;
    case FIELD_NAV_TIME_TO_DESTINATION:     return _("Nav:Time to dest"); break;
    //case FIELD_LAT: return "Lat"; break;
    //case FIELD_LON: return "Lon"; break;
    case FIELD_ASCENTRATE: return _("Ascent rate"); break;
#ifdef PBL_COLOR
    case FIELD_ASCENTRATE_DATA_AND_GRAPH: return _("Ascent rate all"); break;
#endif
    case FIELD_ASCENTRATE_GRAPH_ONLY: return _("Ascent rate graph"); break;
    //case FIELD_NBASCENT: return "Nb ascent"; break;
    case FIELD_SLOPE: return _("Slope"); break;
    case FIELD_ACCURACY: return _("Accuracy"); break;
    case FIELD_HEARTRATE: return _("Heartrate"); break;
#ifdef PBL_COLOR
    case FIELD_HEARTRATE_DATA_AND_GRAPH: return _("Heartrate all"); break;
#endif
    case FIELD_HEARTRATE_GRAPH_ONLY: return _("Heartrate graph"); break;
    case FIELD_CADENCE: return _("Cadence"); break;
#ifdef PRODUCTION
    case FIELD_TEMPERATURE: return _("Temperature"); break;
#else
    ///@todo(nav)
    case FIELD_TEMPERATURE: return "Nav: index"; break;
#endif
    case FIELD_TIME: return _("Time"); break;
#ifdef PBL_HEALTH
    case FIELD_STEPS: return _("Steps"); break;
    case FIELD_STEPS_CADENCE: return _("Steps cadence"); break;
#endif
    default: return _("Unknown");
  }
}
const char *field_get_text(uint8_t field) {
  switch(field) {
    case FIELD_AVGSPEED: return s_data.avgspeed; break;
    case FIELD_DISTANCE: return s_data.distance; break;
    case FIELD_ALTITUDE:
#ifdef PBL_COLOR
    case FIELD_ALTITUDE_DATA_AND_GRAPH:
#endif
      return s_data.altitude; break;
    case FIELD_ASCENT: return s_data.ascent; break;
    case FIELD_SPEED:
#ifdef PBL_COLOR
    case FIELD_SPEED_DATA_AND_GRAPH:
#endif
      return s_data.speed; break;
    case FIELD_BEARING: return s_data.bearing; break;
    case FIELD_DURATION: return s_data.elapsedtime; break;
    case FIELD_MAXSPEED: return s_data.maxspeed; break;
    //case FIELD_LAT: return s_data.lat; break;
    //case FIELD_LON: return s_data.lon; break;
    case FIELD_NAV_ESTIMATED_TIME_ARRIVAL:  return s_data.nav_eta; break;
    case FIELD_NAV_DISTANCE_NEXT:           return s_data.nav_next_distance; break;
    case FIELD_NAV_DISTANCE_TO_DESTINATION: return s_data.nav_distance_to_destination; break;
    case FIELD_NAV_TIME_TO_DESTINATION:     return s_data.nav_ttd; break;
    case FIELD_ASCENTRATE:
#ifdef PBL_COLOR
    case FIELD_ASCENTRATE_DATA_AND_GRAPH:
#endif
      return s_data.ascentrate; break;
    //case FIELD_NBASCENT: return s_data.nbascent; break;
    case FIELD_SLOPE: return s_data.slope; break;
    case FIELD_ACCURACY: return s_data.accuracy; break;
    case FIELD_HEARTRATE:
#ifdef PBL_COLOR
    case FIELD_HEARTRATE_DATA_AND_GRAPH:
#endif
      return s_data.heartrate; break;
    case FIELD_CADENCE: return s_data.cadence; break;
    case FIELD_TEMPERATURE: return s_data.temperature; break;
    case FIELD_TIME: return s_data.time; break;
#ifdef PBL_HEALTH
    case FIELD_STEPS: return s_data.steps; break;
    case FIELD_STEPS_CADENCE: return s_data.steps_cadence; break;
#endif
    case FIELD_ALTITUDE_GRAPH_ONLY:
    case FIELD_ASCENTRATE_GRAPH_ONLY:
    case FIELD_HEARTRATE_GRAPH_ONLY:
    case FIELD_SPEED_GRAPH_ONLY:
      return ""; break;
    default: return "-";
  }
}

const char *field_get_units(uint8_t field) {
  switch(field) {
    case FIELD_AVGSPEED: return s_data.unitsSpeed; break;
    case FIELD_DISTANCE: return s_data.unitsDistance; break;
    case FIELD_ALTITUDE:
#ifdef PBL_COLOR
    case FIELD_ALTITUDE_DATA_AND_GRAPH:
#endif
      return s_data.unitsAltitude; break;
    case FIELD_ASCENT: return s_data.unitsAltitude; break;
    case FIELD_SPEED:
#ifdef PBL_COLOR
    case FIELD_SPEED_DATA_AND_GRAPH:
#endif
      return s_data.unitsSpeed; break;
    case FIELD_BEARING: return "°"; break;
    case FIELD_DURATION: return "s"; break;
    case FIELD_MAXSPEED: return s_data.unitsSpeed; break;
    //case FIELD_LAT: return ""; break;
    //case FIELD_LON: return ""; break;
    case FIELD_NAV_ESTIMATED_TIME_ARRIVAL: return ""; break;
    ///@todo(nav) imperial units
    case FIELD_NAV_DISTANCE_NEXT: return "m"; break;
    case FIELD_NAV_DISTANCE_TO_DESTINATION: return "km"; break;
    case FIELD_NAV_TIME_TO_DESTINATION: return ""; break;
    case FIELD_ASCENTRATE:
#ifdef PBL_COLOR
      case FIELD_ASCENTRATE_DATA_AND_GRAPH:
#endif
        return s_data.unitsAscentRate; break;
    //case FIELD_NBASCENT: return ""; break;
    case FIELD_SLOPE: return "%"; break;
    case FIELD_ACCURACY: return "m"; break;
    case FIELD_HEARTRATE:
#ifdef PBL_COLOR
    case FIELD_HEARTRATE_DATA_AND_GRAPH:
#endif
      if (s_gpsdata.heartrate > 0 && s_gpsdata.heartrate != 255) {
        return heartrate_zone;
      } else {
        return HEART_RATE_UNIT;
      }
      break;
    case FIELD_CADENCE: return "rpm"; break;
    case FIELD_TEMPERATURE: return s_data.unitsTemperature; break;
    case FIELD_TIME: return ""; break;
#ifdef PBL_HEALTH
    case FIELD_STEPS: return _("steps"); break;
    case FIELD_STEPS_CADENCE: return "spm"; break;
#endif
    default: return "";
  }
}
void config_field_set_text(FieldLayer field_layer, uint8_t type, GTextAlignment force_alignement) {
//  if (field_layer.title_layer != NULL) {
//    text_layer_set_text(field_layer.title_layer, field_get_title(type));
//  }

  if (field_layer.data_layer != NULL) {
    if (field_layer.data_layer == s_data.topbar_layer.field_center_layer.data_layer && title_instead_of_units) {
      // special case for topbar (no unit_layer), the few seconds after button press (title_instead_of_units==true)
      text_layer_set_text(field_layer.data_layer, field_get_title(type));
    } else {
      text_layer_set_text(field_layer.data_layer, field_get_text(type));
    }
  }
  if (field_layer.unit_layer != NULL) {
    text_layer_set_text(field_layer.unit_layer, title_instead_of_units ? field_get_title(type) : field_get_units(type));
#ifdef PBL_COLOR
    if (field_layer.data_layer == s_data.screenData_layer.field_top.data_layer) {
      if (type == FIELD_HEARTRATE && s_gpsdata.heartrate > 0 && s_gpsdata.heartrate != 255) {
        bg_color_data_main = heartrate_color;
      } else {
        bg_color_data_main = BG_COLOR_DATA_MAIN;
      }
    }
#endif

#ifndef PBL_ROUND
    text_layer_set_text_alignment(field_layer.unit_layer, title_instead_of_units ? GTextAlignmentCenter : force_alignement);
#endif
  }
}
void screen_data_update_config(bool change_page) {
  if (s_data.data_subpage == SUBPAGE_UNDEF) {
    config_field_set_text(s_data.topbar_layer.field_center_layer, FIELD_TIME, GTextAlignmentCenter);
    return;
  }
  if (change_page) {
    LOG_DEBUG("screen_data_update_config(true)");
    config_affect_type(&s_data.screen_config[s_data.data_subpage].field_top, s_data.data_subpage == SUBPAGE_B ? config.screenB_top_type : config.screenA_top_type);
    config_affect_type(&s_data.screen_config[s_data.data_subpage].field_top2, s_data.data_subpage == SUBPAGE_B ? config.screenB_top2_type : config.screenA_top2_type);
    config_affect_type(&s_data.screen_config[s_data.data_subpage].field_bottom_left, s_data.data_subpage == SUBPAGE_B ? config.screenB_bottom_left_type : config.screenA_bottom_left_type);
    config_affect_type(&s_data.screen_config[s_data.data_subpage].field_bottom_right, s_data.data_subpage == SUBPAGE_B ? config.screenB_bottom_right_type : config.screenA_bottom_right_type);
    config_affect_type(&s_data.screen_config[s_data.data_subpage].field_topbar_center, s_data.data_subpage == SUBPAGE_B ? config.screenB_topbar_center_type : config.screenA_topbar_center_type);
  }
  config_field_set_text(s_data.screenData_layer.field_top, s_data.page_number == PAGE_HEARTRATE ? FIELD_HEARTRATE : s_data.screen_config[s_data.data_subpage].field_top.type, GTextAlignmentRight);
  config_field_set_text(s_data.screenData_layer.field_top2, s_data.screen_config[s_data.data_subpage].field_top2.type, GTextAlignmentRight);
  config_field_set_text(s_data.screenData_layer.field_bottom_left, s_data.screen_config[s_data.data_subpage].field_bottom_left.type, GTextAlignmentCenter);
  config_field_set_text(s_data.screenData_layer.field_bottom_right, s_data.screen_config[s_data.data_subpage].field_bottom_right.type, GTextAlignmentCenter);
  if (config_screen == CONFIG_SCREEN_DISABLED) {
    config_field_set_text(s_data.topbar_layer.field_center_layer, s_data.screen_config[s_data.data_subpage].field_topbar_center.type, GTextAlignmentCenter);
  }
}

void config_change_visibility(FieldLayer* field_layer, bool hidden) {
//  if (field_layer->title_layer != NULL) {
//    layer_set_hidden(text_layer_get_layer(field_layer->title_layer), hidden);
//  }
  if (field_layer->data_layer != NULL) {
    layer_set_hidden(text_layer_get_layer(field_layer->data_layer), hidden);
  }
  if (field_layer->unit_layer != NULL) {
    layer_set_hidden(text_layer_get_layer(field_layer->unit_layer), hidden);
  }
}
static void config_timer_callback(void *data) {
  config_hidden = !config_hidden;
  config_timer = app_timer_register(config_hidden ? 200 : 1000, config_timer_callback, NULL);
  config_change_visibility(cur_fieldlayer, config_hidden);
}
void config_init() {
#ifdef ENABLE_LOCALIZE_FORCE
  //hard-coded for testing
  const char* locale_str = ENABLE_LOCALIZE_FORCE;
#else
  // Detect system locale
  const char* locale_str = i18n_get_system_locale();
#endif
  if (strncmp(locale_str, "fr", 2) == 0 || strncmp(locale_str, "es", 2) == 0 || strncmp(locale_str, "de", 2) == 0) {
    // sort config_order by title
    // the algo is not optimised (O(n²)) but it should be a problem because CONFIG_NB_FIELD_ORDER is low
    uint8_t tmp;
    for (int i = 0; i < CONFIG_NB_FIELD_ORDER - 1; i++) {
      if (config_order[i] == FIELD__UNUSED) {
        break;
      }
      for (int j = i + 1; j < CONFIG_NB_FIELD_ORDER - 1; j++) {
        if (config_order[j] == FIELD__UNUSED) {
          break;
        }
        if (strcmp(field_get_title(config_order[j]), field_get_title(config_order[i])) < 0) {
          tmp = config_order[i];
          config_order[i] = config_order[j];
          config_order[j] = tmp;
        }
      }
    }
    #ifdef ENABLE_LOCALIZE_FORCE
    for(int i=0; i< CONFIG_NB_FIELD_ORDER; i++) {
      LOG_INFO("%d:%s", i, field_get_title(config_order[i]));
    }
    #endif
  }
  config_affect_type(&s_data.screen_config[SUBPAGE_A].field_top, config.screenA_top_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_A].field_top2, config.screenA_top2_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_A].field_bottom_left, config.screenA_bottom_left_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_A].field_bottom_right, config.screenA_bottom_right_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_A].field_topbar_center, config.screenA_topbar_center_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_B].field_top, config.screenB_top_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_B].field_top2, config.screenB_top2_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_B].field_bottom_left, config.screenB_bottom_left_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_B].field_bottom_right, config.screenB_bottom_right_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_B].field_topbar_center, config.screenB_topbar_center_type);
}
void config_start() {
  if (s_data.data_subpage == SUBPAGE_UNDEF) {
    return;
  }

#ifdef PBL_HEALTH
  health_init();
#endif

  config_field = CONFIG_FIELD_SCREEN_TOP;
  config_screen = s_data.data_subpage == SUBPAGE_B ? CONFIG_SCREEN_B : CONFIG_SCREEN_A;
  cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_top;
  screen_data_update_config(false);
  cur_fieldlayer = &s_data.screenData_layer.field_top;
  layer_set_hidden(text_layer_get_layer(s_data.topbar_layer.accuracy_layer), true);
  layer_set_hidden(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer), true);
  text_layer_set_text(s_data.topbar_layer.field_center_layer.data_layer, field_get_title(cur_fieldconfig->type));
  layer_mark_dirty(s_data.topbar_layer.layer);

  vibes_short_pulse();
  if (config_timer != NULL) {
    app_timer_cancel(config_timer);
  }
  config_timer = app_timer_register(1000, config_timer_callback, NULL);
  config_hidden = false;
  action_bar_set_menu_up_down_buttons();
}
void config_stop() {
  config_screen = CONFIG_SCREEN_DISABLED;
  config_change_visibility(cur_fieldlayer, false);
  app_timer_cancel(config_timer);
  config_timer = NULL;
  config_field_set_text(s_data.topbar_layer.field_center_layer, s_data.screen_config[s_data.data_subpage].field_topbar_center.type, GTextAlignmentCenter);
  layer_set_hidden(text_layer_get_layer(s_data.topbar_layer.accuracy_layer), s_data.state == STATE_STOP);
  layer_set_hidden(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer), false);
  layer_mark_dirty(s_data.topbar_layer.layer);
  config_save();
#ifdef PBL_HEALTH
  health_init_if_needed();
#endif
}
void config_change_field() {
  config_change_visibility(cur_fieldlayer, false);
  config_field++;
  if (config_screen == CONFIG_SCREEN_A && config_field == CONFIG_FIELD_SCREEN__MAX) {
    config_field = CONFIG_FIELD_SCREEN__MIN;
  } else if (config_screen == CONFIG_SCREEN_B && config_field == CONFIG_FIELD_SCREEN__MAX) {
    config_field = CONFIG_FIELD_SCREEN__MIN;
  }
  switch (config_field) {
    case CONFIG_FIELD_SCREEN_TOP:           cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_top;          cur_fieldlayer = &s_data.screenData_layer.field_top; break;
    case CONFIG_FIELD_SCREEN_TOP2:          cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_top2;         cur_fieldlayer = &s_data.screenData_layer.field_top2; break;
    case CONFIG_FIELD_SCREEN_TOP_BAR:       cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_topbar_center;         cur_fieldlayer = &s_data.topbar_layer.field_center_layer; break;
    case CONFIG_FIELD_SCREEN_BOTTOM_LEFT:   cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_bottom_left;  cur_fieldlayer = &s_data.screenData_layer.field_bottom_left; break;
    case CONFIG_FIELD_SCREEN_BOTTOM_RIGHT:  cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_bottom_right; cur_fieldlayer = &s_data.screenData_layer.field_bottom_right; break;
  }

  config_change_visibility(cur_fieldlayer, true);

  text_layer_set_text(s_data.topbar_layer.field_center_layer.data_layer, field_get_title(cur_fieldconfig->type));
  layer_mark_dirty(s_data.topbar_layer.layer);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "config_field %d", config_field);
}
bool config_field_contain_graph(uint8_t type_index) {
  bool result = false;
  result = result || config_order[type_index] == FIELD_ALTITUDE_GRAPH_ONLY;
  result = result || config_order[type_index] == FIELD_ASCENTRATE_GRAPH_ONLY;
  result = result || config_order[type_index] == FIELD_HEARTRATE_GRAPH_ONLY;
  result = result || config_order[type_index] == FIELD_SPEED_GRAPH_ONLY;
#ifdef PBL_COLOR
  result = result || config_order[type_index] == FIELD_ALTITUDE_DATA_AND_GRAPH;
  result = result || config_order[type_index] == FIELD_ASCENTRATE_DATA_AND_GRAPH;
  result = result || config_order[type_index] == FIELD_HEARTRATE_DATA_AND_GRAPH;
  result = result || config_order[type_index] == FIELD_SPEED_DATA_AND_GRAPH;
#endif
  return result;
}
void config_next_type() {
  cur_fieldconfig->type_index++;
  if ((cur_fieldconfig->type_index >= CONFIG_NB_FIELD_ORDER) || (config_order[cur_fieldconfig->type_index] == FIELD__UNUSED)) {
    cur_fieldconfig->type_index = 0;
  }
}
void config_next_type_graph(bool can_display_graph) {
  config_next_type();
  if (!can_display_graph && config_field_contain_graph(cur_fieldconfig->type_index)) {
    config_next_type();
  }
  // twice because _DATA_AND_GRAPH and _ALTITUDE_GRAPH_ONLY,
  if (!can_display_graph && config_field_contain_graph(cur_fieldconfig->type_index)) {
    config_next_type();
  }
}

void config_previous_type() {
  if (cur_fieldconfig->type_index == 0) {
    // search for last index not FIELD__UNUSED
    for (int i = 0; i < CONFIG_NB_FIELD_ORDER; i++) {
      if (config_order[i] == FIELD__UNUSED) {
        cur_fieldconfig->type_index = i -1 ;
        return;
      }
    }
  } else {
    cur_fieldconfig->type_index--;
  }
}
void config_previous_type_graph(bool can_display_graph) {
  config_previous_type();
  if (!can_display_graph && config_field_contain_graph(cur_fieldconfig->type_index)) {
    config_previous_type();
  }
  // twice because _DATA_AND_GRAPH and _ALTITUDE_GRAPH_ONLY,
  if (!can_display_graph && config_field_contain_graph(cur_fieldconfig->type_index)) {
    config_previous_type();
  }
}


void config_change_type(uint8_t direction) {
  if (direction == CONFIG_CHANGE_TYPE_NEXT) {
    config_next_type_graph(config_field == CONFIG_FIELD_SCREEN_TOP2);
    cur_fieldconfig->type = config_order[cur_fieldconfig->type_index];
  } else {
    // CONFIG_CHANGE_TYPE_PREVIOUS
    config_previous_type_graph(config_field == CONFIG_FIELD_SCREEN_TOP2);
    cur_fieldconfig->type = config_order[cur_fieldconfig->type_index];
  }
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "%d: type %d", cur_fieldconfig->type_index, cur_fieldconfig->type);

  //todo
  screen_data_update_config(false);
  text_layer_set_text(s_data.topbar_layer.field_center_layer.data_layer, field_get_title(cur_fieldconfig->type));
  layer_mark_dirty(s_data.topbar_layer.layer);
  //LOG_DEBUG("config_change_type newtype=%d [index:%d]", cur_fieldconfig->type, cur_fieldconfig->type_index);
}

void config_load() {
  if (persist_exists(PERSIST_CONFIG_KEY)) {
    persist_read_data(PERSIST_CONFIG_KEY, &config, sizeof(config));
    int version = 0;
    if (persist_exists(PERSIST_VERSION)) {
      version = persist_read_int(PERSIST_VERSION);
    }
    if (version < MIN_VERSION_PEBBLE_SCREEN_A_TOP2) {
      config.screenA_top2_type          = FIELD_ALTITUDE;
    }
    if (version < MIN_VERSION_PEBBLE_SCREEN_A_TOP_BAR) {
      config.screenA_topbar_center_type = FIELD_TIME;
      config.screenB_topbar_center_type = FIELD_TIME;
    }
  } else {
#ifdef ENABLE_DEMO
//    config.screenA_top_type           = FIELD_MAXSPEED;
    config.screenA_top_type           = FIELD_HEARTRATE;
    config.screenA_top2_type          = FIELD_HEARTRATE_GRAPH_ONLY;
#else
    config.screenA_top_type           = FIELD_SPEED;
  #ifdef PBL_COLOR
      config.screenA_top2_type          = FIELD_ALTITUDE_DATA_AND_GRAPH;
  #else
      config.screenA_top2_type          = FIELD_ALTITUDE;
  #endif
#endif
    config.screenA_bottom_left_type   = FIELD_DISTANCE;
    config.screenA_bottom_right_type  = FIELD_AVGSPEED;
    config.screenA_topbar_center_type = FIELD_TIME;
    ///@todo(nav) restore screenB initial fields
    config.screenB_top_type           = FIELD_NAV_DISTANCE_NEXT; // FIELD_ALTITUDE;
    config.screenB_top2_type          = FIELD_NAV_ESTIMATED_TIME_ARRIVAL; //FIELD_ASCENT;
    config.screenB_bottom_left_type   = FIELD_NAV_DISTANCE_TO_DESTINATION; // FIELD_ASCENTRATE;
    config.screenB_bottom_right_type  = FIELD_NAV_TIME_TO_DESTINATION; //FIELD_SLOPE;
#ifdef PRODUCTION
    config.screenB_topbar_center_type = FIELD_TIME;
#else
    config.screenB_topbar_center_type = FIELD_NAV_NEXT_INDEX;
#endif
  }
#ifdef PBL_HEALTH
  health_init_if_needed();
#endif
}
void config_save() {
  config.screenA_top_type           = s_data.screen_config[SUBPAGE_A].field_top.type;
  config.screenA_top2_type          = s_data.screen_config[SUBPAGE_A].field_top2.type;
  config.screenA_bottom_left_type   = s_data.screen_config[SUBPAGE_A].field_bottom_left.type;
  config.screenA_bottom_right_type  = s_data.screen_config[SUBPAGE_A].field_bottom_right.type;
  config.screenA_topbar_center_type = s_data.screen_config[SUBPAGE_A].field_topbar_center.type;
  config.screenB_top_type           = s_data.screen_config[SUBPAGE_B].field_top.type;
  config.screenB_top2_type          = s_data.screen_config[SUBPAGE_B].field_top2.type;
  config.screenB_bottom_left_type   = s_data.screen_config[SUBPAGE_B].field_bottom_left.type;
  config.screenB_bottom_right_type  = s_data.screen_config[SUBPAGE_B].field_bottom_right.type;
  config.screenB_topbar_center_type = s_data.screen_config[SUBPAGE_B].field_topbar_center.type;
  persist_write_data(PERSIST_CONFIG_KEY, &config, sizeof(config));
  persist_write_int(PERSIST_VERSION, VERSION_PEBBLE);
}
void config_affect_type(FieldConfig *field, uint8_t type) {
  field->type = type;
  field->type_index = 0;
  for(int i = 0; i < CONFIG_NB_FIELD_ORDER; i++) {
    if (config_order[i] == type) {
      field->type_index = i;
      break;
    }
  }
  //LOG_DEBUG("type %d -> index: %d", field->type, field->type_index);
}
