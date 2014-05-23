<?php
/**
 * @file
 * The ACL group.
 */

class mediamosa_acl_group {
  // ---------------------------------------------------------------- Functions.
  /**
   * Create an acl group.
   *
   * @param int $app_id
   *   The client app ID.
   * @param string $group_name
   *   The name of the ACL group.
   * @param string $acl_type
   *   The ACL type.
   *
   * @return string
   *   The created ACL group ID.
   *
   * @throws mediamosa_exception_error_invalid_data_parameter
   */
  public static function create($app_id, $group_name, $acl_type) {

    $group_name = trim($group_name);

    switch ($acl_type) {
      case mediamosa_acl::ACL_NAME_TYPE_DOMAIN:
        if (!mediamosa_acl::is_group_domain($group_name)) {
          throw new mediamosa_exception_error_invalid_data_parameter(array("@parameter" => $group_name, "@data" => "domain group"));
        }
        break;

      case mediamosa_acl::ACL_NAME_TYPE_REALM:
        if (!mediamosa_acl::is_group_realm($group_name)) {
          throw new mediamosa_exception_error_invalid_data_parameter(array("@parameter" => $group_name, "@data" => "domain group"));
        }
        break;

      default:
        throw new mediamosa_exception_error_invalid_data_parameter(array("@parameter" => $group_name, "@data" => "domain group or realm group"));
    }

    // @todo: not master slave save.
    $a_object = mediamosa_db::db_query(
      'SELECT * FROM {mediamosa_acl_group} WHERE app_id = :app_id AND acl_group_type = :acl_group_type AND acl_group_name = :acl_group_name',
      array(
        ':app_id' => $app_id,
        ':acl_group_type' => $acl_type,
        ':acl_group_name' => $group_name,
      )
    )->fetchAssoc();

    if ($a_object === FALSE) {
      return mediamosa_db::db_query(
        'INSERT INTO {mediamosa_acl_group} SET app_id = :app_id, acl_group_type = :acl_group_type, acl_group_name = :acl_group_name',
        array(
          ':app_id' => $app_id,
          ':acl_group_type' => $acl_type,
          ':acl_group_name' => $group_name,
        )
      );
    }

    return $a_object['acl_group_id'];
  }

  /**
   * Retrieve group listing.
   *
   * @param array $app_ids
   *   The app IDs.
   * @param int $offset
   *   The offset within the result record set.
   * @param int $limit
   *   The maximum items to return.
   *
   * @return DatabaseStatementInterface
   *   A prepared statement object, already executed.
   */
  public static function get_list(array $app_ids, $offset, $limit) {

    $a_query = array();
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = mediamosa_acl_group_db::ACL_GROUP_NAME;
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = mediamosa_acl_group_db::ACL_GROUP_TYPE;
    $a_query[mediamosa_db_query::A_FROM][] = '{' . mediamosa_acl_group_db::TABLE_NAME . '}';

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf('%s IN(%s)', mediamosa_acl_group_db::APP_ID, implode(',', $app_ids));
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['group_type'][mediamosa_db_query::WHERE_OR][] = mediamosa_acl_group_db::ACL_GROUP_TYPE . ' IS NOT NULL';
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['group_type'][mediamosa_db_query::WHERE_OR][] = mediamosa_acl_group_db::ACL_GROUP_TYPE . ' IS NULL';
    $a_query[mediamosa_db_query::A_ORDER_BY][] = mediamosa_acl_group_db::ACL_GROUP_NAME . ' ASC';

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
   * @param array $app_ids
   *   The application IDs.
   * @param string $group_name
   *   The name of the group to get.
   */
  public static function get_by_group_name(array $app_ids, $group_name) {

    return mediamosa_db::db_select(mediamosa_acl_group_db::TABLE_NAME, 'ag')
      ->fields('ag', array(
        mediamosa_acl_group_db::ACL_GROUP_NAME,
        mediamosa_acl_group_db::ACL_GROUP_TYPE,
        mediamosa_acl_group_db::ID,
      ))
      ->condition(mediamosa_acl_group_db::APP_ID, $app_ids, 'IN')
      ->condition(db_or()->isNull(mediamosa_acl_group_db::ACL_GROUP_TYPE)->isNotNull(mediamosa_acl_group_db::ACL_GROUP_TYPE))
      ->condition(mediamosa_acl_group_db::ACL_GROUP_NAME, $group_name)
      ->execute()->fetchAssoc();
  }

  /**
   * Retrieve hostnames from group.
   *
   * @param array $app_ids
   *   The application IDs.
   * @param string $group_name
   *   The name of the group.
   * @param int $offset
   *   The offset in the record set.
   * @param int $limit
   *   The maximum items.
   *
   * @return DatabaseStatementInterface
   *   A prepared statement object, already executed.
   */
  public static function get_hostnames_by_group_name(array $app_ids, $group_name, $offset, $limit) {
    $a_query = array();

    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "{mediamosa_acl_name}.acl_name";
    $a_query[mediamosa_db_query::A_FROM][] = "{mediamosa_acl_name}";

    $a_query[mediamosa_db_query::A_JOIN]["acl_group"] = "LEFT JOIN {mediamosa_acl_group} USING (acl_group_id)";

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf('{mediamosa_acl_name}.%s IN(%s)', mediamosa_acl_group_db::APP_ID, implode(',', $app_ids));
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]["group_type"][mediamosa_db_query::WHERE_OR][] = "acl_group_type IS NOT NULL";
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]["group_type"][mediamosa_db_query::WHERE_OR][] = "acl_group_type IS NULL";
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("{mediamosa_acl_group}.acl_group_name = '%s'", mediamosa_db::escape_string($group_name));

    $a_query[mediamosa_db_query::A_ORDER_BY][] = "{mediamosa_acl_name}.acl_name ASC";

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
   * Create hostname(s) for acl group.
   *
   * @param int $app_id
   *   The application ID.
   * @param string $group_name
   *   The group name for hostnames.
   * @param array $host_names
   *   The hostnames to create.
   *
   * @return array
   *   Information about the created hostnames.
   */
  public static function create_hostnames_for_group($app_id, $group_name, array $host_names) {
    $group = mediamosa_acl_group::get_by_group_name(array($app_id), $group_name);
    if (!$group) {
      throw new mediamosa_exception_error_acl_group_not_found(array('@group' => $group_name));
    }

    $acl_group_id = $group[mediamosa_acl_group_db::ID];
    $host_type_group = $group[mediamosa_acl_group_db::ACL_GROUP_TYPE];

    $result = array();
    foreach ($host_names as $host_name) {
      $host_name = trim($host_name);

      $acl_type = mediamosa_acl::is_realm($host_name) ? 'REALM' : 'DOMAIN';

      if ($acl_type != $host_type_group) {
        $error = mediamosa_error::error_code_find(
          mediamosa_error::ERRORCODE_HOSTNAME_TYPE_NO_MATCH_GROUP_TYPE,
          array(
            '@hostname' => $host_name,
            '@hostname_type' => $acl_type,
            '@group' => $group_name,
            '@group_type' => $host_type_group
          )
        );

        $result[] = array(
          'hostname' => $host_name,
          'result' => mediamosa_response::ERROR,
          'result_id' => $error[mediamosa_error_db::CODE],
          'result_description' => $error[mediamosa_error_db::MESSAGE],
        );

        // skip.
        continue;
      }

      if (mediamosa_db::db_count_rows(
          mediamosa_acl_name_db::TABLE_NAME,
          array(
            mediamosa_acl_name_db::APP_ID => $app_id,
            mediamosa_acl_name_db::ACL_TYPE => $host_type_group,
            mediamosa_acl_name_db::ACL_GROUP_ID => $acl_group_id,
            mediamosa_acl_name_db::ACL_NAME => $host_name
          )
      )) {
        $error = mediamosa_error::error_code_find(
          mediamosa_error::ERRORCODE_HOSTNAME_ALREADY_IN_GROUP,
          array(
            '@hostname' => $host_name,
            '@group' => $group_name
          )
        );

        $result[] = array(
          'hostname' => $host_name,
          'result' => mediamosa_response::ERROR,
          'result_id' => $error[mediamosa_error_db::CODE],
          'result_description' => $error[mediamosa_error_db::MESSAGE],
        );
      }
      else {
        $fields = array(
          mediamosa_acl_name_db::APP_ID => $app_id,
          mediamosa_acl_name_db::ACL_TYPE => $host_type_group,
          mediamosa_acl_name_db::ACL_GROUP_ID => $acl_group_id,
          mediamosa_acl_name_db::ACL_NAME => $host_name,
        );

        mediamosa_db::db_insert(mediamosa_acl_name_db::TABLE_NAME)
          ->fields($fields)
          ->execute();

        $result[] = array(
          'hostname' => $host_name,
          'result' => mediamosa_response::SUCCESS,
          'result_id' => mediamosa_error::ERRORCODE_OKAY,
          'result_description' => '',
        );
      }
    }

    return $result;
  }

  /**
   * Delete hostname for group.
   *
   * @param int $app_id
   *   The client application.
   * @param string $group_name
   *   The group name.
   * @param array $host_names
   *   The ACL hostnames.
   */
  public static function delete_hostname_for_group($app_id, $group_name, array $host_names) {
    // Lets make sure its not an array here.
    assert(!is_array($app_id));

    $group = mediamosa_acl_group::get_by_group_name(array($app_id), $group_name);
    if (!$group) {
      throw new mediamosa_exception_error_acl_group_not_found(array('@group' => $group_name));
    }

    $acl_group_id = $group[mediamosa_acl_group_db::ID];

    $result = array();
    foreach ($host_names as $acl_name) {
      $acl_name = trim($acl_name);

      $acl_type = mediamosa_acl::is_realm($acl_name) ? 'REALM' : 'DOMAIN';

      try {
        mediamosa_acl_name::delete($app_id, $acl_type, $acl_name, $acl_group_id);

        $result[] = array(
          'hostname' => $acl_name,
          'result' => mediamosa_response::SUCCESS,
          'result_id' => mediamosa_error::ERRORCODE_OKAY,
          'result_description' => '',
        );
      }
      catch (mediamosa_exception_error $e) {
        $error = $e->mediamosa_exception_error_array_get();
        $result[] = array(
          'hostname' => $acl_name,
          'result' => mediamosa_response::ERROR,
          'result_id' => $error[mediamosa_error_db::CODE],
          'result_description' => $error[mediamosa_error_db::MESSAGE],
        );
      }
    }

    return $result;
  }
}
