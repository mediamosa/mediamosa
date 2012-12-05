<?php
/**
 * @file
 * The APP authentication class.
 */

class mediamosa_app_dbus {
  // ------------------------------------------------------------------ Consts.
  // End User Application authentication settings
  const APP_AUTH_CONTEXT = 'vpx';
  const APP_AUTH_SECRET_PHRASE = '41d0c72bd73afaa2c207064d81d5a3d9';
  const APP_AUTH_SERVER_GUID = 'server_guid';

  // ---------------------------------------------------------------- Functions.
  /**
   * Get the shared key for the application.
   *
   * @param string $name
   * @return string
   */
  public function get_shared_key($name) {
    return mediamosa_db::db_query(
      "SELECT #shared_key FROM {#table_name} WHERE #name = :name AND #active = :active_true",
      array(
        '#shared_key' => mediamosa_app_db::SHARED_KEY,
        '#table_name' => mediamosa_app_db::TABLE_NAME,
        '#name' => mediamosa_app_db::APP_NAME,
        ':name' => $name,
        '#active' => mediamosa_app_db::ACTIVE,
        ':active_true' => mediamosa_app_db::ACTIVE_TRUE,
      )
    )->fetchField();
  }

  /**
   * Returns the server guid.
   */
  public static function get_server_guid() {
    return self::APP_AUTH_SERVER_GUID;
  }

  /**
   * Test if application is enabled (active).
   *
   * @param string $name
   * @return boolean
   */
  public function is_app_active($name) {
    return mediamosa_db::db_query(
      "SELECT #active FROM {#table_name} WHERE #name = :name",
      array(
        '#table_name' => mediamosa_app_db::TABLE_NAME,
        '#name' => mediamosa_app_db::APP_NAME,
        ':name' => $name,
        '#active' => mediamosa_app_db::ACTIVE,
      )
    )->fetchField() == mediamosa_app_db::ACTIVE_TRUE ? TRUE : FALSE;
  }


  /**
   * Get the app.
   *
   * @param string $name
   * @return array
   */
  public static function get_app($name) {
    return mediamosa_app::get_by_name($name);
  }

  /**
   * A (cookie) ID authentication method.
   *
   * @param string $command
   * @param string $param
   * @return boolean / string
   */
  function dbus_cookie_sha1($command, $param) {

    switch ($command) {
      case 'AUTH':
        if (!mediamosa_app_dbus::is_app_active($param)) {
          session_destroy();
          return "ERROR invalid account";
        }

        $_SESSION['mediamosa_dbus']['username'] = $param;
        $_SESSION['mediamosa_dbus']['cookie_id'] = 0;
        $_SESSION['mediamosa_dbus']['challenge'] = md5(microtime() . self::APP_AUTH_SECRET_PHRASE);
        return
          'DATA ' . self::APP_AUTH_CONTEXT . ' ' .
          $_SESSION['mediamosa_dbus']['cookie_id'] . ' ' .
          $_SESSION['mediamosa_dbus']['challenge'];

      case 'DATA':
        $cookie = self::get_shared_key($_SESSION['mediamosa_dbus']['username']);
        list($random, $response) = explode(' ', $param);
        return $response == sha1($_SESSION['mediamosa_dbus']['challenge'] . ':' . $random . ':' . $cookie);
    }

    return 'ERROR syntax error';
  }

  /**
   * Process the incoming DBUS line.
   *
   * @param string $line
   * @return string
   */
  public function app_dbus_protocol($line) {
    // Initialize identification.
    $auth_methods = array(
      'DBUS_COOKIE_SHA1' => 'dbus_cookie_sha1'
    );

    // Make sure session is clean.
    if (!isset($_SESSION['mediamosa_dbus'])) {
      $_SESSION['mediamosa_dbus'] = array(
        'mechanism'  => FALSE,
        'challenge'  => FALSE,
        'username'   => ''
      );
    }

    if (strpos($line, ' ') === FALSE) {
      return 'ERROR unknown error';
    }

    // Parse and handle command.
    list($command, $param) = explode(' ', trim($line), 2);

    switch ($command) {
      case 'AUTH':
        if ($param == '') {
          return 'REJECTED ' . implode(' ', array_keys($auth_methods));
        }

        list($mechanism, $init) = explode(' ', $param, 2);
        if (!isset($auth_methods[$mechanism])) {
          return 'ERROR unknown mechanism';
        }

        // Will assert if not exists.
        assert(method_exists('mediamosa_app_dbus', $auth_methods[$mechanism]));

        $_SESSION['mediamosa_dbus']['mechanism'] = $auth_methods[$mechanism];
        $_SESSION['mediamosa_dbus']['username'] = FALSE;
        $_SESSION['mediamosa_dbus']['challenge'] = FALSE;
        $param = $init;

      case 'DATA':
        // Test if we already been identified.
        if (isset($_SESSION['mediamosa_app']) && $_SESSION['mediamosa_app']) {
          return 'ERROR already identified';
        }

        // Check if chosen mechanism is available.
        if ($_SESSION['mediamosa_dbus']['mechanism'] == FALSE) {
          return 'ERROR mechanism not specified';
        }

        // Call the chosen method.
        $result = call_user_func_array(
          array('mediamosa_app_dbus', $_SESSION['mediamosa_dbus']['mechanism']),
          array($command, $param)
        );

        if (!is_bool($result)) {
          return $result;
        }

        if (!$result) {
          $_SESSION['mediamosa_dbus']['mechanism'] = FALSE;
          $_SESSION['mediamosa_dbus']['challenge'] = FALSE;
          return 'REJECTED';
        }

        $a_app = mediamosa_app_dbus::get_app($_SESSION['mediamosa_dbus']['username']);
        // All ok?
        if (!$a_app) {
          unset($_SESSION['mediamosa_dbus']);
          return 'ERROR database error';
        }

        // Set the identification in session.
        $_SESSION['mediamosa_app'] = array(
          'app_id' =>  $a_app[mediamosa_app_db::APP_ID],
          'name' => $a_app[mediamosa_app_db::APP_NAME]
        );

        return 'OK ' . self::get_server_guid();

      case 'BEGIN':
        if ($_SESSION['mediamosa_app'] == FALSE) {
          return 'ERROR not identified';
        }

        return mediamosa_response::SUCCESS;

      case 'CANCEL':
        unset($_SESSION['mediamosa_dbus']);
        unset($_SESSION['mediamosa_app']);
        return 'REJECTED';

      case 'ERROR':
        break;
    }

    return 'ERROR unknown error';
  }
}
