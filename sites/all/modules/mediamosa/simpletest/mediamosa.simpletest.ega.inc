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
  * Higher class for REST call unit tests with EGA (app) login.
  */

/**
 * Use this class for tests that uses a EGA default login for doing
 * REST calls.
 */
class MediaMosaTestCaseEga extends MediaMosaTestCase {
  // ------------------------------------------------------------------ Members.
  protected $a_app = NULL;
  protected $a_app_2 = NULL;
  protected $a_app_3 = NULL;

  // By default all goes through app 1.
  protected $switch_to_app = NULL;

  // Contains the login sessions, app_id => array('curlHandle' => curlHandle)
  protected $a_app_sessions = array();

  // Array with mediafile ids of mediafiles that have files that need to be
  // deleted on teardown.
  private $mediafiles_cleanup = array();

  // TRUE/FALSE setting that allows the default server settings instead of the
  // current server settings to be copied into our test environment.
  protected $default_servers = FALSE; // FALSE: use the current server setup.

  // ------------------------------------------------------------------ Functions.
  /**
   * Implements setUp().
   */
  protected function setUp() {
    // Reset.
    $this->switch_to_app = NULL;

    // Get the current mediamosa server content.
    // @todo: convert these to nodes.
    $servers = mediamosa_server::search();
    $server_containers = mediamosa_server::search_containers();
    $server_tools = mediamosa_server::search_tools();

    // Run parent first so we are inside sandbox.
    // Call parent::setUp and preserve arguments.
    $args = func_get_args();

    // Create and setup the CORE module.
    $args = array_unique(array_merge(array(
      'mediamosa_user',
      'mediamosa_app_authentication',
      'mediamosa_asset',
      'mediamosa_aut',
      'mediamosa_collection',
      'mediamosa_server',
      'mediamosa_statistics',
      'mediamosa_ftp_user',
    ), $args));

    if (drupal_substr(phpversion(), 0, 3) < '5.3') {
      call_user_func_array(array($this, 'parent::setUp'), $args);
    }
    else {
      call_user_func_array('parent::setUp', $args);
    }

    // Now we are in sandbox, create default APP, our default EGA.
    $this->a_app = $this->createApp();

    // Create 2 extra.
    $this->a_app_2 = $this->createApp(array(mediamosa_app_db::APP_ID => $this->a_app[mediamosa_app_db::APP_ID] + 1)); // in case we need other.
    $this->a_app_3 = $this->createApp(array(mediamosa_app_db::APP_ID => $this->a_app_2[mediamosa_app_db::APP_ID] + 1)); // in case we need other.

    if (!$this->default_servers) {
      // Truncate current and insert new.
      db_truncate(mediamosa_server_db::TABLE_NAME)->execute();

      // Copy the current servers to sandbox.
      foreach ($servers as $server) {
        db_insert(mediamosa_server_db::TABLE_NAME)
          ->fields($server)
          ->execute();
      }

      // Truncate current and insert new containers.
      db_truncate(mediamosa_server_streaming_container_db::TABLE_NAME)->execute();

      // Copy the containers.
      // Copy the current server containers to sandbox.
      foreach ($server_containers as $server_container) {
        db_insert(mediamosa_server_streaming_container_db::TABLE_NAME)
          ->fields($server_container)
          ->execute();
      }

      // Truncate current and insert new containers.
      db_truncate(mediamosa_server_tool_db::TABLE_NAME)->execute();

      // Copy the server tools.
      // Copy the current server tools to sandbox.
      foreach ($server_tools as $server_tool) {
        db_insert(mediamosa_server_tool_db::TABLE_NAME)
          ->fields($server_tool)
          ->execute();
      }
    }

    // Create and login user
    $rest_user = $this->drupalCreateUser(array(mediamosa_permission::ADMIN_MEDIAMOSA));
    $this->drupalLogin($rest_user);

    // Do EGA / APP login (app 1).
    // If your test needs more, call doEgaLogin_2 and doEgaLogin_3 in your test.
    $this->doEgaLogin();
  }

  /**
   * Implements tearDown().
   */
  protected function tearDown() {

    // Clean up stuff.
    mediamosa_ticket::ticket_cleanup();

    // Now clean out all mediafile that might have files.
    $mediafile_ids = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')->fields('mf', array(mediamosa_asset_mediafile_db::ID))->execute()->fetchCol();
    foreach ($mediafile_ids as $mediafile_id) {
      $this->addToCleanUp($mediafile_id);
    }

    // If its ever more than 100, then Im sure we are not in our simpletest
    // database by error. Prevent this!
    if (count($this->mediafiles_cleanup) < 100) {
      // Delete possible uploaded files.
      foreach ($this->mediafiles_cleanup as $mediafile_id) {
        // Get the mediafile.
        $mediafile = mediamosa_asset_mediafile::get($mediafile_id);

        // Might already been deleted.
        if ($mediafile) {
          // Delete uploaded file by deleting the mediafile.
          mediamosa_asset_mediafile::delete($mediafile_id);

          // Check if the file has been deleted.
          $this->assertFalse(file_exists(mediamosa_configuration_storage::mediafile_filename_get($mediafile)), 'File has been deleted.');
        }
      }
    }
    else {
      assert(0);
    }


    // Clean up array.
    $this->mediafiles_cleanup = array();

    // Call parent.
    parent::tearDown();
  }

  /*
   * Delete mediafile_id from clean up.
   * @param $mediafile_id
   */
  protected function deleteFromCleanUp($mediafile_id) {
    // Delete the unnecessary mediafile_id from the clean up array.
    if (($key = array_search($mediafile_id, $this->mediafiles_cleanup)) !== FALSE) {
      unset($this->mediafiles_cleanup[$key]);
    }
  }

  /**
   * Add mediafile_id to clean up.
   * @param $mediafile_id
   */
  protected function addToCleanUp($mediafile_id) {
    if (empty($this->mediafiles_cleanup[$mediafile_id])) {
      // Add to clean up.
      $this->mediafiles_cleanup[$mediafile_id] = $mediafile_id;
    }
  }

  /**
   * Login using generated app, name and shared_key (password).
   */
  protected function doEgaLogin(array $a_app = NULL) {
    if (empty($a_app)) {
      $a_app = $this->a_app;
    }

    // Unset the session.
    unset($this->a_app_sessions[$a_app[mediamosa_rest_call::APP_ID]]);
    $this->curlHandle = NULL;

    // Do login.
    $app_id = $a_app[mediamosa_app_db::APP_ID];
    $app_name = $a_app[mediamosa_app_db::APP_NAME];
    $shared_key = $a_app[mediamosa_app_db::SHARED_KEY];

    // Step 1: start auth and capture cookie.

    // We must call parent to skip the different session stuff.
    $response = parent::restCall('login', 'POST', array('dbus' => 'AUTH DBUS_COOKIE_SHA1 ' . $app_name));
    $this->pass(check_plain(var_export($response['xml'], TRUE)));
    $dbus_data = explode(' ', $response['xml']->items->item->dbus);

    $this->assertTrue(drupal_substr($response['xml']->items->item->dbus, 0, 5) == 'DATA ', 'Login fase 1 success');

    list($cookie) = explode(';', $response['headers']['Set-Cookie']);
    $this->assertTrue(isset($response['headers']['Set-Cookie']), 'Cookie received in HTTP header: ' . $cookie);

    // Step 2: send the credentials.
    $challenge = $dbus_data[3];
    $random = mediamosa_unicode::substr(md5(microtime(TRUE)), 0, 10);
    $post_data = array('dbus' => 'DATA ' . $random . ' ' . sha1(sprintf('%s:%s:%s', $challenge, $random, $shared_key)));

    // We must call parent to skip the different session stuff.
    $response = parent::restCall('login', 'POST', $post_data);

    $this->assertTrue($response['xml']->items->item->dbus == 'OK ' . mediamosa_app_authentication::get_server_guid(), 'Login fase 2 success');
    $this->pass(check_plain(var_export($response['xml'], TRUE)));

    // Store session stuff so we can switch sessions during REST calls.
    // Although cookieFile is not 'working' or doesnt do much, will included anyway.
    $this->a_app_sessions[$app_id] = array('curlHandle' => $this->curlHandle, 'cookieFile' => $this->cookieFile);
  }

  /*
   * Create login for EGA app 2.
   */
  protected function doEgaLogin_2() {
    $this->doEgaLogin($this->a_app_2);
  }

  /*
   * Create login for EGA app 3.
   */
  protected function doEgaLogin_3() {
    $this->doEgaLogin($this->a_app_3);
  }

  /**
   * Call to switch session to EGA.
   * Better to use toEga_x().
   */
  protected function toEga($app_id) {
    $result = $this->switch_to_app;
    $this->switch_to_app = $app_id;
    return $result;
  }

  /**
   * Call to switch session to EGA 1.
   * Use the return value to switch back, using toEga().
   */
  protected function toEga_1() {
    return $this->toEga($this->a_app[mediamosa_app_db::APP_ID]);
  }

  /**
   * Call to switch session to EGA 2.
   * Use the return value to switch back, using toEga().
   */
  protected function toEga_2() {
    return $this->toEga($this->a_app_2[mediamosa_app_db::APP_ID]);
  }

  /**
   * Call to switch session to EGA 3.
   * Use the return value to switch back, using toEga().
   */
  protected function toEga_3() {
    return $this->toEga($this->a_app_3[mediamosa_app_db::APP_ID]);
  }

  /**
   * Enable the master/slave on app.
   */
  protected function enableMasterSlave($master_app_id) {

    // Which app to enable?
    $slave_app_id = $this->switch_to_app;

    // Get the current master/slave.
    $allowed_masterslave_apps = mediamosa_app::get_allowed_master_slave_apps($master_app_id);

    if (!in_array($slave_app_id, $allowed_masterslave_apps)) {
      $allowed_masterslave_apps[] = $slave_app_id;
    }

    // Change the master setting.
    mediamosa_app::update($master_app_id, array(mediamosa_app_db::ALLOW_MASTERSLAVE_APPS => serialize($allowed_masterslave_apps)));
  }

  /**
   * Enable the master/slave on app.
   */
  protected function disableMasterSlave($master_app_id) {

    // Which app to disable?
    $slave_app_id = $this->switch_to_app;

    // Get the current master/slave.
    $allowed_masterslave_apps = mediamosa_app::get_allowed_master_slave_apps($master_app_id);

    $key = array_search($slave_app_id, $allowed_masterslave_apps);

    if ($key === FALSE) {
      return; // Not found, not enabled.
    }

    // Remove it.
    unset($allowed_masterslave_apps[$key]);

    // Change the master setting.
    mediamosa_app::update($master_app_id, array(mediamosa_app_db::ALLOW_MASTERSLAVE_APPS => serialize($allowed_masterslave_apps)));
  }

  /**
   * Do a REST call inside the sandbox.
   *
   * @param array $a_app
   *  The app_id used during the call, this can be more than one.
   * @param string $uri
   *  The uri of the REST call.
   * @param string $request_method
   *  POST / GET / PUT / DELETE.
   * @param array $parameters
   *  name => value
   * @param array $a_headers
   *  Name: value
   * @param array $expected_result_ids
   *  Contains the mediamosa_error::ERRORCODE_* that we expect. For example, you
   *  might expect an error instead of OKAY; specify the error code here.
   */
  protected function restCall($uri, $request_method = 'POST', array $parameters = array(), array $a_headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {

    // Switch to what EGA login?
    $switch_to_app = $this->switch_to_app;

    // Now if app_id is not set, we use app_id of app 1.
    if (empty($switch_to_app)) {
      $switch_to_app = $this->a_app[mediamosa_app_db::APP_ID];
    }

    // Save old handle, if we overwrite it, put it back.
    $old_curlHandle = NULL;

    // If session was set, use it.
    if (isset($this->a_app_sessions[$switch_to_app])) {
      $old_curlHandle = $this->curlHandle;
      $this->curlHandle = $this->a_app_sessions[$switch_to_app]['curlHandle'];
    }

    // Do the REST call.
    try {
      $result = parent::restCall($uri, $request_method, $parameters, $a_headers, $expected_result_ids, $do_internal);
    }
    catch (Exception $e) {
      if (isset($old_curlHandle)) {
        $this->curlHandle = $old_curlHandle;
      }
      throw $e;
    }

    // If we changed curlHandle, then put it back.
    if (isset($old_curlHandle)) {
      $this->curlHandle = $old_curlHandle;
    }

    return $result;
  }

  /**
   * Wrapper for restCall, for app 2.
   */
  protected function restCall_2($uri, $request_method = 'POST', array $parameters = array(), array $a_headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {

    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 2.
    $this->switch_to_app = $this->a_app_2[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $result = $this->restCall($uri, $request_method, $parameters, $a_headers, $expected_result_ids, $do_internal);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;

    // Return result.
    return $result;
  }

  /**
   * Wrapper for restCall, for app 3.
   */
  protected function restCall_3($uri, $request_method = 'POST', array $parameters = array(), array $a_headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {

    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 2.
    $this->switch_to_app = $this->a_app_3[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $result = $this->restCall($uri, $request_method, $parameters, $a_headers, $expected_result_ids, $do_internal);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;

    // Return result.
    return $result;
  }

  /**
   * Wrapper for POST restCall, for app 3.
   */
  protected function restCallPost_2($uri, array $parameters = array(), array $a_headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Do call.
    return $this->restCall_2($uri, 'POST', $parameters, $a_headers, $expected_result_ids, $do_internal);
  }

  /**
   * Wrapper for POST restCall, for app 3.
   */
  protected function restCallPost_3($uri, array $parameters = array(), array $a_headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Do call.
    return $this->restCall_3($uri, 'POST', $parameters, $a_headers, $expected_result_ids, $do_internal);
  }

  /**
   * Wrapper for GET restCall, for app 2.
   */
  protected function restCallGet_2($uri, array $parameters = array(), array $a_headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Do call.
    return $this->restCall_2($uri, 'GET', $parameters, $a_headers, $expected_result_ids, $do_internal);
  }

  /**
   * Wrapper for GET restCall, for app 3.
   */
  protected function restCallGet_3($uri, array $parameters = array(), array $a_headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Do call.
    return $this->restCall_3($uri, 'GET', $parameters, $a_headers, $expected_result_ids, $do_internal);
  }

  /**
   * Wrapper for PUT restCall, for app 2.
   */
  protected function restCallPut_2($uri, array $parameters = array(), array $a_headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Do call.
    return $this->restCall_2($uri, 'PUT', $parameters, $a_headers, $expected_result_ids, $do_internal);
  }

  /**
   * Wrapper for PUT restCall, for app 3.
   */
  protected function restCallPut_3($uri, array $parameters = array(), array $a_headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Do call.
    return $this->restCall_3($uri, 'PUT', $parameters, $a_headers, $expected_result_ids, $do_internal);
  }

  /**
   * Basic upload of test wmf file.
   */
  protected function uploadTestFile($filename = '') {
    // Create an asset.
    $asset_id = $this->createAsset();

    // Create mediafile.
    $mediafile_id = $this->createMediafile($asset_id);

    $filename = $this->uploadTestFileForMediafile($asset_id, $mediafile_id, $filename);

    // Lets return the mediafile_id for now.
    return array('asset_id' => $asset_id, 'mediafile_id' => $mediafile_id, 'filename' => $filename);
  }

  /**
   * Basic upload of test wmf file.
   */
  protected function uploadTestFileForMediafile($asset_id, $mediafile_id, $filename = '') {
    // Create upload ticket.
    $uploadticket = $this->createMediafileUploadTicket($mediafile_id);

    $result = mediamosa_db::db_query('SELECT * FROM {mediamosa_ticket}');
    foreach ($result as $row) {
      $this->var_export($row, 'Dump Ticket');
    }

    // Check ticket link etc.
    $this->assertTrue(!empty($uploadticket['action']), 'action set');
    $this->assertTrue(!empty($uploadticket['uploadprogress_url']), 'uploadprogress_url set');

    // Get link to upload file.
    $filename = empty($filename) ? self::getTestVideo() : $filename;

    // URL.
    $action = $uploadticket['action'];

    // File.
    $a_post['file'] = '@' . $filename;

    // No headers.
    $headers = array();

    // Upload the test file.
    $out = $this->curlExec(array(CURLOPT_URL => $action, CURLOPT_POST => TRUE, CURLOPT_POSTFIELDS => $a_post, CURLOPT_HTTPHEADER => $headers));

    // Add for deletion.
    $this->mediafiles_cleanup[] = $mediafile_id;

    // Replace original page output with new output from redirected page(s).
    $new = $this->checkForMetaRefresh();
    if ($new) {
      $out = $new;
    }

    // Our upload.
    $this->verbose('POST request to: ' . $action .
                   '<hr />Ending URL: ' . $this->getUrl() .
                   '<hr />Fields: ' . highlight_string('<?php ' . var_export($a_post, TRUE), TRUE) .
                   '<hr />' . $out);

    list($response, $data) = explode("\r\n\r\n", $out, 2);
    $response = preg_split("/\r\n|\n|\r/", $response);

    // Parse the response status line.
    list($protocol, $code, $status_message) = explode(' ', trim(array_shift($response)), 3);

    // Check 200 resonse.
    $this->assertTrue($code == 200, 'Upload HTML response 200.');

    // Get the upload job.
    $a_job = mediamosa_job::get_by_mediafileid($mediafile_id);
    $this->var_export($a_job, 'Dump Job');

    // Other.
    $a_job_upload = mediamosa_job_upload::get($a_job[mediamosa_job_db::ID]);
    $this->var_export($a_job_upload, 'Dump Job Upload');

    // Get the mediafile.
    $mediafile = mediamosa_asset_mediafile::get($mediafile_id);

    // Check if the file has been uploaded.
    $this->assertTrue(file_exists(mediamosa_configuration_storage::mediafile_filename_get($mediafile)), 'File has been uploaded.');

    // Lets return the filename.
    return mediamosa_configuration_storage::mediafile_filename_get($mediafile);
  }

  /**
   * Basic upload of still.
   */
  protected function uploadTestStill($order = 0, $default = FALSE) {
    // Create an asset.
    $asset_id = $this->createAsset();

    // Create mediafile.
    $mediafile_id = $this->createMediafile($asset_id);

    // Create upload ticket.
    $uploadticket = $this->createMediafileUploadTicket($mediafile_id);

    $result = mediamosa_db::db_query('SELECT * FROM {mediamosa_ticket}');
    foreach ($result as $row) {
      $this->var_export($row, 'Dump Ticket');
    }

    // Check ticket link etc.
    $this->assertTrue(!empty($uploadticket['action']), 'action set');
    $this->assertTrue(!empty($uploadticket['uploadprogress_url']), 'uploadprogress_url set');
    $this->assertTrue(!empty($uploadticket['ticket_id']), 'ticket_id set');

    // Get link to upload file.
    $filename = self::getTestImage_NotBlackJpg();

    // URL.
    // Still upload does not use the action from the upload ticket.
    $uri = strtr('asset/@asset_id/still/upload', array('@asset_id' => $asset_id));

    // Post parameters.
    $postfields['file'] = '@' . $filename;
    $postfields['mediafile_id'] = $mediafile_id;
    $postfields['upload_ticket'] = $uploadticket['ticket_id'];
    $postfields['order'] = $order;
    $postfields['default'] = $default ? 'true' : 'false';

    // No headers.
    $headers = array();

    // Upload the test file.
    $out = $this->curlExec(array(CURLOPT_URL => url($uri, array('absolute' => TRUE)), CURLOPT_POST => TRUE, CURLOPT_POSTFIELDS => $postfields, CURLOPT_HTTPHEADER => $headers));

    // Add for deletion.
    $this->mediafiles_cleanup[] = $mediafile_id;

    // Replace original page output with new output from redirected page(s).
    $new = $this->checkForMetaRefresh();
    if ($new) {
      $out = $new;
    }

    // Our upload.
    $this->verbose('POST request to: ' . url($uri, array('absolute' => TRUE)) .
                   '<hr />Ending URL: ' . $this->getUrl() .
                   '<hr />Fields: ' . highlight_string('<?php ' . var_export($postfields, TRUE), TRUE) .
                   '<hr />' . $out);

    list($response, $data) = explode("\r\n\r\n", $out, 2);
    $response = preg_split("/\r\n|\n|\r/", $response);

    try {
      $xml = new SimpleXMLElement($data);
    }
    catch (Exception $e) {
      $this->assert(FALSE, 'simple XML creation failed! ' . $e->getMessage());
    }

    // Get the still ID.
    $still_id = (string)$xml->items->item->still_id;

    // Parse the response status line.
    list($protocol, $code, $status_message) = explode(' ', trim(array_shift($response)), 3);

    // Check 200 resonse.
    $this->assertTrue($code == 200, 'Upload HTML response 200.');

    // Get path to still.
    $still_filename = mediamosa_configuration_storage::mediafile_still_filename_get($still_id);

    // Add to unlinks.
    $this->unlinks[] = $still_filename;

    // Check if the file has been uploaded.
    $this->assertTrue(file_exists($still_filename), 'File has been uploaded.');

    // Lets return the mediafile_id for now.
    return array('asset_id' => $asset_id, 'mediafile_id' => $mediafile_id, 'filename' => $still_filename);
  }

  /**
   * Upload of still on a existing mediafile.
   */
  protected function uploadTestStillAttach($order = 0, $default = FALSE) {
    // Create an asset.
    $asset_id = $this->createAsset();

    // Create mediafile.
    $mediafile_id = $this->createMediafile($asset_id);

    // Create upload ticket.
    $uploadticket = $this->createMediafileUploadTicket($mediafile_id);

    $result = mediamosa_db::db_query('SELECT * FROM {mediamosa_ticket}');
    foreach ($result as $row) {
      $this->var_export($row, 'Dump Ticket');
    }

    // Check ticket link etc.
    $this->assertTrue(!empty($uploadticket['action']), 'action set');
    $this->assertTrue(!empty($uploadticket['uploadprogress_url']), 'uploadprogress_url set');
    $this->assertTrue(!empty($uploadticket['ticket_id']), 'ticket_id set');

    // Get link to upload file.
    $filename = self::getTestImage_NotBlackJpg();

    // URL.
    // Still upload does not use the action from the upload ticket.
    $uri = $uploadticket['action'] . '&still_upload=true';

    // Post parameters.
    $postfields['file'] = '@' . $filename;
    $postfields['mediafile_id'] = $mediafile_id;
    $postfields['upload_ticket'] = $uploadticket['ticket_id'];
    $postfields['order'] = $order;
    $postfields['default'] = $default ? 'true' : 'false';

    // No headers.
    $headers = array();

    // Upload the test file.
    $out = $this->curlExec(array(CURLOPT_URL => url($uri, array('absolute' => TRUE)), CURLOPT_POST => TRUE, CURLOPT_POSTFIELDS => $postfields, CURLOPT_HTTPHEADER => $headers));

    // Add for deletion.
    $this->mediafiles_cleanup[] = $mediafile_id;

    // Replace original page output with new output from redirected page(s).
    $new = $this->checkForMetaRefresh();
    if ($new) {
      $out = $new;
    }

    // Our upload.
    $this->verbose('POST request to: ' . url($uri, array('absolute' => TRUE)) .
                   '<hr />Ending URL: ' . $this->getUrl() .
                   '<hr />Fields: ' . highlight_string('<?php ' . var_export($postfields, TRUE), TRUE) .
                   '<hr />' . $out);

    list($response, $data) = explode("\r\n\r\n", $out, 2);
    $response = preg_split("/\r\n|\n|\r/", $response);

    try {
      $xml = new SimpleXMLElement($data);
    }
    catch (Exception $e) {
      $this->assert(FALSE, 'simple XML creation failed! ' . $e->getMessage());
    }

    // Get the still ID.
    $still_id = (string)$xml->items->item->still_id;

    // Add for deletion.
    $this->mediafiles_cleanup[] = $still_id;

    // Parse the response status line.
    list($protocol, $code, $status_message) = explode(' ', trim(array_shift($response)), 3);

    // Check 200 resonse.
    $this->assertTrue($code == 200, 'Upload HTML response 200.');

    // Get path to still.
    $still_filename = mediamosa_configuration_storage::mediafile_still_filename_get($still_id);

    // Add to unlinks.
    $this->unlinks[] = $still_filename;

    // Check if the file has been uploaded.
    $this->assertTrue(file_exists($still_filename), 'File has been uploaded.');

    // Lets return the mediafile_id for now.
    return array('asset_id' => $asset_id, 'mediafile_id' => $mediafile_id, 'filename' => $still_filename);
  }

  /**
   * Compare input insert/update with result.
   * @param string $table
   *  Table name.
   * @param string $object_id
   * @param array $parameters
   *  Values to compare. Remember will only work if parameters match the database field names.
   * @param array $a_db_row
   */
  protected function do_compare($table, $object_id, array $parameters, array $a_db_row) {
    $fail = FALSE;
    foreach ($parameters as $key => $value) {
      if (!isset($a_db_row[$key]) || $a_db_row[$key] != $value) {
        $this->fail(strtr("Row value of table @table with row ID @object_id different from input; got '@output', expecting '@input' on field @key.",
          array(
            '@table' => $table,
            '@object_id' => $object_id,
            '@input' => $value,
            '@output' => $a_db_row[$key],
            '@key' => $key
          )
        ));

        $fail = TRUE;
      }
    }

    if (!$fail) {
      $this->pass(strtr('Row values of table @table with row ID @object_id passed.',
        array(
          '@table' => $table,
          '@object_id' => $object_id,
        )
      ));
    }
  }

  // ------------------------------------------------------------------ Get items functions.
  /**
   * Get an asset.
   * Based on the GET rest call /asset/$asset_id (GET)
   */
  protected function getAsset($asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_get::IS_APP_ADMIN => 'false',
      mediamosa_rest_call_asset_get::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Required.
    $uri = strtr('asset/@asset_id', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Get asset with ID: @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Must be ok to return object.
    if (!in_array(mediamosa_error::ERRORCODE_OKAY, $expected_result_ids)) {
      return FALSE;
    }

    $asset = mediamosa_lib::simplexml2array($response['xml']->items->item[0]);

    // Did we get something?
    $this->assertTrue(
      $asset[mediamosa_asset_db::ID] == $asset_id,
      'Got the asset'
    );

    // Return data.
    return $asset;
  }

  /**
   * Asset search.
   * Based on the GET rest call /asset (GET)
   */
  protected function searchAsset($limit, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_get::IS_APP_ADMIN => 'false',
      mediamosa_rest_call_asset_get::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_asset_get::LIMIT => $limit,
    );

    // Required.
    $uri = 'asset';

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Search asset, got result @result (@result_description)",
        array(
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Must be ok to return object.
    if (!in_array(mediamosa_error::ERRORCODE_OKAY, $expected_result_ids)) {
      return FALSE;
    }

    $a_assets = mediamosa_lib::responsexml2array($response['xml']);

    // Return data.
    return $a_assets;
  }

  /**
   * Get an asset.
   * Based on the GET rest call /asset/$asset_id/mediafile (GET)
   */
  protected function getAssetMediafile($asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_get::IS_APP_ADMIN => 'false',
      mediamosa_rest_call_asset_get::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Required.
    $uri = strtr('asset/@asset_id/mediafile', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Get asset/mediafile with ID: @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Must be ok to return object.
    if (!in_array(mediamosa_error::ERRORCODE_OKAY, $expected_result_ids)) {
      return FALSE;
    }

    $a_asset = mediamosa_lib::responsexml2array($response['xml']);

    // Return data.
    return $a_asset;
  }

  /**
   * Get an mediafile.
   * Based on the rest call /mediafile/$mediafile_id (GET)
   */
  protected function getMediafile($mediafile_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Set uri.
    $uri = strtr('mediafile/@mediafile_id', array('@mediafile_id' => $mediafile_id));

    // Do get call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Get mediafile with ID: @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Must be ok to return object.
    if (!in_array(mediamosa_error::ERRORCODE_OKAY, $expected_result_ids)) {
      return FALSE;
    }

    // Get response.
    $mediafile = mediamosa_lib::simplexml2array($response['xml']->items->item[0]);

    // Did we get something?
    $this->assertTrue(
      $mediafile[mediamosa_asset_mediafile_db::ID] == $mediafile_id,
      'Got the mediafile'
    );

    // Return data.
    return $mediafile;
  }

  /**
   * Get asset supplement.
   * Based on the GET rest call /asset/$asset_id/supplement (GET)
   */
  protected function getAssetSupplement($asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_get::IS_APP_ADMIN => 'false',
      mediamosa_rest_call_asset_get::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Required.
    $uri = strtr('asset/@asset_id/supplement', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Get asset supplement with ID: @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    $supplements = mediamosa_lib::responsexml2array($response['xml']);

    // Return data.
    return $supplements;
  }

  /**
   * Get the still(s) from a mediafile..
   * Based on the rest call /mediafile/$mediafile_id/still (GET)
   */
  protected function getMediafileStill($mediafile_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters still.
    $parameters += array(
      mediamosa_rest_call_get_still_for_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set uri.
    $uri = strtr('mediafile/@mediafile_id/still', array('@mediafile_id' => $mediafile_id));

    // Do Get call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Get mediafile still with ID: @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Get response.
    $response = mediamosa_lib::responsexml2array($response['xml']);
    $stills = $response['items']['item'];

    // Did we get something?
    $this->assertTrue(
      count($stills),
      'Got the mediafile stills'
    );

    // Return data.
    return $stills;
  }


  /**
   * Get metadata definitions.
   * Based on the REST call /metadata_tag (GET)
   */
  protected function getMetadataDefinitions(array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    $uri = 'metadata_tag';

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Get metadata tags, got result @result (@result_description)",
        array(
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    $result = mediamosa_lib::simplexml2array($response['xml']->items);
    return isset($result['item']) ? $result['item'] : array();
  }

  /**
   * Get a collection.
   *
   * @param string $coll_id
   * @param string $user_id
   * @param array $parameters
   */
  protected function getCollection($coll_id, $user_id = self::SIMPLETEST_USER_ID, array $parameters = array()) {

    // Required parameter.
    $parameters[mediamosa_rest_call_collection_create::USER_ID] = $user_id;

    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($parameters[mediamosa_rest_call_collection_create::APP_ID]));

    $uri = strtr('collection/@coll_id', array('@coll_id' => $coll_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters);

    $this->pass(strtr("Gets the collection id: @coll_id'.", array('@coll_id' => $coll_id)));

    // Return data.
    return mediamosa_lib::simplexml2array($response['xml']->items->item);
  }

  /**
   * Get the asset count collection.
   *
   * @param string $coll_id
   * @param string $user_id
   * @param array $parameters
   */
  protected function getCollectionAssetCount($coll_id, $user_id = self::SIMPLETEST_USER_ID, array $parameters = array()) {

    // Required parameter.
    $parameters[mediamosa_rest_call_collection_create::USER_ID] = $user_id;

    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($parameters[mediamosa_rest_call_collection_create::APP_ID]));

    $uri = strtr('collection/@coll_id/asset/count', array('@coll_id' => $coll_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters);

    // Numofvideos.
    $numofvideos = $response['xml']->items->item[0]->numofvideos;

    // Show message.
    $this->pass(strtr("Gets the asset count for collection id: @coll_id, videos in collection: @numofvideos.", array('@coll_id' => $coll_id, '@numofvideos' => $numofvideos)));

    // Return data.
    return $numofvideos;
  }

  /**
   * Search collections.
   *
   * @param string $user_id
   * @param array $parameters
   */
  protected function searchCollections($user_id = self::SIMPLETEST_USER_ID, array $parameters = array()) {
    // Parameters collection
    $parameters += array(
      mediamosa_rest_call_collection_search::HIDE_EMPTY_ASSETS => FALSE,
    );

    // Required.
    $parameters[mediamosa_rest_call_collection_search::USER_ID] = $user_id;

    // Optional, convert.
    $parameters[mediamosa_rest_call_collection_search::HIDE_EMPTY_ASSETS] = $parameters[mediamosa_rest_call_collection_search::HIDE_EMPTY_ASSETS] ? 'TRUE' : 'FALSE';

    // Do not set the app ID, handle by us.
    $this->assertFalse(isset($parameters[mediamosa_rest_call_collection_search::APP_ID]));

    $uri = 'collection';

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters);

    // Get response.
    return mediamosa_lib::responsexml2array($response['xml']);
  }

  /**
   * Get an user.
   * Based on the rest call /user/$user_id (GET)
   */
  protected function getUser($user_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Set uri.
    $uri = strtr('user/@user_id', array('@user_id' => $user_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Get user with ID: @user_id, got result @result (@result_description)",
        array(
          '@user_id' => $user_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Get response.
    $user = mediamosa_lib::simplexml2array($response['xml']->items->item[0]);

    // Did we get something?
    $this->assertTrue(
      isset($user[mediamosa_user_db::CREATED]),
      'Got the user'
    );

    // Return data.
    return $user;
  }

  /**
   * Get user list.
   * Based on the rest call /user (GET)
   */
  protected function getUserList(array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Set uri.
    $uri = 'user';

    // Do Get call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Getting list users, got result @result (@result_description)",
        array(
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return mediamosa_lib::simplexml2array($response['xml']->items);
  }

  /**
   * Get an user.
   * Based on the rest call /group/$group_id (GET)
   */
  protected function getGroup($group_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Set uri.
    $uri = strtr('group/@group_id', array('@group_id' => $group_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Get user group with ID: @group_id, got result @result (@result_description)",
        array(
          '@group_id' => $group_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Get response.
    $a_group = mediamosa_lib::simplexml2array($response['xml']->items->item[0]);

    // Did we get something?
    $this->assertTrue(
      isset($a_group[mediamosa_user_group_db::CREATED]),
      'Got the user group'
    );

    // Return data.
    return $a_group;
  }

  /**
   * Get an autorisation group.
   * Based on the rest call /autorisation_group/$groupname (GET)
   */
  protected function getAutGroup($groupname, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Set uri.
    $uri = strtr('autorisation_group/@groupname', array('@groupname' => $groupname));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Get aut group with name: @groupname, got result @result (@result_description)",
        array(
          '@groupname' => $groupname,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Get response.
    $a_aut_group = mediamosa_lib::simplexml2array($response['xml']->items->item[0]);

    // Return data.
    return $a_aut_group;
  }

  /**
   * Get an autorisation hostname.
   * Based on the rest call /autorisation_group/$groupname/hostname (GET)
   */
  protected function getAutHostname($groupname, $limit, $offset, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_aut_group_get_hostnames::LIMIT => $limit,
      mediamosa_rest_call_aut_group_get_hostnames::OFFSET => $offset,
    );

    // Set uri.
    $uri = strtr('autorisation_group/@groupname/hostname', array('@groupname' => $groupname));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Get aut hostname with name: @groupname, got result @result (@result_description)",
        array(
          '@groupname' => $groupname,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Get response.
    return mediamosa_lib::responsexml2array($response['xml']);
  }

  /**
   * Get the AUT/ACL settings on mediafile.
   */
  protected function getMediafileAut($mediafile_id, array $parameters = array(), $description = 'Get ACL on mediafile', array $expected_results = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_aut_mediafile_get_rights::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('mediafile/@mediafile_id/acl', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: @mediafile_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Convert to nice array.
    return mediamosa_lib::responsexml2array($response['xml']);
  }

  /**
   * Get statistics data upload.
   * Based on the rest call /statistics/dataupload (GET)
   */
  protected function getStatisticsDataupload($year, $month, $limit, array $parameters = array(), $description = 'Get statistics data upload', array $expected_results = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_statistics_get_historical_uploaded_mediafiles::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_statistics_get_historical_uploaded_mediafiles::YEAR  => $year,
      mediamosa_rest_call_statistics_get_historical_uploaded_mediafiles::MONTH => $month,
      'limit' => $limit,
    );

    // The uri.
    $uri = 'statistics/dataupload';

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: got result @result (@result_description)",
        array(
          '@description' => $description,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Convert to nice array.
    return mediamosa_lib::responsexml2array($response['xml']);
  }

  /**
   * Get statistics data upload.
   * Based on the rest call /statistics/dataupload (GET)
   */
  protected function getFtpUser($user, array $parameters = array(), $description = 'Get FTP User', array $expected_results = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
    );

    // The uri.
    $uri = strtr('ftp/@user', array('@user' => $user));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: user = @user, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@user' => $user,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Convert to nice array.
    return mediamosa_lib::responsexml2array($response['xml']);
  }

  // ------------------------------------------------------------------ Create items functions.
  /**
   * Create an asset.
   * Based on REST call /asset/create (POST)
   */
  protected function createAsset(array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_create::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_asset_create::GROUP_ID => self::SIMPLETEST_GROUP_ID,
      mediamosa_rest_call_asset_create::IS_APP_ADMIN => 'false',
      mediamosa_rest_call_asset_create::PROVIDER_ID => '',
      mediamosa_rest_call_asset_create::REFERENCE_ID => '',
    );

    // Set uri.
    $uri = 'asset/create';

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Get the new asset_id.
    $asset_id = empty($response['xml']->items->item[0]->asset_id) ? NULL : (string) $response['xml']->items->item[0]->asset_id;

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Created asset with ID: @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Return the asset_id.
    return $asset_id;
  }

  /**
   * Create asset metadata.
   * Based on REST call /asset/$asset_id/metadata (POST)
   */
  protected function createAssetMetadata($asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_metadata_create::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Required.
    $parameters[mediamosa_rest_call_create_mediafile::ASSET_ID] = $asset_id;

    // Set the URI.
    $uri = strtr('asset/@asset_id/metadata', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Created metadata for asset ID: @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response['xml'];
  }

  /**
   * Create asset metadata.
   * Based on REST call /asset/$asset_id/metadata (POST)
   */
  protected function createAssetMetadataAppend($asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Do append.
    $parameters['action'] = 'append';

    // Call create metadata.
    $this->createAssetMetadata($asset_id, $parameters, $expected_result_ids, $do_internal);
  }
  /**
   * Create asset metadata definition.
   * Based on REST call /metadata_tag/create (POST)
   */
  protected function createAssetMetadataDefinition($name, $type, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_metadata_create::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Required.
    $parameters['name'] = $name;
    $parameters['type'] = $type;

    // Set the URI.
    $uri = 'metadata_tag/create';

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Created asset metadata definition, got result @result (@result_description)",
        array(
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response['xml'];
  }

  /**
   * Create an mediafile
   * Based on REST call /mediafile/create (POST)
   */
  protected function createMediafile($asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_create_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_create_mediafile::IS_APP_ADMIN => 'false',
    );

    // Required.
    $parameters[mediamosa_rest_call_create_mediafile::ASSET_ID] = $asset_id;

    // Do Post call.
    $response = $this->restCallPost('mediafile/create', $parameters, array(), $expected_result_ids, $do_internal);

    // Get the new mediafile_id.
    $mediafile_id = empty($response['xml']->items->item[0]->mediafile_id) ? NULL : (string) $response['xml']->items->item[0]->mediafile_id;

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Created mediafile with ID: @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Return the mediafile ID.
    return $mediafile_id;
  }

  /**
   * Create upload ticket.
   * Based on REST call /mediafile/$mediafile_id/uploadticket/create (POST)
   */
  protected function createMediafileUploadTicket($mediafile_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_mediafile_uploadticket_create::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_asset_mediafile_uploadticket_create::IS_APP_ADMIN => 'false',
      mediamosa_rest_call_asset_mediafile_uploadticket_create::GROUP_ID => '',
    );

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id/uploadticket/create', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Get values.
    $action = empty($response['xml']->items->item[0]->action) ? NULL : (string) $response['xml']->items->item[0]->action;
    $uploadprogress_url = empty($response['xml']->items->item[0]->uploadprogress_url) ? NULL : (string) $response['xml']->items->item[0]->uploadprogress_url;

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Created upload ticket with action '@action', uploadprogress_url '@uploadprogress_url', got result @result (@result_description)",
        array(
          '@action' => $action,
          '@uploadprogress_url' => $uploadprogress_url,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Return data instead of one ID, is different behavour than other create functions.
    return mediamosa_lib::simplexml2array($response['xml']->items->item[0]);
  }

  /**
   * Create a asset supplement.
   * Based on REST call asset/$asset_id/supplement/create (POST)
   */
  protected function createAssetSupplement($asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_supplement_create::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('asset/@asset_id/supplement/create', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Get the new asset_supplement_id.
    $asset_supplement_id = empty($response['xml']->items->item[0]->supplement_id) ? NULL : (string) $response['xml']->items->item[0]->supplement_id;

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Created asset supplement with ID '@asset_supplement_id', got result @result (@result_description)",
        array(
          '@asset_supplement_id' => $asset_supplement_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Return data.
    return $asset_supplement_id;
  }


  /**
   * Create a collection.
   * Based on REST call /collection/create (POST)
   *
   * @param array $parameters
   */
  protected function createCollection(array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters Collection.
    $parameters += array(
      mediamosa_rest_call_collection_create::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_collection_create::GROUP_ID => self::SIMPLETEST_GROUP_ID,
      mediamosa_rest_call_collection_create::IS_APP_ADMIN => 'false',
    );

    // Set uri.
    $uri = 'collection/create';

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Get the new coll_id.
    $coll_id = empty($response['xml']->items->item[0]->coll_id) ? NULL : (string) $response['xml']->items->item[0]->coll_id;

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids) && !empty($coll_id),
      strtr(
        "Created collection with ID: @coll_id, got result @result (@result_description)",
        array(
          '@coll_id' => $coll_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Return the collection id.
    return $coll_id;
  }

  /**
   * Create a collection asset relation.
   * Based on REST call /collection/@coll_id/asset_relation (POST)
   */
  protected function createCollectionAssetRelation($coll_id, array $asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_collection_create::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_asset_collection_create::ASSET_ID => $asset_id,
    );

    // Set uri.
    $uri = strtr('collection/@coll_id/asset_relation', array('@coll_id' => $coll_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    return $response['xml'];
  }

  /**
   * Create a quota user.
   * Based on REST call user/create (POST)
   */
  protected function createUser($user_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_user_create::QUOTUM => 0,
    );

    // Required.
    $parameters[mediamosa_rest_call_user_create::USER_ID] = $user_id;

    // Set the URI.
    $uri = 'user/create';

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Created user with ID '@user_id', got result @result (@result_description)",
        array(
          '@user_id' => $user_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );
  }

  /**
   * Create a quota group.
   * Based on REST call group/create (POST)
   */
  protected function createGroup($group_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_group_create::QUOTUM => 0,
    );

    // Required.
    $parameters[mediamosa_rest_call_group_create::GROUP_ID] = $group_id;

    // Set the URI.
    $uri = 'group/create';

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Created group with ID '@group_id', got result @result (@result_description)",
        array(
          '@group_id' => $group_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );
  }

  /**
   * Create a aut group.
   * Based on REST call autorisation_group/create (POST)
   */
  protected function createAutGroup($groupname, $group_type, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_aut_group_create::GROUPNAME => $groupname,
      mediamosa_rest_call_aut_group_create::GROUP_TYPE => $group_type,
    );

    // Set the URI.
    $uri = 'autorisation_group/create';

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Created aut group with ID '@groupname, type @group_type', got result @result (@result_description)",
        array(
          '@groupname' => $groupname,
          '@group_type' => $group_type,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );
  }

  /**
   * Create a aut hostname.
   * Based on REST call autorisation_group/$groupname/hostname/create (POST)
   */
  protected function createAutHostname($groupname, array $a_hostnames, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_aut_group_create_hostnames::HOSTNAME => $a_hostnames,
    );

    // Set the URI.
    $uri = strtr('autorisation_group/@groupname/hostname/create', array('@groupname' => $groupname));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Created aut hostnames @hostname for group with ID '@groupname', got result @result (@result_description)",
        array(
          '@groupname' => $groupname,
          '@hostname' => implode(', ', $a_hostnames),
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );
  }

  /**
   * Create an ftp user.
   * Based on REST call ftp/create (POST)
   */
  protected function createFtpUser($user, $password, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_ftp_user_create::USER => $user,
      mediamosa_rest_call_ftp_user_create::PASSWORD => $password,
    );

    // Set the URI.
    $uri = 'ftp/create';

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Created ftp user: user = @user, got result @result (@result_description)",
        array(
          '@user' => $user,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );
  }

  /**
   * Delete an aut group.
   * Based on REST call autorisation_group/$groupname/delete (POST)
   */
  protected function deleteAutGroup($groupname, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Set the URI.
    $uri = strtr('autorisation_group/@groupname/delete', array('@groupname' => $groupname));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Deleted aut group ID '@groupname', got result @result (@result_description)",
        array(
          '@groupname' => $groupname,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );
  }

  /**
   * Delete an aut hostname.
   * Based on REST call autorisation_group/$groupname/hostname/delete (POST)
   */
  protected function deleteAutHostname($groupname, array $a_hostnames, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_aut_group_create_hostnames::HOSTNAME => $a_hostnames,
    );

    // Set the URI.
    $uri = strtr('autorisation_group/@groupname/hostname/delete', array('@groupname' => $groupname));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Deleted aut hostnames @hostname for group with ID '@groupname', got result @result (@result_description)",
        array(
          '@groupname' => $groupname,
          '@hostname' => implode(', ', $a_hostnames),
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );
  }

  /**
   * Delete an FTP User.
   * Based on REST call ftp/$user/delete (POST)
   */
  protected function deleteFtpUser($user, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_ftp_user_delete::USER => $user,
    );

    // Set the URI.
    $uri = strtr('ftp/@user/delete', array('@user' => $user));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Deleted FTP User @user, got result @result (@result_description)",
        array(
          '@user' => $user,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );
  }

  // ------------------------------------------------------------------ Other functions.
  /**
   * Change ownership.
   * Based on the POST rest call /change_ownership (POST)
   */
  protected function changeOwnership(array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_update_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = 'change_ownership';

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr("Change ownership: got result @result (@result_description)", array(
        '@result' => (string) $response['xml']->header->request_result_id,
        '@result_description' => $response['xml']->header->request_result_description
      ))
    );

    // Return response.
    return $response;
  }


  // ------------------------------------------------------------------ Update items functions.
  /**
   * Updates an asset.
   * Based on the REST call /asset/$asset_id (POST)
   */
  protected function updateAsset($asset_id, array $parameters = array(), $description = 'Update on asset ID', array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {

    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_update::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // More than one, we need to do something.
    $this->assertFalse(count($parameters) == 1, 'Any update parameters set');

    // The uri.
    $uri = strtr('asset/@asset_id', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: @asset_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@asset_id' => $asset_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Update mediafile.
   * Based on the POST rest call /mediafile/$mediafile_id (POST)
   */
  protected function updateMediafile($mediafile_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_update_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr("Updated mediafile with ID: @mediafile_id, got result @result (@result_description)", array('@mediafile_id' => $mediafile_id, '@result' => (string) $response['xml']->header->request_result_id, '@result_description' => $response['xml']->header->request_result_description))
    );

    // Return response.
    return $response;
  }

  /**
   * Update collection.
   * Based on the POST rest call /collection/$collection_id (POST)
   */
  protected function updateCollection($collection_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_collection_update::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('collection/@collection_id', array('@collection_id' => $collection_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr("Updated collection with ID: @collection_id, got result @result (@result_description)", array(
        '@collection_id' => $collection_id,
        '@result' => (string) $response['xml']->header->request_result_id,
        '@result_description' => $response['xml']->header->request_result_description
      ))
    );

    // Return response.
    return $response;
  }

  /**
   * Updates an asset supplement.
   * Based on the REST call /asset/$asset_id/supplement/$supplement_id/update (POST)
   */
  protected function updateAssetSupplement($asset_id, $supplement_id, $supplement, array $parameters = array(), $description = 'Update on asset supplement', array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_update::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_asset_supplement_db::SUPPLEMENT => $supplement,
    );

    // The uri.
    $uri = strtr('asset/@asset_id/supplement/@supplement_id/update', array('@asset_id' => $asset_id, '@supplement_id' => $supplement_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: @asset_id, @supplement_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@asset_id' => $asset_id,
          '@supplement_id' => $supplement_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Updates an user.
   * Based on the REST call /user/$user_id (POST)
   */
  protected function updateUser($user_id, array $parameters = array(), $description = 'Update on user', array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {

    // More than one, we need to do something.
    $this->assertTrue(count($parameters), 'Any update parameters set');

    // The uri.
    $uri = strtr('user/@user_id', array('@user_id' => $user_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: @user_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@user_id' => $user_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Updates an group.
   * Based on the REST call /group/$group_id (POST)
   */
  protected function updateGroup($group_id, array $parameters = array(), $description = 'Update on group', array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {

    // More than one, we need to do something.
    $this->assertTrue(count($parameters), 'Any update parameters set');

    // The uri.
    $uri = strtr('group/@group_id', array('@group_id' => $group_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: @group_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@group_id' => $group_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Updates an aut group.
   * Based on the REST call /autorisation_group/$groupname (POST)
   */
  protected function updateAutGroup($groupname, array $parameters = array(), $description = 'Update on group', array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {

    // More than one, we need to do something.
    $this->assertTrue(count($parameters), 'Any update parameters set');

    // The uri.
    $uri = strtr('autorisation_group/@groupname', array('@groupname' => $groupname));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: @groupname, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@groupname' => $groupname,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Updates an FTP user.
   * Based on the REST call /ftp/$user/update (POST)
   */
  protected function updateFtpUser($user, array $parameters = array(), $description = 'Update FTP User', array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {

    // More than one, we need to do something.
    $this->assertTrue(count($parameters), 'Any update parameters set');

    // The uri.
    $uri = strtr('ftp/@user/update', array('@user' => $user));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: @user, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@user' => $user,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Updates ACL/AUT rights on mediafile
   * Based on the REST call /mediafile/$mediafile_id/acl (POST)
   *
   * mediafile_id
   * user_id      must match the owner fo the mediafile.
   * replace      Standard is TRUE. Define if 'FALSE' if you want to keep the old rights.
   * aut_app[]    application(s) that gain access to the mediafile
   * aut_user[]   user(s) that gain access to the mediafile
   * aut_group[]  group(s) that gain access to the mediafile
   * aut_domain[] domein(s) that gain access to the mediafile
   * aut_realm[]  realm(s) that gain access to the mediafile
   *
   */
  protected function setMediafileAut($mediafile_id, array $parameters = array(), $description = 'Setting ACL on mediafile', array $expected_results = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_aut_mediafile_set_rights::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_aut_mediafile_set_rights::REPLACE => 'true',
    );

    // The uri.
    $uri = strtr('mediafile/@mediafile_id/acl', array('@mediafile_id' => $mediafile_id));

    // Do Post call, the call itself should not fail, even when we dont have access in some cases.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: @mediafile_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Convert to nice array.
    $a_xml = mediamosa_lib::responsexml2array($response['xml']);

    if (!isset($a_xml['items']['item']) || empty($a_xml['items']['item'])) {
      $this->assertTrue(empty($expected_results), 'Nothing changed, must expect nothing too.');
      return $a_xml;
    }

    // Get the result.
    $items = $a_xml['items']['item'];

    // First check if we found result about our input.
    foreach ($parameters as $type => $a_changes) {
      if (!in_array($type, array('aut_app', 'aut_user', 'aut_group', 'aut_domain', 'aut_realm'))) {
        continue;
      }

      $found = FALSE;
      foreach ($items as $item) {
        if (isset($item[$type])) {
          $found = TRUE;

          if (!isset($expected_results[$type][$item[$type]['value']])) {
            $expected_results[$type][$item[$type]['value']] = mediamosa_error::ERRORCODE_OKAY;
          }

          break;
        }
      }

      // Must be found, no matter what outcome.
      $this->assertTrue($found, strtr('Found type @type in result.', array('@type' => $type)));
    }

    // Now check if all results we got, also match what we expected.
    foreach ($expected_results as $type => $a_expected_result) {
      foreach ($items as $item) {
        if (isset($item[$type])) {
          foreach ($a_expected_result as $value => $result_code) {
            if ($item[$type]['value'] == $value) {
              $this->assertTrue(
                is_array($result_code) ? in_array($item[$type]['result_id'], $result_code) : $item[$type]['result_id'] == $result_code,
                strtr('Change on @type, expecting code @result_code, got @item_result_code',
                  array(
                    '@type' => $type,
                    '@result_code' => $result_code,
                    '@item_result_code' => $item[$type]['result_id']
                  )
                )
              );
            }
          }
        }
      }
    }

    // Return the array XML.
    return $a_xml;
  }

  /**
   * Same as setMediafileAut, except will add to rights and not replace.
   */
  protected function addMediafileAut($mediafile_id, array $parameters = array(), $description = 'Setting ACL on mediafile', array $expected_results = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    $parameters[mediamosa_rest_call_aut_mediafile_set_rights::REPLACE] = 'false';
    $this->setMediafileAut($mediafile_id, $parameters, $description, $expected_results, $expected_result_ids, $do_internal);
  }

  /**
   * Updates ACL/AUT rights on asset.
   * Based on the REST call /asset/$asset_id/acl (POST)
   *
   * mediafile_id
   * user_id      must match the owner fo the mediafile.
   * replace      Standard is TRUE. Define if 'FALSE' if you want to keep the old rights.
   * aut_app[]    application(s) that gain access to the mediafile
   * aut_user[]   user(s) that gain access to the mediafile
   * aut_group[]  group(s) that gain access to the mediafile
   * aut_domain[] domein(s) that gain access to the mediafile
   * aut_realm[]  realm(s) that gain access to the mediafile
   *
   */
  protected function setAssetAut($asset_id, array $parameters = array(), $description = 'Setting ACL on asset', array $expected_results = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_aut_asset_set_rights::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_aut_asset_set_rights::REPLACE => 'true',
    );

    // The uri.
    $uri = strtr('asset/!asset_id/acl', array('!asset_id' => $asset_id));

    // Do Post call, the call itself should not fail, even when we dont have access in some cases.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: @asset_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@asset_id' => $asset_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Convert to nice array.
    $a_xml = mediamosa_lib::responsexml2array($response['xml']);

    if (!isset($a_xml['items']['item']) || empty($a_xml['items']['item'])) {
      $this->assertTrue(empty($expected_results), 'Nothing changed, must expect nothing too.');
      return $a_xml;
    }

    // Get the result.
    $items = $a_xml['items']['item'];

    // First check if we found result about our input.
    foreach ($parameters as $type => $a_changes) {
      if (!in_array($type, array('aut_app', 'aut_user', 'aut_group', 'aut_domain', 'aut_realm'))) {
        continue;
      }

      $found = FALSE;
      foreach ($items as $item) {
        if (isset($item[$type])) {
          $found = TRUE;

          if (!isset($expected_results[$type][$item[$type]['value']])) {
            $expected_results[$type][$item[$type]['value']] = mediamosa_error::ERRORCODE_OKAY;
          }

          break;
        }
      }

      // Must be found, no matter what outcome.
      $this->assertTrue($found, strtr('Found type @type in result.', array('@type' => $type)));
    }

    // Now check if all results we got, also match what we expected.
    foreach ($expected_results as $type => $a_expected_result) {
      foreach ($items as $item) {
        if (isset($item[$type])) {
          foreach ($a_expected_result as $value => $result_code) {
            if ($item[$type]['value'] == $value) {
              $this->assertTrue(
                is_array($result_code) ? in_array($item[$type]['result_id'], $result_code) : $item[$type]['result_id'] == $result_code,
                strtr('Change on @type, expecting code @result_code, got @item_result_code',
                  array(
                    '@type' => $type,
                    '@result_code' => $result_code,
                    '@item_result_code' => $item[$type]['result_id']
                  )
                )
              );
            }
          }
        }
      }
    }

    // Return the array XML.
    return $a_xml;
  }

  /**
   * Same as setAssetAut, except will add to rights and not replace.
   */
  protected function addAssetAut($asset_id, array $parameters = array(), $description = 'Adding ACL on asset', array $expected_results = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    $parameters[mediamosa_rest_call_aut_asset_set_rights::REPLACE] = 'false';
    $this->setAssetAut($asset_id, $parameters, $description, $expected_results, $expected_result_ids, $do_internal);
  }

  /**
   * Updates ACL/AUT rights on collection
   * Based on the REST call /collection/$coll_id/acl (POST)
   *
   * coll_id
   * user_id      must match the owner fo the collection.
   * replace      Standard is TRUE. Define if 'FALSE' if you want to keep the old rights.
   * aut_app[]    application(s) that gain access to the collection
   */
  protected function setCollectionAut($collection_id, array $parameters = array(), $description = 'Setting ACL on collection', array $expected_results = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_aut_collection_set_rights::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_aut_collection_set_rights::REPLACE => 'true',
    );

    // The uri.
    $uri = strtr('collection/@collection_id/acl', array('@collection_id' => $collection_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "@description: @collection_id, got result @result (@result_description)",
        array(
          '@description' => $description,
          '@collection_id' => $collection_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Convert to nice array.
    $a_xml = mediamosa_lib::responsexml2array($response['xml']);

    // Get the result.
    $items = $a_xml['items']['item'];

    // First check if we found result about our input.
    foreach ($parameters as $type => $a_changes) {
      if (!in_array($type, array('aut_app'))) {
        continue;
      }

      $found = FALSE;
      foreach ($items as $item) {
        if (isset($item[$type])) {
          $found = TRUE;

          if (!isset($expected_results[$type][$item[$type]['value']])) {
            $expected_results[$type][$item[$type]['value']] = mediamosa_error::ERRORCODE_OKAY;
          }

          break;
        }
      }

      // Must be found, no matter what outcome.
      $this->assertTrue($found, strtr('Found type @type in result.', array('@type' => $type)));
    }

    // Now check if all results we got, also match what we expected.
    foreach ($expected_results as $type => $a_expected_result) {
      foreach ($items as $item) {
        if (isset($item[$type])) {
          foreach ($a_expected_result as $value => $result_code) {
            if ($item[$type]['value'] == $value) {
              $this->assertTrue(
                is_array($result_code) ? in_array($item[$type]['result_id'], $result_code) : $item[$type]['result_id'] == $result_code,
                strtr('Change on @type, expecting code @result_code, got @item_result_code',
                  array(
                    '@type' => $type,
                    '@result_code' => $result_code,
                    '@item_result_code' => $item[$type]['result_id']
                  )
                )
              );
            }
          }
        }
      }
    }

    // Return the array XML.
    return $a_xml;
  }

  /**
   * Same as setMediafileAut, except will add to rights and not replace.
   */
  protected function addCollectionAut($collection_id, array $parameters = array(), $description = 'Setting ACL on collection', array $expected_results = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    $parameters[mediamosa_rest_call_aut_collection_set_rights::REPLACE] = 'false';
    $this->setCollectionAut($collection_id, $parameters, $description, $expected_results, $expected_result_ids, $do_internal);
  }

  /**
   * Set is private on asset.
   * Based on the POST rest call /asset/$asset_id (POST)
   */
  protected function set_isprivate($asset_id, $isprivate = FALSE, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_update::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_asset_update::ISPRIVATE => (
        $isprivate ?
          mediamosa_rest_call_asset_update::ISPRIVATE_TRUE :
          mediamosa_rest_call_asset_update::ISPRIVATE_FALSE
      ),
    );

    // Set the URI.
    $uri = strtr('asset/@asset_id', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr("Set isprivate with ID: @asset_id, got result @result (@result_description)", array(
        '@asset_id' => $asset_id,
        '@result' => (string) $response['xml']->header->request_result_id,
        '@result_description' => $response['xml']->header->request_result_description
      ))
    );

    // Return response.
    return $response;
  }

  /**
   * Set unappropiate on asset.
   * Based on the POST rest call /asset/$asset_id (POST)
   */
  protected function set_unappropiate($asset_id, $unappropiate = FALSE, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_asset_update::USER_ID => self::SIMPLETEST_USER_ID,
      mediamosa_rest_call_asset_update::IS_UNAPPROPRIATE => ($unappropiate ? 'TRUE' : 'FALSE'),
    );

    // Set the URI.
    $uri = strtr('asset/@asset_id', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr("Set isprivate with ID: @asset_id, got result @result (@result_description)", array(
        '@asset_id' => $asset_id,
        '@result' => (string) $response['xml']->header->request_result_id,
        '@result_description' => $response['xml']->header->request_result_description
      ))
    );

    // Return response.
    return $response;
  }


  // ------------------------------------------------------------------ Delete items functions.
  /**
   * Delete a asset.
   * Based on the REST call /asset/$asset_id/delete (POST)
   */
  protected function deleteAsset($asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters.
    $parameters += array(
      mediamosa_rest_call_asset_supplement_update::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('asset/@asset_id/delete', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Delete asset with ID: @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Delete multiple assets.
   * Based on the REST call /asset/delete (POST)
   */
  protected function deleteAssetMultiple(array $a_asset_ids, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters.
    $parameters += array(
      mediamosa_rest_call_asset_supplement_update::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Add the asset ids to the parameters
    $parameters[mediamosa_rest_call_asset_delete_all::ASSET_ID] = $a_asset_ids;

    // The uri.
    $uri = 'asset/delete';

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Delete assets, got result @result (@result_description)",
        array(
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Delete a mediafile.
   * Based on the REST call /mediafile/$mediafile_id/delete (POST)
   */
  protected function deleteMediafile($mediafile_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters.
    $parameters += array(
      mediamosa_rest_call_asset_supplement_update::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('mediafile/@mediafile_id/delete', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Delete mediafile with ID: @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Delete still.
   * Based on the REST call /asset/$asset_id/still/delete (POST)
   */
  protected function deleteStill($asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters.
    $parameters += array(
      mediamosa_rest_call_asset_supplement_update::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('asset/@asset_id/still/delete', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Delete still with ID: @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Delete collection.
   * Based on the REST call /collection/$coll_id/delete (POST)
   */
  protected function deleteCollection($coll_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters.
    $parameters += array(
      mediamosa_rest_call_collection_delete::DELETE => 'cascade',
      mediamosa_rest_call_collection_delete::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('collection/@coll_id/delete', array('@coll_id' => $coll_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Delete collection with ID: @coll_id, got result @result (@result_description)",
        array(
          '@coll_id' => $coll_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Delete an asset property definition.
   * Based on the REST call /metadata_tag/delete (POST)
   */
  protected function deleteAssetMetadataDefinition($name, $delete = NULL, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    // Parameters.
    $parameters += array(
      mediamosa_rest_call_asset_supplement_update::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Name.
    $parameters['name'] = $name;

    // Delete.
    if ($delete == 'cascade') {
      $parameters['delete'] = 'cascade';
    }

    // The uri.
    $uri = 'metadata_tag/delete';

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Delete asset metadata definition, got result @result (@result_description)",
        array(
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Delete asset supplement.
   * Based on the REST call /asset/$asset_id/supplement/$supplement_id/delete (POST)
   */
  protected function deleteAssetSupplement($asset_id, $supplement_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {

    // Parameters.
    $parameters += array(
      mediamosa_rest_call_asset_supplement_update::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('asset/@asset_id/supplement/@supplement_id/delete', array('@asset_id' => $asset_id, '@supplement_id' => $supplement_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Delete asset supplement with ID: @supplement_id, got result @result (@result_description)",
        array(
          '@supplement_id' => $supplement_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  /**
   * Delete ACL rights on mediafile.
   */
  protected function deleteMediafileAut($mediafile_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_aut_mediafile_delete_rights::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('mediafile/!mediafile_id/acl/delete', array('!mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Removed ACL rights from mediafile @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Get the ACL rights to be sure its really gone.

    // The uri.
    $uri = strtr('mediafile/!mediafile_id/acl', array('!mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), array(mediamosa_error::ERRORCODE_EMPTY_RESULT), $do_internal);
    $items = (array)$response['xml']->items;

    $this->assertTrue(empty($items), 'ACL rights on mediafile are really gone.');

    return $response;
  }

  /**
   * Delete ACL rights on asset.
   */
  protected function deleteAssetAut($asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_aut_mediafile_delete_rights::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('asset/!asset_id/acl/delete', array('!asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Removed ACL rights from asset @asset_id, got result @result (@result_description)",
        array(
          '@asset_id' => $asset_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Get the ACL rights to be sure its really gone.

    // The uri.
    $uri = strtr('asset/!asset_id/acl', array('!asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), array(mediamosa_error::ERRORCODE_EMPTY_RESULT), $do_internal);
    $items = (array)$response['xml']->items;

    $this->assertTrue(empty($items), 'ACL rights on asset are really gone.');

    return $response;
  }
  /**
   * Delete ACL rights on mediafile.
   * Based on the REST call /asset/$asset_id/supplement/$supplement_id/delete (POST)
   */
  protected function deleteCollectionAut($coll_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_aut_collection_delete_rights::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // The uri.
    $uri = strtr('collection/@coll_id/acl/delete', array('@coll_id' => $coll_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Removed ACL rights from collection @coll_id, got result @result (@result_description)",
        array(
          '@coll_id' => $coll_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return $response;
  }

  // ------------------------------------------------------------------ Play Proxy.
  /*
   * Do a play proxy call.
   */
  protected function play_proxy($file, $line, $asset_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    $parameters += array(
      mediamosa_rest_call_asset_mediafile_play::USER_ID => self::SIMPLETEST_USER_ID,
    );

    $parameters[mediamosa_rest_call_asset_mediafile_play::ASSET_ID] = $asset_id;


    // The uri.
    $uri = strtr('asset/@asset_id/play', array('@asset_id' => $asset_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        'Do play proxy call in file @file on line @line, on asset @asset_id, got result @result (@result_description)',
        array(
          '@file' => $file,
          '@line' => $line,
          '@asset_id' => $asset_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return mediamosa_lib::simplexml2array($response['xml']);
  }

  // -------------------------------------------------------------- Statistics.
  /*
   * Do a search word stats call.
   */
  protected function stats_get(array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {

    $parameters += array(
      'begindate' => sprintf('%02d-%02d', date('Y'), date('n')),
      'enddate' => sprintf('%02d-%02d', (date('n') + 1 == 13 ? date('Y') + 1 : date('Y')), (date('n') + 1 == 13 ? 1 : date('n') + 1)),
      'limit' => 10,
      'offset' => 0,
    );

    // The uri.
    $uri = 'statistics/searchedwords';

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Test return status.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        'Do stats call, got result @result (@result_description)',
        array(
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    return mediamosa_lib::simplexml2array($response['xml']);
  }

  // ------------------------------------------------------------------ CQL functions.
  public function cql_do_search_test_assets($cql, $expected_asset_ids, array $parameters = array(), $aut_user_id = FALSE, $aut_group_id = FALSE, $aut_domain = FALSE, $aut_realm = FALSE, $granted = FALSE, $is_public_list = FALSE) {
    $parameters += array(
      mediamosa_rest_call_asset_search::CQL => $cql,
      mediamosa_rest_call_asset_search::GRANTED => $granted ? 'true' : 'false',
    );

    if ($aut_user_id !== FALSE) {
      $parameters[mediamosa_rest_call_asset_search::AUT_USER_ID] = $aut_user_id;
    }

    if ($aut_group_id !== FALSE) {
      $parameters[mediamosa_rest_call_asset_search::AUT_GROUP_ID] = $aut_group_id;
    }

    if ($aut_domain !== FALSE) {
      $parameters[mediamosa_rest_call_asset_search::AUT_DOMAIN] = $aut_domain;
    }

    if ($aut_realm !== FALSE) {
      $parameters[mediamosa_rest_call_asset_search::AUT_REALM] = $aut_realm;
    }

    // private listing ?
    $parameters[mediamosa_rest_call_asset_search::IS_PUBLIC_LIST] = $is_public_list ? 'true' : 'false';

    // Set uri.
    $uri = 'asset';
    $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY, mediamosa_error::ERRORCODE_EMPTY_RESULT);

    // Do get call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Executed CQL: '@cql', got result @result (@result_description)",
        array(
          '@cql' => $parameters[mediamosa_rest_call_asset_search::CQL],
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Make array.
    $found = isset($response['xml']->items) && !empty($response['xml']->items) ? mediamosa_lib::simplexml2array($response['xml']) : array('items' => array('item' => array()));
    $found = !isset($found['items']['item'][0]) && !empty($found['items']['item']) ? array(0 => $found['items']['item']) : $found['items']['item'];

    $expected = $this->cql_expected_ids($found, $expected_asset_ids, 'asset_id', $granted);

    $found_ids_2 = $found_ids = array();
    foreach ($found as $key_item => $item) {
      $found_ids_2[] = $item['asset_id'] . ($granted ? ($item['granted'] == 'TRUE' ? ' (granted)' : ' (not granted)') : ' [always granted]');
      $found_ids[] = $item['asset_id'];
    }

    // Now see if we got what we wanted.
    $this->assertTrue($expected, strtr('Found assets; @found, expected; @expected.', array('@found' => implode(', ', $found_ids_2), '@expected' => implode(', ', array_keys($expected_asset_ids)))));

    // Return the asset IDs in order of the items.
    return $found_ids;
  }

  public function cql_do_search_test_collections($cql, $expected_collection_ids, array $parameters = array()) {
    $parameters += array(
      mediamosa_rest_call_asset_search::CQL => $cql,
    );

    // Set uri.
    $uri = 'collection';
    $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY, mediamosa_error::ERRORCODE_EMPTY_RESULT);

    // Do get call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Executed CQL: '@cql', got result @result (@result_description)",
        array(
          '@cql' => $parameters[mediamosa_rest_call_asset_search::CQL],
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Make array.
    $found = isset($response['xml']->items) && !empty($response['xml']->items) ? mediamosa_lib::simplexml2array($response['xml']) : array('items' => array('item' => array()));
    $found = !isset($found['items']['item'][0]) && !empty($found['items']['item']) ? array(0 => $found['items']['item']) : $found['items']['item'];

    $expected = $this->cql_expected_ids($found, $expected_collection_ids, 'coll_id');

    $found_ids = array();
    foreach ($found as $key_item => $item) {
      $found_ids[] = $item['coll_id'];
    }

    // Now see if we got what we wanted.
    $this->assertTrue($expected, strtr('Found collections; @found, expected; @expected.', array('@found' => implode(', ', $found_ids), '@expected' => implode(', ', array_keys($expected_collection_ids)))));
  }

  public function cql_do_search_test_jobs($asset_id, $cql, $expected_job_ids, array $parameters = array()) {
    $parameters += array(
      mediamosa_rest_call_asset_search::CQL => $cql,
    );

    // Set uri.
    $uri = sprintf('asset/%s/joblist', $asset_id);
    $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY, mediamosa_error::ERRORCODE_EMPTY_RESULT);

    // Do get call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Executed CQL: '@cql', got result @result (@result_description)",
        array(
          '@cql' => $parameters[mediamosa_rest_call_asset_search::CQL],
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    // Make array.
    $found = isset($response['xml']->items) && !empty($response['xml']->items) ? mediamosa_lib::simplexml2array($response['xml']) : array('items' => array('item' => array()));
    $found = !isset($found['items']['item'][0]) && !empty($found['items']['item']) ? array(0 => $found['items']['item']) : $found['items']['item'];

    // Value to key.
    $a_tmp = array();
    foreach ($expected_job_ids as $job_id) {
      $a_tmp[$job_id] = TRUE;
    }
    $expected_job_ids = $a_tmp;

    $expected = $this->cql_expected_ids($found, $expected_job_ids, 'id');

    $found_ids = array();
    foreach ($found as $key_item => $item) {
      $found_ids[] = $item['id'];
    }

    // Now see if we got what we wanted.
    $this->assertTrue($expected, strtr('Found jobs; @found, expected; @expected.', array('@found' => implode(', ', $found_ids), '@expected' => implode(', ', array_keys($expected_job_ids)))));
  }

  function cql_expected_ids($found, $ids, $name, $granted = FALSE) {
    if (!count($found) && count($ids)) {
      return FALSE;
    }

    // If we dont expect results we must know it at this point
    $is_empty = count($ids) ? FALSE : TRUE;

    foreach ($found as $key_item => $item) {
      if (!isset($ids[$item[$name]])) {
        continue;
      }

      // If granted flag matches, then we found it.
      if (!isset($item['granted']) || (($ids[$item[$name]] && $item['granted'] == 'TRUE') || (!$ids[$item[$name]] && $item['granted'] == 'FALSE')) ) {
        // Remove so we know we found it...
        unset($ids[$item[$name]]);// any left overs where not found
        unset($found[$key_item]);// remove the found asset
      }
    }

    // If we still have found, thats not ok...
    if (count($found)) {
      return FALSE;
    }

    // If we dont expect any results, then we response must empty as well
    if ($is_empty && !count($found)) {
      return TRUE;
    }

    return (count($ids) ? FALSE : TRUE);
  }

  // Search for app 1.
  public function cql_do_search_test_assets_1($cql, $expected_asset_ids, array $parameters = array(), $aut_user_id = FALSE, $aut_group_id = FALSE, $aut_domain = FALSE, $aut_realm = FALSE, $granted = FALSE, $is_public_list = FALSE) {

    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 1.
    $this->switch_to_app = $this->a_app[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $found_ids = $this->cql_do_search_test_assets($cql, $expected_asset_ids, $parameters, $aut_user_id, $aut_group_id, $aut_domain, $aut_realm, $granted, $is_public_list);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;

    return $found_ids;
  }

  // Search for app 2.
  public function cql_do_search_test_assets_2($cql, $expected_asset_ids, array $parameters = array(), $aut_user_id = FALSE, $aut_group_id = FALSE, $aut_domain = FALSE, $aut_realm = FALSE, $granted = FALSE, $is_public_list = FALSE) {

    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 2.
    $this->switch_to_app = $this->a_app_2[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $found_ids = $this->cql_do_search_test_assets($cql, $expected_asset_ids, $parameters, $aut_user_id, $aut_group_id, $aut_domain, $aut_realm, $granted, $is_public_list);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;

    return $found_ids;
  }

  // Search for app 3.
  public function cql_do_search_test_assets_3($cql, $expected_asset_ids, array $parameters = array(), $aut_user_id = FALSE, $aut_group_id = FALSE, $aut_domain = FALSE, $aut_realm = FALSE, $granted = FALSE, $is_public_list = FALSE) {

    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 3.
    $this->switch_to_app = $this->a_app_3[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $found_ids = $this->cql_do_search_test_assets($cql, $expected_asset_ids, $parameters, $aut_user_id, $aut_group_id, $aut_domain, $aut_realm, $granted, $is_public_list);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;

    return $found_ids;
  }

  public function cql_do_search_test_collections_1($cql = '', array $expected_collection_ids = array(), array $parameters = array()) {
    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 1.
    $this->switch_to_app = $this->a_app[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $this->cql_do_search_test_collections($cql, $expected_collection_ids, $parameters);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;
  }

  public function cql_do_search_test_collections_2($cql = '', array $expected_collection_ids = array(), array $parameters = array()) {
    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 2.
    $this->switch_to_app = $this->a_app_2[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $this->cql_do_search_test_collections($cql, $expected_collection_ids, $parameters);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;
  }

  public function cql_do_search_test_collections_3($cql = '', array $expected_collection_ids = array(), array $parameters = array()) {
    // Set old.
    $old_switch_to_app = $this->switch_to_app;

    // Set to app id 3.
    $this->switch_to_app = $this->a_app_3[mediamosa_app_db::APP_ID];

    // Do call.
    try {
      $this->cql_do_search_test_collections($cql, $expected_collection_ids, $parameters);
    }
    catch (Exception $e) {
      $this->switch_to_app = $old_switch_to_app;
      throw $e;
    }

    // Switch back.
    $this->switch_to_app = $old_switch_to_app;
  }

  public function correct_sorting($got, $expected) {
    $sorting_ok = count($got) ==  count($expected);


    if ($sorting_ok) {
      $got2 = $got;
      $expected2 = $expected;

      while (count($got2)) {
        if (array_shift($got2) != array_shift($expected2)) {
          $sorting_ok = FALSE;
          break;
        }
      }
    }

    $this->assertTrue($sorting_ok, 'Items are sorted in the right order.');

    if (!$sorting_ok) {
      $this->pass('Got IDs in order; ' . implode(', ', $got) . '; expecting in order;' .  implode(', ', $expected));
    }
  }

  /**
   * Search engine switches.
   */

  /**
   * Can find "" around word(s).
   */
  public function engine_fulltext_can_find_with_quotes() {
    return TRUE;
  }

  /**
   * Indexes per word or per line?
   *
   * Text; master : m*r will match?
   *       maste foor : m*r will match?
   */
  public function engine_fulltext_wildcard_indexes_lines() {
    return TRUE;
  }
}
