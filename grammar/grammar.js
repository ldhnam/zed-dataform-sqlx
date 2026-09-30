/**
 * Dataform SQLX: SQL with `config {}` / `js {}` / `pre_operations {}` /
 * `post_operations {}` / `input "x" {}` blocks and `${...}` JS interpolation.
 * SQL and JS regions are exposed as nodes for editor injections.
 */
module.exports = grammar({
  name: 'sqlx',

  externals: $ => [$.sql_text, $.block_sql_text, $.js_body, $.js_expression],

  extras: $ => [/\s/, $.comment],

  rules: {
    source_file: $ => repeat(choice(
      $.config_block,
      $.js_block,
      $.pre_operations_block,
      $.post_operations_block,
      $.input_block,
      $.interpolation,
      $.sql_text,
    )),

    config_block: $ => seq('config', $.object),

    js_block: $ => seq('js', '{', optional($.js_body), '}'),

    pre_operations_block: $ => seq('pre_operations', $._sql_block_body),

    post_operations_block: $ => seq('post_operations', $._sql_block_body),

    input_block: $ => seq('input', $.string, $._sql_block_body),

    _sql_block_body: $ => seq('{', repeat(choice($.block_sql_text, $.interpolation)), '}'),

    interpolation: $ => seq('${', optional($.js_body), '}'),

    object: $ => seq('{', commaSep($.pair), optional(','), '}'),

    array: $ => seq('[', commaSep($._value), optional(','), ']'),

    pair: $ => seq(
      field('key', choice($.property_identifier, $.string)),
      ':',
      field('value', $._value),
    ),

    _value: $ => choice($.object, $.array, $.js_expression),

    property_identifier: _ => /[A-Za-z_$][A-Za-z0-9_$]*/,

    string: _ => choice(/"([^"\\\n]|\\.)*"/, /'([^'\\\n]|\\.)*'/),

    comment: _ => token(choice(
      seq('//', /[^\n]*/),
      seq('/*', /[^*]*\*+([^/*][^*]*\*+)*/, '/'),
    )),
  },
});

function commaSep(rule) {
  return optional(seq(rule, repeat(seq(',', rule))));
}
