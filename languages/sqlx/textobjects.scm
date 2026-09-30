((comment)+ @comment.around)

(config_block
  (object
    "{"
    (_)* @class.inside
    "}")) @class.around

(js_block
  "{"
  (_)* @class.inside
  "}") @class.around

(pre_operations_block
  "{"
  (_)* @class.inside
  "}") @class.around

(post_operations_block
  "{"
  (_)* @class.inside
  "}") @class.around

(incremental_where_block
  "{"
  (_)* @class.inside
  "}") @class.around

(input_block
  "{"
  (_)* @class.inside
  "}") @class.around

(interpolation
  "${"
  (_)? @function.inside
  "}") @function.around
