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
  *
  */

class mediamosa_aut_group {
  // ------------------------------------------------------------------ Static Function.
  static public function create($app_id, $group_name, $aut_type) {

    // Trim it
    $group_name = trim($group_name);

    switch ($aut_type) {
      case mediamosa_aut::AUT_NAME_TYPE_DOMAIN:
        if (!mediamosa_aut::is_group_domain($group_name)) {
          throw new mediamosa_exception_error_invalid_data_parameter(array("@parameter" => $group_name, "@data" => "domain group"));
        }

        break;

      case mediamosa_aut::AUT_NAME_TYPE_REALM:
        if (!mediamosa_aut::is_group_realm($group_name)) {
          throw new mediamosa_exception_error_invalid_data_parameter(array("@parameter" => $group_name, "@data" => "domain group"));
        }

        break;

      default:
        throw new mediamosa_exception_error_invalid_data_parameter(array("@parameter" => $group_name, "@data" => "domain group or realm group"));
    }

    // @todo: not master slave save.
    $a_object = mediamosa_db::db_query(
      'SELECT * FROM {mediamosa_aut_group} WHERE app_id = :app_id AND aut_group_type = :aut_group_type AND aut_group_name = :aut_group_name',
      array(
        ':app_id' => $app_id,
        ':aut_group_type' => $aut_type,
        ':aut_group_name' => $group_name,
      )
    )->fetchAssoc();

    if ($a_object === FALSE) {
      return mediamosa_db::db_query(
        'INSERT INTO {mediamosa_aut_group} SET app_id = :app_id, aut_group_type = :aut_group_type, aut_group_name = :aut_group_name',
        array(
          ':app_id' => $app_id,
          ':aut_group_type' => $aut_type,
          ':aut_group_name' => $group_name,
        )
      );
    }

    return $a_object['aut_group_id'];
  }

  /**
   * Retrieve group listing.
   *
   * @param array $a_app_ids
   * @param integer $offset
   * @param integer $limit
   * @return db result
   *
   */
  static function get_list(array $a_app_ids, $offset, $limit) {

    $a_query = array();
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = mediamosa_aut_group_db::AUT_GROUP_NAME;
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = mediamosa_aut_group_db::AUT_GROUP_TYPE;
    $a_query[mediamosa_db_query::A_FROM][] = '{' . mediamosa_aut_group_db::TABLE_NAME . '}';

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf('%s IN(%s)', mediamosa_aut_group_db::APP_ID, implode(',', $a_app_ids));
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['group_type'][mediamosa_db_query::WHERE_OR][] = mediamosa_aut_group_db::AUT_GROUP_TYPE . ' IS NOT NULL';// trick to keep the index in order
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['group_type'][mediamosa_db_query::WHERE_OR][] = mediamosa_aut_group_db::AUT_GROUP_TYPE . ' IS NULL';
    $a_query[mediamosa_db_query::A_ORDER_BY][] = mediamosa_aut_group_db::AUT_GROUP_NAME . ' ASC';

    $a_query[mediamosa_db_query::LIMIT] = $limit;
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    $s_query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => TRUE));

    // Do the query.
    $result = mediamosa_db_query::query($s_query);

    // Set the found_rows on the object.
    $result->found_rows = mediamosa_db_query::found_rows();

    return $result;
  }

  /**
   * Get group by given name.
   *
   * @param array $a_app_ids
   * @param string $group_name
   */
  static function get_by_group_name(array $a_app_ids, $group_name) {

    return mediamosa_db::db_select(mediamosa_aut_group_db::TABLE_NAME, 'ag')
      ->fields('ag', array(
        mediamosa_aut_group_db::AUT_GROUP_NAME,
        mediamosa_aut_group_db::AUT_GROUP_TYPE,
        mediamosa_aut_group_db::ID,
      ))
      ->condition(mediamosa_aut_group_db::APP_ID, $a_app_ids, 'IN')
      ->condition(db_or()->isNull(mediamosa_aut_group_db::AUT_GROUP_TYPE)->isNotNull(mediamosa_aut_group_db::AUT_GROUP_TYPE))
      ->condition(mediamosa_aut_group_db::AUT_GROUP_NAME, $group_name)
      ->execute()->fetchAssoc();
    }

  /**
   * Retrieve hostnames from group.
   *
   * @param array $a_app_ids
   * @param string $group_name
   * @param integer $offset
   * @param integer $limit
   * @return db result
   */
  static function get_hostnames_by_group_name(array $a_app_ids, $group_name, $offset, $limit) {
    $a_query = array();

    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "{mediamosa_aut_name}.aut_name";
    $a_query[mediamosa_db_query::A_FROM][] = "{mediamosa_aut_name}";

    $a_query[mediamosa_db_query::A_JOIN]["aut_group"] = "LEFT JOIN {mediamosa_aut_group} USING (aut_group_id)";

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf('{mediamosa_aut_name}.%s IN(%s)', mediamosa_aut_group_db::APP_ID, implode(',', $a_app_ids));
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]["group_type"][mediamosa_db_query::WHERE_OR][] = "aut_group_type IS NOT NULL";// trick to keep the index in order
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]["group_type"][mediamosa_db_query::WHERE_OR][] = "aut_group_type IS NULL";
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("{mediamosa_aut_group}.aut_group_name = '%s'", mediamosa_db::escape_string($group_name));

    $a_query[mediamosa_db_query::A_ORDER_BY][] = "{mediamosa_aut_name}.aut_name ASC";

    $a_query[mediamosa_db_query::LIMIT] = $limit;
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => TRUE));

    // Do the query.
    $result = mediamosa_db_query::query($query);

    // Set the found_rows on the object.
    $result->found_rows = mediamosa_db_query::found_rows();

    return $result;
  }

  /**
   * Create hostname(s) for aut group.
   *
   * @param integer $app_id
   * @param string $group_name
   * @param array $a_host_names
   * @return array
   */
  static function create_hostnames_for_group($app_id, $group_name, array $a_host_names) {
    // Lets make sure its not an array here.
    assert(!is_array($app_id));

    $a_group = mediamosa_aut_group::get_by_group_name(array($app_id), $group_name);
    if (!$a_group) {
      throw new mediamosa_exception_error_aut_group_not_found(array('@group' => $group_name));
    }

    $aut_group_id = $a_group[mediamosa_aut_group_db::ID];
    $host_type_group = $a_group[mediamosa_aut_group_db::AUT_GROUP_TYPE];

    $a_result = array();
    foreach ($a_host_names as $host_name) {
      $host_name = trim($host_name);

      $aut_type = mediamosa_aut::is_realm($host_name) ? 'REALM' : 'DOMAIN';

      if ($aut_type != $host_type_group) {
        $a_error = mediamosa_error::error_code_find(
          mediamosa_error::ERRORCODE_HOSTNAME_TYPE_NO_MATCH_GROUP_TYPE,
          array(
            '@hostname' => $host_name,
            '@hostname_type' => $aut_type,
            '@group' => $group_name,
            '@group_type' => $host_type_group
          )
        );

        $a_result[] = array(
          'hostname' => $host_name,
          'result' => mediamosa_response::ERROR,
          'result_id' => $a_error[mediamosa_error_db::CODE],
          'result_description' => $a_error[mediamosa_error_db::MESSAGE],
        );

        // skip.
        continue;
      }

      if (mediamosa_db::db_count_rows(
          mediamosa_aut_name_db::TABLE_NAME,
          array(
            mediamosa_aut_name_db::APP_ID => $app_id,
            mediamosa_aut_name_db::AUT_TYPE => $host_type_group,
            mediamosa_aut_name_db::AUT_GROUP_ID => $aut_group_id,
            mediamosa_aut_name_db::AUT_NAME => $host_name
          )
      )) {
        $a_error = mediamosa_error::error_code_find(
          mediamosa_error::ERRORCODE_HOSTNAME_ALREADY_IN_GROUP,
          array(
            '@hostname' => $host_name,
            '@group' => $group_name
          )
        );

        $a_result[] = array(
          'hostname' => $host_name,
          'result' => mediamosa_response::ERROR,
          'result_id' => $a_error[mediamosa_error_db::CODE],
          'result_description' => $a_error[mediamosa_error_db::MESSAGE],
        );
      }
      else {
        $a_fields = array(
          mediamosa_aut_name_db::APP_ID => $app_id,
          mediamosa_aut_name_db::AUT_TYPE => $host_type_group,
          mediamosa_aut_name_db::AUT_GROUP_ID => $aut_group_id,
          mediamosa_aut_name_db::AUT_NAME => $host_name,
        );

        mediamosa_db::db_insert(mediamosa_aut_name_db::TABLE_NAME)
          ->fields($a_fields)
          ->execute();

        $a_result[] = array(
          'hostname' => $host_name,
          'result' => mediamosa_response::SUCCESS,
          'result_id' => mediamosa_error::ERRORCODE_OKAY,
          'result_description' => '',
        );
      }
    }

    return $a_result;
  }

  /**
   * Delete hostname for group.
   *
   * @param integer $app_id
   * @param string $group_name
   * @param array $a_host_names
   */
  static function delete_hostname_for_group($app_id, $group_name, array $a_host_names) {
    // Lets make sure its not an array here.
    assert(!is_array($app_id));

    $a_group = mediamosa_aut_group::get_by_group_name(array($app_id), $group_name);
    if (!$a_group) {
      throw new mediamosa_exception_error_aut_group_not_found(array('@group' => $group_name));
    }

    $aut_group_id = $a_group[mediamosa_aut_group_db::ID];

    $a_result = array();
    foreach ($a_host_names as $aut_name) {
      $aut_name = trim($aut_name);

      $aut_type = mediamosa_aut::is_realm($aut_name) ? 'REALM' : 'DOMAIN';

      try {
        mediamosa_aut_name::delete($app_id, $aut_type, $aut_name, $aut_group_id);

        $a_result[] = array(
          'hostname' => $aut_name,
          'result' => mediamosa_response::SUCCESS,
          'result_id' => mediamosa_error::ERRORCODE_OKAY,
          'result_description' => '',
        );
      }
      catch (mediamosa_exception_error $e) {
        $a_error = $e->mediamosa_exception_error_array_get();
        $a_result[] = array(
          'hostname' => $aut_name,
          'result' => mediamosa_response::ERROR,
          'result_id' => $a_error[mediamosa_error_db::CODE],
          'result_description' => $a_error[mediamosa_error_db::MESSAGE],
        );
      }
    }

    return $a_result;
  }
}
