<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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

      $a_defs = mediamosa_asset_metadata_property::get_metadata_properties_full($this->app_ids);

      foreach ($a_defs as $name => $a_def) {
        switch ($a_def['propgroup_name']) {
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
            $context_set = 'mediamosa:' . $a_def['propgroup_name'];
            break;
        }

        switch ($a_def['propdef_type']) {
          case mediamosa_asset_metadata_property_db::TYPE_INT:
            $column = $name . '_vi';
            $sort_column = $name . '_svi';
            $type = mediamosa_sdk::TYPE_INT;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;
          case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
            $column = $name . '_vd';
            $sort_column = $name . '_svd';
            $type = mediamosa_sdk::TYPE_DATETIME;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;
          default:
            assert(0);
          case mediamosa_asset_metadata_property_db::TYPE_CHAR:
            $column = $name . '_vc';
            $sort_column = $name . '_svc';
            $type = mediamosa_sdk::TYPE_STRING;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;
        }

        $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$name] = array(
          MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => $column,
          MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => $sort_column,
          MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'unused',
          MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => $type,
          MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID => $a_def['propdef_id'], // indication its a asset metadata field.
//          MEDIAMOSA_CQL_CONTEXT_KEY_JOIN => sprintf('LEFT JOIN {mediamosa_asset_metadata} AS mediamosa_asset_metadata_%s ON mediamosa_asset_metadata_%s.asset_id = a.asset_id AND mediamosa_asset_metadata_%s.prop_id = %d', $name, $name, $name, $a_def['propdef_id']),
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
      $str_index = mediamosa_unicode::substr($str_index, $pos + 1); // default context set id
    }

    $a_relation_modifiers = $cql_part_relation->mediamosa_cql_modifier_get_array();

    $index2column = $this->mediamosa_core_cql_context_index2column($str_index);
    assert($index2column);// use verify 1st so this will not happen

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

    $str_table_column = $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN];
    $str_table_column_exact = $is_metadata ? $str_table_column . '_exact' : $str_table_column;

    if (isset($index2column['select_expr'])) {
      $result['a_select_expr'][$str_index] = $index2column['select_expr'];
    }

    // Get the search tern.
    $str_search_term = $cql_part_search_term->mediamosa_cql_part_str_get();

    // NULL values allowed.
    $has_null = isset($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_NULL]) ? $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_NULL] : FALSE;

    $parts_or = $parts = $sub_parts = array();

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

        $str_search_term = (mediamosa_unicode::strtolower($str_search_term) == "true" ? "TRUE" : "FALSE");

        $bool_is_true = $str_search_term == "TRUE";

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

    switch ($str_relation) {
      case 'bool':
        $sub_parts[] = $str_table_column . ':' . mediamosa_solr_apache_solr_service::escape($str_search_term);
        break;

      case 'all':
        $str_sub_parts_glue = ' AND ';

      case '=':
      case 'any':
        if (!$is_case_sensitive) {
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
              $sub_parts[] = $str_table_column . ':' . mediamosa_solr_apache_solr_service::escape($word);
              break;

            default:
              // Remove trailing ^.
              if (mediamosa_unicode::substr($word, 0, 1) == '^' && mediamosa_unicode::substr($word, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $word), -2) != '\^') {
                $sub_parts[] = $str_table_column_exact . ':' . mediamosa_solr_apache_solr_service::escapeForSpace(mediamosa_unicode::substr($word, 1, -1));
              }
              elseif (mediamosa_unicode::substr($word, 0, 1) == '^') {
                // Left ancor.
                $sub_parts[] = $str_table_column_exact . ':' . mediamosa_solr_apache_solr_service::escapeForSpace(mediamosa_unicode::substr($word, 1)) . '*';
              }
              elseif (mediamosa_unicode::substr($word, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $word), -2) != '\^') {
                // Right ancor.
                $sub_parts[] = $str_table_column_exact . ':*' . mediamosa_solr_apache_solr_service::escapeForSpace(mediamosa_unicode::substr($word, 0, -1));
              }
              else {
                // No ancor search.
                $sub_parts[] = $str_table_column . ':*' . mediamosa_solr_apache_solr_service::escapeForSpace($word) . '*';
              }
          }
        }
        break;

      case '==':
      case 'adj':
        switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
          case mediamosa_sdk::TYPE_INT:
          case mediamosa_sdk::TYPE_UINT:
            $sub_parts[] = $str_table_column . ':' . mediamosa_solr_apache_solr_service::escape($str_search_term);
            break;

          default:
            if (!$is_case_sensitive) {
              $str_search_term = mediamosa_unicode::strtolower($str_search_term);
            }

            if ($is_metadata) {
              // Remove trailing ^.
              if (mediamosa_unicode::substr($str_search_term, 0, 1) == '^' && mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $str_search_term), -2) != '\^') {
                $sub_parts[] = $str_table_column_exact . ':' . mediamosa_solr_apache_solr_service::escapeForSpace(mediamosa_unicode::substr($str_search_term, 1, -1));
              }
              elseif (mediamosa_unicode::substr($str_search_term, 0, 1) == '^') {
                // Left ancor.
                $sub_parts[] = $str_table_column_exact . ':' . mediamosa_solr_apache_solr_service::escapeForSpace(mediamosa_unicode::substr($str_search_term, 1)) . '*';
              }
              elseif (mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $str_search_term), -2) != '\^') {
                // Right ancor.
                $sub_parts[] = $str_table_column_exact . ':*' . mediamosa_solr_apache_solr_service::escapeForSpace(mediamosa_unicode::substr($str_search_term, 0, -1));
              }
              else {
                // No ancor search.
                $sub_parts[] = $str_table_column . ':' . mediamosa_solr_apache_solr_service::escapeForSpace('*' . $str_search_term . '*');
              }
            }
            else {
              // Remove trailing ^.
              if (mediamosa_unicode::substr($str_search_term, 0, 1) == '^' && mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $str_search_term), -2) != '\^') {
                if ($cql_part_search_term->mediamosa_cql_contains_wildcards($str_search_term)) {
                  $str_search_term = mediamosa_unicode::strtolower($str_search_term);
                }

                $sub_parts[] = $str_table_column . ':' . mediamosa_solr_apache_solr_service::escapeForSpace(mediamosa_unicode::substr($str_search_term, 1, -1));
              }
              elseif (mediamosa_unicode::substr($str_search_term, 0, 1) == '^') {
                // Left ancor.
                $sub_parts[] = $str_table_column . ':' . mediamosa_solr_apache_solr_service::escapeForSpace(mediamosa_unicode::substr($str_search_term, 1)) . '*';
              }
              elseif (mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $str_search_term), -2) != '\^') {
                // Right ancor.
                $sub_parts[] = $str_table_column . ':*' . mediamosa_solr_apache_solr_service::escapeForSpace(mediamosa_unicode::substr($str_search_term, 0, -1));
              }
              else {
                // No ancor search.
                $sub_parts[] = $str_table_column . ':*' . mediamosa_solr_apache_solr_service::escapeForSpace($str_search_term) . '*';
              }
            }
        }
        break;

      case '<>':
        $has_not = !$has_not;
        $sub_parts = array($str_table_column . ':"' . $str_search_term . '"');
        break;

      case '>':
        $sub_parts = array(strtr('@field:{@value TO *}', array('@field' => $str_table_column, '@value' => mediamosa_solr_apache_solr_service::escape($str_search_term))));
        break;

      case '>=':
        $sub_parts = array(strtr('@field:[@value TO *]', array('@field' => $str_table_column, '@value' => mediamosa_solr_apache_solr_service::escape($str_search_term))));
        break;

      case '<':
        $sub_parts = array(strtr('@field:{* TO @value}', array('@field' => $str_table_column, '@value' => mediamosa_solr_apache_solr_service::escape($str_search_term))));
        break;

      case '<=':
        $sub_parts = array(strtr('@field:[* TO @value]', array('@field' => $str_table_column, '@value' => mediamosa_solr_apache_solr_service::escape($str_search_term))));
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

        $sub_parts = array(strtr('@field:[@value_1 TO @value_2]', array('@field' => $str_table_column, '@value_1' => mediamosa_solr_apache_solr_service::escape($values[0]), '@value_2' => mediamosa_solr_apache_solr_service::escape($values[1]))));
        break;

      // Special cases.
      case 'IS NULL':
        $sub_parts = array(strtr('-@field:*', array('@field' => $str_table_column)));
        break;

      case 'IS NOT NULL':
        $sub_parts = array(strtr('@field:*', array('@field' => $str_table_column)));
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
  static public function parse_asset($str_cql, $cql_store_stats, array $app_ids) {

    // Create the CQL object.
    $mediamosa_cql = new mediamosa_solr_asset_cql_context($app_ids);
    return $mediamosa_cql->mediamosa_core_parse_cql($str_cql, $cql_store_stats);
  }
}

