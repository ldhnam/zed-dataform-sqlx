# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

Zed editor extension for Dataform `.sqlx` files: a tree-sitter grammar (`grammar/`) plus Zed language config and queries (`languages/sqlx/`). No Rust: Zed's publishing rules forbid Rust code in an extension that doesn't ship a language server.

## Commands

All grammar commands run from `grammar/` (the tree-sitter CLI is a local devDependency, so use `npx`):

```sh
npm install
npm run generate                                   # tree-sitter generate --abi 14 → src/parser.c, grammar.json, node-types.json
npm test                                           # corpus tests in test/corpus/
npx tree-sitter test -i 'incremental_where'        # single test, regex on the test name
npx tree-sitter test -u                            # rewrite expected trees from current output (review the diff)
npx tree-sitter parse ../examples/definitions/orders_daily.sqlx
npx tree-sitter query ../languages/sqlx/highlights.scm ../examples/definitions/*.sqlx   # exits 1 if a query doesn't compile
```

Always regenerate after editing `grammar.js` and commit `src/` with it; CI fails if `src/` is stale. Keep `--abi 14` for compatibility with Zed's tree-sitter.

## Architecture

The grammar does not parse SQL or JavaScript. It only splits a file into regions, and Zed parses those regions with its own grammars through `languages/sqlx/injections.scm`:

- `sql_text` (top level) and `block_sql_text` (inside `pre_operations`/`post_operations`/`incremental_where`/`input`) → injected as `SQL` with `injection.combined`, so all SQL fragments are parsed as one document with the Dataform parts cut out. `SQL` comes from Zed's separate SQL extension, which users must install.
- `js_body` (inside `js { }` and `${ }`) and `js_expression` (each value in `config { }`) → injected as `JavaScript`, each separately.
- `config` keys, block keywords and braces are the only things `highlights.scm` colors directly.

Region boundaries come from the external scanner `grammar/src/scanner.c`, not `grammar.js`. Invariants that are easy to break:

- A block keyword only starts a block at the top level, at the start of a line, and when followed by `{` (after an optional string for `input`). Anything else stays SQL, so columns like `input_date` or `js` are safe.
- Adding a block type needs changes in both `grammar.js` (rule plus `source_file` choice) and `is_block_keyword()` in the scanner. The scanner's keyword buffer is `word[24]`, so a longer keyword needs a bigger buffer. Then update `highlights.scm`, `indents.scm`, `outline.scm` and `textobjects.scm`, and add a corpus test.
- Whitespace-only SQL runs are skipped rather than emitted as tokens.
- When every external symbol is valid, the parser is in error recovery and the scanner returns false.
- JS scanning tracks `{[(` depth, strings, template literals (with nested `${}`) and comments to find the closing brace. `js_expression` stops at a top-level `,` and declines values starting with `{` or `[`, which the grammar parses as `object`/`array`.

`languages/sqlx/tasks.json` + `runnables.scm` put a run button on `config`. Each task runs `sh -c` with positional args: it walks up from `$ZED_DIRNAME` to the folder containing `workflow_settings.yaml` or `dataform.json`, then calls `dataform <cmd> <project> <flags>`. The project dir must come before flags because `--actions` is an array option. Zed substitutes only `$ZED_*` variables and leaves other `$vars` for the shell.

`overrides.scm` marks `config_block` as scope `js` so that `[overrides.js]` in `config.toml` switches comment toggling to `//` inside `config`.

## Releasing: grammar is pinned by commit

Zed builds the grammar from `[grammars.sqlx]` in `extension.toml` (`repository` = this GitHub repo, `rev` = a commit SHA, `path = "grammar"`), never from the working tree. So a grammar change takes two commits:

1. Commit the `grammar/` change.
2. Set `rev` to that commit's SHA, bump `version` in `extension.toml` (also `grammar/package.json` and `grammar/tree-sitter.json`), add a `CHANGELOG.md` entry, and commit.

CI (`.github/workflows/ci.yml`) fails if `grammar/grammar.js` or `grammar/src` differ between the pinned `rev` and `HEAD`. Changes under `languages/` need no `rev` bump. To test grammar changes in Zed before pushing, temporarily set `repository = "file:///<abs path to repo>"` with a local commit as `rev` and run `zed: install dev extension` on the repo folder; build errors appear in `zed: open log`.

CI also parses every `.sqlx` file in `dataform-co/dataform`, so new syntax found there should get a corpus test.

## Repository conventions

- `examples/` and `docs/screenshot.png` must contain only generic sample code, never code from private projects.
- Publishing to Zed's registry (`zed-industries/extensions`) is done by the maintainer: its AI policy forbids agent-authored PRs and comments. The extension id `dataform-sqlx` must not contain "zed" or "extension".
