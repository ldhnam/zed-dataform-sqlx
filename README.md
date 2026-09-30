# Dataform SQLX for Zed

Syntax highlighting for [Dataform](https://cloud.google.com/dataform) `.sqlx` files in [Zed](https://zed.dev).

A small tree-sitter grammar splits each file into its Dataform parts, then Zed highlights each part with the right language:

| Part | Highlighted as |
|------|----------------|
| SQL body, `pre_operations { }`, `post_operations { }`, `input "x" { }` | SQL (via Zed's SQL extension) |
| `config { ... }` | Object keys natively, values as JavaScript |
| `js { ... }`, `${ ... }` | JavaScript |

The SQL parser only sees the SQL, so `config` blocks and `${ref(...)}` no longer break highlighting for the rest of the file.

## Install

Requires the **SQL** extension (Zed → Extensions → search "SQL").

1. Clone this repo.
2. In Zed, run `zed: install dev extension` from the command palette and pick the cloned folder.
3. Open a `.sqlx` file; the status bar should show **Dataform SQLX**.

The first install takes about a minute while Zed downloads its WebAssembly toolchain and compiles the grammar.

## Layout

```
extension.toml        Zed manifest; pins the grammar commit
languages/sqlx/       Zed language config and highlight/injection/bracket queries
grammar/              tree-sitter grammar (grammar.js + external scanner in src/scanner.c)
```

## Developing the grammar

```sh
cd grammar
npm install
npm run generate   # regenerate src/parser.c after editing grammar.js
npm test           # run test/corpus
```

Zed builds the grammar from the commit pinned in `extension.toml`, not from your working tree. After changing anything under `grammar/`, commit it, push, and set `rev` in `extension.toml` to that commit's SHA.

## Known limitations

- SQL around a `${...}` interpolation is parsed with the interpolation removed, so `FROM ${ref("t")}` shows a small local parse error after `FROM`. Keywords are still highlighted.
- Highlighting of the SQL itself is only as good as Zed's SQL grammar, which doesn't support some BigQuery syntax (e.g. `SELECT * EXCEPT(...)`).

## License

MIT
