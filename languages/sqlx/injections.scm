; SQL fragments are combined so the SQL parser sees one document with the
; Dataform-only regions cut out.
((sql_text) @injection.content
  (#set! injection.language "SQL")
  (#set! injection.combined))

((block_sql_text) @injection.content
  (#set! injection.language "SQL")
  (#set! injection.combined))

((js_body) @injection.content
  (#set! injection.language "JavaScript"))

((js_expression) @injection.content
  (#set! injection.language "JavaScript"))
