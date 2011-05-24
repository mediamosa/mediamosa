<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
