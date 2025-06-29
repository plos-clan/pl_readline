//
// This file is part of pl_readline.
// pl_readline is free software: you can redistribute it and/or modify
// it under the terms of MIT license.
// See file LICENSE or https://opensource.org/licenses/MIT for full license
// details.
//
// Copyright (c) 2024 min0911_ https://github.com/min0911Y
//

// pl_readline_color: 着色

#include "pl_readline.h"
#include <stdio.h>
#include <string.h>


// Helper function to find the color of a command
int get_command_color(_self, const char *word, int is_first_word) {

    // printf("get_command_color: word='%s', is_first_word=%d\n", word, is_first_word);
    // Initialize a temporary word list to hold commands
    pl_readline_words_t word_list = pl_readline_word_maker_init();

    // Get all defined commands
    self->pl_readline_get_words((char *)word, word_list);

    // Check if the word exactly matches any of our defined commands
    for (isize i = 0; i < word_list->len; i++) {
        // Skip commands that require first position but aren't first
        if (word_list->words[i].first && !is_first_word) { continue; }

        // Check exact match
        if (strcmp(word_list->words[i].word, word) == 0) {
            int color = word_list->words[i].color;
            pl_readline_word_maker_destroy(word_list);
            return color;
        }
    }

    // Clean up
    pl_readline_word_maker_destroy(word_list);
    return PL_COLOR_RESET; // Default color
}

// Function to redisplay the buffer with colorized commands
void redisplay_buffer_with_colors(_self, int show_prompt) {
    // Save original cursor position
    size_t original_ptr = self->ptr;
    size_t prompt_len   = 0;

    // Calculate prompt length (only if shown)
    if (self->prompt) {
        // Count visible characters in the prompt (ignoring ANSI escape sequences)
        char *p = self->prompt;
        while (*p) {
            if (*p == '\033') {
                // Skip ANSI escape sequence
                while (*p && *p != 'm')
                    p++;
                if (*p) p++; // Skip the 'm'
                continue;
            }
            prompt_len++;
            p++;
        }
    }

    // Move cursor back to start of line using absolute positioning
    pl_readline_print(self, "\r");

    // Display prompt only if requested
    if (show_prompt) {
        pl_readline_print(self, self->prompt);
    } else if (prompt_len > 0) {
        // If not showing prompt but there is a prompt, move cursor forward to skip prompt area
        char forward_buf[32];
        sprintf(forward_buf, "\033[%zuC", prompt_len);
        pl_readline_print(self, forward_buf);
    }

    // Track position considering the prompt if shown
    size_t display_position = prompt_len; // Start after prompt

    // Clear the line after cursor
    pl_readline_print(self, "\033[K");

    // Parse and colorize each word in the buffer
    char  *buffer_copy = strdup(self->buffer);
    size_t buffer_len  = strlen(buffer_copy);

    // Handle empty buffer
    if (buffer_len == 0) {
        free(buffer_copy);
        return;
    }

    // Get all tokens and preserve spaces
    char *words[256] = {0};
    int   word_count = 0;
    char *p          = buffer_copy;
    char *word_start = p;

    // Parse words and spaces
    while (*p) {
        if (*p == ' ') {
            // Found a space - terminate the current word
            *p = '\0';
            if (p > word_start) { // Non-empty word
                words[word_count++] = word_start;
            }
            // Mark this as a space position
            words[word_count++] = NULL; // NULL indicates a space
            word_start          = p + 1;
        }
        p++;
    }

    // Don't forget the last word if it's not empty
    if (*word_start) { words[word_count++] = word_start; }

    // Render all words and spaces with correct coloring
    for (int i = 0; i < word_count; i++) {
        if (words[i] == NULL) {
            // This is a space
            pl_readline_print(self, " ");
            display_position++;
        } else {
            // This is a word
            // Check if this is the first word in the line
            int is_first = (i == 0 || (i > 0 && words[i - 1] == NULL && i == 1));

            int color = get_command_color(self, words[i], is_first);

            if (color != PL_COLOR_RESET) {
                // Apply color
                char color_str[16];
                sprintf(color_str, "\033[%dm", color);
                pl_readline_print(self, color_str);
            }

            // Print the word
            pl_readline_print(self, words[i]);
            display_position += strlen(words[i]);

            if (color != PL_COLOR_RESET) {
                // Reset color
                pl_readline_print(self, "\033[0m");
            }
        }
    }

    free(buffer_copy);

    // If original buffer ends with space, print it
    if (self->length > 0 && self->buffer[self->length - 1] == ' ') {
        pl_readline_print(self, " ");
        display_position++;
    }

    // Calculate cursor position (prompt + original_ptr)
    size_t target_position = prompt_len + original_ptr;

    // Position cursor correctly
    if (display_position != target_position) {
        // Use absolute positioning from the left edge
        pl_readline_print(self, "\r");

        if (target_position > 0) {
            char move_buf[32];
            sprintf(move_buf, "\033[%zuC", target_position);
            pl_readline_print(self, move_buf);
        }
    }
}