<?php
/**
 * @file
 * Shows a filtered listing of the simpletests of mediamosa.
 */

// Need these.
module_load_include('inc', 'simpletest', 'simpletest.pages');

/**
 * List tests arranged in groups that can be selected and run.
 */
function mediamosa_maintenance_browse_test_form($form) {

  $form['tests'] = array(
    '#type' => 'fieldset',
    '#title' => t('Tests'),
    '#description' => t('Select the test(s) or test group(s) you would like to run, and click <em>Run tests</em>.'),
  );

  $form['tests']['table'] = array(
    '#theme' => 'mediamosa_simpletest_test_table',
  );

  // Generate the list of tests arranged by group.
  $groups = simpletest_test_get_all();
  foreach ($groups as $group => $tests) {
    if (drupal_strtolower(drupal_substr($group, 0, drupal_strlen('mediamosa'))) != 'mediamosa') {
      continue;
    }

    foreach ($tests as $class => $info) {
      if (!empty($info['automatic_run_disabled'])) {
        continue;
      }

      $form['tests']['table'][$group][$class] = array(
        '#type' => 'checkbox',
        '#title' => $info['name'],
        '#description' => $info['description'],
      );
    }

    if (!empty($form['tests']['table'][$group])) {
      $form['tests']['table'][$group]['#collapsed'] = TRUE;
    }
  }

  // Operation buttons.
  $form['tests']['op'] = array(
    '#type' => 'submit',
    '#value' => t('Run tests'),
  );
  $form['clean'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Clean test environment (developers only)'),
    '#description' => t('Remove tables with the prefix "simpletest" and temporary directories that are left over from tests that crashed. This is intended for developers when creating tests.'),
  );
  $form['clean']['op'] = array(
    '#type' => 'submit',
    '#value' => t('Clean environment'),
    '#submit' => array('_mediamosa_maintenance_browse_simpletest_clean_environment'),
  );

  return $form;
}

/**
 * Clean up simpletest extended to clean up on servers as well.
 */
function _mediamosa_maintenance_browse_simpletest_clean_environment() {
  // Cleanup on our servers.
  mediamosa_server::cleanup_simpletest_enabled_servers();

  // Call our cleanup hook for other stuff.
  module_invoke_all('mediamosa_simpletest_clean_environment');

  // Run the normal clean up.
  return simpletest_clean_environment();
}

/**
 * Run selected tests.
 */
function mediamosa_maintenance_browse_test_form_submit($form, &$form_state) {

  // Get list of tests.
  $tests_list = array();
  foreach ($form_state['values'] as $class_name => $value) {
    if (class_exists($class_name) && $value === 1) {
      $tests_list[] = $class_name;
    }
  }

  if (!empty($tests_list)) {
    $test_id = simpletest_run_tests($tests_list, 'drupal');
    $form_state['redirect'] = 'admin/mediamosa/tools/testing/results/' . $test_id;
  }
  else {
    drupal_set_message(t('No test(s) selected.'), 'error');
  }
}

/**
 * wrapper for displaying the results.
 */
function mediamosa_maintenance_browse_simpletest_result_form($form, &$form_state, $test_id) {

  // Get the form.
  $form = simpletest_result_form($form, $form_state, $test_id);

  // Change it.
  $form['#action'] = url('admin/mediamosa/tools/testing/results/re-run');

  $form['action']['return']['#href'] = 'admin/mediamosa/tools/testing';

  return $form;
}

/**
 * Action form submit.
 *
 * @param array $form
 * @param array $form_state
 */
function mediamosa_maintenance_browse_simpletest_result_form_submit($form, &$form_state) {

  $form = simpletest_result_form_submit($form, $form_state);

  // Change the redirect.
  if (drupal_substr($form_state['redirect'], 0, drupal_strlen('admin/config/development/testing/results/')) == 'admin/config/development/testing/results/') {
    $form_state['redirect'] = 'admin/mediamosa/tools/testing/results/' . drupal_substr($form_state['redirect'], drupal_strlen('admin/config/development/testing/results/'));
  }
  elseif ($form_state['redirect'] == 'admin/config/development/testing') {
    $form_state['redirect'] = 'admin/mediamosa/tools/testing';
  }

  return $form;
}

/**
 * Reuse simpletest test table.
 */
function theme_mediamosa_simpletest_test_table($variables) {
  // Call simpletest theme.
  $table = theme('simpletest_test_table', $variables);

  // Add our classes.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance_browse') . '/mediamosa_maintenance_browse_simpletest.css');

  return $table;
}

/**
 * Provides settings form for SimpleTest variables.
 *
 * @ingroup forms
 * @see simpletest_settings_form_validate()
 */
function mediamosa_maintenance_browse_simpletest_settings_form($form, &$form_state) {

  // Include the simple test pages inc file.
  module_load_include('inc', 'simpletest', 'simpletest.pages');

  // Call original form.
  return simpletest_settings_form($form, $form_state);
}

/**
 * Validation handler for simpletest_settings_form().
 */
function mediamosa_maintenance_browse_simpletest_settings_form_validate($form, &$form_state) {

  // Include the simple test pages inc file.
  module_load_include('inc', 'simpletest', 'simpletest.pages');

  // Call original.
  simpletest_settings_form_validate($form, $form_state);
}
