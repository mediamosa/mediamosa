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
  * REST calls for automated simpletest page.
  */

/**
 * URI: /cron/simpletests
 * Method: GET
 */
class mediamosa_rest_call_simpletests extends mediamosa_rest_call {

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Fill the queue (if needed).
    mediamosa_maintenance_automatedsimpletest::fill_queue();

    // Run them.
    mediamosa_maintenance_automatedsimpletest::run_simple_tests();

    // Set ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /external/simpletest
 * Method: GET
 */
class mediamosa_rest_call_simpletest extends mediamosa_rest_call {
  const SKIP_EMPTY = 'skip_empty';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array(
      self::VARS => array(
        mediamosa_settings::MEDIAMOSA_RUN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Type of simpletest that are returned.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_settings::MEDIAMOSA_RUN_15_MINUTES,
            mediamosa_settings::MEDIAMOSA_RUN_3_HOURS,
            mediamosa_settings::MEDIAMOSA_RUN_DAILY,
            mediamosa_settings::MEDIAMOSA_RUN_ALL,
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_settings::MEDIAMOSA_RUN_15_MINUTES,
        ),
        self::SKIP_EMPTY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Skip the tests have failed to run.',
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get variables.
    $mediamosa_run = $this->get_param_value(mediamosa_settings::MEDIAMOSA_RUN);
    $skip_empty = $this->get_param_value(self::SKIP_EMPTY);

    // Fill the output.
    $test_results = mediamosa_maintenance_automatedsimpletest::get_test_results();

    // Go through all kind of tests, eg. 15 min test.
    foreach ($test_results as $test_type => $test_data) {
      if ($mediamosa_run == mediamosa_settings::MEDIAMOSA_RUN_ALL || $test_type == $mediamosa_run) {
        foreach ($test_data as $name => $test_result) {
          if (empty($test_result['results']) && $skip_empty) {
            // Skip the test, that have failed to run.
            continue;
          }

          // Test if the simpletest is enabled.
          if (method_exists($name, mediamosa_settings::MEDIAMOSA_RUN_ENABLED)) {
            if (!call_user_func_array($name . '::' . mediamosa_settings::MEDIAMOSA_RUN_ENABLED, array())) {
              continue; // skip in listing.
            }
          }

          // Skip the currently running test.
          if (isset($test_result['info']['name'])) {
            // Add the data.
            $mediamosa->add_item(array(
              'test' => $test_result['info']['name'],
              'test_description' => $test_result['info']['description'],
              'passes' => (isset($test_result['results']['pass']) ? $test_result['results']['pass'] : ''),
              'fails' => (isset($test_result['results']['fail']) ? $test_result['results']['fail'] : ''),
              'exceptions' => (isset($test_result['results']['exception']) ? $test_result['results']['exception'] : ''),
              'last_run' => format_date(round($test_result['started']), 'custom', 'Y-m-d H:i:s'),
              'ttr' => round($test_result['ended'] - $test_result['started'], 2),
            ));
          }
        }
      }
    }
  }
}


/**
 * URI: /internal/simpletest/run
 * Method: POST
 *
 * Internal only.
 *
 */
class mediamosa_rest_call_simpletest_run extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const CLASS_NAME = 'class_name';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $a_var_setup = array(
      self::VARS => array(
        self::CLASS_NAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM_UNDERSCORE,
          self::VAR_DESCRIPTION => 'The class name of the simpletest.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the class name.
    $class_name = $this->get_param_value(self::CLASS_NAME);

    // Check if I may run.
    $run_last = variable_get('mediamosa_automatedsimpletest_timeout_tests_running');

    // May run?
    if (!empty($run_last) && round(REQUEST_TIME - $run_last['started']) < mediamosa_maintenance_automatedsimpletest::timeout_test_run()) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_TEST_RUNNING, array('@test' => $run_last['class']));
    }

    // Ok, set the var.
    variable_set('mediamosa_automatedsimpletest_timeout_tests_running', array('started' => REQUEST_TIME, 'class' => $class_name));

    // Add default info.
    $item = array('runphase' => mediamosa_maintenance_automatedsimpletest::RUNPHASE_STARTED);

    // So we can access it in exception.
    $my_simpletest = NULL;

    try {
      // Now create and add.
      $test_id = db_insert('simpletest_test_id')
        ->useDefaults(array('test_id'))
        ->execute();

      // Create.
      $my_simpletest = new $class_name($test_id);

      // Add default info.
      $item = array(
        'info' => $my_simpletest->getInfo(),
        'results' => array(),
        'assertions' => array(),
        'runphase' => mediamosa_maintenance_automatedsimpletest::RUNPHASE_STARTED
      );

      // Run.
      $start = microtime(TRUE);
      $my_simpletest->runSafe();
      $end = microtime(TRUE);

      // Store results.
      $item['results'] = $my_simpletest->results;
      $item['started'] = $start;
      $item['ended'] = $end;
      $assertions = $my_simpletest->get_assertions();

      // Now add the messages.
      foreach ($assertions as $num => $assertion) {
        $item['assertions']['assertion']['#' . serialize(array('id' => $num + 1))] = $assertion;
      }

      // Fix.
      $item['results']['pass'] = $item['results']['#pass'];
      $item['results']['fail'] = $item['results']['#fail'];
      $item['results']['exception'] = $item['results']['#exception'];
      $item['results']['debug'] = $item['results']['#debug'];

      // Unset these.
      unset($item['results']['#pass']);
      unset($item['results']['#fail']);
      unset($item['results']['#exception']);
      unset($item['results']['#debug']);

      // Set to completed.
      $item['runphase'] = mediamosa_maintenance_automatedsimpletest::RUNPHASE_COMPLETED;
    }
    catch (Exception $e) {
      mediamosa_watchdog::log(
        'Unittest running failed! @message (Exception thrown on line @line, in @file)<br />Partial backtrace: <pre>@backtrace</pre>',
        array(
          '@message' => $e->getMessage(),
          '@line' => $e->getLine(),
          '@file' => $e->getFile(),
          '@backtrace' => $e->getTraceAsString(),
        )
      );

      // ignore.
      $item['runphase'] = mediamosa_maintenance_automatedsimpletest::RUNPHASE_FAILED;
    }

    // Done, reset running.
    variable_del('mediamosa_automatedsimpletest_timeout_tests_running');

    // Add result.
    $mediamosa->add_item($item);
  }
}
