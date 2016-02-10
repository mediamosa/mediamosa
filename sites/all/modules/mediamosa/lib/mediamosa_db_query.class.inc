<?php
/**
 * @file
 * A basic class to create DB Queries.
 * Although in future we should use db_ classes of drupal 7 instead (see
 * mediamosa_db).
 */

class mediamosa_db_query {
  // ------------------------------------------------------------------- Consts.
  // Select options
  const SQL_CALC_FOUND_ROWS = 'SQL_CALC_FOUND_ROWS';

  const WHERE_AND = 'AND';
  const WHERE_OR = 'OR';

// Names of sub-arrays
  const A_SELECT_EXPR = 'a_select_expr';
  const A_SELECT_OPTIONS = 'a_select_options';
  const A_WHERE = 'a_where';
  const A_ORDER_BY = 'a_order_by';
  const A_FROM = 'a_from';
  const A_JOIN = 'a_join';
  const A_GROUP_BY = 'a_group_by';
  const A_HAVING = 'a_having';
  const LIMIT = 'limit';
  const OFFSET = 'offset';
  const ALLOW_DISTINCT = 'allow_distinct';

  // ---------------------------------------------------------------- Functions.
  /**
   * Create the WHERE syntax
   *
   * @param array $a_where
   * @return string
   */
  public static function where($a_where) {
    return implode(' AND ', self::where_2($a_where));
  }

  /**
   * self::where helper function
   *
   * @param array $a_where
   * @param string $l
   * @param string $r
   * @param string $glue
   * @return array
   */
  public static function where_2($a_where, $l = '', $r = '', $glue = ' AND ') {
    if (!is_array($a_where)) {
      return array($a_where);
    }

    $a_result = array();
    foreach ($a_where as $type => $a_statements) {
      if ($type == self::WHERE_AND) {
        $glue = " AND \n";
        $a_statements = self::where_2($a_statements, '(', ')', " AND \n");
        $a_result[] = $l . implode(" AND \n", $a_statements) . $r;
      }
      elseif ($type == self::WHERE_OR) {
        $a_statements = self::where_2($a_statements, '(', ')', ' OR ');
        $a_result[] = $l . implode(' OR ', $a_statements) . $r;
      }
      else {
        $a_result[] = implode($glue, self::where_2($a_statements, '(', ')', $glue));
      }
    }

    return $a_result;
  }

  /**
   * Simple where create
   *
   * @param array $a_where (e.g. array("test = 'value'", "foo = 'test'))
   * @param string $glue
   * @return string
   */
  public static function simple_where($a_where, $glue = ' AND ') {
    return ' WHERE ' . implode($glue, $a_where);
  }

  /**
   * Simple SET create
   *
   * @param array $a_set (e.g. array("test = 'value'", "foo = 'test'))
   * @return string
   */
  public static function simple_set($a_set) {
    return ' SET ' . implode(',', $a_set);
  }

  /*
   * Execute a query.
   */
  public static function query($query) {
    // Escape % so its not removed by db_query
    $query = str_replace("%", "%%", $query);

    // Call drupal wrapper.
    return mediamosa_db::db_query($query);
  }

  /**
   * When using SQL_CALC_FOUND_ROWS, use this function
   * to retrieve the value.
   *
   * @return integer
   *  number of rows
   */
  public static function found_rows() {
    return mediamosa_db::db_query("SELECT found_rows()")->fetchField();
  }

/**
   * Create the query part from the a_join array.
   *
   * @param array $a_joins
   *  The join array.
   * @return string
   *  The SQL result.
   */
  public static function query_joins($a_joins) {
    // make sure the using() joins are first
    $a_joins_sorted = self::query_join_collect($a_joins);

    $a_merged = array_merge($a_joins_sorted['a_using'], $a_joins_sorted['a_other']);
    return implode("\n ", $a_merged);
  }

  /**
   * query_joins helper function.
   *
   * @param array $a_joins
   * @param array $a_joins_sorted
   * @return array
   */
  public static function query_join_collect($a_joins, $a_joins_sorted = array('a_using' => array(), 'a_other' => array())) {

    foreach ($a_joins as $mixed_join) {
      if (is_array($mixed_join)) {
        $a_joins_sorted = self::query_join_collect($mixed_join, $a_joins_sorted);
        continue;
      }

      if (stripos($mixed_join, 'using') !== FALSE AND stripos($mixed_join, " on ") === FALSE) {
        $a_joins_sorted['a_using'][] = $mixed_join;
      }
      else {
        $a_joins_sorted['a_other'][] = $mixed_join;
      }
    }

    return $a_joins_sorted;
  }

  /**
   * Simple creation of the select query
   * See media_management_search for example
   *
   * @param array $a_query
   * @return string
   */
  public static function query_select($a_query, $a_options = array(self::SQL_CALC_FOUND_ROWS => FALSE)) {
    assert(isset($a_query[self::A_FROM]));

    $a_query[self::A_SELECT_OPTIONS] = array();
    if (isset($a_options[self::SQL_CALC_FOUND_ROWS]) && $a_options[self::SQL_CALC_FOUND_ROWS]) {
      $a_query[self::A_SELECT_OPTIONS] = array(self::SQL_CALC_FOUND_ROWS);
    }

    if (isset($a_query[self::ALLOW_DISTINCT]) && $a_query[self::ALLOW_DISTINCT] && !isset($a_query[self::A_HAVING])) {
      $a_query[self::A_SELECT_OPTIONS][] = 'DISTINCT';
      unset($a_query[self::A_GROUP_BY]);
    }

    $a_db_query[] = 'SELECT';
    $a_db_query[] = implode(' ', $a_query[self::A_SELECT_OPTIONS]);

    if (!isset($a_query[self::A_SELECT_EXPR]) || empty($a_query[self::A_SELECT_EXPR])) {
      $a_query[self::A_SELECT_EXPR][] = '*';
    }

    $a_db_query[] = implode(',', $a_query[self::A_SELECT_EXPR]);
    $a_db_query[] = 'FROM';
    $a_db_query[] = implode(",\n", $a_query[self::A_FROM]);

    // Joins
    if (isset($a_query[self::A_JOIN])) {
      $a_db_query[] = self::query_joins($a_query[self::A_JOIN]);
    }

    // Where
    if (isset($a_query[self::A_WHERE]) && !empty($a_query[self::A_WHERE])) {
      $a_db_query[] = "\nWHERE";
      $a_db_query[] = self::where($a_query[self::A_WHERE]);
    }

    if (isset($a_query[self::A_GROUP_BY]) && !empty($a_query[self::A_GROUP_BY])) {
      $a_db_query[] = "\nGROUP BY";
      $a_db_query[] = implode(", ", $a_query[self::A_GROUP_BY]);
    }

    if (isset($a_query[self::A_HAVING]) && !empty($a_query[self::A_HAVING])) {
      $a_db_query[] = "\nHAVING";
      $a_db_query[] = implode(", ", $a_query[self::A_HAVING]);
    }

    if (isset($a_query[self::A_ORDER_BY]) && !empty($a_query[self::A_ORDER_BY])) {
      $a_db_query[] = "\nORDER BY";
      $a_db_query[] = implode(", ", $a_query[self::A_ORDER_BY]);
    }

    if (isset($a_query[self::LIMIT])) {
      $a_db_query[] = sprintf("\nLIMIT %d", mediamosa_db::escape_string($a_query[self::LIMIT]));

      if (isset($a_query[self::OFFSET]) && $a_query[self::OFFSET] > 0) {
        $a_db_query[] = sprintf("\nOFFSET %d", mediamosa_db::escape_string($a_query[self::OFFSET]));
      }
    }

    return implode(' ', $a_db_query);
  }

  /**
   * Create a UNION query from supplied queries.
   *
   * @param array $a_queries
   * @param int $limit
   * @param int $offset
   * @param array $a_options
   * @return string
   */
  public static function query_select_union($a_queries, $limit = 0, $offset = 0, $a_options = array(self::SQL_CALC_FOUND_ROWS => TRUE)) {

    $a_db_query = array();
    foreach ($a_queries as $a_query) {
      $a_s_queries[] = self::query_select($a_query, $a_options);
      unset($a_options[self::SQL_CALC_FOUND_ROWS]);
    }

    $a_db_query = array("(" . implode(")\nUNION\n(", $a_s_queries) . ')');

    if ($limit) {
      $a_db_query[] = sprintf("\nLIMIT %d", $limit);

      if ($offset > 0) {
        $a_db_query[] = sprintf("\nOFFSET %d", $offset);
      }
    }

    return implode(' ', $a_db_query);
  }

  /**
   * Returns the last inserted ID when used autoincrement on previous insert
   * query. Make sure you call it inside the db_set_active() as the insert(!).
   *
   * @return integer
   */
  public static function get_last_id() {
    assert(0);
    throw new mediamosa_exception_program_error('Stop using get_last_id()');
    //return (int)db_result(db_query("SELECT LAST_INSERT_ID()"));
  }

  /**
   * Returns either NULL as string or a quoted string with the value or integer when numeric
   * Usefull when creating sprintf("INSERT INTO {foo} SET foo%s", self::query_value($my_var, TRUE));
   * DO NOT USE IN COMBINATION WITH db_query, IT WILL ESCAPE ' AND SCREW THE QUERY, do sprintf 1st
   * We also quote values, because var=24 will match all var rows that start with 24 when its
   * varchar in as column type
   * @param string/integer/NULL $s_value
   * @return string
   */
  public static function query_value($s_value, $is_insert = FALSE) {
    return (is_null($s_value) ? ($is_insert ? ' = NULL' : ' IS NULL') : " = '" . mediamosa_db::escape_string($s_value) . "'");
  }

  /**
   * Returns a valid string that been escaped.
   * Do not use, use mediamosa_db::escape_string instead(!).
   *
   * @param string $str_like
   */
  public static function query_escape($str_like) {
    return mediamosa_db::escape_string($str_like);
  }

  /**
   * Returns a valid escaped string when using like
   * Do not use, use mediamosa_db::escape_string_like instead(!).
   *
   * @param string $str_like
   * @return string
   */
  public static function query_escape_like($str_like) {
    return mediamosa_db::escape_string_like($str_like);
  }

  /**
   * Returns the name of the database.
   *
   * @todo: really should try to get rid of this function.
   *
   * @param $name
   *  The key of $db_url array.
   * @returns
   *  Name of the database.
   */
  public static function query_dbname($name) {
    global $db_url;

    // Check
    if (is_array($db_url)) {
      if (isset($db_url[$name])) {
        $url = $db_url[$name];
      }
      else {
        $url = $db_url['default'];
      }
    }
    else {
      $url = $db_url;
    }

    $url = parse_url($url);
    return mediamosa_unicode::substr(urldecode($url['path']), 1);
  }
}
