<?php
/**
 * @file
 * Contains cql_prefix_assignment class
 */

class mediamosa_cql_prefix_assignment {

  private $m_str_prefix = '';
  private $m_str_indentifier = '';

  final public function __construct($str_prefix, $str_indentifier) {
    $this->m_str_prefix = $str_prefix;
    $this->m_str_indentifier = $str_indentifier;
  }

  /**
   * Get the prefix
   *
   * @param string $str_prefix
   */
  final public function mediamosa_cql_prefix_assignment_prefix_get() {
    return $this->m_str_prefix;
  }

  /**
   * Get the indentified
   *
   * @param string $str_indentifier
   */
  final public function mediamosa_cql_prefix_assignment_indentifier_get() {
    return $this->m_str_indentifier;
  }

  /**
   * Set the prefix
   *
   * @param string $str_prefix
   */
  final public function mediamosa_cql_prefix_assignment_prefix_set($str_prefix) {
    if ($str_prefix == '') {
      throw new mediamosa_cql_exception_string_is_empty();
    }

    $this->m_str_prefix = $str_prefix;
  }

  /**
   * Set the indentified
   *
   * @param string $str_indentifier
   */
  final public function mediamosa_cql_prefix_assignment_indentifier_set($str_indentifier) {
    if ($str_indentifier == '') {
      throw new mediamosa_cql_exception_string_is_empty();
    }

    $this->m_str_indentifier = $str_indentifier;
  }
}
