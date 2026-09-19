/*************************************************/

#include <bread/wayland/wayland_clipboard.h>

#if BREAD_WAYLAND

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <bread/log.h>

/*************************************************/

/** @brief The text mime types offered and accepted. */
static const cstr *const text_mimes[] = {
    "text/plain;charset=utf-8",
    "text/plain;charset=UTF-8",
    "UTF8_STRING",
    "text/plain",
};

//
//
//

/**
 * @brief Whether @c mime is one of the text mime types.
 *
 * @param mime The mime type to test.
 *
 * @return true for a supported text mime.
 */
static b32 mime_is_text(const cstr *mime) {
  for (u32 i = 0; i < sizeof(text_mimes) / sizeof(text_mimes[0]); i++)
    if (strcmp(mime, text_mimes[i]) == 0)
      return true;
  return false;
}

//
//
//

/**
 * @brief Whether @c mime is a UTF-8 text mime type.
 *
 * @param mime The mime type to test.
 *
 * @return true for a UTF-8 text mime.
 */
static b32 mime_is_utf8(const cstr *mime) {
  return strcmp(mime, "text/plain;charset=utf-8") == 0 ||
         strcmp(mime, "text/plain;charset=UTF-8") == 0 ||
         strcmp(mime, "UTF8_STRING") == 0;
}

//
//
//

/**
 * @brief Record the best text mime an offer supports.
 * @details Prefers a UTF-8 mime, falling back to the first text mime offered.
 *
 * @param data The @ref wl_state_t.
 * @param offer The offer the mime belongs to.
 * @param mime The offered mime type.
 */
static void data_offer_offer(void *data, wl_data_offer_t *offer,
                             const cstr *mime) {
  wl_state_t *state = data;
  if (offer != state->data_offer || !mime_is_text(mime))
    return;
  if (mime_is_utf8(mime) || !state->offer_mime) {
    free(state->offer_mime);
    state->offer_mime = strdup(mime);
  }
}

//
//
//

/** @brief The data-offer listener (records offered text mime types). */
static const wl_data_offer_listener_t data_offer_listener = {
    .offer = data_offer_offer,
};

//
//
//

/**
 * @brief Write the clipboard text into the fd a requestor passed.
 *
 * @param data The @ref wl_state_t.
 * @param source The source the request is for.
 * @param mime The requested mime type.
 * @param fd The file descriptor to write to; closed here.
 */
static void data_source_send(void *data, wl_data_source_t *source,
                             const cstr *mime, i32 fd) {
  (void)mime;
  wl_state_t *state = data;
  if (state->data_source == source && state->clipboard) {
    u32 off = 0;
    while (off < state->clipboard_len) {
      ssize_t n = write(fd, state->clipboard + off, state->clipboard_len - off);
      if (n < 0 && errno == EINTR)
        continue;
      if (n <= 0)
        break;
      off += (u32)n;
    }
  }
  close(fd);
}

//
//
//

/**
 * @brief Drop ownership after the compositor cancels the source.
 *
 * @param data The @ref wl_state_t.
 * @param source The cancelled source.
 */
static void data_source_cancelled(void *data, wl_data_source_t *source) {
  wl_state_t *state = data;
  wl_data_source_destroy(source);
  if (state->data_source == source)
    state->data_source = null;
}

//
//
//

/** @brief The data-source listener (serves and cancels our selection). */
static const wl_data_source_listener_t data_source_listener = {
    .send = data_source_send,
    .cancelled = data_source_cancelled,
};

//
//
//

/**
 * @brief Track a new incoming selection offer.
 *
 * @param data The @ref wl_state_t.
 * @param device The data device.
 * @param offer The new offer.
 */
static void data_device_data_offer(void *data, wl_data_device_t *device,
                                   wl_data_offer_t *offer) {
  (void)device;
  wl_state_t *state = data;
  if (state->data_offer && state->data_offer != offer)
    wl_data_offer_destroy(state->data_offer);
  state->data_offer = offer;
  free(state->offer_mime);
  state->offer_mime = null;
  wl_data_offer_add_listener(offer, &data_offer_listener, state);
}

//
//
//

/**
 * @brief Clear the tracked offer when the selection is dropped.
 *
 * @param data The @ref wl_state_t.
 * @param device The data device.
 * @param offer The new selection offer, or null.
 */
static void data_device_selection(void *data, wl_data_device_t *device,
                                  wl_data_offer_t *offer) {
  (void)device;
  wl_state_t *state = data;
  if (!offer && state->data_offer) {
    wl_data_offer_destroy(state->data_offer);
    state->data_offer = null;
    free(state->offer_mime);
    state->offer_mime = null;
  }
}

//
//
//

/** @brief The data-device listener (tracks the clipboard offer). */
static const wl_data_device_listener_t data_device_listener = {
    .data_offer = data_device_data_offer,
    .selection = data_device_selection,
};

//
//
//

void bread_wayland_clipboard_init(wl_state_t *state) {
  if (!state || state->data_device || !state->data_device_manager ||
      !state->seat)
    return;

  state->data_device = wl_data_device_manager_get_data_device(
      state->data_device_manager, state->seat);
  wl_data_device_add_listener(state->data_device, &data_device_listener, state);
}

void bread_wayland_clipboard_cleanup(wl_state_t *state) {
  if (!state)
    return;

  if (state->data_source) {
    wl_data_source_destroy(state->data_source);
    state->data_source = null;
  }
  if (state->data_device) {
    wl_data_device_destroy(state->data_device);
    state->data_device = null;
  }
  if (state->data_offer) {
    wl_data_offer_destroy(state->data_offer);
    state->data_offer = null;
  }

  free(state->clipboard);
  state->clipboard = null;
  state->clipboard_len = 0;
  free(state->clipboard_read);
  state->clipboard_read = null;
  free(state->offer_mime);
  state->offer_mime = null;
}

void bread_wayland_clipboard_set(wl_state_t *state, const cstr *text) {
  if (!state)
    return;

  free(state->clipboard);
  state->clipboard = null;
  state->clipboard_len = 0;

  if (state->data_source) {
    wl_data_source_destroy(state->data_source);
    state->data_source = null;
  }

  if (!text || !*text || !state->data_device_manager || !state->data_device)
    return;

  u32 len = (u32)strlen(text);
  state->clipboard = malloc(len + 1);
  if (!state->clipboard)
    return;
  memcpy(state->clipboard, text, len + 1);
  state->clipboard_len = len;

  state->data_source =
      wl_data_device_manager_create_data_source(state->data_device_manager);
  wl_data_source_add_listener(state->data_source, &data_source_listener, state);
  for (u32 i = 0; i < sizeof(text_mimes) / sizeof(text_mimes[0]); i++)
    wl_data_source_offer(state->data_source, text_mimes[i]);
  wl_data_device_set_selection(state->data_device, state->data_source,
                               state->input_serial);
  wl_display_flush(state->display);
}

const cstr *bread_wayland_clipboard_get(wl_state_t *state) {
  if (!state)
    return null;

  if (state->data_source && state->clipboard)
    return state->clipboard;

  if (!state->data_offer || !state->offer_mime)
    return null;

  int fds[2];
  if (pipe(fds) != 0)
    return null;

  wl_data_offer_receive(state->data_offer, state->offer_mime, fds[1]);
  close(fds[1]);
  wl_display_flush(state->display);

  free(state->clipboard_read);
  state->clipboard_read = null;

  size_t cap = 0;
  char *out = null;
  char buf[4096];
  for (;;) {
    ssize_t n = read(fds[0], buf, sizeof(buf));
    if (n < 0 && errno == EINTR)
      continue;
    if (n <= 0)
      break;

    char *grown = realloc(out, cap + (size_t)n + 1);
    if (!grown)
      break;
    out = grown;
    memcpy(out + cap, buf, (size_t)n);
    cap += (size_t)n;
    out[cap] = '\0';
  }
  close(fds[0]);

  state->clipboard_read = out;
  return out;
}

#endif // !BREAD_WAYLAND
