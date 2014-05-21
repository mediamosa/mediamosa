<?php
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
  /**
   * Returns a new DeleteQuery object for the active database.
   *
   * @param $table
   *   The table from which to delete.
   * @param $options
   *   An array of options to control how the query operates.
   *
   * @return DeleteQuery
   *   A new DeleteQuery object for this connection.
   */
  public static function db_delete(/* $table, $options = NULL*/) {
    $args = func_get_args(); // Must copy into $args, cant use it directly in call_user_func_array

    if (mediamosa::is_db_logging()) {
      $start_time = microtime(TRUE);
      $result = call_user_func_array('db_delete', $args);
      mediamosa_watchdog::log_db(round(microtime(TRUE) - $start_time, 4), $result && isset($result->queryString) ? $result->queryString : $args[0]);
      return $result;
    }

    return call_user_func_array('db_delete', $args);
  }

  public static function db_truncate($table, array $options = array()) {
    return db_truncate($table, $options);
  }

  /**
   * Return the number of found rows with SELECT queries using
   * SQL_CALC_FOUND_ROWS (mysql).
   *
   * Warning: MySQL specific.
   */
  public static function db_found_rows() {
    return mediamosa_db::db_query('SELECT found_rows()')->fetchField();
  }

  /**
   * Increase/retrieve query_count.
   *
   * @param int $add
   *  Number to increase, use 0 to retrieve.
   */
  public static function query_count($add = 0) {
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
  private static function query($func = 'db_query', array $args = array()) {

    // Increase query count.
    self::query_count(1);

    // Because the order can change of the a_args per func, lets create pointers to them.
    switch ($func) {
      case 'db_query_range':
        $args += array(
          3 => array(),
          4 => array(),
        );

        // By default, specify we want array's and not objects.
        $args[4] += array(
          'fetch' => PDO::FETCH_ASSOC,
        );

        $query = &$args[0]; // The Query
        $args_cpy = &$args[3];  // The Arguments.
        break;

      case 'db_query_temporary':
      case 'db_query':
        $args += array(
          1 => array(),
          2 => array(),
        );

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
   * Executes a query against the active database, restricted to a range.
   *
   * @param $query
   *   The prepared statement query to run. Although it will accept both named and
   *   unnamed placeholders, named placeholders are strongly preferred as they are
   *   more self-documenting.
   * @param $from
   *   The first record from the result set to return.
   * @param $count
   *   The number of records to return from the result set.
   * @param $args
   *   An array of values to substitute into the query. If the query uses named
   *   placeholders, this is an associative array in any order. If the query uses
   *   unnamed placeholders (?), this is an indexed array and the order must match
   *   the order of placeholders in the query string.
   * @param $options
   *   An array of options to control how the query operates.
   *
   * @return DatabaseStatementInterface
   *   A prepared statement object, already executed.
   *
   * @see DatabaseConnection::defaultOptions()
   */
  public static function db_query_range(/* $query, $from, $count, array $args = array(), array $options = array()*/) {
    $args = func_get_args();
    return self::query('db_query_range', $args);
  }


  /**
   * Execute DB query.
   *
   * @return DatabaseStatementInterface
   *   A prepared statement object, already executed.
   */
  public static function db_query(/* $query, $args = array(), $options = array()*/) {
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
   * @return
   *   The name of the temporary table.
   */
  public static function db_query_temporary(/* $query, array $args = array(), array $options = array()*/) {
    $args = func_get_args();
    return self::query('db_query_temporary', $args);
  }

  /**
   * Returns a new SelectQuery object for the active database.
   *
   * @param $table
   *   The base table for this query. May be a string or another SelectQuery
   *   object. If a query object is passed, it will be used as a subselect.
   * @param $alias
   *   The alias for the base table of this query.
   * @param $options
   *   An array of options to control how the query operates.
   *
   * @return SelectQuery
   *   A new SelectQuery object for this connection.
   */
  public static function db_select(/* $table, $alias = NULL, array $options = array()*/) {
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
   * Returns a new InsertQuery object for the active database.
   *
   * @param $table
   *   The table into which to insert.
   * @param $options
   *   An array of options to control how the query operates.
   *
   * @return InsertQuery
   *   A new InsertQuery object for this connection.
   */
  public static function db_insert(/* $table, array $options = array()*/) {
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
   * @param array $fields
   *   The fields to extend.
   *
   * @return array
   *
   */
  public static function db_insert_enrich(array $fields = array()) {
    $fields[mediamosa_db::CREATED] = mediamosa_datetime::utc_current_timestamp_now();
    $fields[mediamosa_db::CHANGED] = $fields[mediamosa_db::CREATED];
    return $fields;
  }

  /**
   * Returns a new UpdateQuery object for the active database.
   *
   * @param $table
   *   The table to update.
   * @param $options
   *   An array of options to control how the query operates.
   *
   * @return UpdateQuery
   *   A new UpdateQuery object for this connection.
   */
  public static function db_update(/* $table, array $options = array()*/) {
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
   *
   * Only when table has an 'changed' field.
   *
   * @param array $fields
   *   The fields to enrich for update.
   *
   * @return array
   *   The result fields.
   *
   */
  public static function db_update_enrich(array $fields) {
    $fields[mediamosa_db::CHANGED] = mediamosa_datetime::utc_current_timestamp_now();
    return $fields;
  }

 /**
   * Returns a new MergeQuery object for the active database.
   *
   * Allows you to do an update and an insert. However, Drupal does not seems to
   * work with the MySQL INSERT ON DUPLICATE KEY UPDATE, which is kinda bad
   * because the current implementation will never be MASTER<->MASTER safe.
   *
   * Use ->fields() when you wish a UPDATE or INSERT.
   *
   * @code
   * MERGE INTO table_name_1 USING table_name_2 ON (condition)
   *   WHEN MATCHED THEN
   *   UPDATE SET column1 = value1 [, column2 = value2 ...]
   *   WHEN NOT MATCHED THEN
   *   INSERT (column1 [, column2 ...]) VALUES (value1 [, value2 ...
   * @endcode
   *
   * @code
   *   mediamosa_db::db_merge('example')
   *     ->key(array('name' => $name))
   *     ->fields(array(
   *       'field1' => $value1,
   *       'field2' => $value2,
   *     ))
   *     ->execute();
   * @endcode
   *
   * Use ->insertFields to insert and not to update; this is useful when the row
   * is unique. With ->insertfields, the row is inserted when row does not
   * exist.
   *
   * @code
   *   mediamosa_db::db_merge('example')
   *     ->key(array('name' => $name))
   *     ->insertFields(array(
   *       'field1' => $value1,
   *       'field2' => $value2,
   *     ))
   *     ->execute();
   * @endcode
   *
   * @param string $table
   *   The table into which to merge.
   * @param array $options
   *   A array of options to control how the query operates.
   *
   * @return MergeQuery
   *   A new MergeQuery object for this connection.
   *
   * @see db_merge()
   */
  public static function db_merge($table, array $options = array()) {
    // Increase query count.
    self::query_count(1);

    // Call db_select function and attach ouy select extender.
    return db_merge($table, $options);
  }

  /**
   * Escape the value for database
   * 'db_escape_string' is removed from drupal 7, D7 uses prepare instead. For
   * now we will use PDO::quote().
   */
  public static function escape_string($mixed_string, $glue = ',') {
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
  public static function escape_string_quoted($string, $glue = ',') {
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
  public static function escape_string_like($string_like) {
    return self::escape_string(db_like($string_like));
  }

  /**
   * Returns a valid escaped string when for fulltext.
   *
   * @param string $str_like
   * @return string
   */
  public static function escape_string_fulltext($string_fulltext) {
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
  public static function db_set_active($key = 'default') {
    return db_set_active($key);
  }
  /**
   * Get current active database wrapper.
   *
   * @return string
   *  selected database.
   */
  public static function db_get_active() {
    $current_key = db_set_active();
    db_set_active($current_key);

    return $current_key;
  }

  /**
   * Change varchars of IDs into varbinary, so these will be case-sensitive.
   *
   * @param $module_name
   */
  public static function db_install_schema_preprocess($module, $schema) {
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
   * Drupal 7 at this time only describes how it wants to implement foreign keys
   * in their schema layer. For now we will add foreign keys ourselfs after
   * creation of the table.
   *
   * @param array $schema
   */
  public static function db_install_schema_foreign_keys($module_name) {
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
          // We are not using mediamosa::log() here because it might be
          // inaccessible during install.
          watchdog('Mediamosa',
            "Failed to create foreign key 'fk_@table_name_@column' on table '@table_name', query; '@query', message: @message.",
            array(
              '@table_name' => $module_name,
              '@column' => $column,
              '@query' => $query,
              '@message' => $e->getMessage(),
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
   * @param array $columns_and
   *  column => value.
   * @return integer
   *    returns number of rows.
   */
  public static function db_count_rows($table_name, array $columns_and = array()) {
    // Create select object.
    $query = self::db_select($table_name, 'c');
    foreach ($columns_and as $column => $value) {
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
   * @param array $columns_and
   *  column => value.
   * @return integer
   *    returns number of rows.
   */
  public static function db_get_assoc($table_name, array $columns_and = array()) {
    // Create select object.
    $query = self::db_select($table_name, 'row');
    $query->fields('row');
    foreach ($columns_and as $column => $value) {
      if (is_null($value)) {
        $query->isNull($column);
      }
      elseif (is_array($value)) {
        $query->condition($column, $value, 'IN');
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
   * @param array $columns_and
   *  column => value.
   * @param $errorcode
   *  Optional error code to throw when error.
   * @param array $errorcode_args
   *  Optional arguments for the generated error.
   * @param bool $log_error
   *  Optional TRUE/FALSE to log the error, default is TRUE.
   */
  public static function db_must_exists($table_name, array $columns_and = array(), $errorcode = NULL, array $errorcode_args = array(), $log_error = TRUE) {

    $row = self::db_get_assoc($table_name, $columns_and);
    if ($row) {
      // Exists.
      return $row;
    }

    // Custom error supplied?
    if ($errorcode) {
      throw new mediamosa_exception_error($errorcode, $errorcode_args);
    }

    switch ($table_name) {
      case mediamosa_asset_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ASSET_NOT_FOUND, array('@asset_id' => $columns_and[mediamosa_asset_db::ID]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_asset_mediafile_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_NOT_FOUND, array('@mediafile_id' => $columns_and[mediamosa_asset_mediafile_db::ID]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_acl_group_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ACL_GROUP_NOT_FOUND, array('@group' => $columns_and[mediamosa_acl_group_db::ACL_GROUP_NAME]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_ftp_batch_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_FTP_BATCH_NOT_FOUND, array('@batch_id' => $columns_and['batch_id']), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_collection_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_COLLECTION_NOT_FOUND, array('@coll_id' => $columns_and['coll_id']), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_asset_collection_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_COLLECTION_ASSET_RELATION_NOT_FOUND, array('@coll_id' => $columns_and[mediamosa_asset_collection_db::COLL_ID], '@asset_id' => $columns_and[mediamosa_asset_collection_db::ASSET_ID]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_asset_supplement_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ASSET_SUPPLEMENT_NOT_FOUND, array('@supplement_id' => $columns_and[mediamosa_asset_supplement_db::ID]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_ftp_user_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_FTP_UNKNOWN_USER, array(), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_user_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_USERMAN_INVALID_USER, array('@user_id' => $columns_and['name']), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_user_group_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_USERMAN_INVALID_GROUP, array('@group_id' => $columns_and['group_id']), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_job_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_NOT_FOUND, array('@job_id' => $columns_and['job_id']), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_transcode_profile_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_TRANSCODE_PROFILE_NOT_FOUND, array('@profile_id' => $columns_and[mediamosa_transcode_profile_db::ID]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_asset_metadata_db::TABLE_NAME:
      case mediamosa_asset_metadata_property_db::TABLE_NAME:
      case mediamosa_asset_metadata_property_group_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNEXPECTED_ERROR, array(), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
      case mediamosa_storage_profile_db::TABLE_NAME:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_PROFILE_NOT_FOUND, array('@profile_id' => $columns_and[mediamosa_storage_profile_db::ID]), mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error);
    }

    assert(0);
    throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNEXPECTED_ERROR);
  }

  /**
   * Returns TRUE/FALSE if rows where found.
   *
   * @param string $table_name
   *  The table name.
   * @param array $columns_and
   *  column => value.
   *
   * @return bool
   *  Returns TRUE if one or more rows where found.
   */
  public static function db_exists($table_name, array $columns_and = array()) {

    // Create select object.
    $query = self::db_select($table_name, 'row');
    foreach ($columns_and as $column => $value) {
      if (is_null($value)) {
        $query->isNull($column);
      }
      else {
        $query->condition($column, $value);
      }
    }

    return $query->countQuery()->execute()->fetchField() > 0;
  }

  /**
   * Count rows depending on the app, owner and group.
   *
   * @param string $table
   * @param int $app_id
   * @param string $owner_id
   * @param string $group_id
   * @return integer
   */
  public static function db_count_items($table_name, $app_id, $owner_id, $group_id) {

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
  public static function db_count_tables() {
    $tables = db_find_tables('%');
    return count($tables);
  }


  /**
   * Return the column names from an table.
   * NOTE: might not work on all databases, mysql only for now.
   *
   * @return array
   *  Array containing columns.
   */
  public static function db_get_columns($table_name, $schema = NULL) {
    global $databases;
    $schema = ($schema ? $schema : $databases['default']['default']['database']);

    // Use db_query directly here, not the mediamosa_db version.
    return db_query('SELECT COLUMN_NAME FROM information_schema.columns WHERE TABLE_NAME = :table AND TABLE_SCHEMA = :schema', array(
      ':table' => $table_name,
      ':schema' => $schema,
    ))->fetchCol();
  }

  /**
   * Return the column names, data types and column types from an table.
   *
   * FIXME: might not work on all databases, mysql only for now.
   *
   * @return array
   *  Array containing columns.
   */
  public static function db_get_columns_with_type($table_name, $schema = NULL) {
    global $databases;
    $schema = ($schema ? $schema : $databases['default']['default']['database']);

    // Use db_query directly here, not mediamosa_db version.
    // Use Aliases, so we are sure its in upper-case.
    return db_query('SELECT COLUMN_NAME AS COLUMN_NAME, DATA_TYPE AS DATA_TYPE, COLUMN_TYPE AS COLUMN_TYPE FROM information_schema.columns WHERE TABLE_NAME = :table AND TABLE_SCHEMA = :schema', array(
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
 */
  public static function db_transaction($name = NULL, array $options = array()) {
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
  public static function uuid($app_id) {
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
    $uuid = mediamosa_unicode::substr($hash_62, 0, mediamosa_sdk::UUID_LENGTH);

    // Lets make sure its not shorter than the length of mediamosa_sdk::UUID_LENGTH.
    for ($x = mediamosa_unicode::strlen($uuid); mediamosa_unicode::strlen($uuid) < mediamosa_sdk::UUID_LENGTH; $x++) {
      $uuid .= $conf_31_to_62{rand(0, 61)};
    }

    // Simpletest box IDs must be different because we might share IDs between
    // sandbox and real live databases like Solr which don't have same level of
    // sandboxing. We prevent this by giving simpletest IDs always end with X.
    if (mediamosa::in_simpletest_sandbox()) {
      $uuid = substr($uuid, 0, mediamosa_sdk::UUID_LENGTH - 1) . 'X';
    }
    else {
      $uuid = substr($uuid, -1) == 'X' ? substr($uuid, 0, mediamosa_sdk::UUID_LENGTH - 1) . 'Y' : $uuid;
    }

    // Return the new ID.
    return $uuid;
  }

  /**
   * Create a table according to the schema.
   *
   * @param string name
   *  Name of the table.
   * @param array table
   *  Schema definition.
   */
  public static function db_create_table($name, array $table) {
    return db_create_table($name, $table);
  }

  /**
   * Drop a table.
   *
   * @param sting name
   *   Name of the table.
   */
  public static function db_drop_table($name) {
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
  public static function db_change_field($table, $field, $field_new, array $spec, array $keys_new = array()) {
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
  public static function db_table_exists($table) {
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
   public static function db_like($string) {
    return db_like($string);
  }
}
