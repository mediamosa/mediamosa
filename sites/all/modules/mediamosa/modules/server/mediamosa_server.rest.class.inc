<?php
/**
 * @file
 * REST calls for mediamosa_server.
 */

/**
 * URI: /server/status
 * Method: GET
 *
 * Internal only.
 */
class mediamosa_rest_call_server_status extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  const INCLUDE_STATUS = 'include_status';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::INCLUDE_STATUS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include status information.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      )
    );

    return self::get_var_setup_default($var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $include_status = $this->get_param_value(self::INCLUDE_STATUS);

    // Fill up info about the server.
    $item = array(
      'server_id' => mediamosa::get_server_id(),
      'uri' => mediamosa::get_request_uri(),
      'host' => $_SERVER['SERVER_NAME'],
    );

    if ($include_status) {
      // Check run-time requirements and status information.
      // Reset = TRUE, With Drupal variable = FALSE.
      $statuses_realtime = module_invoke_all('mediamosa_status_collect_realtime');

      // Add realtime.
      foreach ($statuses_realtime as $key => $status) {

        if (!empty($status['in_status_output'])) {
          $statuses_realtime[$key]['title'] = $statuses_realtime[$key]['title'];
          $statuses_realtime[$key . '_realtime'] = $statuses_realtime[$key];
        }

        unset($statuses_realtime[$key]);
      }

      $statuses = module_invoke_all('mediamosa_status_collect', TRUE);

      $item['server_status'] = array_merge_recursive($statuses_realtime, $statuses);
    }

    $mediamosa->add_item($item);
  }
}

/**
 * URI: /server/upload
 *
 * Returns the server upload settings.
 */
class mediamosa_rest_call_server_upload extends mediamosa_rest_call {

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    return self::get_var_setup_default($var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get all upload servers and call us.
    $upload_servers = mediamosa_server::get_upload_servers(array(mediamosa_server_db::SERVER_STATUS_ON));
    foreach ($upload_servers as $upload_server) {
      $server_id = $upload_server['nid'];
      unset($upload_server['nid'], $upload_server['vid']);
      $upload_server = array_merge(array('server_id' => $server_id), $upload_server);

      $xml = mediamosa_server::getStatusUpload($upload_server[mediamosa_server_db::SERVER_URI], $server_id);

      if (!empty($xml->server_settings->php)) {
        foreach ($xml->server_settings->php as $php) {
          foreach ($php as $name => $value) {
            $upload_server['php'][(string) $name] = (string) $value;
          }
        }
      }
      $real_max_upload_bytes = 0;
      if (!empty($xml->server_settings->max_upload_bytes)) {
        $real_max_upload_bytes = (int) $xml->server_settings->max_upload_bytes;
      }
      $upload_server['real_max_upload_bytes'] = $real_max_upload_bytes;

      $mediamosa->add_item($upload_server);
    }
  }
}

/**
 * URI: /server/upload/$server_id
 *
 * Returns the server upload settings. Should be used run on the upload server.
 */
class mediamosa_rest_call_server_upload_details extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  const SERVER_ID = 'server_id';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::SERVER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'The numeric ID of the server.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    return self::get_var_setup_default($var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $server_id = $this->get_param_value(self::SERVER_ID);

    $upload_server = mediamosa_server::get_upload_by_nid($server_id);
    if (empty($upload_server)) {
      throw new mediamosa_exception(mediamosa_error::ERRORCODE_SERVER_UPLOAD_NOT_FOUND);
    }

    // Fill up info about the server.
    $item = array(
      'server_id' => mediamosa::get_server_id(),
      'uri' => mediamosa::get_request_uri(),
      'host' => $_SERVER['SERVER_NAME'],
    );

    $item['server_info'] = $upload_server;

    // PHP settings.
    $upload_max_filesize = mediamosa_lib::php_config_value_to_bytes(ini_get('upload_max_filesize'));
    $post_max_size = mediamosa_lib::php_config_value_to_bytes(ini_get('post_max_size'));

    // Unlimited?
    if (empty($upload_max_filesize)) {
      $upload_max_filesize = $post_max_size;
    }
    if (empty($post_max_size)) {
      $post_max_size = $upload_max_filesize;
    }
    $max_upload_bytes = max($upload_max_filesize, $post_max_size);
    $max_upload_bytes = empty($max_upload_bytes) ? PHP_INT_MAX : $max_upload_bytes;

    $item['server_settings'] = array(
      'php' => array(
        'file_uploads' => ini_get('file_uploads'),
        'upload_tmp_dir' => ini_get('upload_tmp_dir'),
        'upload_max_filesize' => ini_get('upload_max_filesize'),
        'upload_max_filesize_bytes' => mediamosa_lib::php_config_value_to_bytes(ini_get('upload_max_filesize')),
        'post_max_size' => ini_get('post_max_size'),
        'post_max_size_bytes' => mediamosa_lib::php_config_value_to_bytes(ini_get('post_max_size')),
        'max_file_uploads' => ini_get('max_file_uploads'),
      ),
      'max_upload_bytes' => $max_upload_bytes,
    );

    $mediamosa->add_item($item);
  }
}
