# Changelog

All notable changes to this extension are documented here. The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project uses [Semantic Versioning](https://semver.org/).

## [0.2.0] - 2026-09-30

### Added

- `incremental_where { }` blocks (older Dataform versions) are parsed and highlighted as SQL.
- Outline entries for `config`, its keys, and every block.
- Run button on `config` with Dataform CLI tasks: compile, dry run, run, and run with dependencies.
- Auto-indent inside `config`, arrays, and all blocks.
- `//` comment toggling inside `config`; `/* */` block comments.
- Vim text objects for blocks and `${ }` interpolations.
- Quotes no longer auto-close inside strings and comments.
- CI: corpus tests, query checks, a parse of every `.sqlx` file in the official Dataform repository, and a check that `extension.toml` pins the current grammar.

## [0.1.0] - 2026-09-30

### Added

- Tree-sitter grammar for Dataform SQLX with SQL and JavaScript injections.
- Highlighting for `config`, `js`, `pre_operations`, `post_operations`, `input` blocks and `${ }` interpolation.

[0.2.0]: https://github.com/ldhnam/zed-dataform-sqlx/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/ldhnam/zed-dataform-sqlx/releases/tag/v0.1.0
