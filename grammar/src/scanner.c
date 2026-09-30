#include "tree_sitter/parser.h"

#include <stdbool.h>
#include <stdint.h>

enum TokenType { SQL_TEXT, BLOCK_SQL_TEXT, JS_BODY, JS_EXPRESSION };

static void advance(TSLexer *lexer) { lexer->advance(lexer, false); }

static bool is_hspace(int32_t c) { return c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v'; }

static bool is_space(int32_t c) { return is_hspace(c) || c == '\n'; }

static bool is_ident_start(int32_t c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_ident(int32_t c) { return is_ident_start(c) || (c >= '0' && c <= '9'); }

static bool str_eq(const char *a, const char *b) {
  while (*a && *a == *b) {
    a++;
    b++;
  }
  return *a == *b;
}

static bool is_block_keyword(const char *w) {
  return str_eq(w, "config") || str_eq(w, "js") || str_eq(w, "pre_operations") ||
         str_eq(w, "post_operations") || str_eq(w, "incremental_where") || str_eq(w, "input");
}

static void scan_js(TSLexer *lexer, bool stop_at_comma);

// Lookahead is the opening quote. Template literals may nest `${...}`.
static void scan_js_string(TSLexer *lexer, int32_t quote) {
  advance(lexer);
  while (!lexer->eof(lexer)) {
    int32_t c = lexer->lookahead;
    if (c == '\\') {
      advance(lexer);
      if (!lexer->eof(lexer)) advance(lexer);
    } else if (c == quote) {
      advance(lexer);
      return;
    } else if (quote == '`' && c == '$') {
      advance(lexer);
      if (lexer->lookahead == '{') {
        advance(lexer);
        scan_js(lexer, false);
        if (lexer->lookahead == '}') advance(lexer);
      }
    } else if (quote != '`' && c == '\n') {
      return;
    } else {
      advance(lexer);
    }
  }
}

// Returns true if the lookahead '/' started a comment (which is consumed).
static bool scan_js_comment(TSLexer *lexer) {
  advance(lexer);
  if (lexer->lookahead == '/') {
    while (!lexer->eof(lexer) && lexer->lookahead != '\n') advance(lexer);
    return true;
  }
  if (lexer->lookahead == '*') {
    advance(lexer);
    while (!lexer->eof(lexer)) {
      if (lexer->lookahead == '*') {
        advance(lexer);
        if (lexer->lookahead == '/') {
          advance(lexer);
          break;
        }
      } else {
        advance(lexer);
      }
    }
    return true;
  }
  return false;
}

// Consumes JS until an unbalanced closer (or a top-level ',' when
// stop_at_comma), leaving that character as the lookahead.
static void scan_js(TSLexer *lexer, bool stop_at_comma) {
  int depth = 0;
  while (!lexer->eof(lexer)) {
    int32_t c = lexer->lookahead;
    if (c == '{' || c == '[' || c == '(') {
      depth++;
      advance(lexer);
    } else if (c == '}' || c == ']' || c == ')') {
      if (depth == 0) return;
      depth--;
      advance(lexer);
    } else if (c == ',' && depth == 0 && stop_at_comma) {
      return;
    } else if (c == '"' || c == '\'' || c == '`') {
      scan_js_string(lexer, c);
    } else if (c == '/') {
      scan_js_comment(lexer);
    } else {
      advance(lexer);
    }
  }
}

static bool scan_js_body(TSLexer *lexer) {
  lexer->result_symbol = JS_BODY;
  bool consumed = false;
  while (!lexer->eof(lexer) && lexer->lookahead != '}') {
    consumed = true;
    scan_js(lexer, false);
    if (lexer->lookahead == ']' || lexer->lookahead == ')') advance(lexer);
  }
  lexer->mark_end(lexer);
  return consumed;
}

static bool scan_js_expression(TSLexer *lexer) {
  lexer->result_symbol = JS_EXPRESSION;
  while (is_space(lexer->lookahead)) lexer->advance(lexer, true);
  int32_t c = lexer->lookahead;
  if (lexer->eof(lexer) || c == '{' || c == '[' || c == '}' || c == ']' || c == ',') return false;
  scan_js(lexer, true);
  lexer->mark_end(lexer);
  return true;
}

// SQL runs until `${`, EOF, and either a line-leading block keyword
// (top level) or the unbalanced `}` closing the enclosing block.
static bool scan_sql(TSLexer *lexer, bool in_block) {
  lexer->result_symbol = in_block ? BLOCK_SQL_TEXT : SQL_TEXT;
  bool has_content = false;
  bool line_start = !in_block && lexer->get_column(lexer) == 0;
  int depth = 0;

  for (;;) {
    if (lexer->eof(lexer)) {
      lexer->mark_end(lexer);
      return has_content;
    }
    int32_t c = lexer->lookahead;

    if (c == '$') {
      lexer->mark_end(lexer);
      advance(lexer);
      if (lexer->lookahead == '{') return has_content;
      has_content = true;
      line_start = false;
      continue;
    }

    if (in_block && c == '}') {
      if (depth == 0) {
        lexer->mark_end(lexer);
        return has_content;
      }
      depth--;
    } else if (in_block && c == '{') {
      depth++;
    }

    if (c == '\n') {
      line_start = !in_block;
    } else if (!is_hspace(c)) {
      if (line_start && is_ident_start(c)) {
        lexer->mark_end(lexer);
        char word[24];
        int len = 0;
        bool too_long = false;
        while (is_ident(lexer->lookahead)) {
          if (len < 23) word[len++] = (char)lexer->lookahead;
          else too_long = true;
          advance(lexer);
        }
        word[len] = '\0';
        if (!too_long && is_block_keyword(word)) {
          while (is_space(lexer->lookahead)) advance(lexer);
          if (str_eq(word, "input") && (lexer->lookahead == '"' || lexer->lookahead == '\'')) {
            scan_js_string(lexer, lexer->lookahead);
            while (is_space(lexer->lookahead)) advance(lexer);
          }
          if (lexer->lookahead == '{') return has_content;
        }
        has_content = true;
        line_start = false;
        continue;
      }
      line_start = false;
    }

    // Leading whitespace is skipped so whitespace-only runs produce no token.
    if (is_space(c)) {
      lexer->advance(lexer, !has_content);
    } else {
      advance(lexer);
      has_content = true;
    }
  }
}

void *tree_sitter_sqlx_external_scanner_create(void) { return NULL; }

void tree_sitter_sqlx_external_scanner_destroy(void *payload) {}

unsigned tree_sitter_sqlx_external_scanner_serialize(void *payload, char *buffer) { return 0; }

void tree_sitter_sqlx_external_scanner_deserialize(void *payload, const char *buffer, unsigned length) {}

bool tree_sitter_sqlx_external_scanner_scan(void *payload, TSLexer *lexer, const bool *valid_symbols) {
  // All externals valid at once means the parser is in error recovery.
  if (valid_symbols[SQL_TEXT] && valid_symbols[BLOCK_SQL_TEXT] && valid_symbols[JS_BODY] &&
      valid_symbols[JS_EXPRESSION]) {
    return false;
  }
  if (valid_symbols[JS_BODY]) return scan_js_body(lexer);
  if (valid_symbols[JS_EXPRESSION]) return scan_js_expression(lexer);
  if (valid_symbols[BLOCK_SQL_TEXT]) return scan_sql(lexer, true);
  if (valid_symbols[SQL_TEXT]) return scan_sql(lexer, false);
  return false;
}
