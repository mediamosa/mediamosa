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
 * OpenAPI rest calls.
 */


/**
 * URI: /openapi/search
 * Method: GET
 */
class mediamosa_rest_call_openapi_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const QUERY = 'query';
  const QLIMIT = 'qlimit';
  const QOFFSET = 'qoffset';
  const QORDER = 'qorder';
  const QDIRECTION = 'qdirection';
  const SUBJECT = 'subject';
  const LANGUAGE = 'language';
  const LOCATION = 'location';
  const SINCE = 'since';
  const UNTIL = 'until';
  const AUTHOR = 'author';
  const CONTRIBUTOR = 'contributor';
  const PUBLISHER = 'publisher';
  const CATEGORY = 'category';
  const FORMAT = 'format';
  const DURATION = 'duration';
  const CALLBACK = 'callback';

  const ALT = 'alt';
  const REQUEST_PARAMETER_FORMAT_VALUE = 'Request-Parameter-Format-Value';
  const REQUEST_PARAMETER_FORMET_VALUE = 'Request-Parameter-Formet-Value';

  // Other vars;
  const DEFAULT_LIMIT = 10;
  const DEFAULT_OFFSET = 0;

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array(
      self::VARS => array(
        self::ALT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_RESPONSE_TYPE,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_response::RESPONSE_TYPE_XML,
            mediamosa_response::RESPONSE_TYPE_ATOM,
            mediamosa_response::RESPONSE_TYPE_RSS,
            mediamosa_response::RESPONSE_TYPE_JSON,
          ),
          self::VAR_DESCRIPTION => mediamosa::t('Optional parameter to specify the response output format.'),
        ),
        self::REQUEST_PARAMETER_FORMET_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_RESPONSE_TYPE,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_response::RESPONSE_TYPE_XML,
            mediamosa_response::RESPONSE_TYPE_ATOM,
            mediamosa_response::RESPONSE_TYPE_RSS,
            mediamosa_response::RESPONSE_TYPE_JSON,
          ),
          self::VAR_DESCRIPTION => mediamosa::t('Optional parameter to specify the response output format.'),
        ),
        self::REQUEST_PARAMETER_FORMAT_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_RESPONSE_TYPE,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_response::RESPONSE_TYPE_XML,
            mediamosa_response::RESPONSE_TYPE_ATOM,
            mediamosa_response::RESPONSE_TYPE_RSS,
            mediamosa_response::RESPONSE_TYPE_JSON,
          ),
          self::VAR_DESCRIPTION => mediamosa::t('Optional parameter to specify the response output format.'),
        ),
        self::QUERY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Search query.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::SUBJECT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The subject mapped to DC:subject.',
        ),
        self::LANGUAGE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The language mapped to DC:language.',
        ),
        self::LOCATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The location mapped to DC:coverage_spatial.',
        ),
        self::SINCE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'The since mapped to videotimestamp (search from a date).',
        ),
        self::UNTIL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'The until mapped to videotimestamp (search till a date).',
        ),
        self::AUTHOR => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The author mapped to DC:creator.',
        ),
        self::CONTRIBUTOR => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The contributor mapped to DC:contributor.',
        ),
        self::PUBLISHER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The publisher mapeed to DC:publisher.',
        ),
        self::CATEGORY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'The category mapped to coll_id.',
        ),
        self::FORMAT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The format mapped to content type of mediafile.',
        ),
        self::DURATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The duration mappped to the duration of mediafile.',
        ),
        self::CALLBACK => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The callback for JSON.',
        ),
      )
    );

    // Include limit, offset vars.
    $a_var_setup = self::get_var_setup_range($a_var_setup);
    // Alter the params.
    $a_var_setup[self::VARS][self::QLIMIT] = $a_var_setup[self::VARS][self::LIMIT];
    $a_var_setup[self::VARS][self::QOFFSET] = $a_var_setup[self::VARS][self::OFFSET];
    unset($a_var_setup[self::VARS][self::LIMIT]);
    unset($a_var_setup[self::VARS][self::OFFSET]);

    // Include order by.
    $a_var_setup = self::get_var_setup_order_by($a_var_setup);
    // Alter the params.
    $a_var_setup[self::VARS][self::QORDER] = $a_var_setup[self::VARS][self::ORDER_BY];
    $a_var_setup[self::VARS][self::QDIRECTION] = $a_var_setup[self::VARS][self::ORDER_DIRECTION];
    unset($a_var_setup[self::VARS][self::ORDER_BY]);
    unset($a_var_setup[self::VARS][self::ORDER_DIRECTION]);

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Override Validate Rest Args.
  protected function validate_rest_args(array $a_var_setup) {

    // Validate first.
    parent::validate_rest_args($a_var_setup);

    $app_id = variable_get(mediamosa_settings::MEDIAMOSA_OPENAPI_OPEN_APP_ID, 0);

    if (!$app_id) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_OPENAPI_MISSING_OPEN_APP_ID);
    }
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = array(variable_get(mediamosa_settings::MEDIAMOSA_OPENAPI_OPEN_APP_ID, 0));

    // Get params.
    $limit = $this->get_param_value(self::QLIMIT);
    $offset = $this->get_param_value(self::QOFFSET);
    $order_by = $this->get_param_value(self::QORDER);
    $order_direction = $this->get_param_value(self::QDIRECTION);
    $callback = $this->get_param_value(self::CALLBACK);

    $cql = array();

    // Get params with escape.
    $query = addslashes($this->get_param_value(self::QUERY));
    $subject = addslashes($this->get_param_value(self::SUBJECT));
    $language = addslashes($this->get_param_value(self::LANGUAGE));
    $location = addslashes($this->get_param_value(self::LOCATION));
    $since = addslashes($this->get_param_value(self::SINCE));
    $until = addslashes($this->get_param_value(self::UNTIL));
    $author = addslashes($this->get_param_value(self::AUTHOR));
    $contributor = addslashes($this->get_param_value(self::CONTRIBUTOR));
    $publisher = addslashes($this->get_param_value(self::PUBLISHER));
    $category = addslashes($this->get_param_value(self::CATEGORY));
    $format = addslashes($this->get_param_value(self::FORMAT));
    $duration = addslashes($this->get_param_value(self::DURATION));

    if ($query) {
      $cql[] = '(title="' . $query . '" OR description="' . $query . '")';
    }

    if ($subject) {
      $cql[] = 'subject="' . $subject . '"';
    }

    if ($language) {
      $cql[] = 'language="' . $language . '"';
    }

    if ($location) {
      $cql[] = 'coverage_spatial="' . $location . '"';
    }

    if ($this->isset_param(self::SINCE)) {
      $cql[] = 'videotimestamp>="' . $since . '"';
    }

    if ($this->isset_param(self::UNTIL)) {
      $cql[] = 'videotimestamp<="' . $until . '"';
    }

    if ($author) {
      $cql[] = 'creator="' . $author . '"';
    }

    if ($contributor) {
      $cql[] = 'contributor="' . $contributor . '"';
    }

    if ($publisher) {
      $cql[] = 'publisher="' . $publisher . '"';
    }

    if ($category) {
      $cql[] = 'coll_id="' . $category . '"';
    }

    if ($format) {
      $cql[] = 'mediafile_container_type="' . $format . '"';
    }

    if ($duration) {
      $cql[] = 'mediafile_duration="' . $duration . '"';
    }

    // Gluing CQL.
    $cql = implode(' AND ', $cql);

    // TODO: Implement callback.

    // Add possible order by to the CQL.
    if ($order_by != '' && $cql) {
      $cql .= (!empty($cql) ? ' ' : '') . 'sortby ' . $order_by;
      $cql .= (empty($order_direction) ? '' : '/' . (mediamosa_unicode::strtolower($order_direction) == mediamosa_type::ORDER_DIRECTION_ASC ? 'ascending' : 'descending'));
    }

    if ($cql) {
      $asset_ids = mediamosa_search::asset(array(
        'app_ids' => $app_ids,
        'cql' => $cql,
        'cql_store_stats' => TRUE,
        'limit' => $limit,
        'offset' => $offset,
        'show_deleted' => FALSE,
      ));

      // Fill response.
      $acl_user_id = '';
      $granted = '';
      $acl_group_ids = array();
      $acl_domain = '';
      $acl_realm = '';
      $show_stills = TRUE;
      $show_collections = FALSE;
      $add_has_streamable_mediafiles = FALSE;
      $items = mediamosa_asset::asset_collect(
        $asset_ids['asset_ids'],
        $app_ids,
        $acl_user_id,
        $granted,
        $acl_group_ids,
        $acl_domain,
        $acl_realm,
        $this->get_param_value(self::IS_APP_ADMIN),
        $show_stills,
        $show_collections,
        $add_has_streamable_mediafiles
      );

      foreach ($items as $item) {
        // Fix app_id.
        $app_id = in_array($item['app_id'], $app_ids) ? $item['app_id'] : reset($app_ids);

        // Add mediafiles.
        $mediafile_ids = mediamosa_asset_mediafile::mediafiles_search($item['asset_id'], NULL, FALSE, ($item['app_id'] == $app_id ? array() : $app_ids));

        // Now enrich the output with mediafiles.
        $show_stills = TRUE;
        $item['mediafiles'] = mediamosa_asset_mediafile::enrich_response_mediafile($mediafile_ids, $app_ids, FALSE, $show_stills);

        // Add streamable setting on asset.
        $item['has_streamable_mediafiles'] = mediamosa_asset::enrich_response_has_streamable_mediafiles($item['mediafiles']['mediafile']) ? 'TRUE' : 'FALSE';

        if (!empty($item['mediafiles']['mediafile']) && is_array($item['mediafiles']['mediafile'])) {
          $mediafile = reset($item['mediafiles']['mediafile']);

          if (!empty($mediafile['still']) && is_array($mediafile['still'])) {
            $still = reset($mediafile['still']);
          }

          $item['mediaitem'] = array(
            'album_id' => $item['asset_id'],
            'created' => !empty($item['qualified_dublin_core']['created']) ? $item['qualified_dublin_core']['created'] : NULL,
            'description' => !empty($item['dublin_core']['description']) ? $item['dublin_core']['description'] : NULL,
            'duration' => !empty($mediafile['metadata']['file_duration']) ? $mediafile['metadata']['file_duration'] : NULL,
            'file_size' => !empty($mediafile['metadata']['filesize']) ? $mediafile['metadata']['filesize'] : NULL,
            'id' => !empty($mediafile['mediafile_id']) ? $mediafile['mediafile_id'] : NULL,
            'language' => !empty($item['dublin_core']['language']) ? $item['dublin_core']['language'] : NULL,
            'last_updated' => !empty($mediafile['changed']) ? $mediafile['changed'] : NULL,
            'location' => NULL,
            'mime_type' => !empty($mediafile['metadata']['mime_type']) ? $mediafile['metadata']['mime_type'] : NULL,
            'num_comments' => NULL,
            'num_views' => !empty($item['viewed']) ? $item['viewed'] : NULL,
            'num_votes' => NULL,
            'rating' => NULL,
            'start_time' => NULL,
            'tagged_people' => NULL,
            'tags' => !empty($mediafile['tag']) ? array($mediafile['tag']) : NULL,
            'still_url' => !empty($still['still_ticket']) ? $still['still_ticket'] : NULL,
            'title' => !empty($item['dublin_core']['title'][0]) ? $item['dublin_core']['title'][0] : NULL,
            'type' => !empty($item['dublin_core']['type'][0]) ? $item['dublin_core']['type'][0] : NULL,
            'url' => !empty($mediafile['uri']) ? $mediafile['uri'] : NULL,
          );
        }

        $mediamosa->add_item($item);
      }
    }
  }
}

/**
 * URI: /openapi/mediaItems/$items_user_id/$group_id
 * Method: GET
 */
class mediamosa_rest_call_openapi_mediaitems_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';

  // Standard Request Parameters.
  const REQUEST_PARAMETER_UPDATEDSINCE_VALUE = 'Request-Parameter-UpdatedSince-Value';
  const REQUEST_PARAMETER_FORMAT_VALUE = 'Request-Parameter-Format-Value';
  const REQUEST_PARAMETER_FORMET_VALUE = 'Request-Parameter-Formet-Value';

  // Collection Request Parameters.
  const REQUEST_PARAMETER_COUNT_VALUE = 'Request-Parameter-Count-Value';
  const REQUEST_PARAMETER_FILTERBY_VALUE = 'Request-Parameter-FilterBy-Value';
  const REQUEST_PARAMETER_FILTEROP_VALUE = 'Request-Parameter-FilterOp-Value';
  const REQUEST_PARAMETER_FILTERVALUE_VALUE = 'Request-Parameter-FilterValue-Value';
  const REQUEST_PARAMETER_SORTORDER_VALUE = 'Request-Parameter-SortOrder-Value';
  const REQUEST_PARAMETER_STARTINDEX_VALUE = 'Request-Parameter-StartIndex-Value';

  // Call related paramters.
  const ITEMS_USER_ID = 'items_user_id';
  const GROUP_ID = 'group_id';
  const ALBUM_ID = 'album_id';
  const APPID = 'appid';
  const MEDIAITEM_ID = 'mediaitem_id';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User id.',
        ),

        // Standard Request Parameters.
        self::REQUEST_PARAMETER_UPDATEDSINCE_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'Only return items whose updated date & time is equal to or more recent then the specified value (e.g. 2008-01-23T04:56:22Z).',
        ),
        self::REQUEST_PARAMETER_FORMAT_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_RESPONSE_TYPE,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_response::RESPONSE_TYPE_XML,
            mediamosa_response::RESPONSE_TYPE_ATOM,
            mediamosa_response::RESPONSE_TYPE_RSS,
            mediamosa_response::RESPONSE_TYPE_JSON,
          ),
          self::VAR_DESCRIPTION => mediamosa::t('Optional parameter to specify the response output format.'),
        ),
        self::REQUEST_PARAMETER_FORMET_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_RESPONSE_TYPE,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_response::RESPONSE_TYPE_XML,
            mediamosa_response::RESPONSE_TYPE_ATOM,
            mediamosa_response::RESPONSE_TYPE_RSS,
            mediamosa_response::RESPONSE_TYPE_JSON,
          ),
          self::VAR_DESCRIPTION => mediamosa::t('Optional parameter to specify the response output format.'),
        ),

        // Collection Request Parameters.
        self::REQUEST_PARAMETER_COUNT_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_LIMIT,
          self::VAR_DESCRIPTION => 'The page size for a paged collection.',
          self::VAR_DEFAULT_VALUE => mediamosa_settings::LIMIT_DEFAULT,
          self::VAR_RANGE_START => 0,
          self::VAR_RANGE_END => mediamosa_settings::LIMIT_MAX,
        ),
        self::REQUEST_PARAMETER_FILTERBY_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'For a collection, return entries filtered by the given field name.',
        ),
        self::REQUEST_PARAMETER_FILTEROP_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The operation to use when filtering a collection by a field specified in "filterBy", defaults to "contains".',
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_rest_call::FILTEROP_CONTAINS,
            mediamosa_rest_call::FILTEROP_EQUALS,
            mediamosa_rest_call::FILTEROP_STARTSWITH,
            mediamosa_rest_call::FILTEROP_PRESENT,
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_rest_call::FILTEROP_CONTAINS,
        ),
        self::REQUEST_PARAMETER_FILTERVALUE_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The value to use when filtering a collection.',
        ),
        self::REQUEST_PARAMETER_SORTORDER_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Can either be "ascending" or "descending", defaults to ascending. Used to sort objects in a collection.',
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_rest_call::SORTORDER_ASCENDING,
            mediamosa_rest_call::SORTORDER_DESCENDING,
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_rest_call::SORTORDER_ASCENDING,
        ),
        self::REQUEST_PARAMETER_STARTINDEX_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'Index into a paged collection.',
          self::VAR_DEFAULT_VALUE => 0,
        ),

        // Call related paramters.
        self::ITEMS_USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The person whose MediaItems are to be returned. Defaults to "@me", indicating the currently authenticated user.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
          self::VAR_DEFAULT_VALUE => self::USER_ME,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID of the group of users whose MediaItems are to be returned. Defaults to "@self".',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
          self::VAR_DEFAULT_VALUE => self::GROUP_SELF,
        ),
        self::ALBUM_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
          self::VAR_DESCRIPTION => 'The ID of the album whose MediaItems are to be returned. ',
        ),
        self::APPID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
          self::VAR_DESCRIPTION => mediamosa::t('Specifies that the response should only contain MediaItems generated by the given appId.'),
        ),
        self::MEDIAITEM_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'MediaItem ID specifying the MediaItems to retrieve.',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Override Validate Rest Args.
  protected function validate_rest_args(array $a_var_setup) {

    // Validate first.
    parent::validate_rest_args($a_var_setup);

    $app_id = variable_get(mediamosa_settings::MEDIAMOSA_OPENAPI_OPEN_APP_ID, 0);

    if (!$app_id) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_OPENAPI_MISSING_OPEN_APP_ID);
    }
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = array(variable_get(mediamosa_settings::MEDIAMOSA_OPENAPI_OPEN_APP_ID, 0));
    $app_id = reset($app_ids);

    $cql = array();

    // Get params.

    $user_id = $this->get_param_value(self::USER_ID);

    // Standard Request Parameters.
    $since = addslashes($this->get_param_value(self::REQUEST_PARAMETER_UPDATEDSINCE_VALUE));
    $alt = $this->get_param_value(self::REQUEST_PARAMETER_FORMAT_VALUE);

    // Collection Request Parameters.
    $limit = $this->get_param_value(self::REQUEST_PARAMETER_COUNT_VALUE);
    $RequestParameterFilterByValue = addslashes($this->get_param_value(self::REQUEST_PARAMETER_FILTERBY_VALUE));
    $RequestParameterFilterOpValue = $this->get_param_value(self::REQUEST_PARAMETER_FILTEROP_VALUE);
    $RequestParameterFilterValueValue = addslashes($this->get_param_value(self::REQUEST_PARAMETER_FILTERVALUE_VALUE));
    $order_direction = $this->get_param_value(self::REQUEST_PARAMETER_SORTORDER_VALUE);
    $offset = $this->get_param_value(self::REQUEST_PARAMETER_STARTINDEX_VALUE);

    // Call related paramters.
    $user_id_parameter = addslashes($this->get_param_value(self::ITEMS_USER_ID));
    $group_id = addslashes($this->get_param_value(self::GROUP_ID));
    $asset_id = addslashes($this->get_param_value(self::ALBUM_ID));
    $appid_parameter = addslashes($this->get_param_value(self::APPID));
    $mediafile_id = addslashes($this->get_param_value(self::MEDIAITEM_ID));

    if ($since) {
      $cql[] = 'videotimestamp >= "' . $since . '"';
    }

    // We don't know, who the owner the page (USER_OWNER), that the current user visit.
    // The client application should change it to the appropriate user.
    if ($user_id_parameter && $user_id_parameter != self::USER_OWNER) {
      // Me and viewer is the current user.
      if ($user_id_parameter == self::USER_ME || $user_id_parameter == self::USER_VIEWER) {
        $user_id_parameter = $user_id;
      }
      if ($user_id_parameter) {
        $cql[] = 'owner_id=="^' . $user_id_parameter . '^"';
      }
    }

    if ($group_id !== '') {
      if ($user_id_parameter && $user_id_parameter!= self::USER_OWNER && $group_id == self::GROUP_SELF) {
        // Get the group_id.
        $a_user = mediamosa_user::get($user_id_parameter, $app_id);

        if (isset($a_user['group_id']) && !empty($a_user['group_id'])) {
          $group_id = mediamosa_user::get_primary_group($app_id, $user_id_parameter);
          if ($group_id !== '') {
            $cql[] = 'group_id=="^' . $group_id . '^"';
          }
        }
      }
      if ($group_id != self::GROUP_SELF) {
        $cql[] = 'group_id=="^' . $group_id . '^"';
      }
    }

    if ($asset_id !== '') {
      $cql[] = 'asset_id=="^' . $asset_id . '^"';
    }

    if ($appid_parameter) {
      $cql[] = 'app_id="' . $appid_parameter . '"';
    }

    if ($mediafile_id !== '') {
      $cql[] = 'mediafile_id=="^' . $mediafile_id . '^"';
    }

    // Gluing CQL.
    $cql = implode(' AND ', $cql);

    // TODO: Message collection fields. Should we do it?
    // REQUEST_PARAMETER_FILTERBY_VALUE, REQUEST_PARAMETER_FILTEROP_VALUE, REQUEST_PARAMETER_FILTERVALUE_VALUE

    // Add possible order by to the CQL.
    // Fixed order by. There is not parameter for order by, just for order direction.
    $order_by = 'title';
    if ($order_by != '') {
      $cql .= (!empty($cql) ? ' ' : '') . 'sortby ' . $order_by;
      $cql .= (empty($order_direction) ? '' : '/' . (mediamosa_unicode::strtolower($order_direction) == mediamosa_type::ORDER_DIRECTION_ASCENDING ? 'ascending' : 'descending'));
    }

    $asset_ids = mediamosa_search::asset(array(
      'app_ids' => $app_ids,
      'cql' => $cql,
      'limit' => $limit,
      'offset' => $offset,
      'show_deleted' => FALSE,
    ));

    // Fill response.
    $acl_user_id = '';
    $granted = '';
    $acl_group_ids = array();
    $acl_domain = '';
    $acl_realm = '';
    $show_stills = TRUE;
    $show_collections = FALSE;
    $add_has_streamable_mediafiles = FALSE;
    $items = mediamosa_asset::asset_collect(
      $asset_ids['asset_ids'],
      $app_ids,
      $acl_user_id,
      $granted,
      $acl_group_ids,
      $acl_domain,
      $acl_realm,
      $this->get_param_value(self::IS_APP_ADMIN),
      $show_stills,
      $show_collections,
      $add_has_streamable_mediafiles
    );

    foreach ($items as $item) {
      // Fix app_id.
      $app_id = in_array($item['app_id'], $app_ids) ? $item['app_id'] : reset($app_ids);

      // Add mediafiles.
      $mediafile_ids = mediamosa_asset_mediafile::mediafiles_search($item['asset_id'], NULL, FALSE, ($item['app_id'] == $app_id ? array() : $app_ids));

      // Now enrich the output with mediafiles.
      $show_stills = TRUE;
      $item['mediafiles'] = mediamosa_asset_mediafile::enrich_response_mediafile($mediafile_ids, $app_ids, FALSE, $show_stills);

      // Add streamable setting on asset.
      $item['has_streamable_mediafiles'] = mediamosa_asset::enrich_response_has_streamable_mediafiles($item['mediafiles']['mediafile']) ? 'TRUE' : 'FALSE';

      if (!empty($item['mediafiles']['mediafile']) && is_array($item['mediafiles']['mediafile'])) {
        $mediafile = reset($item['mediafiles']['mediafile']);

        if (!empty($mediafile['still']) && is_array($mediafile['still'])) {
          $still = reset($mediafile['still']);
        }

        $item['mediaitem'] = array(
          'album_id' => $item['asset_id'],
          'created' => !empty($item['qualified_dublin_core']['created']) ? $item['qualified_dublin_core']['created'] : NULL,
          'description' => !empty($item['dublin_core']['description']) ? $item['dublin_core']['description'] : NULL,
          'duration' => !empty($mediafile['metadata']['file_duration']) ? $mediafile['metadata']['file_duration'] : NULL,
          'file_size' => !empty($mediafile['metadata']['filesize']) ? $mediafile['metadata']['filesize'] : NULL,
          'id' => !empty($mediafile['mediafile_id']) ? $mediafile['mediafile_id'] : NULL,
          'language' => !empty($item['dublin_core']['language']) ? $item['dublin_core']['language'] : NULL,
          'last_updated' => !empty($mediafile['changed']) ? $mediafile['changed'] : NULL,
          'location' => NULL,
          'mime_type' => !empty($mediafile['metadata']['mime_type']) ? $mediafile['metadata']['mime_type'] : NULL,
          'num_comments' => NULL,
          'num_views' => !empty($item['viewed']) ? $item['viewed'] : NULL,
          'num_votes' => NULL,
          'rating' => NULL,
          'start_time' => NULL,
          'tagged_people' => NULL,
          'tags' => !empty($mediafile['tag']) ? array($mediafile['tag']) : NULL,
          'still_url' => !empty($still['still_ticket']) ? $still['still_ticket'] : NULL,
          'title' => !empty($item['dublin_core']['title'][0]) ? $item['dublin_core']['title'][0] : NULL,
          'type' => !empty($item['dublin_core']['type'][0]) ? $item['dublin_core']['type'][0] : NULL,
          'url' => !empty($mediafile['uri']) ? $mediafile['uri'] : NULL,
        );
      }

      $mediamosa->add_item($item);
    }
  }
}

/**
 * URI: /openapi/mediaItems/$items_user_id/$group_id/$album
 * Method: POST
 */
class mediamosa_rest_call_openapi_mediaitems_post extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';

  // Standard Request Parameters.
  const REQUEST_PARAMETER_UPDATEDSINCE_VALUE = 'Request-Parameter-UpdatedSince-Value';
  const REQUEST_PARAMETER_FORMAT_VALUE = 'Request-Parameter-Format-Value';
  const REQUEST_PARAMETER_FORMET_VALUE = 'Request-Parameter-Formet-Value';

  // Call related paramters.
  const ITEMS_USER_ID = 'items_user_id';
  const GROUP_ID = 'group_id';
  const ALBUM_ID = 'album_id';

  // Data parameters.
  const MEDIAITEM_ID = 'id';
  //
  const CREATED = 'created';
  const DESCRIPTION = 'description';
  const DURATION = 'duration';
  const FILE_SIZE = 'file_size';
  const LANGUAGE = 'language';
  const LAST_UPDATED = 'last_updated';
  const LOCATION = 'location';
  const MIME_TYPE = 'mime_type';
  const NUM_COMMENTS = 'num_comments';
  const NUM_VIEWS = 'num_views';
  const NUM_VOTES = 'num_votes';
  const RATING = 'rating';
  const START_TIME = 'start_time';
  const TAGGED_PEOPLE = 'tagged_people';
  const TAGS = 'tags';
  const STILL_URL = 'still_url';
  const TITLE = 'title';
  const TYPE = 'type';
  const URL = 'url';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User id.',
        ),

        // Standard Request Parameters.
        self::REQUEST_PARAMETER_UPDATEDSINCE_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'Only return items whose updated date & time is equal to or more recent then the specified value (e.g. 2008-01-23T04:56:22Z).',
        ),
        self::REQUEST_PARAMETER_FORMAT_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_RESPONSE_TYPE,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_response::RESPONSE_TYPE_XML,
            mediamosa_response::RESPONSE_TYPE_ATOM,
            mediamosa_response::RESPONSE_TYPE_RSS,
            mediamosa_response::RESPONSE_TYPE_JSON,
          ),
          self::VAR_DESCRIPTION => mediamosa::t('The response type paramter.'),
        ),
        self::REQUEST_PARAMETER_FORMET_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_RESPONSE_TYPE,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_response::RESPONSE_TYPE_XML,
            mediamosa_response::RESPONSE_TYPE_ATOM,
            mediamosa_response::RESPONSE_TYPE_RSS,
            mediamosa_response::RESPONSE_TYPE_JSON,
          ),
          self::VAR_DESCRIPTION => mediamosa::t('The response type paramter.'),
        ),

        // Call related paramters.
        self::ITEMS_USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The person whose MediaItems are to be returned. Defaults to "@me", indicating the currently authenticated user.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
          self::VAR_DEFAULT_VALUE => self::USER_ME,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID of the group of users whose MediaItems are to be returned. Defaults to "@self".',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
          self::VAR_DEFAULT_VALUE => self::GROUP_SELF,
          self::VAR_ALLOWED_VALUES => array(
            self::GROUP_SELF,
          ),
        ),
        self::ALBUM_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
          self::VAR_DESCRIPTION => 'Album to which the media item belongs. ',
        ),

        // Data parameters.
        self::MEDIAITEM_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'Id Associated with the media item.',
        ),
        //
        self::CREATED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'Creation datetime associated with the media item - assigned by container in UTC (e.g. 2008-01-23T04:56:22Z).',
        ),
        self::DESCRIPTION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Description of the media item.',
        ),
        self::DURATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'For audio/video clips - playtime length in seconds. set to -1/not defined if unknown.',
          self::VAR_DEFAULT_VALUE => -1,
        ),
        self::FILE_SIZE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Number of bytes (set to -1/undefined if unknown).',
          self::VAR_DEFAULT_VALUE => -1,
        ),
        self::LANGUAGE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_LANGUAGE_CODE_ISO_639_3,
          self::VAR_DESCRIPTION => 'Language associated with the media item in ISO 639-3 format.',
        ),
        self::LAST_UPDATED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'Update datetime associated with the media item - assigned by container in UTC (e.g. 2008-01-23T04:56:22Z).',
        ),
        self::LOCATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Location corresponding to the media item.',
        ),
        self::MIME_TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MIMETYPE,
          self::VAR_DESCRIPTION => 'The MIME type of media, specified as a string.',
        ),
        self::NUM_COMMENTS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'Number of comments on the media item.',
        ),
        self::NUM_VIEWS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'Number of views for the media item.',
        ),
        self::NUM_VOTES => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'Number of votes received for voting.',
        ),
        self::RATING => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'Average rating of the media item on a scale of 0-10.',
          self::VAR_RANGE_START => 0,
          self::VAR_RANGE_END => 10,
        ),
        self::START_TIME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'For streaming/live content, datetime when the content is available (e.g. 2008-01-23T04:56:22Z).',
        ),
        self::TAGGED_PEOPLE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Array of string (IDs) of people tagged in the media item.',
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::TAGS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Tags associated with this media item.',
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::STILL_URL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'URL to a still image of the media item.',
        ),
        self::TITLE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Describing the media item.',
        ),
        self::TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The type of media, specified as a MediaItem.Type object.',
          self::VAR_ALLOWED_VALUES => array(
            // We don't support others, just video type.
            mediamosa_rest_call::MEDIAITEM_TYPE,
          ),
        ),
        self::URL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Specifying the URL where the media can be found.',
        ),

      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Override Validate Rest Args.
  protected function validate_rest_args(array $a_var_setup) {

    // Validate first.
    parent::validate_rest_args($a_var_setup);

    $app_id = variable_get(mediamosa_settings::MEDIAMOSA_OPENAPI_OPEN_APP_ID, 0);

    if (!$app_id) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_OPENAPI_MISSING_OPEN_APP_ID);
    }
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = array(variable_get(mediamosa_settings::MEDIAMOSA_OPENAPI_OPEN_APP_ID, 0));
    $app_id = reset($app_ids);

    // Get params.

    $user_id = $this->get_param_value(self::USER_ID);

    // Standard Request Parameters.
    $since = $this->get_param_value(self::REQUEST_PARAMETER_UPDATEDSINCE_VALUE);
    $alt = $this->get_param_value(self::REQUEST_PARAMETER_FORMAT_VALUE);

    // Call related paramters.
    $user_id_parameter = addslashes($this->get_param_value(self::ITEMS_USER_ID));
    if ($user_id_parameter == self::USER_ME || $user_id_parameter == self::USER_VIEWER) {
      $user_id_parameter = $user_id;
    }
    if (!$user_id_parameter) {
      $user_id_parameter = 'anonymous';
    }
    $group_id = addslashes($this->get_param_value(self::GROUP_ID));
    $asset_id = addslashes($this->get_param_value(self::ALBUM_ID));
    $mediafile_id = addslashes($this->get_param_value(self::MEDIAITEM_ID));

    // Data parameters.
    $created = $this->get_param_value(self::CREATED);
    $description = strip_tags(addslashes($this->get_param_value(self::DESCRIPTION)));
    $duration = $this->get_param_value(self::DURATION);
    $file_size = $this->get_param_value(self::FILE_SIZE);
    $language = $this->get_param_value(self::LANGUAGE);
    $last_updated = $this->get_param_value(self::LAST_UPDATED);
    $location = strip_tags(addslashes($this->get_param_value(self::LOCATION)));
    $mime_type = strip_tags(addslashes($this->get_param_value(self::MIME_TYPE)));
    $num_comments = $this->get_param_value(self::NUM_COMMENTS);
    $num_views = $this->get_param_value(self::NUM_VIEWS);
    $num_votes = $this->get_param_value(self::NUM_VOTES);
    $rating = $this->get_param_value(self::RATING);
    $start_time = $this->get_param_value(self::START_TIME);
    $tagged_people = $this->get_param_value(self::TAGGED_PEOPLE);
    if ($tagged_people) {
      foreach ($tagged_people as $key => $value) {
        $tagged_people[$key] = strip_tags(addslashes($value));
      }
    }
    $tags = $this->get_param_value(self::TAGS);
    if ($tags) {
      foreach ($tags as $key => $value) {
        $tags[$key] = strip_tags(addslashes($value));
      }
    }
    $still_url = strip_tags(addslashes($this->get_param_value(self::STILL_URL)));
    $title = strip_tags(addslashes($this->get_param_value(self::TITLE)));
    $type = $this->get_param_value(self::TYPE);
    $url = strip_tags(addslashes($this->get_param_value(self::URL)));

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Must exists.
    $asset = mediamosa_asset::must_exists($asset_id);

    // Check owner of parent asset, need ownership.
    mediamosa_acl::owner_check($app_id, $user_id_parameter, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID]);

    // Set mediafile ID to something if not present.
    // Commented out, because we don't want, that the user can set the mediafile_id.
    // Otherwise we have to check, if the mediafile with mediafile_id is not exists.
    //if (!$mediafile_id) {
      $mediafile_id = mediamosa_db::uuid($app_id);
    //}

    // Fields to insert.
    $fields = array(
      mediamosa_asset_mediafile_db::GROUP_ID => NULL,
      mediamosa_asset_mediafile_db::IS_DOWNLOADABLE => mediamosa_asset_mediafile_db::IS_DOWNLOADABLE_FALSE,
      mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE,
      mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => NULL,
      mediamosa_asset_mediafile_db::FILENAME => NULL,
      mediamosa_asset_mediafile_db::URI => $url,
      mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE => NULL,
      mediamosa_asset_mediafile_db::TAG => $tags ? implode(', ', $tags) : '',
    );

    // Insert the mediafile.
    $mediafile_id = mediamosa_asset_mediafile::create($mediafile_id, $app_id, $asset_id, $user_id_parameter, $fields);

    // Create metadata.

    // Metadata function call parameters.
    $action = 'replace';
    // Get full definitions.
    $metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full();
    // Mapping metadata parameters.
    $params = array(
      'created' => array($created),
      'description' => array($description),
      'language' => array($language),
      'title' => array($title),
      // Skipped fields, because we don't have metadata equivalients.
      // $duration, $file_size, $last_updated, $location, $mime_type, $num_comments, $num_views, $num_votes, $rating, $start_time, $tagged_people, $still_url, $type,
    );

    // Create the metadata.
    $value_set = mediamosa_asset_metadata::metadata_create($asset_id, $metadata_definitions_full, $params, $action);

    // Create response.

    $response = array();

    $response['asset_id'] = $asset_id;
    $response['mediafile_id'] = $mediafile_id;

    // Return what has been inserted into the metadata.
    foreach ($value_set as $name => $value) {
      $response[$name] = $value;
    }

    $response['mediaitem'] = array(
      'album_id' => $asset_id,
      'created' => $created,
      'description' => $description,
      'duration' => NULL,
      'file_size' => NULL,
      'id' => $mediafile_id,
      'language' => $language,
      'last_updated' => NULL,
      'location' => NULL,
      'mime_type' => NULL,
      'num_comments' => NULL,
      'num_views' => NULL,
      'num_votes' => NULL,
      'rating' => NULL,
      'start_time' => NULL,
      'tagged_people' => NULL,
      'tags' => !empty($tags) ? $tags : NULL,
      'still_url' => NULL,
      'title' => $title,
      'type' => NULL,
      'url' => $url,
    );

    $mediamosa->add_item($response);
  }
}

/**
 * URI: /openapi/album/$album_id
 * URI: /openapi/album/$album_id/mediaitem
 * URI: /openapi/album/$album_id/mediaitem/$mediaitem_id
 *
 * Method: POST
 *
 */
class mediamosa_rest_call_openapi_upload extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';

  // Standard Request Parameters.
  const REQUEST_PARAMETER_UPDATEDSINCE_VALUE = 'Request-Parameter-UpdatedSince-Value';
  const REQUEST_PARAMETER_FORMAT_VALUE = 'Request-Parameter-Format-Value';
  const REQUEST_PARAMETER_FORMET_VALUE = 'Request-Parameter-Formet-Value';

  // Call related paramters.
  const ALBUM_ID = 'album_id';

  // Data parameters.
  const MEDIAITEM_ID = 'id';
  //
  const CREATED = 'created';
  const DESCRIPTION = 'description';
  const DURATION = 'duration';
  const FILE_SIZE = 'file_size';
  const LANGUAGE = 'language';
  const LAST_UPDATED = 'last_updated';
  const LOCATION = 'location';
  const MIME_TYPE = 'mime_type';
  const NUM_COMMENTS = 'num_comments';
  const NUM_VIEWS = 'num_views';
  const NUM_VOTES = 'num_votes';
  const RATING = 'rating';
  const START_TIME = 'start_time';
  const TAGGED_PEOPLE = 'tagged_people';
  const TAGS = 'tags';
  const STILL_URL = 'still_url';
  const TITLE = 'title';
  const TYPE = 'type';
  const URL = 'url';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User id.',
        ),

        // Standard Request Parameters.
        self::REQUEST_PARAMETER_UPDATEDSINCE_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'Only return items whose updated date & time is equal to or more recent then the specified value (e.g. 2008-01-23T04:56:22Z).',
        ),
        self::REQUEST_PARAMETER_FORMAT_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_RESPONSE_TYPE,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_response::RESPONSE_TYPE_XML,
            mediamosa_response::RESPONSE_TYPE_ATOM,
            mediamosa_response::RESPONSE_TYPE_RSS,
            mediamosa_response::RESPONSE_TYPE_JSON,
          ),
          self::VAR_DESCRIPTION => mediamosa::t('The response type paramter.'),
        ),
        self::REQUEST_PARAMETER_FORMET_VALUE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_RESPONSE_TYPE,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_response::RESPONSE_TYPE_XML,
            mediamosa_response::RESPONSE_TYPE_ATOM,
            mediamosa_response::RESPONSE_TYPE_RSS,
            mediamosa_response::RESPONSE_TYPE_JSON,
          ),
          self::VAR_DESCRIPTION => mediamosa::t('The response type paramter.'),
        ),

        // Call related paramters.
        self::ALBUM_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
          self::VAR_DESCRIPTION => 'Album to which the media item belongs. ',
        ),

        // Data parameters.
        self::MEDIAITEM_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'Id Associated with the media item.',
        ),
        //
        self::CREATED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'Creation datetime associated with the media item - assigned by container in UTC (e.g. 2008-01-23T04:56:22Z).',
        ),
        self::DESCRIPTION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Description of the media item.',
        ),
        self::DURATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'For audio/video clips - playtime length in seconds. set to -1/not defined if unknown.',
          self::VAR_DEFAULT_VALUE => -1,
        ),
        self::FILE_SIZE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Number of bytes (set to -1/undefined if unknown).',
          self::VAR_DEFAULT_VALUE => -1,
        ),
        self::LANGUAGE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_LANGUAGE_CODE_ISO_639_3,
          self::VAR_DESCRIPTION => 'Language associated with the media item in ISO 639-3 format.',
        ),
        self::LAST_UPDATED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'Update datetime associated with the media item - assigned by container in UTC (e.g. 2008-01-23T04:56:22Z).',
        ),
        self::LOCATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Location corresponding to the media item.',
        ),
        self::MIME_TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MIMETYPE,
          self::VAR_DESCRIPTION => 'The MIME type of media, specified as a string.',
        ),
        self::NUM_COMMENTS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'Number of comments on the media item.',
        ),
        self::NUM_VIEWS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'Number of views for the media item.',
        ),
        self::NUM_VOTES => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'Number of votes received for voting.',
        ),
        self::RATING => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'Average rating of the media item on a scale of 0-10.',
          self::VAR_RANGE_START => 0,
          self::VAR_RANGE_END => 10,
        ),
        self::START_TIME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'For streaming/live content, datetime when the content is available (e.g. 2008-01-23T04:56:22Z).',
        ),
        self::TAGGED_PEOPLE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Array of string (IDs) of people tagged in the media item.',
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::TAGS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Tags associated with this media item.',
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::STILL_URL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'URL to a still image of the media item.',
        ),
        self::TITLE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Describing the media item.',
        ),
        // This variable is "media_type" in the upload documentation, but the mediaItem description clearly said it is "type".
        // References:
        // http://opensocial-resources.googlecode.com/svn/spec/1.1/Core-API-Server.xml#Content-Upload
        // http://opensocial-resources.googlecode.com/svn/spec/1.1/Social-Data.xml#MediaItem
        self::TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The type of media, specified as a MediaItem.Type object.',
          self::VAR_ALLOWED_VALUES => array(
            // We don't support others, just video type.
            mediamosa_rest_call::MEDIAITEM_TYPE,
          ),
        ),
        self::URL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Specifying the URL where the media can be found.',
        ),

      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Override Validate Rest Args.
  protected function validate_rest_args(array $a_var_setup) {

    // Validate first.
    parent::validate_rest_args($a_var_setup);

    $app_id = variable_get(mediamosa_settings::MEDIAMOSA_OPENAPI_OPEN_APP_ID, 0);

    if (!$app_id) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_OPENAPI_MISSING_OPEN_APP_ID);
    }
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = array(variable_get(mediamosa_settings::MEDIAMOSA_OPENAPI_OPEN_APP_ID, 0));
    $app_id = reset($app_ids);

    // Get params.

    $user_id = $this->get_param_value(self::USER_ID);

    // Standard Request Parameters.
    $since = $this->get_param_value(self::REQUEST_PARAMETER_UPDATEDSINCE_VALUE);
    $alt = $this->get_param_value(self::REQUEST_PARAMETER_FORMAT_VALUE);

    // Call related paramters.
    $asset_id = addslashes($this->get_param_value(self::ALBUM_ID));
    $mediafile_id = addslashes($this->get_param_value(self::MEDIAITEM_ID));

    // Data parameters.
    $created = $this->get_param_value(self::CREATED);
    $description = strip_tags(addslashes($this->get_param_value(self::DESCRIPTION)));
    $duration = $this->get_param_value(self::DURATION);
    $file_size = $this->get_param_value(self::FILE_SIZE);
    $language = $this->get_param_value(self::LANGUAGE);
    $last_updated = $this->get_param_value(self::LAST_UPDATED);
    $location = strip_tags(addslashes($this->get_param_value(self::LOCATION)));
    $mime_type = strip_tags(addslashes($this->get_param_value(self::MIME_TYPE)));
    $num_comments = $this->get_param_value(self::NUM_COMMENTS);
    $num_views = $this->get_param_value(self::NUM_VIEWS);
    $num_votes = $this->get_param_value(self::NUM_VOTES);
    $rating = $this->get_param_value(self::RATING);
    $start_time = $this->get_param_value(self::START_TIME);
    $tagged_people = $this->get_param_value(self::TAGGED_PEOPLE);
    if ($tagged_people) {
      foreach ($tagged_people as $key => $value) {
        $tagged_people[$key] = strip_tags(addslashes($value));
      }
    }
    $tags = $this->get_param_value(self::TAGS);
    if ($tags) {
      foreach ($tags as $key => $value) {
        $tags[$key] = strip_tags(addslashes($value));
      }
    }
    $still_url = strip_tags(addslashes($this->get_param_value(self::STILL_URL)));
    $title = strip_tags(addslashes($this->get_param_value(self::TITLE)));
    $type = $this->get_param_value(self::TYPE);
    $url = strip_tags(addslashes($this->get_param_value(self::URL)));

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Must exists.
    $asset = mediamosa_asset::must_exists($asset_id);

    // Check owner of parent asset, need ownership.
    mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID]);

    // Mediafile check / create.
    if ($mediafile_id) {
      // If we have mediafile_id, the mediafile must exists.
      $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id, $app_id);
    }
    else {
      // New mediafile.

      // Set mediafile ID to something if not present.
      $mediafile_id = mediamosa_db::uuid($app_id);

      // Fields to insert.
      $fields = array(
        mediamosa_asset_mediafile_db::GROUP_ID => NULL,
        mediamosa_asset_mediafile_db::IS_DOWNLOADABLE => mediamosa_asset_mediafile_db::IS_DOWNLOADABLE_FALSE,
        mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE,
        mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => NULL,
        mediamosa_asset_mediafile_db::FILENAME => NULL,
        mediamosa_asset_mediafile_db::URI => $url,
        mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE => NULL,
        mediamosa_asset_mediafile_db::TAG => $tags ? implode(', ', $tags) : '',
      );

      // Insert the mediafile.
      $mediafile_id = mediamosa_asset_mediafile::create($mediafile_id, $app_id, $asset_id, $user_id, $fields);
    }

    // Create metadata.

    // Metadata function call parameters.
    $action = 'replace';
    // Get full definitions.
    $metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full();
    // Mapping metadata parameters.
    $params = array(
      'created' => array($created),
      'description' => array($description),
      'language' => array($language),
      'title' => array($title),
      // Skipped fields, because we don't have metadata equivalients.
      // $duration, $file_size, $last_updated, $location, $mime_type, $num_comments, $num_views, $num_votes, $rating, $start_time, $tagged_people, $still_url, $type,
    );

    // Create the metadata.
    $value_set = mediamosa_asset_metadata::metadata_create($asset_id, $metadata_definitions_full, $params, $action);

    // Upload file.

    // The file was sent as POST data.
    $video = 'video/';
    $post_upload = (isset($_SERVER['RESPONSE_TYPE']) && drupal_substr($_SERVER['RESPONSE_TYPE'], 0, drupal_strlen($video)) == $video);
    if ($post_upload || count($_FILES)) {

      // file_size depends on method used.
      if ($post_upload) {
        $file_size = (int) $_SERVER['CONTENT_LENGTH'];
      }
      elseif (count($_FILES)) {
        $file_size = 0;
        foreach ($_FILES as $file) {
          $file_size = $file['size'];
          break;
        }
      }
      $response['file_size'] = $file_size;

      // Create job.
      $is_app_admin = FALSE;
      $retranscode = FALSE;
      $create_still = TRUE;
      $still_parameters = array(
        'still_type' => mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE,
        'still_per_mediafile' => NULL,
        'still_every_second' => NULL,
        'start_frame' => NULL,
        'end_frame' => NULL,
        'size' => NULL,
        'h_padding' => NULL,
        'v_padding' => NULL,
        'tag' => NULL,
        'frametime' => NULL,
        'width' => NULL,
        'height' => NULL,
        // Watermark.
        'watermark_id' => NULL,
        'watermark_dst_x' => NULL,
        'watermark_dst_y' => NULL,
        'watermark_pct' => NULL,
        'watermark_v_align' => NULL,
        'watermark_h_align' => NULL,
      );
      $job_id = mediamosa_job::create_job_upload($app_id,  $asset[mediamosa_asset_db::OWNER_ID], $asset[mediamosa_asset_db::GROUP_ID], $is_app_admin, $mediafile_id, $file_size, $retranscode, $create_still, $still_parameters);

      // Build the filename.
      $filename = mediamosa_storage::get_realpath_data_file($mediafile_id, TRUE);

      if ($post_upload) {
        // POST: file in the data.
        $mime_type = $_SERVER['RESPONSE_TYPE'];

        $written_bytes = mediamosa_asset_mediafile_upload::handle_upload_post($filename);
        $response['written_bytes'] = $written_bytes;
        $success = ($written_bytes ? TRUE : FALSE);
      }
      elseif (count($_FILES)) {
        // POST: Multi-part.
        $success = FALSE;
        $mime_type = NULL;
        foreach ($_FILES as $file) {
          $success = move_uploaded_file($file['tmp_name'], $filename);
          $mime_type = $file['type'];
          // We support only one file.
          // TODO Support more files.
          // TODO The OpenSocial request can refer to the content being uploaded by setting the URL to "@field:<fieldname>".
          // http://opensocial-resources.googlecode.com/svn/spec/1.1/Core-API-Server.xml#Content-Upload
          break;
        }
      }


      // When upload has failed, then set the upload job on 'failed'.
      if (!$success) {
        // Get complete job.
        $job_ext = mediamosa_job::get_job_ext($job_id);

        // Change status to failed.
        mediamosa_job::update_status($job_ext, mediamosa_job_db::JOB_STATUS_FAILED, '1.000');

        // Throw exception.
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANNOT_COPY_MEDIAFILE);
      }

      // Update the mediafile.
      $fields = array(
        mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => mediamosa_storage::get_uri(),
        mediamosa_asset_mediafile_db::FILENAME => $filename,
        mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => 'TRUE',
        mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL => mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_TRUE,
      );

      // Don't overwrite the tag when the tag was not provided.
      if (!is_null($tags)) {
        $fields[mediamosa_asset_mediafile_db::TAG] = $tags;
      }

      // Update mediafile.
      mediamosa_asset_mediafile::update($app_id, $mediafile_id, $asset[mediamosa_asset_db::OWNER_ID], $fields);

      // Update the job.
      mediamosa_job::update_progress_upload($job_id, $file_size);
    }

    // Create response.

    $response['asset_id'] = $asset_id;
    $response['mediafile_id'] = $mediafile_id;

    // Return what has been inserted into the metadata.
    foreach ($value_set as $name => $value) {
      $response[$name] = $value;
    }

    $response['mediaitem'] = array(
      'album_id' => $asset_id,
      'created' => $created,
      'description' => $description,
      'duration' => NULL,
      'file_size' => $file_size,
      'id' => $mediafile_id,
      'language' => $language,
      'last_updated' => NULL,
      'location' => NULL,
      'mime_type' => $mime_type,
      'num_comments' => NULL,
      'num_views' => NULL,
      'num_votes' => NULL,
      'rating' => NULL,
      'start_time' => NULL,
      'tagged_people' => NULL,
      'tags' => !empty($tags) ? $tags : NULL,
      'still_url' => NULL,
      'title' => $title,
      'type' => $type,
      'url' => $url,
    );

    $mediamosa->add_item($response);
  }
}
