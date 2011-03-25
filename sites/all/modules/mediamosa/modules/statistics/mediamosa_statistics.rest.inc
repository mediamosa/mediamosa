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
 * REST calls statistics.
 */

/**
 * URI: /statistics/newestmediafiles
 * Method: GET
 *
 * 1.x: vpx_statistics_get_newest_mediafiles
 *
 */
class mediamosa_rest_call_statistics_get_newest_mediafiles extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with limit, offset.
    $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX, mediamosa_settings::LIMIT_DEFAULT_STATISTICS);

    // Enrich with order by.
    $var_setup = self::get_var_setup_order_by($var_setup, '', array(), mediamosa_type::ORDER_DIRECTION_DESC);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    $order_by = $this->get_param_value_order_by();
    $order_direction = $this->get_param_value_order_direction();
    $limit = $this->get_param_value_limit();
    $offset =  $this->get_param_value_offset();

    // Allowed Order By List
    $a_order_by = array(
      'created' => array('column' => 'mf.created'),
      'owner_id' => array('column' => 'mf.owner_id'),
      'filename' => array('column' => 'mf.filename'),
      'mediafile_id' => array('column' => 'mf.mediafile_id'),
      'app_id' => array('column' => 'a.app_id'),
      'asset_id' => array('column' => 'a.asset_id'),
      'container_type' => array('column' => 'mfm.container_type'),
      'filesize' => array('column' => 'mfm.filesize'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'created';
    }

    $a_query = array();

    // First query only retrieves the mediafile_ids
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_asset} AS a';

    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'a.asset_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'a.app_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'a.owner_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'a.group_id';

    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'mf.mediafile_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'mf.filename';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'mf.created';

    // app_id...
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("a.app_id IN(%s)", mediamosa_db::escape_string(implode(',', $app_ids)));

    // Only parent assets
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = "a.parent_id IS NULL";

    // Dont count private assets
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = "a.isprivate = 'FALSE'";

    // Dont count unappropiate assets
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = "a.is_unappropriate = 'FALSE'";

    // Join the mediafiles
    $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON mf.asset_id = a.asset_id";

    // MF must be original
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = "mf.is_original_file='TRUE'";

    // Order by....
    $a_query[mediamosa_db_query::A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit...
    $a_query[mediamosa_db_query::LIMIT] = $limit;

     // Offset...
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Do query
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => TRUE));

    $result = mediamosa_db::db_query($query);

    // Set total.
    $o_mediamosa->item_count_total = mediamosa_db_query::found_rows();

    // Fill items.
    foreach ($result as $row) {
      // Join the metadata stuff
      $mediafile_id = $row[mediamosa_asset_mediafile_db::ID];
      $row[mediamosa_asset_mediafile_metadata::CONTAINER_TYPE] = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::CONTAINER_TYPE);
      $row[mediamosa_asset_mediafile_metadata::FILESIZE] = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($mediafile_id, mediamosa_asset_mediafile_metadata::FILESIZE);

      // UTC -> App timezone.
      $row[mediamosa_asset_mediafile_db::CREATED] = mediamosa::utcdate2appdate($row[mediamosa_asset_mediafile_db::CREATED]);

      $o_mediamosa->add_item($row);
    }
  }
}

/**
 * URI: /statistics/popularmediafiles
 * Method: GET
 *
 * Deprecated.
 * 1.x: vpx_statistics_get_most_popular_mediafiles
 *
 */
class mediamosa_rest_call_statistics_get_most_popular_mediafiles extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with limit, offset.
    $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX, mediamosa_settings::LIMIT_DEFAULT_STATISTICS);

    // Enrich with order by.
    $var_setup = self::get_var_setup_order_by($var_setup, '', array(), mediamosa_type::ORDER_DIRECTION_DESC);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // TODO

    /*
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $order_by = $this->get_param_value_order_by();
    $order_direction = $this->get_param_value_order_direction();
    $limit = $this->get_param_value_limit();
    $offset =  $this->get_param_value_offset();

    // Allowed Order By List
    $a_order_by = array(
      'count' => array('column' => 'count'),
      'owner_id' => array('column' => 'ssr.owner_id'),
      'group_id' => array('column' => 'ssr.group_id'),
      'filesize' => array('column' => 'ssr.filesize'),
      'container_type' => array('column' => 'ssr.container_type'),
      'app_id' => array('column' => 'ssr.app_id'),
      'created' => array('column' => 'mf.created'),
      'filename' => array('column' => 'mf.filename'),
      'asset_id' => array('column' => 'ssr.asset_id'),
      'mediafile_id' => array('column' => 'ssr.mediafile_id'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'count';
    }

    $a_query = array();
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_statistics_stream_request} AS ssr';

    // Join the mediafiles
    $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf USING(mediafile_id)";

    // Select these....
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'ssr.mediafile_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'ssr.asset_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'ssr.app_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'ssr.owner_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'ssr.group_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'ssr.container_type';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'ssr.filesize';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'COUNT(ssr.mediafile_id) AS count';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'mf.filename';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'mf.created';

    // App_id.
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf('ssr.app_id IN(%s)', mediamosa_db::escape_string(implode(',', $app_ids)));

    // Group by.
    $a_query[mediamosa_db_query::A_GROUP_BY][] = 'ssr.mediafile_id';

    // Order by....
    $a_query[mediamosa_db_query::A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit.
    $a_query[mediamosa_db_query::LIMIT] = $limit;

     // Offset.
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Do query
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => TRUE));
    $result = mediamosa_db::db_query($query);

    // Set total.
    $o_mediamosa->item_count_total = mediamosa_db_query::found_rows();

    // Fill items.
    foreach ($result as $row) {
      // UTC -> App timezone.
      $row[mediamosa_asset_mediafile_db::CREATED] = mediamosa::utcdate2appdate($row[mediamosa_asset_mediafile_db::CREATED]);

      $o_mediamosa->add_item($row);
    }
    */
  }
}

/**
 * URI: /statistics/newestcollections
 * Method: GET
 *
 * 1.x: vpx_statistics_get_newest_collections
 *
 */
class mediamosa_rest_call_statistics_get_newest_collections extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with limit, offset.
    $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX, mediamosa_settings::LIMIT_DEFAULT_STATISTICS);

    // Enrich with order by.
    $var_setup = self::get_var_setup_order_by($var_setup, '', array(), mediamosa_type::ORDER_DIRECTION_DESC);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $order_by = $this->get_param_value_order_by();
    $order_direction = $this->get_param_value_order_direction();
    $limit = $this->get_param_value_limit();
    $offset =  $this->get_param_value_offset();

    // Allowed Order By List
    $a_order_by = array(
      'app_id' => array('column' => 'c.app_id'),
      'name' => array('column' => 'c.title'),
      'owner_id' => array('column' => 'c.owner_id'),
      'created' => array('column' => 'c.created'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'created';
    }

    $a_query = array();
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_collection} AS c';

    // Select this
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.coll_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.app_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.owner_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.title';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.description';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.created';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.changed';

    // App_id.
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf('c.app_id IN(%s)', mediamosa_db::escape_string(implode(',', $app_ids)));

    // Order by....
    $a_query[mediamosa_db_query::A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit.
    $a_query[mediamosa_db_query::LIMIT] = $limit;

     // Offset.
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Do query
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => TRUE));
    $result = mediamosa_db::db_query($query);

    // Set total.
    $o_mediamosa->item_count_total = mediamosa_db_query::found_rows();

    // Fill items.
    foreach ($result as $row) {
      // UTC -> App timezone.
      $row[mediamosa_collection_db::CREATED] = mediamosa::utcdate2appdate($row[mediamosa_collection_db::CREATED]);
      $row[mediamosa_collection_db::CHANGED] = mediamosa::utcdate2appdate($row[mediamosa_collection_db::CHANGED]);

      $o_mediamosa->add_item($row);
    }
  }
}

/**
 * URI: /statistics/popularcollections
 * Method: GET
 *
 * Deprecated.
 * 1.x: vpx_statistics_get_most_popular_collections
 *
 */
class mediamosa_rest_call_statistics_get_most_popular_collections extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with limit, offset.
    $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX, mediamosa_settings::LIMIT_DEFAULT_STATISTICS);

    // Enrich with order by.
    $var_setup = self::get_var_setup_order_by($var_setup, '', array(), mediamosa_type::ORDER_DIRECTION_DESC);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // TODO

    /*
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $order_by = $this->get_param_value_order_by();
    $order_direction = $this->get_param_value_order_direction();
    $limit = $this->get_param_value_limit();
    $offset =  $this->get_param_value_offset();

    // Allowed Order By List
    $a_order_by = array(
      'count' => array('column' => 'count'),
      'coll_id' => array('column' => 'c.coll_id'),
      'title' => array('column' => 'c.title'),
      'description' => array('column' => 'c.description'),
      'app_id' => array('column' => 'c.app_id'),
      'owner_id' => array('column' => 'c.owner_id'),
      'created' => array('column' => 'c.created'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'count';
    }

    // Main select
    $a_query = array();
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_collection} AS c';

    // Join the asset_collection
    $a_query[mediamosa_db_query::A_JOIN]['asset_collection'] = "JOIN {mediamosa_asset_collection} AS ac USING(coll_id)";

    // Join the asset_collection
    $a_query[mediamosa_db_query::A_JOIN]['statistics_stream_request'] = "JOIN {mediamosa_statistics_stream_request} AS ssr USING(asset_id)";

    // Select this
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.coll_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.title';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.description';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.app_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.owner_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.created';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'COUNT(*) AS count';

    // Correct app_id(s) if given
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf('c.app_id IN(%s)', mediamosa_db::escape_string(implode(',', $app_ids)));

    // Group by
    $a_query[mediamosa_db_query::A_GROUP_BY][] = 'c.coll_id';

    // Order by....
    $a_query[mediamosa_db_query::A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit.
    $a_query[mediamosa_db_query::LIMIT] = $limit;

     // Offset.
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Do query
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => TRUE));

    $result = mediamosa_db::db_query($query);

    // Set total.
    $o_mediamosa->item_count_total = mediamosa_db_query::found_rows();

    // Fill items.
    foreach ($result as $row) {
      // UTC -> App timezone.
      $row[mediamosa_collection_db::CREATED] = mediamosa::utcdate2appdate($row[mediamosa_collection_db::CREATED]);

      $o_mediamosa->add_item($row);
    }
    */
  }
}

/**
 * URI: /statistics/dataupload
 * Method: POST
 *
 * 1.x: vpx_statistics_set_historical_uploaded_mediafiles
 *
 * Internal Only.
 *
 */
class mediamosa_rest_call_statistics_set_historical_uploaded_mediafiles extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const FILE_SIZE = 'file_size';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Owner of the upload.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Group of the upload.',
          self::VAR_DEFAULT_VALUE => '',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::FILE_SIZE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The file size of the upload.',
        ),
      )
    );
    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $user_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $file_size = $this->get_param_value(self::FILE_SIZE);

    mediamosa_statistics::log_event_file_upload($app_id, $user_id, $group_id, $file_size);

    $o_mediamosa->set_result_okay();
  }
}


/**
 * URI: /statistics/dataupload
 * Method: GET
 *
 * 1.x: vpx_statistics_get_historical_uploaded_mediafiles
 *
 */
class mediamosa_rest_call_statistics_get_historical_uploaded_mediafiles extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const MONTH = 'month';
  const YEAR = 'year';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::MONTH => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The month of the query.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_START => 1,
          self::VAR_RANGE_END => 12,
        ),
        self::YEAR => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The year of the query.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_START => 2000,
          self::VAR_RANGE_END => 2099,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Owner of the upload.',
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Group of the upload.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
      )
    );

    // Enrich with limit, offset.
    $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX, mediamosa_settings::LIMIT_DEFAULT_STATISTICS_NO_PAGER);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $year = $this->get_param_value(self::YEAR);
    $month = $this->get_param_value(self::MONTH);
    $user_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);

    $limit = $this->get_param_value_limit();
    $offset =  $this->get_param_value_offset();

    $date_begin = strtotime($year . '-' . $month . '-1 00:00:00');
    $date_end   = strtotime('+1 MONTH', $date_begin);

    $a_query = array();
    $a_query[mediamosa_db_query::A_FROM][] = "{mediamosa_statistics_file_upload}";
    $a_query[mediamosa_db_query::A_SELECT_EXPR] = array('app_id', 'owner_id', 'group_id', 'file_size', 'timestamp');

    $a_query[mediamosa_db_query::A_WHERE][] = 'app_id=' . intval($app_id);

    // FIXME: timestamp is now UTC.
    $a_query[mediamosa_db_query::A_WHERE][] = 'UNIX_TIMESTAMP(timestamp) >= ' . $date_begin;
    $a_query[mediamosa_db_query::A_WHERE][] = 'UNIX_TIMESTAMP(timestamp) < ' . $date_end;

    if (!is_null($user_id)) {
      $a_query[mediamosa_db_query::A_WHERE][] = sprintf("owner_id = '%s'", mediamosa_db::escape_string($user_id));
    }

    if (!is_null($group_id)) {
      $a_query[mediamosa_db_query::A_WHERE][] = sprintf("group_id = '%s'", mediamosa_db::escape_string($group_id));
    }

    // FIXME: Seem to be missing order by (!)

    // Limit.
    $a_query[mediamosa_db_query::LIMIT] = $limit;

     // Offset.
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Do query
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => FALSE));
    $result = mediamosa_db::db_query($query);

    // Fill items.
    foreach ($result as $row) {
      // UTC -> App timezone.
      $row[mediamosa_statistics_file_upload_db::TIMESTAMP] = mediamosa::utcdate2appdate($row[mediamosa_statistics_file_upload_db::TIMESTAMP]);

      $o_mediamosa->add_item($row);
    }
  }
}

/**
 * URI: /statistics/datausagevideo
 * Method: GET
 *
 * 1.x: vpx_statistics_get_used_diskspace
 *
 */
class mediamosa_rest_call_statistics_get_used_diskspace extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MONTH = 'month';
  const YEAR = 'year';
  const TYPE = 'type';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::MONTH => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The month of the query.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_START => 1,
          self::VAR_RANGE_END => 12,
        ),
        self::YEAR => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The year of the query.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_START => 1970,
          self::VAR_RANGE_END => 2099,
        ),
        self::TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The type.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_statistics_diskspace_used_db::TYPE_CONTAINER,
            mediamosa_statistics_diskspace_used_db::TYPE_GROUP,
            mediamosa_statistics_diskspace_used_db::TYPE_USER,
          ),
        ),
      )
    );

    // Enrich with limit, offset.
    $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX, mediamosa_settings::LIMIT_DEFAULT_STATISTICS);

    // Enrich with order by.
    $var_setup = self::get_var_setup_order_by($var_setup, '', array(), mediamosa_type::ORDER_DIRECTION_ASC);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $order_by = $this->get_param_value_order_by();
    $order_direction = $this->get_param_value_order_direction();
    $limit = $this->get_param_value_limit();
    $offset =  $this->get_param_value_offset();

    $type = $this->get_param_value(self::TYPE);
    $month = $this->get_param_value(self::MONTH);
    $year = $this->get_param_value(self::YEAR);

    // Allowed Order By List
    $a_order_by = array(
      'diskspace_mb' => array('column' => 'diskspace_mb'),
      'type' => array('column' => 'type'),
      'app_id' => array('column' => 'app_id'),
      'container_type' => array('column' => 'container_type'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'app_id';
    }

    $a_query = array();
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_statistics_diskspace_used}';

    // Select this.
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'container_type';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = sprintf('keyword AS %s_id', mediamosa_db::escape_string($type));
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'type';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'app_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'diskspace AS diskspace_mb';

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("app_id IN(%s)", mediamosa_db::escape_string(implode(",", $app_ids)));

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("type = '%s'", mediamosa_db::escape_string($type));

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("timestamp BETWEEN '%04d-%02d-01' AND '%04d-%02d-01'", $year, $month, ($month == 12 ? $year + 1 : $year), ($month == 12 ? 1 : $month + 1));

    // Order by.
    $a_query[mediamosa_db_query::A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit.
    $a_query[mediamosa_db_query::LIMIT] = $limit;

     // Offset.
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Do query.
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => TRUE));
    $result = mediamosa_db::db_query($query);

    // Set total.
    $o_mediamosa->item_count_total = mediamosa_db_query::found_rows();

    // Fill items.
    foreach ($result as $row) {
      if ($type == 'container') {
        unset($row['container_id']);
      }

      $o_mediamosa->add_item($row);
    }

    // Total disk usage.
    // Data.
    $a_query_disk_usage = $a_query;
    unset($a_query_disk_usage[mediamosa_db_query::A_SELECT_EXPR]);
    $a_query_disk_usage[mediamosa_db_query::A_SELECT_EXPR][] = 'sum(diskspace) as total_disk_usage';
    unset($a_query_disk_usage[mediamosa_db_query::A_ORDER_BY]);
    unset($a_query_disk_usage[mediamosa_db_query::LIMIT]);
    unset($a_query_disk_usage[mediamosa_db_query::OFFSET]);
    // Do query.
    $query_disk_usage = mediamosa_db_query::query_select($a_query_disk_usage);
    $result = mediamosa_db::db_query($query_disk_usage);
    // Add data.
    $statistics_data = array(
      'total_disk_usage_mb' => $result->fetchField(),
    );
    $o_mediamosa->set_result_header_extra($statistics_data, 'statistics');
  }
}

/**
 * URI: /statistics/playedstreams
 * Method: GET
 *
 * 1.x: vpx_statistics_get_requested_streams
 *
 */
class mediamosa_rest_call_statistics_get_requested_streams extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MONTH = 'month';
  const YEAR = 'year';
  const TYPE = 'type';
  const OWNER_ID = 'owner_id';
  const GROUP_ID = 'group_id';
  const PLAY_TYPE = 'play_type';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::MONTH => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The month of the query.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_START => 1,
          self::VAR_RANGE_END => 12,
        ),
        self::YEAR => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The year of the query.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_START => 2000,
          self::VAR_RANGE_END => 2099,
        ),
      )
    );

        // Enrich with limit, offset.
    $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX, mediamosa_settings::LIMIT_DEFAULT_STATISTICS_NO_PAGER);

    // Enrich with order by.
    $var_setup = self::get_var_setup_order_by($var_setup, '', array(), mediamosa_type::ORDER_DIRECTION_DESC);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $order_by = $this->get_param_value_order_by();
    $order_direction = $this->get_param_value_order_direction();
    $limit = $this->get_param_value_limit();
    $offset =  $this->get_param_value_offset();

    $month = $this->get_param_value(self::MONTH);
    $year = $this->get_param_value(self::YEAR);

    // Allowed Order By List
    $a_order_by = array(
      'app_id' => array('column' => 'app_id'),
      'time' => array('column' => 'time'),
      'mediafile_id' => array('column' => 'mediafile_id'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'time';
    }

    $a_query = array();
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_statistics_stream_request}';

    // Select this.
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = '*';

    // Correct app_id(s) if given.
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("app_id IN(%s)", mediamosa_db::escape_string(implode(",", $app_ids)));

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("time BETWEEN '%04d-%02d-01' AND '%04d-%02d-01'", $year, $month, ($month == 12 ? $year + 1 : $year), ($month == 12 ? 1 : $month + 1));

    // Order by.
    $a_query[mediamosa_db_query::A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit.
    $a_query[mediamosa_db_query::LIMIT] = $limit;

     // Offset.
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Do query.
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => FALSE));
    $result = mediamosa_db::db_query($query);

    // Fill items.
    foreach ($result as $row) {
      // UTC -> App timezone.
      $row[mediamosa_statistics_stream_request_db::TIME] = mediamosa::utcdate2appdate($row[mediamosa_statistics_stream_request_db::TIME]);

      $o_mediamosa->add_item($row);
    }
  }
}

/**
 * URI: /cron/statistics
 * Method: GET
 */
class mediamosa_rest_call_cron_statistics extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $run_last = variable_get('mediamosa_statistics_cron_last', NULL);
    if ($run_last) {
      if (round(REQUEST_TIME - $run_last) < mediamosa_settings::STATISTICS_LOG_MAX_TIME) {
        return; // Still running.
      }
      else {
        // Break the cron semaphore.
        mediamosa_watchdog::log('The streaming statistics cron was time outed.', array(), WATCHDOG_ERROR, 'statistics');
      }
    }

    // If we get here then we can go.
    variable_set('mediamosa_statistics_cron_last', REQUEST_TIME);

    mediamosa_statistics::cron();

    // Done.
    variable_del('mediamosa_statistics_cron_last');
  }
}

/**
 * URI: /statistics/streams
 * Method: GET
 *
 * Deprecated.
 * 1.x: vpx_statistics_get_most_popular_streams
 *
 */
class mediamosa_rest_call_statistics_get_most_popular_streams extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const BEGINDATE = 'begindate';
  const ENDDATE = 'enddate';
  const OWNER_ID = 'owner_id';
  const GROUP_ID = 'group_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::BEGINDATE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME,
          self::VAR_DESCRIPTION => 'Begin date of the period.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::ENDDATE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME,
          self::VAR_DESCRIPTION => 'End date of the period.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::OWNER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Owner of the upload.',
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Group of the upload.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
      )
    );

    // Enrich with limit, offset.
    $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX, mediamosa_settings::LIMIT_DEFAULT_STATISTICS_NO_PAGER);

    // Enrich with order by.
    $var_setup = self::get_var_setup_order_by($var_setup, '', array(), mediamosa_type::ORDER_DIRECTION_DESC);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // TODO

    /*
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $order_by = $this->get_param_value_order_by();
    $order_direction = $this->get_param_value_order_direction();
    $limit = $this->get_param_value_limit();
    $offset =  $this->get_param_value_offset();

    $owner_id = $this->get_param_value(self::OWNER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $begindate = $this->get_param_value(self::BEGINDATE);
    $enddate = $this->get_param_value(self::ENDDATE);

    // Allowed Order By List
    $a_order_by = array(
      'app_id' => array('column' => 'ssr.app_id'),
      'owner_id' => array('column' => 'ssr.owner_id'),
      'group_id' => array('column' => 'ssr.group_id'),
      'filename' => array('column' => 'mf.filename'),
      'requested' => array('column' => 'requested'),
      'mediafile_id' => array('column' => 'ssr.mediafile_id'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'requested';
    }

    $a_query = array();
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_statistics_stream_request} AS ssr';

    // Join with mediafile
    $a_query[mediamosa_db_query::A_JOIN][] = 'LEFT JOIN {mediamosa_asset_mediafile} AS mf USING(mediafile_id)';

    // Select this
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'ssr.mediafile_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'ssr.owner_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'ssr.group_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'mf.filename';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'mf.asset_id AS asset_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'count(*) AS requested';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'ssr.app_id';

    // App_id.
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("ssr.app_id IN(%s)", mediamosa_db::escape_string(implode(",", $app_ids)));

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("ssr.played BETWEEN '%s' AND '%s'", mediamosa_db::escape_string($begindate), mediamosa_db::escape_string($enddate));

    if ($owner_id) {
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("ssr.owner_id='%s'", mediamosa_db::escape_string($owner_id));
    }

    if ($group_id) {
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("ssr.group_id='%s'", mediamosa_db::escape_string($group_id));
    }

    // Group by
    $a_query[mediamosa_db_query::A_GROUP_BY][] = 'ssr.mediafile_id';

    // Order by.
    $a_query[mediamosa_db_query::A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit.
    $a_query[mediamosa_db_query::LIMIT] = $limit;

     // Offset.
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Do query.
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => FALSE));
    $result = mediamosa_db::db_query($query);

    // Fill items.
    foreach ($result as $row) {
      $o_mediamosa->add_item($row);
    }
    */
  }
}

/**
 * URI: /statistics/mostsearchedwords
 *      /statistics/searchedwords
 *
 * Method: GET
 *
 * 1.x: vpx_statistics_get_most_popular_words
 *
 */
class mediamosa_rest_call_statistics_get_most_popular_words extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const BEGINDATE = 'begindate';
  const ENDDATE = 'enddate';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::BEGINDATE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME,
          self::VAR_DESCRIPTION => 'Begin date of the period.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::ENDDATE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME,
          self::VAR_DESCRIPTION => 'End date of the period.',
        ),
      )
    );

    // Enrich with limit, offset.
    $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX, mediamosa_settings::LIMIT_DEFAULT_STATISTICS);

    // Enrich with order by.
    $var_setup = self::get_var_setup_order_by($var_setup, '', array(), mediamosa_type::ORDER_DIRECTION_DESC);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $order_by = $this->get_param_value_order_by();
    $order_direction = $this->get_param_value_order_direction();
    $limit = $this->get_param_value_limit();
    $offset =  $this->get_param_value_offset();

    $begindate = $this->get_param_value(self::BEGINDATE);
    $enddate = $this->get_param_value(self::ENDDATE);

    // Allowed Order By List
    $a_order_by = array(
      'app_id' => array('column' => 'app_id'),
      'word' => array('column' => 'keyword'),
      'count' => array('column' => 'count'),
    );

    if (!isset($a_order_by[$order_by])) {
      $order_by = 'count';
    }

    $a_query = array();
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_statistics_search_request}';

    // Select this
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'keyword AS word';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'COUNT(keyword) AS count';

    $a_query[mediamosa_db_query::A_WHERE][] = sprintf("searched >= '%s'", $begindate);
    if (!is_null($enddate)) {
      $a_query[mediamosa_db_query::A_WHERE][] = sprintf("searched < '%s'", $enddate);
    }
    elseif (!is_null($enddate)) {
      $a_query[mediamosa_db_query::A_WHERE][] = sprintf("searched < DATE_ADD('%s', INTERVAL 1 MONTH)", $begindate);
    }

    $a_query[mediamosa_db_query::A_GROUP_BY][] = 'keyword';

    // Order by....
    $a_query[mediamosa_db_query::A_ORDER_BY][] = $a_order_by[$order_by]['column'] . ' ' . $order_direction;

    // Limit...
    $a_query[mediamosa_db_query::LIMIT] = $limit;

     // Offset...
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Do query.
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => TRUE));

    $result = mediamosa_db::db_query($query);

    // Set total.
    $o_mediamosa->item_count_total = mediamosa_db_query::found_rows();

    // Fill items.
    foreach ($result as $row) {
      $o_mediamosa->add_item($row);
    }
  }
}

/**
 * URI: /statistics/searchrequest
 *
 * Method: GET
 *
 * 1.x: vpx_statistics_get_searchrequest
 *
 */
class mediamosa_rest_call_statistics_get_searchrequest extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MONTH = 'month';
  const YEAR = 'year';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::MONTH => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The month of the query.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_START => 1,
          self::VAR_RANGE_END => 12,
        ),
        self::YEAR => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The year of the query.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_START => 2000,
          self::VAR_RANGE_END => 2099,
        ),
      )
    );

        // Enrich with limit, offset.
    $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX, mediamosa_settings::LIMIT_DEFAULT_STATISTICS);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $limit = $this->get_param_value_limit();
    $offset =  $this->get_param_value_offset();

    $month = $this->get_param_value(self::MONTH);
    $year = $this->get_param_value(self::YEAR);

    $a_query = array();
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_statistics_search_request}';

    // Select this.
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'keyword AS word';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'COUNT(keyword) AS count';

    $begindate = date('Y-m-d', mktime(0, 0, 0, $month, 1, $year));

    $a_query[mediamosa_db_query::A_WHERE][] = sprintf("searched >= '%s'", $begindate);
    $a_query[mediamosa_db_query::A_WHERE][] = sprintf("searched < DATE_ADD('%s', INTERVAL 1 MONTH)", $begindate);

    $a_query[mediamosa_db_query::A_GROUP_BY][] = 'keyword';

    // Limit...
    $a_query[mediamosa_db_query::LIMIT] = $limit;

     // Offset...
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Do query.
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => TRUE));
    $result = mediamosa_db::db_query($query);

    // Set total.
    $o_mediamosa->item_count_total = mediamosa_db_query::found_rows();

    // Fill items.
    foreach ($result as $row) {
      $o_mediamosa->add_item($row);
    }
  }
}
