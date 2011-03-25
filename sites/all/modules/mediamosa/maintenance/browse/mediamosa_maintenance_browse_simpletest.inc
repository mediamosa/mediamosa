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
 * Shows a filtered listing of the simpletests of mediamosa.
 */

// Need these.
module_load_include('inc', 'simpletest', 'simpletest.pages');

/**
 * List tests arranged in groups that can be selected and run.
 */
function mediamosa_maintenance_browse_test_form($form) {

  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  drupal_set_breadcrumb($breadcrumb);

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

    $form['tests']['table'][$group] = array(
      '#collapsed' => TRUE,
    );

    foreach ($tests as $class => $info) {
      $form['tests']['table'][$group][$class] = array(
        '#type' => 'checkbox',
        '#title' => $info['name'],
        '#description' => $info['description'],
      );
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
  // For custom clean ups.

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
    $form_state['redirect'] = 'admin/mediamosa/browse/testing/results/' . $test_id;
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
  $form['#action'] = url('admin/mediamosa/browse/testing/results/re-run');

  $form['action']['return']['#href'] = 'admin/mediamosa/browse/testing';

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
    $form_state['redirect'] = 'admin/mediamosa/browse/testing/results/' . drupal_substr($form_state['redirect'], drupal_strlen('admin/config/development/testing/results/'));
  }
  elseif ($form_state['redirect'] == 'admin/config/development/testing') {
    $form_state['redirect'] = 'admin/mediamosa/browse/testing';
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