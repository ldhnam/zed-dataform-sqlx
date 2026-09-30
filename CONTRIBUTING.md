# Contributing

## How it works

```
extension.toml         Zed manifest; pins the grammar to a commit of this repo
languages/sqlx/        Zed language config and tree-sitter queries
  config.toml          file type, comments, brackets
  highlights.scm       colors for Dataform-only syntax (keywords, config keys)
  injections.scm       hands SQL regions to "SQL" and JS regions to "JavaScript"
  outline.scm          outline panel entries
  indents.scm          auto-indent inside blocks
  overrides.scm        scopes: strings, comments, and `config` (uses `//` comments)
  textobjects.scm      Vim-mode block and interpolation text objects
  runnables.scm        run button on `config`
  tasks.json           Dataform CLI tasks for that button
  brackets.scm         bracket matching
grammar/               tree-sitter grammar
  grammar.js           grammar rules
  src/scanner.c        external scanner that finds SQL/JS region boundaries
  src/parser.c         generated; do not edit by hand
  test/corpus/         parser tests
examples/              sample Dataform project used for tests and the README
```

The grammar never parses SQL or JavaScript itself. The scanner only finds where each region starts and ends:

- `sql_text` / `block_sql_text`: SQL at the top level or inside a SQL block
- `js_body`: the inside of `js { }` and `${ }`
- `js_expression`: one value inside `config { }`

`injections.scm` then asks Zed to parse those nodes with its SQL and JavaScript grammars. All SQL regions of a file are combined into one document (`injection.combined`), so the SQL parser sees the query with the Dataform parts cut out.

## Setup

Requires Node.js 18 or newer.

```sh
cd grammar
npm install
```

## Changing the grammar

1. Edit `grammar/grammar.js` or `grammar/src/scanner.c`.
2. Regenerate the parser and run the tests:

   ```sh
   cd grammar
   npm run generate
   npm test
   npx tree-sitter parse ../examples/definitions/*.sqlx
   ```

3. Add a test case to `grammar/test/corpus/sqlx.txt` for the new behavior.

## Changing queries

Queries in `languages/sqlx/` are read by Zed directly, so changes show up after `zed: rebuild dev extension` (or reinstalling). To check a query compiles against the grammar:

```sh
cd grammar
npx tree-sitter query ../languages/sqlx/highlights.scm ../examples/definitions/orders_daily.sqlx
```

Capture names follow [Zed's language extension docs](https://zed.dev/docs/extensions/languages).

## Testing in Zed

Zed builds the grammar from the commit pinned in `extension.toml`, not from your working tree. To test local grammar changes, temporarily point it at your clone:

```toml
[grammars.sqlx]
repository = "file:///absolute/path/to/zed-dataform-sqlx"
rev = "<a local commit containing your change>"
path = "grammar"
```

Then run `zed: install dev extension` on the repo folder. Zed's log (`zed: open log`) shows grammar build errors.

## Releasing

1. Commit the grammar change.
2. Set `rev` in `extension.toml` to that commit's SHA. CI fails if `grammar/` changed after the pinned commit.
3. Bump `version` in `extension.toml` and add an entry to `CHANGELOG.md`.
4. Commit and push.
