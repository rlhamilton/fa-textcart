<?php
// Updated 2026-10 for PHP 8.x (FA 2.4.20)

// return if criteria is not set or value match it
function match_criteria($value, $criteria) {
  return ! ( isset($criteria) && $value == $criteria);
}

/* This function replace @ by the default value
  # by the value in the template
  # by the template in the value
  and evaluate what's between ()

  Order of expansion is really important.
  Proirity are :
    Line value of template
    Expression : # or () over constante

  Example
   - | 10 => 10
   7 | 10 => 10
   #+3 | 10 => 13
   7 | #+3 => 10
   (7) | 10 => 7 (template is not constant)
   (7) | (10) => 10 (line is not constant either)

 */
function expand_template($template, $value, $default_value=null) {
  // The template is null when no template line is active; PHP 8.1+ deprecates
  // passing null to string functions, so work with an empty string instead.
  $template = (string)$template;

  # replace # placeholder
  if(isset($value) && $value!=="") {
    $value = (string)$value;
    if($template !== "" && $template !== "0") {
      if(field_value_is_constant($value) == false) {
        # Value needs to be evaluated (either # or ())
        # done first as value as priority over template
        $value = str_replace('#', $template, $value);
      }
      else if (field_value_is_constant($template) == false)  {
        $value = str_replace('#', $value, $template);
      }
    }
  }
  elseif (strpos($template, '#') === false)  {
    $value = $template;
  }

  if(isset($value) && $value!=="")  {
    # now use default value if needed
    $value = str_replace('@', (string)$default_value, (string)$value);

    #evaluate arithmetic expression
    if(preg_match('/^\((.*)\)$/', $value, $matches)) {
      $expression = $matches[1];
      if(preg_match('/^!(.*)/', $expression, $matches)) {
        $value = $matches[1];
      }
      else {
        $value = evaluate_arithmetic($expression, $default_value);
      }
    }
    return $value;
  }
  else {
    return $default_value;
  }
}

/* Evaluate a simple arithmetic expression such as "10*1.5+2".
   Only digits, the decimal point, + - * / %, brackets and spaces are kept,
   so nothing else can reach eval(). In PHP 8 a malformed expression throws
   ParseError and division by zero throws DivisionByZeroError; both used to
   be (or produce) fatal errors here, so they are caught and reported instead.
 */
function evaluate_arithmetic($expression, $fallback=null) {
  $to_eval = preg_replace('/[^0-9.+\-*\/%() ]/', '', (string)$expression);
  if (trim($to_eval) === '') {
    return $fallback;
  }
  try {
    return eval("return $to_eval;");
  } catch (\Throwable $e) {
    display_error("Could not evaluate the expression '($expression)'");
    return $fallback;
  }
}

function field_value_is_constant($value) {
  return(!preg_match('/^\(|#/', (string)$value));
}

function get_kit_description($item_code) {
  $db_result = get_item_kit($item_code);
  $props = db_fetch($db_result);
  return $props ? $props['description'] : null;
}
