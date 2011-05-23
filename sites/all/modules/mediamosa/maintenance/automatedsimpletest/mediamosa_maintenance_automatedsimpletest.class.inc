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
  * Static functions for simpletest page.
  */

class mediamosa_maintenance_automatedsimpletest {
  // ------------------------------------------------------------------ Consts.
  const RUNPHASE_TIMED_OUT = 'timed-out';
  const RUNPHASE_STARTED = 'started';
  const RUNPHASE_FAILED = 'failed';
  const RUNPHASE_COMPLETED = 'completed';

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Log for mediamosa_automatedsimpletest.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'mediamosa_automatedsimpletest');
  }

  /**
   * Debug log for mediamosa_automatedsimpletest.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log_debug($message, array $variables = array()) {
    mediamosa_debug::log($message, $variables, 'mediamosa_automatedsimpletest');
  }

  /**
   * Return the amount of seconds before a test may run again if the same test
   * is still in running mode.
   */
  static public function timeout_test_run() {
    // Default is 3 minutes before we consider test timed-out.
    return variable_get('mediamosa_automatedsimpletest_timeout_tests', 360);
  }

  /**
   * Return a array with the class of the tests
   * we wish to run. If modules are not enabled and
   * there for their class is not loaded, we will need
   * to report this.
   */
  static public function get_default_tests() {
    $simpletests = array(
      mediamosa_settings::MEDIAMOSA_RUN_15_MINUTES => array(),
      mediamosa_settings::MEDIAMOSA_RUN_3_HOURS => array(),
      mediamosa_settings::MEDIAMOSA_RUN_DAILY => array(),
    );

    $groups = simpletest_test_get_all();

    foreach ($groups as $group => $tests) {
      if (drupal_strtolower(drupal_substr($group, 0, drupal_strlen('mediamosa'))) != 'mediamosa') {
        continue;
      }

      foreach ($tests as $class => $test) {
        if (!class_exists($class)) {
          continue;
        }

        // Not set? Then run it daily.
        if (!isset($test[mediamosa_settings::MEDIAMOSA_RUN])) {
          $simpletests[mediamosa_settings::MEDIAMOSA_RUN_DAILY][$class] = $test;
          continue;
        }

        $simpletests[$test[mediamosa_settings::MEDIAMOSA_RUN]][$class] = $test;
      }
    }

    return $simpletests;
  }

  /**
   * Returns TRUE / FALSE if we can run a test from the queue.
   */
  static public function may_run() {
    // Variable is set by the REST call that is running the test.
    $run_last = variable_get('mediamosa_automatedsimpletest_timeout_tests_running', NULL);

    // may run?
    if (round(REQUEST_TIME - $run_last['started']) < (self::timeout_test_run() + 60)) { // extra 60 seconds.
      return FALSE;
    }

    // Master switch.
    if (!variable_get('mediamosa_automatedsimpletest_run_tests', FALSE)) {
      return FALSE;
    }

    // Before we start, we count the number of tables. If simpletest fails,
    // it doesn't clean up. We stop running the tests if we have to many tables.
    // Also we'll try to clean up if we detect to many tables.
    $count_tables = mediamosa_db::db_count_tables();

    // Check if we have to many tables, else clean up.
    if ($count_tables > mediamosa_settings::STATUS_MAX_TABLES) {
      self::log('To many simpletest tables detected, trying to clean up, found @count tables.', array('@count' => $count_tables), WATCHDOG_ALERT);

      MediaMosaTestCase::clean();

      $count_tables = mediamosa_db::db_count_tables();

      if ($count_tables > mediamosa_settings::STATUS_MAX_TABLES) {
        // Clean failed.
        self::log('Status page update down, unable to clean up old tests, still @count tables left.', array('@count' => $count_tables), WATCHDOG_CRITICAL);
      }
    }

    // All ok, run new tests.
    return TRUE;
  }

  /**
   * Retrieve the test results.
   */
  static function get_test_results($name = NULL, array $value = NULL, $remove = FALSE) {
    static $test_results = array();

    if (!empty($name)) {
      // Split the name.
      list($run, $class_name) = explode('|', $name, 2);
    }

    // Value provided, then insert/update database.
    if (isset($value) && !empty($name)) {
      $test_results[$run][$class_name] = $value;

      // Store value (master-slave safe).
      mediamosa_db::db_query('INSERT INTO {#mediamosa_maintenance_automatedsimpletest} (#value, #name) VALUES (:value, :name) ON DUPLICATE KEY UPDATE #value = :value',
        array(
          '#mediamosa_maintenance_automatedsimpletest' => mediamosa_maintenance_automatedsimpletest_db::TABLE_NAME,
          '#value' => mediamosa_maintenance_automatedsimpletest_db::VALUE,
          ':value' => serialize($value),
          '#name' => mediamosa_maintenance_automatedsimpletest_db::NAME,
          ':name' => $name,
        )
      );
    }

    // If TRUE, we remove it from the results.
    if ($remove) {
      if (!empty($run) && !empty($class_name)) {
        unset($test_results[$run][$class_name]);
      }

      // Can't be empty.
      assert(!empty($name));

      // Store value (master-slave safe).
      mediamosa_db::db_query('DELETE FROM {#mediamosa_maintenance_automatedsimpletest} WHERE #name = :name',
        array(
          '#mediamosa_maintenance_automatedsimpletest' => mediamosa_maintenance_automatedsimpletest_db::TABLE_NAME,
          '#name' => mediamosa_maintenance_automatedsimpletest_db::NAME,
          ':name' => $name,
        )
      );
    }

    if (empty($test_results)) {
      $result = mediamosa_db::db_select(mediamosa_maintenance_automatedsimpletest_db::TABLE_NAME, 'mms')
        ->fields('mms')
        ->execute();

      $test_results = NULL;
      foreach ($result as $row) {
        if (strpos($row[mediamosa_maintenance_automatedsimpletest_db::NAME], '|') === FALSE) {
          continue;
        }

        list($run, $class_name) = explode('|', $row[mediamosa_maintenance_automatedsimpletest_db::NAME], 2);
        $test_results[$run][$class_name] = unserialize($row[mediamosa_maintenance_automatedsimpletest_db::VALUE]);
      }
    }

    return $test_results;
  }

  /**
   * Set the test results.
   */
  static function set_test_results($run, $class_name, array $value) {
    self::get_test_results($run . '|' . $class_name, $value);
  }

  /**
   * Remove the test results.
   */
  static function remove_test_results($run, $class_name) {
    self::get_test_results($run . '|' . $class_name, NULL, TRUE);
  }

  /**
   * Fill the queue with tests.
   */
  static public function fill_queue() {

    // Get the current queue.
    $queue = variable_get('mediamosa_maintenance_automatedsimpletest_queue', array());

    $runs = array(
      mediamosa_settings::MEDIAMOSA_RUN_15_MINUTES => variable_get('mediamosa_automatedsimpletest_minutes_tests_fast', mediamosa_settings::MEDIAMOSA_RUN_INTERVAL_FAST),
      mediamosa_settings::MEDIAMOSA_RUN_3_HOURS => variable_get('mediamosa_automatedsimpletest_minutes_tests_normal', mediamosa_settings::MEDIAMOSA_RUN_INTERVAL_NORMAL),
      mediamosa_settings::MEDIAMOSA_RUN_DAILY => variable_get('mediamosa_automatedsimpletest_minutes_tests_slow', mediamosa_settings::MEDIAMOSA_RUN_INTERVAL_SLOW),
    );

    // Get the tests.
    $default_tests = self::get_default_tests();

    // Now find the test and add to queue (if not already).
    foreach ($runs as $run => $minutes) {
      // Seed?
      if ($minutes < 1) {
        continue; // seeding turned off.
      }

      // Get the last run of this type.
      $run_last = variable_get($run, NULL);

      // Check if we can seed the queue for this type.
      if (round(REQUEST_TIME - $run_last) < $minutes * 60) {
        continue;
      }

      // Set time.
      mediamosa::variable_set($run, REQUEST_TIME);

      // Seed them.
      foreach ($default_tests[$run] as $class_name => $info) {
        // Check if there is a function on the class that returns TRUE or FALSE
        // if its allowed to run or display.
        if (method_exists($class_name, mediamosa_settings::MEDIAMOSA_RUN_ENABLED)) {
          if (!call_user_func_array($class_name . '::' . mediamosa_settings::MEDIAMOSA_RUN_ENABLED, array())) {

            // If the test has been disabled by test, we remove it also from
            // the results.
            self::remove_test_results($run, $class_name);
            $key = array_search($class_name, $queue);
            if ($key !== FALSE) {
              unset($queue[$key]);
            }

            continue;
          }
        }

        if (!in_array($class_name, $queue)) {
          $queue[] = $class_name;
        }
      }
    }

    // Set the queue.
    variable_set('mediamosa_maintenance_automatedsimpletest_queue', $queue);
  }

  /**
   * Run the tests one by one.
   */
  static public function run_simple_tests() {

    // We are still running something (or we are switched off).
    if (!self::may_run()) {
      return;
    }

    // Now run the test we found in the queue.
    $runs = array(
      mediamosa_settings::MEDIAMOSA_RUN_15_MINUTES,
      mediamosa_settings::MEDIAMOSA_RUN_3_HOURS,
      mediamosa_settings::MEDIAMOSA_RUN_DAILY,
    );

    // Get test results.
    $test_results = self::get_test_results();

    // Put started to timed-out.
    foreach ($runs as $run) {
      if (isset($test_results[$run])) {
        foreach ($test_results[$run] as $class_name => $info) {
          if (isset($info['runphase']) && $info['runphase'] == self::RUNPHASE_STARTED) {
            $test_results[$run][$class_name] = array('runphase' => self::RUNPHASE_TIMED_OUT, 'results' => array(), 'info' => $info);
            mediamosa_maintenance_automatedsimpletest::set_test_results($run, $class_name, $test_results[$run][$class_name]);
          }
        }
      }
    }

    // Get the current queue.
    $queue = variable_get('mediamosa_maintenance_automatedsimpletest_queue', array());

    // Get the next test.
    $queued_class_name = array_shift($queue);

    // No tests?
    if (!$queued_class_name) {
      return;
    }

    // Set the queue.
    variable_set('mediamosa_maintenance_automatedsimpletest_queue', $queue);

    // Get the tests.
    $default_tests = self::get_default_tests();

    $run_test = FALSE;
    foreach ($runs as $run) {
      foreach ($default_tests[$run] as $class_name => $info) {
        if ($queued_class_name != $class_name) {
          continue;
        }

        // Indicate we have run a test.
        $run_test = TRUE;

        // Run test. Dont care if it fails, this will be registered.
        self::run_simple_test($run, $class_name, $info);
      }

      if ($run_test) {
        break;
      }
    }
  }

  /**
   * Run the test.
   * @param string $class_name
   * @param array $info
   */
  static public function run_simple_test($run, $class_name, array $info) {

    // log start.
    mediamosa_debug::log_high('Starting unit test @class_name.', array('@class_name' => $class_name));

    // Call REST call.
    $options = array(
      'data' => array(
        'class_name' => $class_name,
      ),
      'method' => 'POST',
      'timeout' => self::timeout_test_run(),
    );

    // Make sure we wait long enough.
    if (self::timeout_test_run() > 0) {
      set_time_limit(self::timeout_test_run() + 5);
    }

    // Get test results.
    $test_results = self::get_test_results();

    if (isset($test_results[$run][$class_name])) {
      $test_results[$run][$class_name]['runphase'] = self::RUNPHASE_STARTED;
    }
    else {
      $test_results[$run][$class_name] = array('runphase' => self::RUNPHASE_STARTED, 'results' => array(), 'info' => $info);
    }

    // Now store started in variable.
    mediamosa_maintenance_automatedsimpletest::set_test_results($run, $class_name, $test_results[$run][$class_name]);

    // Do the call.
    $start = microtime(TRUE);
    $response = mediamosa_http::do_internal_call_cron('internal/simpletest/run', FALSE, $options);
    $end = microtime(TRUE);
    try {
      $response->xml = new mediamosa_connector_response($response->data);
    }
    catch (Exception $e) {
      mediamosa_debug::log_export($response->data);
    }

    if (isset($response->xml) && isset($response->xml->header) && (int) $response->xml->header->request_result_id == mediamosa_error::ERRORCODE_OKAY) {
      // As array is easier.
      $response = mediamosa_lib::responsexml2array($response->xml);
      $item = reset($response['items']['item']);

      // Add to run list and result.
      $test_results[$run][$class_name] = array('runphase' => self::RUNPHASE_COMPLETED, 'results' => $item['results'], 'assertions' => $item['assertions'], 'info' => $info, 'started' => $start, 'ended' => $end);
    }
    else {
      // Failed.
      $test_results[$run][$class_name] = array('runphase' => self::RUNPHASE_FAILED, 'results' => array(), 'info' => $info, 'started' => $start, 'ended' => $end);
    }

    // Now store in variable.
    mediamosa_maintenance_automatedsimpletest::set_test_results($run, $class_name, $test_results[$run][$class_name]);

    // log start.
    mediamosa_debug::log_high('Ended unit test @class_name.', array('@class_name' => $class_name));
  }

  /**
   * Trigger the maintenance cron.
   *
   * @param string $uri
   *  If you need to change the uri to trigger.
   */
  static public function trigger_simpletests() {

    // Trigger.
    mediamosa_http::do_internal_call_cron('internal/cron/simpletests', TRUE);
  }
}
