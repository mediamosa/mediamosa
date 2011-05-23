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
 * Contains cql_partgroup class, which is a container for cql_part objects.
 */

/**
 * Default CQL Context Class.
 *
 */
class mediamosa_cql_part_group {

  private $m_a_cql_part = array();

  /**
   * Add a part.
   *
   * @param object $o_cql_part
   */
  final public function mediamosa_cql_part_add($o_cql_part) {
    $this->m_a_cql_part[] = $o_cql_part;
  }

  /**
   * dummy version.
   */
  public function mediamosa_cql_part_has_not() {
    return FALSE;
  }

  final public function mediamosa_cql_part_parse($o_obj, $function_name, &$a_result_parse, $a_prefix_assignments = array(), $array_settings = array()) {

    $a_results = array();

    // Call my kids.
    foreach ($this->m_a_cql_part as $o_cql_part) {
      // Attach any new prefix assignments, these are valid over the whole cql string.
      $a_prefix_assignments += $o_cql_part->mediamosa_cql_part_prefix_assignment_get();

      // Call function to convert our part into anything the function wants.
      $str_result = $o_obj->$function_name($o_cql_part, $a_result_parse, FALSE);// front.
      if ($str_result != "") {
        $a_results[] = $str_result;
      }

      $a_parse_tmp = $o_cql_part->mediamosa_cql_part_parse($o_obj, $function_name, $a_result_parse, $a_prefix_assignments, $array_settings);
      if (count($a_parse_tmp)) {
        $a_results[] = $a_parse_tmp;
      }

      // Call function to convert our part into anything the function wants.
      $str_result = $o_obj->$function_name($o_cql_part, $a_result_parse, TRUE);// Back.
      if (trim($str_result) != "") {
        $a_results[] = trim($str_result);
      }
    }

    return $a_results;
  }
}
