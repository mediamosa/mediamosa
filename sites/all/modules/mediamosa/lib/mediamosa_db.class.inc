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
 * Our DB layer between MediaMosa and Drupal DB.
 */

class mediamosa_db {
  // ------------------------------------------------------------------ Consts.
  const LIMIT = 'limit';
  const OFFSET = 'offset';
  const ORDER_BY = 'order_by';
  const ORDER_DIRECTION = 'order_direction';
  const ORDER_DIRECTION_ASC = 'asc';
  const ORDER_DIRECTION_DESC = 'desc';

  // The varchar length of the used HASH ids.
  const HASH_ID_LENGTH = 32;

  // Default names for changed and created columns.
  const CHANGED = 'changed';
  const CREATED = 'created';

  // This is the function we use to get the current time in the database.
  // In this case its the local time of the server. We might choose to use
  // UTC_TIMESTAMP instead.
  const CURRENT_TIMESTAMP = 'CURRENT_TIMESTAMP';

  // ------------------------------------------------------------------ Static Function.
  static public function db_delete(/* $table, $options = NULL*/) {
    $args = func_get_args(); // Must copy into $args, cant use it directly in call_user_func_array

    if (mediamosa::is_db_logging()) {
      $start_time = microtime(TRUE);
      $result = call_user_func_array('db_delete', $args);
      mediamosa_watchdog::log_db(round(microtime(TRUE) - $start_time, 4), $result && isset($result->queryString) ? $result->queryString : $args[0]);
      return $result;
    }

    return call_user_func_array('db_delete', $args);
  }

  /**
   * Return the number of found rows during SQL_CALC_FOUND_ROWS (mysql).
   */
  static public function db_found_rows() {
    return mediamosa_db::db_query('SELECT found_rows()')->fetchField();
  }

  /**
   * Increase/retrieve query_count.
   *
   * @param integer $add
   *  Number to increase, use 0 to retrieve.
   */
  static public function query_count($add = 0) {
    static $queries = 0;

    $queries += $add;
    return $queries;
  }


  /**
   * Wrapper around query functions.
   *
   * @param string $func
   *  String value of the db_query[_*] function.
   * @param string $query
   *  SQL query.
   * @param array $args
   *  The arguments for $query.
   * @return result
   *  DB result set object.
   */
  static private function query($func = 'db_query', array $args = array()) {

    // Increase query count.
    self::query_count(1);

    // Because the order can change of the a_args per func, lets create pointers to them.
    switch ($func) {
      case 'db_query_range':
        if (!isset($args[4])) {
          $args[4] = array();
        }

       // By default, specify we want array's and not objects.
        $args[4] += array(
          'fetch' => PDO::FETCH_ASSOC,
        );
        $query = &$args[0]; // The Query
        $args_cpy = &$args[3];  // The Arguments.
        break;

      case 'db_query_temporary':
      case 'db_query':
        // By default, specify we want array's and not objects.
        $args[2] += array(
          'fetch' => PDO::FETCH_ASSOC,
        );

        $query = &$args[0]; // The Query.
        $args_cpy = &$args[1];  // The Arguments.
        break;

      default:
        throw new mediamosa_exception_program_error('not expected.');
    }

    // Make sure {} are inside the query, this will prevent problems later.
    if (stripos($query, 'FROM') !== FALSE && stripos($query, ' found_rows()') !== FALSE) {
      if (strpos($query, '{') === FALSE || strpos($query, '}') === FALSE) {
        throw new mediamosa_exception_program_error('Missing {} your query; @query.', array('@query' => $query));
      }
    }

    // Define '#vars' for inserting values that are not quoted in the result.
    if (isset($args_cpy)) {
      $extras = array();
      foreach ($args_cpy as $key => $value) {
        if ($key[0] != '#') {
          continue;
        }

        $extras[$key] = $value;
      }

      if (!empty($extras)) {
        $query = strtr($query, $extras);
        foreach ($extras as $key => $value) {
          unset($args_cpy[$key]);
        }
      }
    }

    // Loggin on.
    if (mediamosa::is_db_logging()) {
      $start_time = microtime(TRUE);
    }

    // Do the query.
    $result = call_user_func_array($func, $args);

    // Loggin on.
    if (mediamosa::is_db_logging()) {
      mediamosa_watchdog::log_db(round(microtime(TRUE) - $start_time, 4), $result && isset($result->queryString) ? $result->queryString : $query);
    }

    return $result;
  }

  /**
   * Same as db_query, except for adding an range for query.
   *
   * @return DB result
   *  DB result set object.
   */
  static public function db_query_range(/* $query, $from, $count, array $args = array(), array $options = array()*/) {
    $args = func_get_args();
    return self::query('db_query_range', $args);
  }

  /**
   * Execute DB query.
   *
   * @return DB result
   *  DB result set object.
   */
  static public function db_query(/* $query, $args = array(), $options = array()*/) {
    $args = func_get_args();

    /**
     * FIXME
     * Remove code once we migrated all old db_query sprintf queries.
     * Start Query hack fix for old 6.x drupal queries.
     */
    if (isset($args[1]) && !is_array($args[1])) {
      $query = &$args[0]; // The Query.
      $args_2 = array();

      $y = 1;
      for ($x = 0; $x < mediamosa_unicode::strlen($query); $x++) {
        if ($query{$x} != '%' || $x + 1 == mediamosa_unicode::strlen($query)) {
          continue;
        }

        // We only replace the specifiers which are not escaped and in our array.
        $char = $query{++$x};
        if (in_array($char, array('n', 'd', 'f', 'b', 's'))) {
          $query = mediamosa_unicode::substr($query, 0, $x - 1) . '#mediamosa_var_' . $y . mediamosa_unicode::substr($query, $x + 1);

          $prefix = '#';

          // If we had '%s', then this has become '#mediamosa_var_x', now replace that.
          if ( ($x - 2) >= 0 && mediamosa_unicode::substr($query, $x - 2, mediamosa_unicode::strlen("'#mediamosa_var_" . $y . "'")) == "'#mediamosa_var_" . $y . "'") {
            $query = mediamosa_unicode::substr($query, 0, $x - 2) . ':mediamosa_var_' . $y . mediamosa_unicode::substr($query, $x + mediamosa_unicode::strlen("#mediamosa_var_" . $y));
            $prefix = ':';
          }

          // lets skip the inserted stuff a bit.
          $x += mediamosa_unicode::strlen('#mediamosa_var_' . $y) - 2;

          switch ($char) {
            case 'd':
              $args_2[$prefix . 'mediamosa_var_' . $y] = (int)$args[$y];
              break;
            case 'f':
              $args_2[$prefix . 'mediamosa_var_' . $y] = (float)$args[$y];
              break;
            default:
              $args_2[$prefix . 'mediamosa_var_' . $y] = (string)$args[$y];
              break;
          }

          // Remove the numbered index version.
          unset($args[$y++]);
        }
      }

      // Copy the new array into args[1]
      $args[1] = $args_2;
    }
    /**
     * End Query hack fix for old 6.x drupal queries.
     */

    if (!isset($args[1])) {
      $args[1] = array();
    }
    if (!isset($args[2])) {
      $args[2] = array();
    }

    // Lets make sure we return the inserted ID with insert into!
    if (mediamosa_unicode::strtoupper(mediamosa_unicode::substr($args[0], 0, mediamosa_unicode::strlen('INSERT INTO '))) == 'INSERT INTO ') {
      $args[2] += array(
        'return' => Database::RETURN_INSERT_ID,
      );
    }

    return self::query('db_query', $args);
  }

  /**
   * Execute DB query in an temporary table.
   *
   * @return DB result
   *  DB result set object.
   */
  static public function db_query_temporary(/* $query, array $args = array(), array $options = array()*/) {
    $args = func_get_args();
    return self::query('db_query_temporary', $args);
  }

  /**
   * Create an db_select object.
   *
   * @return object
   *  DB Select object.
   */
  static public function db_select(/* $table, $alias = NULL, array $options = array()*/) {
    // Increase query count.
    self::query_count(1);

    $args = func_get_args(); // Must copy into $args, cant use it directly in call_user_func_array

    // Lets return array's by default and not objects.
    $args += array(2 =>
      array(
        'fetch' => PDO::FETCH_ASSOC,
      ),
    );

    if (mediamosa::is_db_logging()) {
      $start_time = microtime(TRUE);
      $result = call_user_func_array('db_select', $args);
      mediamosa_watchdog::log_db(round(microtime(TRUE) - $start_time, 4), $result && isset($result->queryString) ? $result->queryString : '-failure-');
      return $result;
    }

    // Call db_select function and attach ouy select extender.
    return call_user_func_array('db_select', $args);
  }

  /**
   * Create an db_insert object.
   *
   * @return object
   *  DB Insert object.
   */
  static public function db_insert(/* $table, array $options = array()*/) {
    // Increase query count.
    self::query_count(1);

    $args = func_get_args(); // Must copy into $args, cant use it directly in call_user_func_array

    if (mediamosa::is_db_logging()) {
      $start_time = microtime(TRUE);
      $result = call_user_func_array('db_insert', $args);
      mediamosa_watchdog::log_db(round(microtime(TRUE) - $start_time, 4), $result && isset($result->queryString) ? $result->queryString : '-failure-');
      return $result;
    }

    return call_user_func_array('db_insert', $args);
  }

  /**
   * Enrich the created fields.
   * Only when table has an 'changed' field(!).
   *
   * @param array $a_fields
   * @return array
   *
   */
  static public function db_insert_enrich(array $fields) {
    $fields[mediamosa_db::CREATED] = mediamosa_datetime::utc_current_timestamp_now();
    $fields[mediamosa_db::CHANGED] = mediamosa_datetime::utc_current_timestamp_now();
    return $fields;
  }

  /**
   * Create an db_update object.
   *
   * @return object
   *  DB Update object.
   */
  static public function db_update(/* $table, array $options = array()*/) {
    // Increase query count.
    self::query_count(1);

    $args = func_get_args(); // Must copy into $args, cant use it directly in call_user_func_array

    if (mediamosa::is_db_logging()) {
      $start_time = microtime(TRUE);
      $result = call_user_func_array('db_update', $args);
      mediamosa_watchdog::log_db(round(microtime(TRUE) - $start_time, 4), $result && isset($result->queryString) ? $result->queryString : '-failure-');
      return $result;
    }

    return call_user_func_array('db_update', $args);
  }

  /**
   * Enrich the updated fields.
   * Only when table has an 'changed' field(!).
   *
   * @param array $a_fields
   * @return array
   *
   */
  static public function db_update_enrich(array $fields) {
    $fields[mediamosa_db::CHANGED] = mediamosa_datetime::utc_current_timestamp_now();
    return $fields;
  }

  /**
   * Escape the value for database
   * 'db_escape_string' is removed from drupal 7, D7 uses prepare instead. For
   * now we will use PDO::quote.
   */
  static public function escape_string($mixed_string, $glue = ',') {
    // Hmm well kinda hack here, but dont see any other way atm
    // than calling the getconnection for PDO and using quote...
    // But \' must be escaped as '' in the PDO layer, quote seems to
    // make mistakes here by escaping with \' while SQL advices ''. Anyway, hotfix here. RBL.
    if (is_array($mixed_string)) {
      $a_str = array();
      foreach ($mixed_string as $str) {
        $a_str[] = self::escape_string($str);
      }

      return implode($glue, $a_str);
    }

    // Call PDO quote, and replace \' to ''. Also removes the quotes around the value.
    return is_int($mixed_string) ? mediamosa_unicode::substr(Database::getConnection()->quote($mixed_string), 1, -1) : str_replace('\\\'', "''", mediamosa_unicode::substr(Database::getConnection()->quote($mixed_string), 1, -1));
  }

  /**
   * Escape the value for database
   * 'db_escape_string' is removed from drupal 7, D7 uses prepare instead. For now
   * we will use PDO::quote.
   * Will return string 'quoted' like PDO.
   */
  static public function escape_string_quoted($string, $glue = ',') {
    // Assert when array is given here.
    assert(!is_array($string));

    // Integers don't need quotes.
    if (is_int($string)) {
      return self::escape_string($string);
    }

    // Call PDO quote.
    return str_replace('\\\'', "''", Database::getConnection()->quote($string));
  }

  /**
   * Returns a valid escaped string when using like.
   *
   * @param string $str_like
   * @return string
   */
  static public function escape_string_like($string_like) {
    return self::escape_string(db_like($string_like));
  }

  /**
   * Returns a valid escaped string when for fulltext.
   *
   * @param string $str_like
   * @return string
   */
  static public function escape_string_fulltext($string_fulltext) {
    return self::escape_string($string_fulltext);
  }

  /**
   * Set current active database wrapper.
   *
   * @param string $key
   *  DB name.
   * @return string
   *  previous selected database.
   */
  static public function db_set_active($key = 'default') {
    return db_set_active($key);
  }
  /**
   * Get current active database wrapper.
   *
   * @return string
   *  selected database.
   */
  static public function db_get_active() {
    $current_key = db_set_active();
    db_set_active($current_key);

    return $current_key;
  }

  /**
   * Change varchars of IDs into varbinary, so these will be case-sensitive.
   *
   * @param $module_name
   */
  static public function db_install_schema_preprocess($module, $schema) {
    // Now loop through the fields and replace the varbinary types.
    // We could remove these in the future when drupal supports these types.
    foreach ($schema as $name => $table) {
      foreach ($table['fields'] as $field_name => $field) {
        // Our new var type?
        if ($field['type'] == 'varbinary') {
          $schema[$name]['fields'][$field_name]['mysql_type'] = 'VARBINARY(' . $schema[$name]['fields'][$field_name]['length'] . ')';
          $schema[$name]['fields'][$field_name]['type'] = 'varchar';
        }
      }
    }

    return $schema;
  }

  /**
   * Install the foreign keys.
   * Drupal 7 at this time only describes how it wants to implement foreign keys in
   * their schema layer. For now we will add foreign keys ourselfs after creation of
   * the table.
   *
   * @param array $schema
   */
  static public function db_install_schema_foreign_keys($module_name) {
    $schema = drupal_get_schema_unprocessed($module_name);
    _drupal_schema_initialize($schema, $module_name, FALSE);

    if (!isset($schema[$module_name]) || !isset($schema[$module_name]['foreign keys'])) {
      return;
    }

    if (mediamosa::in_simpletest_sandbox()) {
      // @FIXME not working in simpletest setup.
      return;
    }

    foreach ($schema[$module_name]['foreign keys'] AS $column => $foreign_key) {
      foreach ($foreign_key as $foreign_table_name => $foreign_column) {
        $query = strtr("ALTER TABLE {#table_name} ADD CONSTRAINT `fk_#table_name_#column` FOREIGN KEY (`#column`) REFERENCES `{#foreign_table_name}` (`#foreign_column`)",
          array(
            '#table_name' => $module_name,
            '#column' => $column,
            '#foreign_table_name' => $foreign_table_name,
            '#foreign_column' => $foreign_column,
          )
        );

        try {
          db_query($query);
        }
        catch (PDOException $e) {
          assert($e);

          assert(0);// so we know.

          // We do not using mediamosa::log() here because it might be inaccessible during install.
          watchdog('Mediamosa',
            "Failed to create foreign key 'fk_#table_name_#column' on table '#table_name', query; '#query', message: #message.",
            array(
              '#table_name' => $module_name,
              '#column' => $column,
              '#query' => $query,
              '#message' => $e->getMessage(),
            ),
            WATCHDOG_ERROR
          );
        }
        // only the first is allowed.
        break;
      }
    }
  }

  /**
   * Returns the number of rows.
   *
   * @param string $table_name
   *  The table name.
   * @param array $a_columns_and
   *  column => value.
   * @return integer
   *    returns number of rows.
   */
  static public function db_count_rows($table_name, array $a_columns_and = array()) {
    // Create select object.
    $query = self::db_select($table_name, 'c');
    foreach ($a_columns_and as $column => $value) {
      if (is_null($value)) {
        $query->isNull($column);
      }
      else {
        $query->condition($column, $value);
      }
    }

    return $query->countQuery()->execute()->fetchField();
  }

  /**
   * Returns the first row found.
   *
   * @param string $table_name
   *  The table name.
   * @param array $a_columns_and
   *  column => value.
   * @return integer
   *    returns number of rows.
   */
  static public function db_get_assoc($table_name, array $a_columns_and = array()) {
    // Create select object.
    $query = self::db_select($table_name, 'row');
    $query->fields('row');
    foreach ($a_columns_and as $column => $value) {
      if (is_null($value)) {
        $query->isNull($column);
      }
      else {
        $query->condition($column, $value);
      }
    }

    return $query->range(0, 1)->execute()->fetchAssoc();
  }

  /**
   * Will throw exception if no rows where found.
   *
   * @param string $table_name
   *  The table name.
   * @param array $a_columns_and
   *  column => value.
   *
   */
  static function db_must_exists($table_name, array $a_columns_and = array(), $errorcode = NULL, array $a_errorcode_args = array(), $log_error = TRUE) {

    $a_row = self::db_get_assoc($table_name, $a_columns_and);
    if ($a_row) {
      return $a_row; // exists
    }

    // Custom error supplied?
    if ($errorcode) {
      throw new mediamosa_exception_error($errorcode, $a_errorcode_args);
    }

    switch ($table_name) {
      case mediamosa_asset_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ASSET_NOT_FOUND, array('@asset_id' => $a_columns_and[mediamosa_asset_db::ID]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_asset_mediafile_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_NOT_FOUND, array('@mediafile_id' => $a_columns_and[mediamosa_asset_mediafile_db::ID]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_acl_group_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ACL_GROUP_NOT_FOUND, array('@group' => $a_columns_and[mediamosa_acl_group_db::ACL_GROUP_NAME]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_ftp_batch_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_FTP_BATCH_NOT_FOUND, array('@batch_id' => $a_columns_and['batch_id']), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_collection_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_COLLECTION_NOT_FOUND, array('@coll_id' => $a_columns_and['coll_id']), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_asset_collection_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_COLLECTION_ASSET_RELATION_NOT_FOUND, array('@coll_id' => $a_columns_and[mediamosa_asset_collection_db::COLL_ID], '@asset_id' => $a_columns_and[mediamosa_asset_collection_db::ASSET_ID]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_asset_supplement_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ASSET_SUPPLEMENT_NOT_FOUND, array('@supplement_id' => $a_columns_and[mediamosa_asset_supplement_db::ID]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_ftp_user_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_FTP_UNKNOWN_USER, array(), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_user_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_USERMAN_INVALID_USER, array('@user_id' => $a_columns_and['name']), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_user_group_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_USERMAN_INVALID_GROUP, array('@group_id' => $a_columns_and['group_id']), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_job_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_NOT_FOUND, array('@job_id' => $a_columns_and['job_id']), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_transcode_profile_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_TRANSCODE_PROFILE_NOT_FOUND, array('@profile_id' => $a_columns_and[mediamosa_transcode_profile_db::ID]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_asset_metadata_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNEXPECTED_ERROR, array(), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_asset_metadata_property_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNEXPECTED_ERROR, array(), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_asset_metadata_property_group_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNEXPECTED_ERROR, array(), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
    }

    assert(0);
    throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNEXPECTED_ERROR);
  }

  /**
   * Will throw exception if no rows where found.
   *
   * @param string $table_name
   *  The table name.
   * @param array $a_columns_and
   *  column => value.
   * @return boolean
   *  Return true or false on exists one or more row match.
   */
  static function db_exists($table_name, array $a_columns_and = array()) {
    try {
      self::db_must_exists($table_name, $a_columns_and, NULL, array(), FALSE);
    }
    catch (mediamosa_exception $e) {
      assert($e);
      return FALSE;
    }

    return TRUE;
  }

  /**
   * Count rows depending on the app, owner and group.
   *
   * @param string $table
   * @param integer $app_id
   * @param string $owner_id
   * @param string $group_id
   * @return integer
   */
  static public function db_count_items($table_name, $app_id, $owner_id, $group_id) {

    // Create select query.
    $query = mediamosa_db::db_select($table_name, 'tbl');
    $query->fields('tbl', array('app_id'));
    $query->condition('app_id', $app_id);
    if ($owner_id != '') {
      $query->condition('owner_id', $owner_id);
    }
    if ($group_id != '') {
      $query->condition('group_id', $group_id);
    }

    // return count.
    return $query->countQuery()->execute()->fetchField();
  }

  /**
   * Returns the number of tables in the current database.
   */
  static public function db_count_tables() {
    $a_tables = db_find_tables('%');
    return count($a_tables);
  }


  /**
   * Return the column names from an table.
   * NOTE: might not work on all databases, mysql only for now.
   *
   * @return array
   *  Array containing columns.
   */
  static public function db_get_columns($table_name, $schema = NULL) {
    global $databases;
    $schema = ($schema ? $schema : $databases['default']['default']['database']);

    // Use db_query directly here, not mediamosa_db version.
    return db_query('SELECT COLUMN_NAME FROM information_schema.columns WHERE TABLE_NAME = :table AND TABLE_SCHEMA = :schema', array(
      ':table' => $table_name,
      ':schema' => $schema,
    ))->fetchCol();
  }

  /**
   * Return the column names, data types and column types from an table.
   * NOTE: might not work on all databases, mysql only for now.
   *
   * @return array
   *  Array containing columns.
   */
  static public function db_get_columns_with_type($table_name, $schema = NULL) {
    global $databases;
    $schema = ($schema ? $schema : $databases['default']['default']['database']);

    // Use db_query directly here, not mediamosa_db version.
    return db_query('SELECT COLUMN_NAME, DATA_TYPE, COLUMN_TYPE FROM information_schema.columns WHERE TABLE_NAME = :table AND TABLE_SCHEMA = :schema', array(
      ':table' => $table_name,
      ':schema' => $schema,
    ))->fetchAll();
  }

/**
 * Returns a new transaction object for the active database.
 *
 * @param string $name
 *   Optional name of the transaction.
 * @param array $options
 *   An array of options to control how the transaction operates:
 *   - target: The database target name.
 *
 * @return DatabaseTransaction
 *   A new DatabaseTransaction object for this connection.
 *
 * @see drupal::db_transaction()
 *
 *  $transaction = db_transaction();
 *
 *  try {
 *    ... code
 *  }
 *  catch (Exception $e) {
 *    $transaction->rollback();
 *    throw $e;
 *  }

 *
 */
  static public function db_transaction($name = NULL, array $options = array()) {
    return db_transaction($name, $options);
  }

  /**
   * Generate a new unique ID.
   *
   * @param int $app_id
   *  Application ID, to add a bit more to the randomness.
   * @return string
   *  The unique ID.
   */
  static public function uuid($app_id) {
    $hash = sha1(rand() . ':' . $app_id . ':' . uniqid('', TRUE) . ':' . microtime());

    // Now convert 16 -> 31 based.
    $hash_31 = base_convert($hash, 16, 31); // 0-9 a-u

    // Simple conf. table.
    $a_conf_31 = array(
      '0' => 0, '1' => 1, '2' => 2, '3' => 3, '4' => 4,  '5' => 5, '6' => 6, '7' => 7, '8' => 8, '9' => 9,
      'a' => 10, 'b' => 11, 'c' => 12, 'd' => 13, 'e' => 14,  'f' => 15, 'g' => 16, 'h' => 17, 'i' => 18, 'j' => 19,
      'k' => 20, 'l' => 21, 'm' => 22, 'n' => 23, 'o' => 24,  'p' => 25, 'q' => 26, 'r' => 27, 's' => 28, 't' => 29,
      'u' => 30
    );

    // Conf to
    $conf_31_to_62 = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';

    $hash_62 = '';
    for ($x = (mediamosa_unicode::strlen($hash_31) - 1); $x >= 0; $x -= 2) {
      $lo = $a_conf_31[$hash_31{$x}];
      $hi = ($x - 1) >= 0 ? $a_conf_31[$hash_31{$x - 1}] + 1 : 0; // Hi + 1 so range is 1 - 31, lo will be 0 - 30.
      $hash_62 = mediamosa_unicode::substr($conf_31_to_62, $hi + $lo, 1) . $hash_62;
    }

    // MediaMosa doesn't like ID starting with numbers, so lets attach the 1st one with alpha.
    $hash_62 = $conf_31_to_62{rand(10, 61)} . $hash_62;

    // Max length 24.
    $uuid = mediamosa_unicode::substr($hash_62, 0, mediamosa_settings::UUID_LENGTH);

    // Lets make sure its not shorter than the length of mediamosa_settings::UUID_LENGTH.
    for ($x = mediamosa_unicode::strlen($uuid); mediamosa_unicode::strlen($uuid) < mediamosa_settings::UUID_LENGTH; $x++) {
      $uuid .= $conf_31_to_62{rand(0, 61)};
    }

    // Simpletest box IDs must be different because we might share IDs between
    // sandbox and real live databases like Solr which don't have same level of
    // sandboxing. We prevent this by giving simpletest IDs always end with X.
    if (mediamosa::in_simpletest_sandbox()) {
      $uuid = substr($uuid, 0, mediamosa_settings::UUID_LENGTH - 1) . 'X';
    }
    else {
      $uuid = substr($uuid, -1) == 'X' ? substr($uuid, 0, mediamosa_settings::UUID_LENGTH - 1) . 'Y' : $uuid;
    }

    // Return the new ID.
    return $uuid;
  }

  /**
   * Create a table according to the schema.
   *
   * @param string name
   *   Name of the table.
   * @param array table
   *   Schema definition.
   */
  static public function db_create_table($name, array $table) {
    return db_create_table($name, $table);
  }

  /**
   * Drop a table.
   *
   * @param sting name
   *   Name of the table.
   */
  static public function db_drop_table($name) {
    return db_drop_table($name);
  }

  /**
   * Change specific column.
   *
   * @param string $table
   * @param string $field
   * @param string $field_new
   * @param array $spec
   * @param array $keys_new
   */
  static public function db_change_field($table, $field, $field_new, array $spec, array $keys_new = array()) {
    return db_change_field($table, $field, $field_new, $spec, $keys_new);
  }

  /**
   * Checks if a table exists.
   *
   * @param $table
   *   The name of the table in drupal (no prefixing).
   *
   * @return
   *   TRUE if the given table exists, otherwise FALSE.
   */
  static public function db_table_exists($table) {
    return db_table_exists($table);
  }
  /**
   * Escape characters for like.
   *
   * @param $string
   *   The string to escape.
   *
   * @return
   *   The escaped string.
   */
   static public function db_like($string) {
    return db_like($string);
  }
}
