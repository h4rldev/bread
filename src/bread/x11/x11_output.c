#include <bread/x11/x11.h>

#if BREAD_X11

#include <stdlib.h>

#include <xcb/randr.h>
#include <xcb/xcb.h>

#include <bread/window.h>

void bread_x11_query_refresh_rate(x11_state_t *state) {
  xcb_connection_t *connection = state->connection;

  xcb_randr_query_version_cookie_t vcookie =
      xcb_randr_query_version(connection, 1, 1);
  xcb_randr_query_version_reply_t *vreply =
      xcb_randr_query_version_reply(connection, vcookie, NULL);

  if (!vreply) {
    state->refresh_mhz = 60000;
    return;
  }
  free(vreply);

  xcb_randr_get_screen_resources_cookie_t rcookie =
      xcb_randr_get_screen_resources(connection, state->xcb_window);
  xcb_randr_get_screen_resources_reply_t *rreply =
      xcb_randr_get_screen_resources_reply(connection, rcookie, NULL);
  if (!rreply) {
    state->refresh_mhz = 60000;
    return;
  }

  xcb_randr_get_screen_info_cookie_t scookie =
      xcb_randr_get_screen_info(connection, state->screen->root);
  xcb_randr_get_screen_info_reply_t *sreply =
      xcb_randr_get_screen_info_reply(connection, scookie, NULL);

  int modes_len = xcb_randr_get_screen_resources_modes_length(rreply);
  xcb_randr_mode_info_t *modes = xcb_randr_get_screen_resources_modes(rreply);

  u32 current_mode_id = 0;

  int crtcs_len = xcb_randr_get_screen_resources_crtcs_length(rreply);
  xcb_randr_crtc_t *crtcs = xcb_randr_get_screen_resources_crtcs(rreply);

  for (int i = 0; i < crtcs_len; i++) {
    xcb_randr_get_crtc_info_cookie_t ccookie =
        xcb_randr_get_crtc_info(connection, crtcs[i], XCB_CURRENT_TIME);
    xcb_randr_get_crtc_info_reply_t *creply =
        xcb_randr_get_crtc_info_reply(connection, ccookie, NULL);

    if (creply && creply->mode != XCB_NONE && creply->x <= 0 &&
        creply->y <= 0) {
      current_mode_id = creply->mode;
      free(creply);
      break;
    }

    free(creply);
  }

  if (current_mode_id == 0)
    goto fallback;

  for (int i = 0; i < modes_len; i++) {
    if (modes[i].id == current_mode_id) {
      if (modes[i].htotal > 0 && modes[i].vtotal > 0 &&
          modes[i].dot_clock > 0) {
        state->refresh_mhz =
            (u32)((u64)modes[i].dot_clock * 1000ULL /
                  ((u64)modes[i].htotal * (u64)modes[i].vtotal));
      } else
        goto fallback;
    }
  }

  free(rreply);
  free(sreply);
  return;

fallback:
  state->refresh_mhz = 60000;
  free(rreply);
  free(sreply);
}

#endif // !BREAD_X11
