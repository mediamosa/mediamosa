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
  // Rest vars;
  const INCLUDE_STATUS = 'include_status';

  // ------------------------------------------------------------------ Get Var Setup.
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
