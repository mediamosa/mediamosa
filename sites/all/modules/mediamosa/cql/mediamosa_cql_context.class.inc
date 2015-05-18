<?php
/**
 * @file
 * Contains cql_context class
 */

abstract class mediamosa_cql_context extends mediamosa_cql {

  // You must define these when extending with this class
  abstract protected function mediamosa_cql_context_prefix_assignment_get();
  abstract protected function mediamosa_cql_context_is_named_comparitor($str);
  abstract public function mediamosa_cql_context_index2column(mediamosa_cql_part $o_cql_part);

  // Store the last exception here.
  private $m_last_exception = FALSE;

  /**
   * Parse the given CQL string
   *
   * @param string $str_cql
   * @param array $a_settings
   * @return array/boolean
   */
  final protected function mediamosa_cql_parse($str_cql, $a_settings = array()) {

    try {
      parent::mediamosa_cql_parse($str_cql, $a_settings);
    }
    catch (mediamosa_cql_exception $e) {
      $this->m_last_exception = $e;
      return FALSE;
    }

    // If we get here to this point, there were no errors.
    return TRUE;
  }

  /**
   * Verify the given CQL string
   *
   * @param string $str_cql
   * @return bool
   */
  final public function mediamosa_cql_verify($str_cql) {
    try {
      parent::mediamosa_cql_verify($str_cql);
    }
    catch (mediamosa_cql_exception $e) {
      $this->m_last_exception = $e;
      return FALSE;
    }

    // If we get here to this point, there were no errors.
    return TRUE;
  }

  /**
   * Return the last fatal exception that cause cql to stop
   *
   * @return object
   */
  final public function mediamosa_cql_last_exception_get() {
    return $this->m_last_exception;
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
