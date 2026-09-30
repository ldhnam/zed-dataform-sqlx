["config" "js" "pre_operations" "post_operations" "input"] @keyword

(pair key: (property_identifier) @property)
(pair key: (string) @property)
(input_block (string) @string)

(comment) @comment

(interpolation ["${" "}"] @punctuation.special)

(object ["{" "}"] @punctuation.bracket)
(array ["[" "]"] @punctuation.bracket)
(js_block ["{" "}"] @punctuation.bracket)
(pre_operations_block ["{" "}"] @punctuation.bracket)
(post_operations_block ["{" "}"] @punctuation.bracket)
(input_block ["{" "}"] @punctuation.bracket)

[":" ","] @punctuation.delimiter
