<?php
/**
 * @file
 * Extended on the existing mediamosa_core_cql class.
 */

class mediamosa_solr_asset_cql_context extends mediamosa_asset_cql_context {
  /**
   * returns the array containing mapping voor cql2sql
   *
   * @return array
   */
  protected function mediamosa_core_cql_context_array_get() {
    static $mediamosa_context = NULL;

    if (!isset($mediamosa_context)) {
      $mediamosa_context = $this->m_a_mediamosa_context;

      $replace = array(
        'mime_type' => 'mime_type',
        'acl_app' => 'mf_app_id_master',
        'is_master_slaved' => 'asset_is_master_slaved',
        'is_protected' => 'mf_is_protected',
        'acl_app' => 'mf_app_id_master',
        'acl_user' => 'mf_acl_user',
        'acl_group' => 'mf_acl_user_group',
        'acl_domain' => 'mf_acl_domain',
        'acl_realm' => 'mf_acl_realm',
        'acl_realm_prefix' => 'mf_acl_realm_prefix',
      );

      foreach ($mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES] as $context_set => &$lvl1) {
        foreach ($lvl1[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS] as $column_name => $column) {
          if (isset($replace[$column_name])) {
            $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$column_name]['column'] = $replace[$column_name];
          }

          $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$column_name]['table_for_sort'] = $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$column_name]['column'];
        }
      }

      $metadata_properties = mediamosa_asset_metadata_property::get_metadata_properties_full($this->app_ids);

      foreach ($metadata_properties as $name => $metadata_property) {
        $group_name_short = mediamosa_asset_metadata_property_group::property_group_get_context($metadata_property['propgroup_name']);

        switch ($metadata_property['propgroup_name']) {
          case 'qualified_dublin_core':
            $context_set = 'mediamosa:qdc';
            break;

          case 'dublin_core':
            $context_set = 'mediamosa:dc';
            break;

          case 'czp':
            $context_set = 'mediamosa:czp';
            break;

          case 'acl':
          case 'aut':
            $context_set = 'mediamosa:acl';
            break;

          default:
            $context_set = 'mediamosa:' . $metadata_property['propgroup_name'];
            break;
        }

        switch ($metadata_property['propdef_type']) {
          case mediamosa_asset_metadata_property_db::TYPE_INT:
            $column = $group_name_short . '_' . $name . '_vi';
            $sort_column = $group_name_short . '_' . $name . '_svi';
            $type = mediamosa_sdk::TYPE_INT;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;

          case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
            $column = $group_name_short . '_' . $name . '_vd';
            $sort_column = $group_name_short . '_' . $name . '_svd';
            $type = mediamosa_sdk::TYPE_DATETIME;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;

          default:
            assert(0);

          case mediamosa_asset_metadata_property_db::TYPE_CHAR:
            $column = $group_name_short . '_' . $name . '_vc';
            $sort_column = $group_name_short . '_' . $name . '_svc';
            $type = mediamosa_sdk::TYPE_STRING;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;
        }

        $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$name] = array(
          MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => $column,
          MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => $sort_column,
          MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'unused',
          MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => $type,
          // Indication its an asset metadata field.
          MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID => $metadata_property['propdef_id'],
        );
      }
    }

    return $mediamosa_context;
  }

  /**
   * Finalize the relation, index, search term
   *
   * @param array $a_parts
   * @param cql_part $cql_part
   * @param cql_part_relation $o_part_relation
   * @param array $a_result
   * @param string $str_index
   *
   * @return array
   */
  public function mediamosa_core_cql_context_index2column_finalize(mediamosa_cql_part $cql_part, mediamosa_cql_part_relation $cql_part_relation, array &$result, $str_index, $do_having) {

    $cql_part_search_term = $cql_part->mediamosa_cql_part_search_term_get();
    $cql_part_boolean = $cql_part->mediamosa_cql_part_boolean_get();

    $pos = strrpos($str_index, '.');
    if ($pos !== FALSE) {
      $str_index = mediamosa_unicode::substr($str_index, $pos + 1);
    }

    $relation_modifiers = $cql_part_relation->mediamosa_cql_modifier_get_array();

    $index2column = $this->mediamosa_core_cql_context_index2column($str_index);
    if ($index2column === FALSE) {
      throw new mediamosa_cql_exception('index not found, use verify before you parse the cql string.', CQL_EXCEPTION_SEVERITY_HIGH);
    }

    // Solr does not support having.
    $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING] = FALSE;

    // Special case when these keys are used.
    $acl_joins = array(
      'ACL_NAME_USER',
      'ACL_NAME_USER_GROUP',
      'ACL_GROUP_DOMAIN',
      'ACL_GROUP_REALM',
      'ACL_APP_MASTER_SLAVE',
    );
    if (!empty($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_JOIN]) && in_array($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_JOIN], $acl_joins)) {
      $result['is_acl_search'] = TRUE;
    }

    // Is case sensitive only?
    $is_case_sensitive = empty($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_CASE_SENSITIVE]) ? FALSE : TRUE;

    // For some special cases.
    if (isset($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_JOIN])) {
      $result['a_joins'][$str_index] = $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_JOIN];
    }

    // Is asset metadata?
    $is_metadata = empty($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID]) ? FALSE : TRUE;

    $wildcard = '*';

    $str_table_column = $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN];
    if (!in_array(mediamosa_cql::STEM, $relation_modifiers)) {
      $str_table_column_exact = $is_metadata ? $str_table_column . '_exact' : $str_table_column;
      $str_table_column_exact_cs = $is_metadata ? $str_table_column . '_exact_cs' : $str_table_column;
    }
    else {
      $str_table_column_exact = $str_table_column;
      $str_table_column_exact_cs = $str_table_column;
      // Stemming does not use wildcard chars.
      $wildcard = '';
    }

    if (isset($index2column['select_expr'])) {
      $result['a_select_expr'][$str_index] = $index2column['select_expr'];
    }

    // Get the search tern.
    $str_search_term = $cql_part_search_term->mediamosa_cql_part_str_get();

    // NULL values allowed.
    $has_null = isset($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_NULL]) ? $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_NULL] : FALSE;

    $parts =  array();
    $sub_parts = array();

    // Default is OR.
    $str_sub_parts_glue = ' OR ';
    $str_relation = $cql_part_relation->mediamosa_cql_part_str_get();
    $has_not = $cql_part->mediamosa_cql_part_has_not();

    switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
      case mediamosa_sdk::TYPE_INT:
        break;

      case mediamosa_sdk::TYPE_DATETIME:
        $is_case_sensitive = TRUE;

        if ($str_relation != 'within') {
          // Convert to DB Date.
          $str_search_term = $this->mediamosa_cql_str_date_2_db_date($str_search_term);
        }
        break;

      case mediamosa_sdk::TYPE_DOMAIN:
      case mediamosa_sdk::TYPE_REALM:
      case mediamosa_sdk::TYPE_STRING:
        // Do NULL compare?
        if (mediamosa_unicode::strtolower($str_search_term) == 'null') {
          switch ($str_relation) {
            case '<>':
              $str_relation = 'IS NOT NULL';
              break;

            default:
              $str_relation = 'IS NULL';
              break;
          }
        }

        break;

      case mediamosa_sdk::TYPE_BOOL:
      case mediamosa_sdk::TYPE_BOOL_IS_SLAVED:
        $is_case_sensitive = TRUE;

        $str_search_term = (mediamosa_unicode::strtolower($str_search_term) == 'true' ? 'TRUE' : 'FALSE');

        $bool_is_true = $str_search_term == 'TRUE';

        // Lets prevent usage of NOT.
        $bool_is_true = ($str_relation == '<>' ? !$bool_is_true : $bool_is_true);

        // Back into text.
        $str_search_term = $bool_is_true ? 'TRUE' : 'FALSE';

        // Prevent usage of <>.
        $str_relation = 'bool';
        break;

      case mediamosa_sdk::TYPE_JOB_PROGRESS:
        throw new mediamosa_solr_exception('Type TYPE_JOB_PROGRESS is not supported by asset SOLR, job context.');
    }

    // Switch NOT value when <> is found.
    if ($str_relation == '<>') {
      $has_not = !$has_not;
      $str_relation = '=';
    }

    // Case sensitive fix (metadata).
    $str_table_column_exact = in_array(mediamosa_cql::RESPECTCASE, $relation_modifiers) && $is_metadata ? $str_table_column_exact_cs : $str_table_column_exact;

    switch ($str_relation) {
      case 'bool':
        $sub_parts[] = $str_table_column_exact . ':' . mediamosa_solr_apache_solr_service::escape($str_search_term);
        break;

      case 'all':
        $str_sub_parts_glue = ' AND ';

      case '=':
      case 'any':
        if (!$is_case_sensitive && !$is_metadata) {
          $str_search_term = mediamosa_unicode::strtolower($str_search_term);
        }

        // Split whitespace up into words.
        $words = preg_split('/\s+/', $str_search_term);
        foreach ($words as $word) {
          if (trim($word) == '') {
            continue;
          }

          switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
            case mediamosa_sdk::TYPE_INT:
            case mediamosa_sdk::TYPE_UINT:
              // Cast to integer, as spaces will break solr.
              $sub_parts[] = strtr(
                '@column:@value',
                array(
                  '@column' => $str_table_column,
                  '@value' => (int) trim($word),
                )
              );
              break;

            default:
              // Remove trailing ^.
              if (mediamosa_unicode::substr($word, 0, 1) == '^' && mediamosa_unicode::substr($word, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $word), -2) != '\^') {
                $sub_parts[] = strtr(
                  '@column:@value',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($word, 1, -1)),
                  )
                );
              }
              elseif (mediamosa_unicode::substr($word, 0, 1) == '^') {
                // Left ancor.
                $sub_parts[] = strtr(
                  '@column:@value{wildcard}',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($word, 1)),
                    '{wildcard}' => $wildcard,
                  )
                );
              }
              elseif (mediamosa_unicode::substr($word, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $word), -2) != '\^') {
                // Right ancor.
                $sub_parts[] = strtr(
                  '@column:{wildcard}@value',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($word, 0, -1)),
                    '{wildcard}' => $wildcard,
                  )
                );
              }
              else {
                // No ancor search.
                $sub_parts[] = strtr(
                  '@column:{wildcard}@value{wildcard}',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards($word),
                    '{wildcard}' => $wildcard,
                  )
                );
              }
          }
        }
        break;

      case '==':
      case 'adj':
        switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
          case mediamosa_sdk::TYPE_INT:
          case mediamosa_sdk::TYPE_UINT:
            // Cast to integer, as spaces will break solr.
            $sub_parts[] = strtr(
              '@column:@value',
              array(
                '@column' => $str_table_column,
                '@value' => (int) $str_search_term,
              )
            );
            break;

          default:
            if (!$is_case_sensitive && !$is_metadata) {
              $str_search_term = mediamosa_unicode::strtolower($str_search_term);
            }

            if ($is_metadata) {
              // Remove trailing ^.
              if (mediamosa_unicode::substr($str_search_term, 0, 1) == '^' && mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $str_search_term), -2) != '\^') {
                // Add our search.
                $sub_parts[] = strtr(
                  '@column:@value',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($str_search_term, 1, -1)),
                  )
                );
              }
              elseif (mediamosa_unicode::substr($str_search_term, 0, 1) == '^') {
                // Left ancor.
                $sub_parts[] = strtr(
                  '@column:@value{wildcard}',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($str_search_term, 1)),
                    '{wildcard}' => $wildcard,
                  )
                );
              }
              elseif (mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $str_search_term), -2) != '\^') {
                // Right ancor.
                $sub_parts[] = strtr(
                  '@column:{wildcard}@value',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($str_search_term, 0, -1)),
                    '{wildcard}' => $wildcard,
                  )
                );
              }
              else {
                // No ancor search.
                $sub_parts[] = strtr(
                  '@column:{wildcard}@value{wildcard}',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards($str_search_term),
                    '{wildcard}' => $wildcard,
                  )
                );
              }
            }
            else {
              // Remove trailing ^.
              if (mediamosa_unicode::substr($str_search_term, 0, 1) == '^' && mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $str_search_term), -2) != '\^') {
                if ($cql_part_search_term->mediamosa_cql_contains_wildcards($str_search_term)) {
                  $str_search_term = mediamosa_unicode::strtolower($str_search_term);
                }

                $sub_parts[] = strtr(
                  '@column:@value',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($str_search_term, 1, -1)),
                  )
                );
              }
              elseif (mediamosa_unicode::substr($str_search_term, 0, 1) == '^') {
                // Left ancor.
                $sub_parts[] = strtr(
                  '@column:@value{wildcard}',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($str_search_term, 1)),
                    '{wildcard}' => $wildcard,
                  )
                );
              }
              elseif (mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $str_search_term), -2) != '\^') {
                // Right ancor.
                $sub_parts[] = strtr(
                  '@column:{wildcard}@value',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($str_search_term, 0, -1)),
                    '{wildcard}' => $wildcard,
                  )
                );
              }
              else {
                // No ancor search.
                $sub_parts[] = strtr(
                  '@column:{wildcard}@value{wildcard}',
                  array(
                    '@column' => $str_table_column_exact,
                    '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards($str_search_term),
                    '{wildcard}' => $wildcard,
                  )
                );
              }
            }
        }
        break;

      case '<>':
        $has_not = !$has_not;
        $sub_parts = strtr(
          '@column:@value',
          array(
            '@column' => $str_table_column_exact,
            '@value' => mediamosa_solr_apache_solr_service::escapeForWildcards($str_search_term),
          )
        );
        break;

      case '>':
        $sub_parts = array(strtr(
          '@column:{@value TO *}',
          array(
            '@column' => $str_table_column,
            '@value' => mediamosa_solr_apache_solr_service::escapeForSpace($str_search_term),
          )
        ));
        break;

      case '>=':
        $sub_parts = array(strtr(
          '@column:[@value TO *]',
          array(
            '@column' => $str_table_column,
            '@value' => mediamosa_solr_apache_solr_service::escapeForSpace($str_search_term),
          )
        ));
        break;

      case '<':
        $sub_parts = array(strtr(
          '@column:{* TO @value}',
          array(
            '@column' => $str_table_column,
            '@value' => mediamosa_solr_apache_solr_service::escapeForSpace($str_search_term),
          )
        ));
        break;

      case '<=':
        $sub_parts = array(strtr(
          '@column:[* TO @value]',
          array(
            '@column' => $str_table_column,
            '@value' => mediamosa_solr_apache_solr_service::escapeForSpace($str_search_term))
        ));
        break;

      case 'within':
        // Split up.
        $values = explode(' ', $str_search_term);

        switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
          case mediamosa_sdk::TYPE_DATETIME:
            $values[0] = $this->mediamosa_cql_str_date_2_db_date($values[0]);
            $values[1] = $this->mediamosa_cql_str_date_2_db_date($values[1]);
            break;

          default:
            break;
        }

        $sub_parts = array(strtr('@column:[@value_1 TO @value_2]', array('@column' => $str_table_column, '@value_1' => mediamosa_solr_apache_solr_service::escape($values[0]), '@value_2' => mediamosa_solr_apache_solr_service::escape($values[1]))));
        break;

      // Special cases.
      case 'IS NULL':
        $sub_parts = array(strtr('-@column:*', array('@column' => $str_table_column)));
        break;

      case 'IS NOT NULL':
        $sub_parts = array(strtr('@column:*', array('@column' => $str_table_column)));
        break;

    }

    if ($cql_part_boolean) {
      $str_boolean = $cql_part_boolean->mediamosa_cql_part_str_get();
      $str_boolean = ($str_boolean == 'NOT' ? ' AND ' : ' ' . $str_boolean . ' ');
    }

    // Into one.
    $parts[] = (count($sub_parts) < 2 ? '' : '(') . implode($str_sub_parts_glue, $sub_parts) . (count($sub_parts) < 2 ? '' : ')');

    return ($has_not ? '-' : '') . (count($parts) < 2 ? '' : '(') . implode (' OR ', $parts) . (count($parts) < 2 ? '' : ')') . (empty($str_boolean) ? '' : $str_boolean);
  }

  /**
   * Converts a any date into a db Solr date (YYYY-MM-DDTHH:MM:SS)
   *
   * Will convert to timezone set in CQL object.
   *
   * Overridden for fixing format problems with Solr.
   *
   * @param string $date
   *  The paramter to convert.
   * @param string $timezone
   *  Supply timezone for conversion.
   */
  protected function mediamosa_cql_str_date_2_db_date($date, $timezone = 'UTC', $format = 'Y-m-d\TH:i:s\Z') {
    return parent::mediamosa_cql_str_date_2_db_date($date, $timezone, $format);
  }
}

class mediamosa_solr_cql extends mediamosa_core_cql {

  /**
   * Start parsing the CQL for asset search.
   *
   * @param string $str_cql
   * @param array $app_ids
   */
  public static function parse_asset($str_cql, $cql_store_stats, array $app_ids) {

    // Create the CQL object.
    $mediamosa_cql = new mediamosa_solr_asset_cql_context($app_ids);
    return $mediamosa_cql->mediamosa_core_parse_cql($str_cql, $cql_store_stats);
  }
}
