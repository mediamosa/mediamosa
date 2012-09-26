<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * Contains cql_part class
 */

/**
 * Default CQL part Class
 * A part class contains [prefix assignment] [index[modifier] relation[modifier]] searchTerm
 *
 */
class mediamosa_cql_part extends mediamosa_cql_part_group {

  private $m_a_cql_part_group = array();

  private $m_cql_part_parent = FALSE;

  // Parts
  private $m_a_prefix_assignments = array();  // optional
  private $m_o_cql_part_index = FALSE;        // optional
  private $m_o_cql_part_relation = FALSE;     // optional
  private $m_o_cql_part_search_term = FALSE;  // required
  private $m_o_cql_part_boolean = FALSE;      // optional
  private $m_b_not_boolean = FALSE;           // optional (has NOT parameter in front)


  /**
   * Constructor
   */
  final public function __construct($o_cql_part_parent, $o_prefix_assignment) {
    $this->m_cql_part_parent = $o_cql_part_parent;
    $this->mediamosa_cql_part_prefix_assignment_add($o_prefix_assignment);
  }

  /**
   * Go back recursive, and check if we are 'notted'
   *
   */
  final public function mediamosa_cql_part_has_not() {
    if ($this->m_b_not_boolean) {
      return TRUE;
    }

    if ($this->m_cql_part_parent) {
      return $this->m_cql_part_parent->mediamosa_cql_part_has_not();
    }

    return FALSE;
  }

  /**
   * Add a prefix assignment (is object, might have modifiers)
   *
   * @param object $o_prefix_assignment
   */
  final public function mediamosa_cql_part_prefix_assignment_add($o_prefix_assignment) {
    $this->m_a_prefix_assignments[] = $o_prefix_assignment;
  }

  /**
   * Get the prefix assignments
   *
   * @param object
   */
  final public function mediamosa_cql_part_prefix_assignment_get() {
    return $this->m_a_prefix_assignments;
  }

  /**
   * Set a index (is object, might have modifiers)
   *
   * @param object $o_cql_part_index
   */
  final public function mediamosa_cql_part_index_set($o_cql_part_index) {
    $this->m_o_cql_part_index = $o_cql_part_index;
  }

  /**
   * Get the index object
   *
   * @return object
   */
  final public function mediamosa_cql_part_index_get() {
    return $this->m_o_cql_part_index;
  }

  /**
   * Set a relation (is object, might have modifiers)
   *
   * @param object $o_cql_part_relation
   */
  final public function mediamosa_cql_part_relation_set($o_cql_part_relation) {
    $this->m_o_cql_part_relation = $o_cql_part_relation;
  }

  /**
   * Get the relation object
   *
   * @return object $o_cql_part_relation
   */
  final public function mediamosa_cql_part_relation_get() {
    return $this->m_o_cql_part_relation;
  }

  /**
   * Set a search term (has no modifiers)
   *
   * @param object $o_cql_part_index
   */
  final public function mediamosa_cql_part_search_term_set($o_cql_part_search_term) {
    $this->m_o_cql_part_search_term = $o_cql_part_search_term;
  }

  /**
   * Get the search term object
   *
   * @return object $o_cql_part_index
   */
  final public function mediamosa_cql_part_search_term_get() {
    return $this->m_o_cql_part_search_term;
  }

  /**
   * Set a boolean (is object, might have modifiers)
   *
   * @param object $o_cql_part_relation
   */
  final public function mediamosa_cql_part_boolean_set($o_cql_part_boolean) {
    $this->m_o_cql_part_boolean = $o_cql_part_boolean;
  }

  /**
   * Get the boolean object (if any)
   *
   * @return object $o_cql_part_relation
   */
  final public function mediamosa_cql_part_boolean_get() {
    return $this->m_o_cql_part_boolean;
  }

  /**
   * checks if any exception was thrown
   *
   * @param cql_context $o_cql_context
   * @param integer $i_pos
   */
  final public function mediamosa_cql_part_verify_index(mediamosa_cql_context $o_cql_context, $i_pos) {
    if ($o_cql_context->mediamosa_cql_context_index2column($this) === FALSE) {
      $o_cql_part_index = $this->mediamosa_cql_part_index_get();
      $str_index = $o_cql_part_index->mediamosa_cql_part_str_get();
      throw new mediamosa_cql_exception_syntax_error(sprintf('unknown index; %s', $str_index), $i_pos - mediamosa_unicode::strlen($str_index));
    }
  }

  /**
   * Set if NOT is present in front of
   *
   * @param boolean $b_set
   */
  final public function mediamosa_cql_part_has_boolean_not_set($b_set) {
    $this->m_b_not_boolean = $b_set ? TRUE : FALSE;
  }
}

/**
 * Index is part of cql_part.
 */
class mediamosa_cql_part_index extends mediamosa_cql_modifiers {

  private $m_str_index = "";

  final public function __construct($str_index) {
    $this->m_str_index = $str_index;
  }

  protected function mediamosa_cql_modifier_allowed_modifier($str) {
    return in_array($str, array()) ? TRUE : FALSE;
  }

  /**
   * return the contents of the relation
   *
   * @return string
   */
  final public function mediamosa_cql_part_str_get() {
    return $this->m_str_index;
  }
}

/**
 * Relation is part of cql_part
 *
 */
class mediamosa_cql_part_relation extends mediamosa_cql_modifiers {

  private $m_str_relation = "";

  final public function __construct($str_relation) {
    $this->m_str_relation = $str_relation;
  }

  protected function mediamosa_cql_modifier_allowed_modifier($str) {
    return in_array($str, array('ignorecase', 'respectcase')) ? TRUE : FALSE;
  }

  /**
   * return the contents of the relation
   *
   * @return string
   */
  final public function mediamosa_cql_part_str_get() {
    return $this->m_str_relation;
  }
}

/**
 * Search term is part of cql_part
 *
 */
final class mediamosa_cql_part_search_term {

  private $m_str_search_term = "";

  final public function __construct($str_search_term) {
    assert($str_search_term != "");

    // Remove any trailing "
    if ($str_search_term == '""') {
      $str_search_term = '';
    }
    elseif (mediamosa_unicode::substr($str_search_term, 0 , 1) == '"' && mediamosa_unicode::substr($str_search_term, -1) == '"') {
      $str_search_term = mediamosa_unicode::substr($str_search_term, 1, -1);
    }

    $this->m_str_search_term = $str_search_term;
  }

  /**
   * return the contents of the search term
   *
   * @return string
   */
  final public function mediamosa_cql_part_str_get() {
    return $this->m_str_search_term;
  }

  /**
   * Check a escaped cql search term if it contains any wildcards
   *
   * @param string $str
   *   The string to test for wildcards.
   *
   * @return boolean
   *   Will return TRUE when string contains wildcards.
   */
  final public function mediamosa_cql_contains_wildcards($str = FALSE) {
    $str = ($str === FALSE ? $this->mediamosa_cql_part_str_get() : $str);
    $str = str_replace(array('\\', '\*', '\?'), array(''), $str);
    return (mb_strpos($str, '?') === FALSE && mb_strpos($str, '*') === FALSE ? FALSE : TRUE);
  }
}

/**
 * Boolean is part of cql_part.
 */
class mediamosa_cql_part_boolean extends mediamosa_cql_modifiers {

  private $m_type = FALSE;

  final public function __construct($str_boolean) {
    $a_booleans = array('or' => MEDIAMOSA_CQL_BOOLEAN_OR, 'and' => MEDIAMOSA_CQL_BOOLEAN_AND, 'not' => MEDIAMOSA_CQL_BOOLEAN_NOT, 'prox' => MEDIAMOSA_CQL_BOOLEAN_PROX);

    assert(isset($a_booleans[mediamosa_unicode::strtolower($str_boolean)]));
    if (!isset($a_booleans[mediamosa_unicode::strtolower($str_boolean)])) {
      throw new mediamosa_cql_exception(sprintf('Program error; %s is not an valid boolean', $str_boolean), mediamosa::MEDIAMOSA_CQL_EXCEPTION_SEVERITY_HIGH);
    }

    $this->m_type = $a_booleans[mediamosa_unicode::strtolower($str_boolean)];
  }

  protected function mediamosa_cql_modifier_allowed_modifier($str) {
    return in_array($str, array()) ? TRUE : FALSE;
  }

  /**
   * Returns the boolean type
   *
   * @return type
   */
  final public function mediamosa_cql_part_str_get() {
    $a_booleans = array(MEDIAMOSA_CQL_BOOLEAN_OR => 'OR', MEDIAMOSA_CQL_BOOLEAN_AND => 'AND', MEDIAMOSA_CQL_BOOLEAN_NOT => 'NOT', MEDIAMOSA_CQL_BOOLEAN_PROX => 'PROX');
    return $a_booleans[$this->m_type];
  }

  /**
   * Returns the boolean type
   *
   * @return type
   */
  final public function mediamosa_cql_part_type_get() {
    return $this->m_type;
  }
}
