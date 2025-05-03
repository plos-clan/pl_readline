void pl_readline_insert_char_and_view(_self, char ch) {
  if (self->length >= self->maxlen) {
    self->maxlen *= 2;
    self->buffer = realloc(self->buffer, self->maxlen);
    self->input_buf = realloc(self->input_buf, self->maxlen);
    self->buffer[self->length] = '\0';
    if (!self->buffer) return; // 炸了算了
  }
  pl_readline_insert_char(self->buffer, ch, self->ptr++);
  self->length++;
  
  // Use colorized redisplay for all edits
  redisplay_buffer_with_colors(self, 0); // Don't show prompt during edit
}