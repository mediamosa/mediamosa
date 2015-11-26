<?php
/**
 * @file
 * Contains cql_context class
 */

abstract class mediamosa_cql_context extends mediamosa_cql {

  // ------------------------------------------------------- Abstract functions.
  abstract protected function mediamosa_cql_context_prefix_assignment_get();
  abstract protected function mediamosa_cql_context_is_named_comparitor($str);
  abstract public function mediamosa_cql_context_index2column(mediamosa_cql_part $o_cql_part);


  // --------------------------------------------------------------------- Vars.
  /**
   * Is CQL successfully parsed and no syntax or parser errors.
   *
   * null: Has not been parsed.
   * true: Parsed and valid.
   * false: Parsed and failed.
   *
   * @var bool
   */
  public $parsed = null;

  /**
   * The last exception caught.
   *
   * @var mediamosa_cql_exception
   */
  private $last_catched_exception = FALSE;

  // ------------------------------------------------------------------ Methods.
  /**
   * Parse the given CQL string
   *
   * @param string $str_cql
   *   The string to parse.
   *
   * @return bool
   *   Was succesfully parsed.
   */
  final protected function mediamosa_cql_parse($str_cql) {

    try {
      $this->parsed = TRUE;
      parent::mediamosa_cql_parse($str_cql);
    }
    catch (mediamosa_cql_exception $e) {
      $this->parsed = FALSE;
      $this->last_catched_exception = $e;
    }

    return $this->parsed;
  }

  /**
   * Verify the given CQL string
   *
   * @param string $str_cql
   * @return bool
   */
  final public function mediamosa_cql_verify($str_cql) {
    try {
      $this->parsed = TRUE;
      parent::mediamosa_cql_verify($str_cql);
    }
    catch (mediamosa_cql_exception $e) {
      $this->parsed = FALSE;
      $this->last_catched_exception = $e;
    }

    return $this->parsed;
  }

  /**
   * Return the last fatal exception that cause cql to stop.
   *
   * @return mediamosa_cql_exception
   *   The last CQL exception caught.
   */
  final public function mediamosa_cql_last_exception_get() {
    return $this->last_catched_exception;
  }

  /**
   * Check if given string is a known relation syntax
   *
   * @param string $str
   * @return bool
   */
  final public function mediamosa_cql_context_is_relation($str) {
    return ($this->mediamosa_cql_context_is_comparitor_symbol($str) || $this->mediamosa_cql_context_is_named_comparitor($str));
  }

  final public function mediamosa_cql_context_is_comparitor_symbol($str) {
    $a_comparitor_symbol = array('=', '>', '<', '>=', '<=', '<>', '==');
    return (array_search($str, $a_comparitor_symbol) === FALSE ? FALSE : TRUE);
  }

  public function mediamosa_cql_context_is_boolean($str) {
    $a_booleans = array('and', 'or', 'not', 'prox');
    return (array_search(mediamosa_unicode::strtolower($str), $a_booleans) === FALSE) ? FALSE : TRUE;
  }

  public function mediamosa_cql_context_is_boolean_not($str) {
    return (mediamosa_unicode::strtolower($str) == 'not' ? TRUE : FALSE);
  }
}
